#!/usr/bin/env python3
"""Apply the local lifecycle overlay to a separate, exact upstream build copy."""
import pathlib,shutil,subprocess,sys
upstream=pathlib.Path(sys.argv[1]).resolve();out=pathlib.Path(sys.argv[2]).resolve()
expected='6c709dd8416ddf89034831073d76fb4ee3372905'
actual=subprocess.check_output(['git','-C',str(upstream),'rev-parse','HEAD'],text=True).strip()
if actual!=expected:raise SystemExit('Unexpected broker upstream commit')
subprocess.run(['git','-C',str(upstream),'diff','--quiet','HEAD','--','xovi-message-broker'],check=True)
if out.exists():raise SystemExit('Fresh output directory required')
shutil.copytree(upstream/'xovi-message-broker',out,ignore=shutil.ignore_patterns('*.o','*.so','Makefile','moc_*','xovi.cpp','xovi.h','.qmake.stash'))
root=pathlib.Path(__file__).resolve().parents[2]
shutil.copyfile(root/'src/gui-dispatch.h',out/'src/gui-dispatch.h')
shutil.copyfile(pathlib.Path(__file__).with_name('pipes.cpp'),out/'src/pipes.cpp')
p=out/'src/pipes.h';p.write_text(p.read_text()+'\nvoid stopPipes();\nvoid shutdownPipes();\n')
p=out/'src/main.cpp';s=p.read_text();s=s.replace('#include "pipes.h"','#include "pipes.h"\n#include "gui-dispatch.h"\n\nstatic void installBrokerShutdown() {\n    auto *app=QCoreApplication::instance();\n    if(app) {\n        QObject::connect(app,&QCoreApplication::aboutToQuit,app,[]{ stopPipes(); },Qt::DirectConnection);\n        QObject::connect(app,&QObject::destroyed,[]{ shutdownPipes(); });\n    }\n}\nQ_COREAPP_STARTUP_FUNCTION(installBrokerShutdown)');p.write_text(s)
p=out/'src/messagebroker.cpp';s=p.read_text().replace('#include <cstring>','#include <cstring>\n#include "gui-dispatch.h"');start=s.index('    int i = 0;',s.index('extern "C" char *broadcast('));end=s.index('\n    return NULL;',start)
s=s[:start]+'''    const QString qSignal=QString::fromUtf8(signal), qValue=QString::fromUtf8(value);
    const auto result=diaryDispatch::run([qSignal,qValue]{
        int hits=0;QString value;
        for(auto *ref:brokers) if(ref->getListeningFor().contains(qSignal)) {
            QVariant raw;
            if(QMetaObject::invokeMethod(ref,"onSignalReceived",Qt::DirectConnection,
                Q_RETURN_ARG(QVariant,raw),Q_ARG(QVariant,qSignal),Q_ARG(QVariant,qValue))) {
                value=raw.toString();++hits;
            }
        }
        return QJsonObject{{"hits",hits},{"value",value}};
    });
    if(result.value("hits").toInt()==1 && !result.value("value").toString().isEmpty())
        return strdup(result.value("value").toString().toUtf8().constData());'''+s[end:];p.write_text(s)
# Preserve upstream license in a sibling metadata file when available.
for name in ['LICENSE','LICENSE.md','LICENSE.MD']:
 if (upstream/name).exists():shutil.copyfile(upstream/name,out/'UPSTREAM-LICENSE');break
print(out)
