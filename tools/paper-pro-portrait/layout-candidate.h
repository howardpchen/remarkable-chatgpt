#pragma once
// Dimension extrapolation only: never a measured profile or runtime admission.
#include <QRect>
#include <QSize>
#include <stdexcept>

namespace paperProPortrait {
struct LayoutCandidate {
    QSize window;
    QRect launcher, owner, compactCard, proposedContent;
};
inline LayoutCandidate extrapolatedLayout(const QSize &window) {
    // The first Ferrari experiment is portrait only. Do not make this a generic
    // device allowlist or derive a uniform calibration from two screen ratios.
    if (window != QSize(1620,2160))
        throw std::runtime_error("Ferrari portrait candidate requires 1620x2160");
    const int right=window.width()-16, bottom=window.height()-16;
    return {window, {right-72,bottom-72,72,72},
        {right-360,bottom-176,360,176}, {right-280,bottom-152,280,152},
        // Proposed inset only; no capture caller consumes this as calibration.
        {16,16,window.width()-32,window.height()-32}};
}
}
