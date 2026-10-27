#pragma once
#include <QByteArray>
#include <QList>
namespace diagnostic {
// Separate compile-time diagnostic targets, never a runtime/model fallback.
// This permits read-only inventory only, not capture, insertion or installation.
#ifdef INK_DIAGNOSTIC_FERRARI
inline constexpr const char *model = "ferrari";
inline constexpr const char *machineName = "reMarkable Ferrari";
#else
inline constexpr const char *model = "chiappa";
inline constexpr const char *machineName = "reMarkable Chiappa";
#endif
inline constexpr const char *release = "3.28.0.172";
inline constexpr const char *qtVersion = "6.10.3";
inline bool permitted(const QByteArray &machine,const QByteArray &osRelease,const QByteArray &qt,
                      bool hasGuiResourceSymbol,bool hasDeviceLockSymbol) {
    if(!hasGuiResourceSymbol || !hasDeviceLockSymbol)return false;
    if(machine.trimmed()!=machineName || qt!=qtVersion)return false;
    int matches=0;
    for(auto line:osRelease.split('\n')) {
        if(!line.startsWith("IMG_VERSION="))continue;
        auto value=line.mid(12).trimmed();
        if(value.startsWith('"') && value.endsWith('"'))value=value.mid(1,value.size()-2);
        if(value!=release)return false;
        ++matches;
    }
    return matches==1;
}
}
