#define INK_PAPER_PRO_PER_REQUEST
#include "native-placement.h"
#include <cassert>
int main() {
    QObject controller;
    auto set=[&](QRectF items,QRectF text=QRectF()) {
        controller.setProperty("itemsBoundingRect",items);
        controller.setProperty("boundingRect",items);
        controller.setProperty("rootDocumentBoundingRect",text);
        return diaryContentBounds::detect(&controller);
    };
    const DiaryPaperProAxes axes{{-370,100},{1,0},{0,1},1,0};
    auto baseline=set({-200,700,50,300});
    assert(diaryNativePlacement::lastRow(baseline,axes,2160)==900);
    assert(diaryNativePlacement::lastRow(set({-200,700,50,100}),axes,2160)==700);
    assert(diaryNativePlacement::lastRow(set({-200,700,50,300}),axes,2160)==900);
    assert(diaryNativePlacement::lastRow(set({-200,700,50,0}),axes,2160)==600);
    assert(diaryNativePlacement::lastRow(set({-200,700,0,300}),axes,2160)==900);
    assert(diaryNativePlacement::lastRow(set({}, {-300,800,500,400}),axes,2160)==1100);
    assert(diaryNativePlacement::lastRow(set({-200,3000,50,50}),axes,2160)==2159);
    assert(diaryNativePlacement::lastRow(set({-200,-500,50,10}),axes,2160)==-1);
    assert(diaryNativePlacement::lastRow(set({}, {-370,0,740,0}),axes,2160)==-1);
    for(int degree:{0,90,180,270}) {
        auto transform=QTransform().rotate(degree).scale(0.5,0.5);
        const DiaryPaperProAxes rotated{{13,29},transform.map(QPointF(1,0)),transform.map(QPointF(0,1)),0.5,degree};
        const auto rect=QRectF(rotated.map({10,300}),rotated.map({50,500})).normalized();
        assert(diaryNativePlacement::lastRow(set(rect),rotated,2160)==500);
    }
    bool refused=false;
    try {diaryNativePlacement::lastRow(diaryContentBounds::detect(nullptr),axes,2160);}catch(...){refused=true;}
    assert(refused);
}
