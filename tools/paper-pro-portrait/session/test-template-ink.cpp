#include "../../../src/move-template-ink.h"
#include <QPainter>
#include <cassert>
#include <iostream>
int main() {
    for(const QSize size:{QSize(1620,2160),QSize(2160,1620)})
    for(const auto &name:{QString("P Dots S"),QString("P Grid small")}) {
        // Deliberately synthetic spacing; no Move/Ferrari factory claim.
        const double pitch=name=="P Dots S"?63.75:47.25;
        QImage image(size.width(),size.height(),QImage::Format_RGB32);image.fill(Qt::white);
        if(name=="P Grid small") {
            for(double y=0;y<size.height();y+=pitch)for(int x=0;x<size.width();x+=3)
                image.setPixelColor(x,int(y),Qt::black);
            for(double x=17;x<size.width();x+=pitch)for(int y=0;y<size.height();y+=3)
                image.setPixelColor(int(x),y,Qt::black);
        } else {
            QPainter p(&image);
            for(double y=0;y<size.height();y+=pitch)for(double x=7.5;x<size.width();x+=pitch)
                p.fillRect(int(x),int(y),2,2,Qt::black);
        }
        const auto original=image;
        assert(moveTemplateInk::lastRowAtPitch(image,name,pitch)==-1);
        assert(image==original);
        {QPainter p(&image);p.fillRect(200,320,1200,12,Qt::black);
            p.fillRect(300,410,1100,2,Qt::black);
            p.fillRect(240,200,8,600,Qt::black);}
        assert(moveTemplateInk::lastRowAtPitch(image,name,pitch)==799);
        image.setPixelColor(501,1000,Qt::black);
        assert(moveTemplateInk::lastRowAtPitch(image,name,pitch)==1000);
        {QPainter p(&image);p.fillRect(400,size.height()-60,40,30,Qt::black);}
        const auto frozen=image;
        assert(moveTemplateInk::lastRowAtPitch(image,name,pitch)>=size.height()-31);
        assert(image==frozen);
        assert(moveTemplateInk::lastRowAtPitch(original,name,pitch+7)>size.height()-160);
    }
    // Enlarged dots across both reading orientations and admitted zoom range.
    for(const QSize size:{QSize(1620,2160),QSize(2160,1620)})
    for(double zoom:{1.0,1.5,2.03,3.0,6.0}) {
        const double pitch=42.5*zoom;
        const int extent=int(std::ceil(2*zoom));
        QImage dots(size,QImage::Format_RGB32);dots.fill(Qt::white);
        {QPainter p(&dots);
            for(double y=19;y<size.height();y+=pitch)
            for(double x=13;x<size.width();x+=pitch)
                p.fillRect(int(x),int(y),extent,extent,Qt::black);}
        const auto original=dots;
        assert(moveTemplateInk::lastRowAtPitch(dots,"P Dots S",pitch)==-1);
        assert(dots==original);
        // Joined writing on a dot must survive even without other ink.
        {QPainter p(&dots);p.fillRect(13,19,extent+15,extent,Qt::black);}
        assert(moveTemplateInk::lastRowAtPitch(dots,"P Dots S",pitch)==19+extent-1);
        dots=original;
        // Off-lattice punctuation remains even if small enough to be a dot.
        {QPainter p(&dots);p.fillRect(35,1000,extent,extent,Qt::black);}
        assert(moveTemplateInk::lastRowAtPitch(dots,"P Dots S",pitch)==1000+extent-1);
        // Joined ink at a genuine dot is never eligible for removal.
        {QPainter p(&dots);p.fillRect(13,19,extent+15,extent,Qt::black);}
        assert(moveTemplateInk::lastRowAtPitch(dots,"P Dots S",pitch)==1000+extent-1);
        // Wrong pitch must not erase the background.
        assert(moveTemplateInk::lastRowAtPitch(original,"P Dots S",pitch-9)>size.height()-200);
        // Sparse plausible dots cannot certify a template.
        QImage sparse(size,QImage::Format_RGB32);sparse.fill(Qt::white);
        {QPainter p(&sparse);for(int i=0;i<5;++i)p.fillRect(int(13+i*pitch),1200,extent,extent,Qt::black);}
        assert(moveTemplateInk::lastRowAtPitch(sparse,"P Dots S",pitch)==1200+extent-1);
    }
    // Grid: zoomed 1--4px rules still certify; 5px rules fail closed.
    for(int thickness:{1,2,3,4,5}) {
        QImage grid(2160,1620,QImage::Format_RGB32);grid.fill(Qt::white);
        {QPainter p(&grid);
            for(int y=20;y<1620;y+=104)p.fillRect(0,y,2160,thickness,Qt::black);
            for(int x=15;x<2160;x+=104)p.fillRect(x,0,thickness,1620,Qt::black);}
        const auto frozen=grid;
        const int last=moveTemplateInk::lastRowAtPitch(grid,"P Grid small",104);
        assert(thickness<=4?last==-1:last>=1600);
        assert(grid==frozen);
    }
    std::cout<<"PASS Ferrari portrait/landscape template analysis: synthetic pitch, dither, tables, punctuation, bottom ink, immutable source, wrong-pitch retention\n";
}
