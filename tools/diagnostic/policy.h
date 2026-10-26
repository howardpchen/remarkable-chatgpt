#pragma once
#include <QByteArray>
#include <QList>
namespace diagnostic {
inline bool permitted(const QByteArray &machine,const QByteArray &osRelease,const QByteArray &qt,
                      bool hasGuiResourceSymbol,bool hasDeviceLockSymbol) {
    if(!hasGuiResourceSymbol || !hasDeviceLockSymbol)return false;
    if(machine.trimmed()!="reMarkable Chiappa" || qt!="6.10.3")return false;
    int matches=0;
    for(auto line:osRelease.split('\n')) {
        if(!line.startsWith("IMG_VERSION="))continue;
        auto value=line.mid(12).trimmed();
        if(value.startsWith('"') && value.endsWith('"'))value=value.mid(1,value.size()-2);
        if(value!="3.28.0.172")return false;
        ++matches;
    }
    return matches==1;
}
}
