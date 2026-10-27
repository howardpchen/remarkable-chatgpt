#include "../../src/move-template-ink.h"
#include <QPainter>
#include <cassert>
#include <iostream>
int main() {
    constexpr double scale=0.859848484848485;
    for(const auto &name:{QString("P Dots S"),QString("P Grid small")}) {
        QImage image(954,1696,QImage::Format_RGB32);image.fill(Qt::white);
        const double pitch=(name=="P Dots S"?42.5:52)/scale;
        if(name=="P Dots S") {
            for(double y=0;y<1696;y+=pitch)for(double x=7.5;x<954;x+=pitch)
                for(int dy=0;dy<2;++dy)for(int dx=0;dx<2;++dx)
                    if(int(y)+dy<1696 && int(x)+dx<954)image.setPixelColor(int(x)+dx,int(y)+dy,Qt::black);
        } else {
            QPainter p(&image);p.setPen(QColor(180,180,180));
            for(double y=0;y<1696;y+=pitch)p.drawLine(0,int(y),953,int(y));
            for(double x=17;x<954;x+=pitch)p.drawLine(int(x),0,int(x),1695);
        }
        const auto original=image;
        assert(moveTemplateInk::lastRow(image,name,scale)==-1);
        assert(image==original);
        {QPainter p(&image);p.fillRect(400,120,50,20,Qt::black);}
        assert(moveTemplateInk::lastRow(image,name,scale)==139);
        if(name=="P Grid small") {
            const int rule=int(pitch*5);
            image.setPixelColor(500,rule,Qt::black);
            assert(moveTemplateInk::lastRow(image,name,scale)==rule);
        }
        image.setPixelColor(500,430,Qt::black); // off-lattice punctuation survives
        assert(moveTemplateInk::lastRow(image,name,scale)==430);
        {QPainter p(&image);p.fillRect(400,1650,30,30,Qt::black);}
        assert(moveTemplateInk::lastRow(image,name,scale)>=1678);
    }
    // Factory grids can be dark too; only a full periodic lattice is excluded.
    QImage dark(954,1696,QImage::Format_RGB32);dark.fill(Qt::white);
    {QPainter p(&dark);p.setPen(Qt::black);
        for(double y=0;y<1696;y+=52/scale)p.drawLine(0,int(y),953,int(y));
        for(double x=17;x<954;x+=52/scale)p.drawLine(int(x),0,int(x),1695);}
    assert(moveTemplateInk::lastRow(dark,"P Grid small",scale)==-1);
    {QPainter p(&dark);p.fillRect(400,320,40,20,Qt::black);}
    assert(moveTemplateInk::lastRow(dark,"P Grid small",scale)>=338);
    // Actual Move captures render factory grid rules as binary 1-in-3 dither.
    QImage dithered(954,1696,QImage::Format_RGB32);dithered.fill(Qt::white);
    for(double y=0;y<1696;y+=52/scale)for(int x=0;x<954;x+=3)
        dithered.setPixelColor(x,int(y),Qt::black);
    for(double x=23;x<954;x+=52/scale)for(int y=0;y<1696;y+=3)
        dithered.setPixelColor(int(x),y,Qt::black);
    const auto originalDithered=dithered;
    assert(moveTemplateInk::lastRow(dithered,"P Grid small",scale)==-1);
    assert(dithered==originalDithered);
    {QPainter p(&dithered);p.fillRect(400,320,40,20,Qt::black);}
    assert(moveTemplateInk::lastRow(dithered,"P Grid small",scale)==339);
    dithered.setPixelColor(501,430,Qt::black);
    assert(moveTemplateInk::lastRow(dithered,"P Grid small",scale)==430);
    // Regression: a wide handwritten table creates high-coverage thick bands
    // and off-lattice thin rules. These must not disqualify all template rows.
    QImage table=originalDithered;
    {QPainter p(&table);p.fillRect(180,320,560,12,Qt::black);
        p.fillRect(200,410,520,2,Qt::black);
        p.fillRect(240,200,8,600,Qt::black);}
    const auto frozenTable=table;
    assert(moveTemplateInk::lastRow(table,"P Grid small",scale)==799);
    assert(table==frozenTable); // only the placement-analysis copy is filtered
    // An extra off-lattice handwritten horizontal rule survives lattice fitting.
    {QPainter p(&table);p.fillRect(0,1000,954,1,Qt::black);}
    assert(moveTemplateInk::lastRow(table,"P Grid small",scale)==1000);
    // Thick ink on a native grid row is never extrapolated away.
    QImage touching=originalDithered;
    {QPainter p(&touching);p.fillRect(0,int(20*52/scale)-2,954,9,Qt::black);}
    assert(moveTemplateInk::lastRow(touching,"P Grid small",scale)==int(20*52/scale)+6);
    // A sparse, periodic handwritten pattern is not a full factory lattice.
    QImage sparse(954,1696,QImage::Format_RGB32);sparse.fill(Qt::white);
    {QPainter p(&sparse);p.setPen(Qt::black);
        for(int i=0;i<5;++i)p.drawLine(0,100+int(i*52/scale),953,100+int(i*52/scale));}
    assert(moveTemplateInk::lastRow(sparse,"P Grid small",scale)==100+int(4*52/scale));
    // Near-but-wrong spacing across the page does not certify a factory grid.
    QImage wrongPitch(954,1696,QImage::Format_RGB32);wrongPitch.fill(Qt::white);
    {QPainter p(&wrongPitch);p.setPen(Qt::black);
        for(int i=0;i<27;++i)p.drawLine(0,25+i*61,953,25+i*61);}
    assert(moveTemplateInk::lastRow(wrongPitch,"P Grid small",scale)==25+26*61);
    // A dark full-width handwritten line on Blank must never be erased.
    QImage blank(954,1696,QImage::Format_RGB32);blank.fill(Qt::white);
    {QPainter p(&blank);p.setPen(Qt::black);p.drawLine(0,1000,953,1000);}
    assert(moveTemplateInk::lastRow(blank,"Blank",scale)==1000);
    // Unrecognized/nonperiodic dark lines remain ink, not a template mask.
    assert(moveTemplateInk::lastRow(blank,"P Grid small",scale)==1000);
    std::cout<<"PASS template placement: empty dotted/grid pages, text, punctuation, bottom ink, immutable source, Blank and nonperiodic dark rules\n";
}
