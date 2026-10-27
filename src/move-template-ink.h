#pragma once
#include "layout.h"
#include <QString>
#include <cmath>
#include <stdexcept>

// Placement analysis only. Never modify the frozen image, API payload or
// freshness comparison. Exact factory identities are admitted separately.
namespace moveTemplateInk {
inline double distance(double a,double b,double pitch) {
    const double d=std::abs(a-b); return std::min(d,pitch-d);
}
inline double phase(double p,double pitch) {
    double v=std::fmod(p,pitch); return v<0?v+pitch:v;
}
inline std::vector<bool> gridBands(const QImage &im,bool horizontal,double pitch) {
    const int n=horizontal?im.height():im.width(), length=horizontal?im.width():im.height();
    std::vector<bool> mask(n,false);
    struct Band {int begin,end;double center;};
    std::vector<Band> candidates;
    if(!std::isfinite(pitch) || pitch<8 || pitch>256)return mask;
    for(int i=0;i<n;) {
        auto line=[&](int k) {int count=0;for(int j=0;j<length;++j)
            count+=qGray(im.pixel(horizontal?j:k,horizontal?k:j))<245;
            // Native gray rules are often binary 1-in-3 dither. Wide writing
            // can meet this coverage too: it must not invalidate every rule.
            return count*100>=length*25;};
        if(!line(i)){++i;continue;}
        int end=i+1;while(end<n && line(end))++end;
        if(end-i<=4)candidates.push_back({i,end,(i+end-1)/2.0});
        // Thick bands remain ink. Never infer a template mask through them.
        i=end;
    }
    // Certify the dominant known-pitch lattice, allowing writing to obscure
    // some rules or introduce unrelated thin bands. Only observed thin bands
    // on that lattice are masked; unmatched/table/handwritten rules remain ink.
    constexpr double tolerance=1.25; // pixel quantization, not a spacing change
    int bestCount=0;double bestPhase=0,bestSpan=0;
    for(const auto &seed:candidates) {
        const double origin=phase(seed.center,pitch);
        int count=0;double first=n,last=0;
        for(const auto &band:candidates)
            if(distance(phase(band.center,pitch),origin,pitch)<=tolerance) {
                ++count;first=std::min(first,band.center);last=std::max(last,band.center);
            }
        if(count>bestCount || (count==bestCount && last-first>bestSpan)) {
            bestCount=count;bestPhase=origin;bestSpan=last-first;
        }
    }
    // A few coincident handwriting strokes are not enough. Demand a dense
    // factory-pitch lattice spanning most of the full capture dimension.
    if(bestCount<4 || bestCount<int(std::ceil(n/pitch*0.75)) || bestSpan<n*0.75)
        return mask;
    for(const auto &band:candidates)
        if(distance(phase(band.center,pitch),bestPhase,pitch)<=tolerance)
            for(int k=band.begin;k<band.end;++k)mask[k]=true;
    return mask;
}
inline int lastRow(const QImage &source,const QString &name,double scenePerPixel) {
    if(name=="Blank")return lastInkRow(source);
    if(source.isNull() || !std::isfinite(scenePerPixel) || scenePerPixel<=0)
        throw std::runtime_error("Invalid template placement geometry");
    QImage image=source.convertToFormat(QImage::Format_RGB32);
    if(name=="P Grid small") {
        const auto rows=gridBands(image,true,52/scenePerPixel);
        const auto cols=gridBands(image,false,52/scenePerPixel);
        auto modes=[&](const std::vector<bool> &bands,bool horizontal) {
            std::vector<int> out(bands.size(),255);
            for(size_t i=0;i<bands.size();++i)if(bands[i]) {
                std::array<int,256> hist{};
                const int length=horizontal?image.width():image.height();
                for(int j=0;j<length;++j)++hist[qGray(image.pixel(horizontal?j:int(i),horizontal?int(i):j))];
                out[i]=int(std::max_element(hist.begin(),hist.begin()+245)-hist.begin());
            }
            return out;
        };
        const auto rm=modes(rows,true),cm=modes(cols,false);
        for(int y=0;y<image.height();++y)for(int x=0;x<image.width();++x) {
            if(!rows[y] && !cols[x])continue;
            // Keep ink darker than a pale rule, including its intersections.
            const int background=rows[y] && cols[x] ? rm[y]*cm[x]/255 : std::min(rm[y],cm[x]);
            if(qGray(image.pixel(x,y))>=std::max(0,background-32))
                image.setPixel(x,y,qRgb(255,255,255));
        }
    } else if(name=="P Dots S") {
        struct Dot {std::vector<QPoint> pixels;double x,y;};
        std::vector<Dot> dots;
        std::vector<bool> seen(size_t(image.width())*image.height(),false);
        for(int y=0;y<image.height();++y)for(int x=0;x<image.width();++x) {
            const auto idx=size_t(y)*image.width()+x;
            if(seen[idx] || qGray(image.pixel(x,y))>=245)continue;
            std::vector<QPoint> pixels{{x,y}};seen[idx]=true;
            int left=x,right=x,top=y,bottom=y;
            for(size_t i=0;i<pixels.size();++i) {
                const auto p=pixels[i];
                for(int dy=-1;dy<=1;++dy)for(int dx=-1;dx<=1;++dx) {
                    int nx=p.x()+dx,ny=p.y()+dy;
                    if(nx<0 || ny<0 || nx>=image.width() || ny>=image.height())continue;
                    const auto ni=size_t(ny)*image.width()+nx;
                    if(seen[ni] || qGray(image.pixel(nx,ny))>=245)continue;
                    seen[ni]=true;pixels.push_back({nx,ny});
                    left=std::min(left,nx);right=std::max(right,nx);
                    top=std::min(top,ny);bottom=std::max(bottom,ny);
                }
            }
            // Joined handwriting is retained, even when it touches a dot.
            if(right-left<3 && bottom-top<3)
                dots.push_back({std::move(pixels),(left+right)/2.0,(top+bottom)/2.0});
        }
        const double pitch=42.5/scenePerPixel;
        const int bins=int(std::ceil(pitch));
        if(bins<8 || bins>256)throw std::runtime_error("Unsupported dot spacing");
        std::vector<int> xs(bins),ys(bins);
        for(const auto &d:dots){++xs[int(phase(d.x,pitch))];++ys[int(phase(d.y,pitch))];}
        auto best=[&](const std::vector<int>&hist) {
            int best=0,score=-1;
            for(int i=0;i<bins;++i){int sum=0;for(int j=0;j<bins;++j)
                if(distance(i,j,pitch)<=2)sum+=hist[j];
                if(sum>score){score=sum;best=i;}}
            return double(best)+0.5;
        };
        const double px=best(xs),py=best(ys);
        int matches=0;double minx=image.width(),maxx=0,miny=image.height(),maxy=0;
        auto aligned=[&](const Dot &d){return distance(phase(d.x,pitch),px,pitch)<=2.5 &&
            distance(phase(d.y,pitch),py,pitch)<=2.5;};
        for(const auto &d:dots)if(aligned(d)){++matches;minx=std::min(minx,d.x);maxx=std::max(maxx,d.x);miny=std::min(miny,d.y);maxy=std::max(maxy,d.y);}
        if(matches>=30 && maxx-minx>image.width()*0.6 && maxy-miny>image.height()*0.6)
            for(const auto &d:dots)if(aligned(d))for(const auto &p:d.pixels)
                image.setPixel(p,qRgb(255,255,255));
    } else throw std::runtime_error("Unsupported placement template");
    for(int y=image.height()-1;y>=0;--y)for(int x=0;x<image.width();++x)
        if(qGray(image.pixel(x,y))<210)return y;
    return -1;
}
}
