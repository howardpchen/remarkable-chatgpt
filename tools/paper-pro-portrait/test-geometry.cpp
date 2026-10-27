#include "geometry.h"
#include <QGuiApplication>
#include <QPainter>
#include <cassert>
#include <functional>
#include <iostream>
#include <limits>

using namespace paperProPortrait;
static int rejected=0;
static void rejects(const std::function<void()> &call) {
    try { call(); } catch(const std::runtime_error &) {++rejected;return;}
    throw std::runtime_error("Expected rejection");
}
int main(int argc,char **argv) {
    QGuiApplication app(argc,argv);
    // Deliberately non-device fixtures: logical pixels are HALF physical,
    // while DPR is 1.25. Scene scale is separately measured as 1.5.
    Viewport reference{{810,1080},1.25,QTransform(1.5,0,0,1.5,17,29),"synthetic-page","Blank"};
    const QTransform physicalToLogical(0.5,0,0,0.5,0,0);
    const QRect crop(100,80,1400,1900);
    Candidate candidate(reference,physicalToLogical,crop);
    QImage capture(candidatePixels(),QImage::Format_RGB32);capture.fill(Qt::white);
    {QPainter p(&capture);p.fillRect(150,180,50,21,Qt::black);p.fillRect(5,2100,20,30,Qt::black);}
    const auto original=capture.copy();
    auto placed=candidate.prepare(capture,reference,"## Synthetic answer\n\nA short **local** response.");
    assert(placed.lastInk==120);
    assert(placed.physicalBounds.y()==246); // last physical ink y=200, gap=45.
    assert(placed.physicalCrop.size()==crop.size());
    assert(placed.rendered.width()==1400);
    assert(placed.nativeCandidate.width()==1050); // 1400 * 0.5 * 1.5, NOT DPR.
    assert(placed.nativeCandidate.height()==std::floor(placed.rendered.height()*0.75));
    assert(placed.logicalBounds==physicalToLogical.mapRect(placed.physicalBounds));
    assert(placed.sceneBounds==reference.logicalToScene.mapRect(placed.logicalBounds));
    assert(capture==original);
    QImage blank(candidatePixels(),QImage::Format_RGB32);blank.fill(Qt::white);
    assert(candidate.prepare(blank,reference,"Blank fixture").lastInk==-1);
    assert(candidate.prepare(blank,reference,"Blank fixture").physicalBounds.y()==125);
    auto construct=[&](Viewport v,QTransform t,QRect r=QRect(100,80,1400,1900)){Candidate other(v,t,r);};
    rejects([&]{construct(reference,QTransform());}); // no implicit resolution/DPR mapping
    rejects([&]{construct(reference,QTransform(0.5,0,0,0.5,1,0));});
    rejects([&]{construct(reference,QTransform(0.5,0.01,0,0.5,0,0));});
    rejects([&]{construct(reference,QTransform(0,0.5,-0.5,0,0,0));});
    rejects([&]{construct(reference,QTransform(-0.5,0,0,-0.5,0,0));});
    rejects([&]{construct(reference,QTransform(0.5,0,0,0.6,0,0));});
    rejects([&]{construct(reference,QTransform(0.5,0,0,0.5,std::numeric_limits<double>::infinity(),0));});
    rejects([&]{construct(reference,QTransform(0.5,0,0,0.5,std::numeric_limits<double>::quiet_NaN(),0));});
    rejects([&]{construct(reference,physicalToLogical,QRect(-1,0,1400,1900));});
    rejects([&]{construct(reference,physicalToLogical,QRect(100,80,1600,1900));});
    rejects([&]{construct(reference,physicalToLogical,QRect(0,0,299,1900));});
    rejects([&]{construct(reference,physicalToLogical,QRect(0,0,1400,0));});
    auto mutate=[&](const std::function<void(Viewport&)> &change){auto v=reference;change(v);return v;};
    for(const auto &v: {
        mutate([](auto &v){v.logicalSize.transpose();}),
        mutate([](auto &v){v.logicalSize.setWidth(0);}),
        mutate([](auto &v){v.dpr=0;}),
        mutate([](auto &v){v.dpr=std::numeric_limits<double>::quiet_NaN();}),
        mutate([](auto &v){v.templateName="P Grid small";}),
        mutate([](auto &v){v.pageId.clear();}),
        mutate([](auto &v){v.logicalToScene=QTransform(1.5,0.01,0,1.5,0,0);}),
        mutate([](auto &v){v.logicalToScene=QTransform(1.5,0,0,1.5,std::numeric_limits<double>::max(),0);}),
        mutate([](auto &v){v.logicalToScene=QTransform(std::numeric_limits<double>::max(),0,0,std::numeric_limits<double>::max(),0,0);}),
        mutate([](auto &v){v.logicalToScene=QTransform(0,1.5,-1.5,0,0,0);})})
        rejects([&]{construct(v,physicalToLogical);});
    for(const auto &v: {
        mutate([](auto &v){v.pageId="different-page";}),
        mutate([](auto &v){v.templateName="P Dots S";}),
        mutate([](auto &v){v.logicalToScene.scale(1.1,1.1);}),
        mutate([](auto &v){v.logicalToScene.translate(1,0);}),
        mutate([](auto &v){v.logicalSize.transpose();}),
        mutate([](auto &v){v.dpr=2;})})
        rejects([&]{candidate.prepare(capture,v,"Changed");});
    rejects([&]{candidate.prepare(QImage(2160,1620,QImage::Format_RGB32),reference,"Landscape");});
    rejects([&]{candidate.prepare(QImage(954,1696,QImage::Format_RGB32),reference,"Move");});
    rejects([&]{candidate.prepare(QImage(),reference,"Null");});
    auto tagged=capture;tagged.setDevicePixelRatio(2);
    rejects([&]{candidate.prepare(tagged,reference,"Tagged logical image");});
    rejects([&]{candidate.prepare(capture,reference,"Negative gap",-1);});
    rejects([&]{candidate.prepare(capture,reference,QString("long answer\n\n").repeated(300));});
    QImage bottom=blank.copy();bottom.setPixelColor(crop.x(),crop.y()+crop.height()-1,Qt::black);
    rejects([&]{candidate.prepare(bottom,reference,"No room");});
    auto huge=reference;huge.logicalToScene=QTransform(100,0,0,100,0,0);
    Candidate hugeScale(huge,physicalToLogical,crop);
    rejects([&]{hugeScale.prepare(capture,huge,"Budget");});
    auto tiny=reference;tiny.logicalToScene=QTransform(0.00001,0,0,0.00001,0,0);
    Candidate tinyScale(tiny,physicalToLogical,crop);
    rejects([&]{tinyScale.prepare(capture,tiny,"Subpixel");});
    std::cout << "Paper Pro local synthetic crop/ink/render/placement passed; " << rejected << " rejection checks\n";
}
