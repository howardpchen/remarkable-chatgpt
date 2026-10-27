#include <QGuiApplication>
#include <QFontDatabase>
#include <QTemporaryDir>
#include <cassert>
#include <iostream>
#include <limits>
#include "../src/capture-layout.h"
#include "../src/layout.h"
#include "../src/markdown.h"

int main(int argc, char **argv) {
    QGuiApplication app(argc,argv);
    const auto rm2=diaryCaptureLayout({1404,1872},{1404,1872});
    assert(rm2.area==diaryCaptureArea() && rm2.answerHeight==1582 && rm2.answerGap==45);
    auto rejects=[](auto operation) {
        bool rejected=false;
        try { operation(); } catch(const std::runtime_error &) { rejected=true; }
        assert(rejected);
    };
    rejects([]{diaryCaptureLayout({954,1696},{954,1696});}); // No implicit promotion.
    rejects([]{diaryCaptureLayout({1620,2160},{1620,2160});}); // Pro remains separate.
    rejects([]{diaryCaptureLayout({1404,1872},{702,936},2);});
    rejects([]{diaryCaptureLayout({1404,1872},{1404,1872},std::numeric_limits<qreal>::quiet_NaN());});
    const auto move=DiaryCaptureProfile::ExperimentalMove;
    rejects([&]{diaryCaptureLayout({954,1696},{954,1696},1,move);}); // Crop is unknown.
    for(const QRect bad : {QRect(-1,0,700,1500),QRect(0,0,955,1696),QRect(150,0,299,1600)})
        rejects([&]{diaryCaptureLayout({954,1696},{954,1696},1,move,bad);});
    rejects([&]{diaryCaptureLayout({1696,954},{1696,954},1,move,{0,0,700,800});});
    rejects([&]{diaryCaptureLayout({954,1696},{954,1696},2,move,{0,0,700,1500});});

    // Synthetic geometry, NOT a measured Move crop or a hardware validation.
    // Exercise actual crop -> ink scan -> Markdown render -> PNG roundtrip
    // without credentials, transport, notebook access or native insertion.
    const auto candidate=diaryCaptureLayout({954,1696},{954,1696},1,move,{150,0,724,1576});
    QImage synthetic(954,1696,QImage::Format_RGB32); synthetic.fill(Qt::white);
    synthetic.setPixelColor(500,100,Qt::black);
    synthetic.setPixelColor(900,1650,Qt::black); // Out-of-crop synthetic UI mark.
    const auto question=synthetic.copy(candidate.area);
    assert(lastInkRow(question)==100);
    const int top=candidate.area.y()+lastInkRow(question)+candidate.answerGap;
    assert(top==145);
    const int font=QFontDatabase::addApplicationFont("fonts/Caveat.ttf"); assert(font>=0);
    assert(QFontDatabase::addApplicationFont("fonts/DejaVuSansMono.ttf")>=0);
    const auto family=QFontDatabase::applicationFontFamilies(font).first();
    QTemporaryDir output; assert(output.isValid());
    for(const QString text : {QString("A short offline answer."),
            QString("## Narrow answer\n\n- First point\n- Second point\n\n| Test | Result |\n|---|---|\n| Offline | Pass |"),
            QString("```\n")+QString(180,'x')+"\n```"}) {
        auto rendered=renderDiaryMarkdown(text,family,candidate.area.width());
        assert(!rendered.image.isNull() && rendered.image.width()==724);
        assert(rendered.image.height()>0 && rendered.image.height()<=12000);
        assert(rendered.image.sizeInBytes()<=724*12000*4);
        const QString path=output.path()+"/answer.png";
        // PNG stores straight-alpha pixels; compare that representation rather
        // than Qt's in-memory premultiplied image format.
        assert(rendered.image.save(path));
        assert(QImage(path).convertToFormat(QImage::Format_ARGB32)==
               rendered.image.convertToFormat(QImage::Format_ARGB32));
    }
    std::cout<<"PASS: capture profiles fail closed; synthetic narrow offline render roundtrip\n";
}
