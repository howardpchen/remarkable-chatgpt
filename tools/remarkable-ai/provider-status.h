#pragma once
#include "../move-live/config.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QSaveFile>
#include <QDir>
namespace remarkableAi {
inline QString keyId(const QString &key) {
    return QString::fromLatin1(QCryptographicHash::hash(key.toUtf8(),QCryptographicHash::Sha256).toHex().left(16));
}
inline QString allowedCode(const QString &code) {
    for(const auto allowed:{"rate_limit_exceeded","insufficient_quota","invalid_api_key","model_not_found",
                           "unsupported_parameter","invalid_value","server_error"})
        if(code==QLatin1String(allowed))return code;
    return {};
}
// Only allowlisted metadata: never an error body, key, request, URL or notebook text.
inline void observe(const QString &root,const QString &fingerprint,int http,const QString &code) {
    struct stat st{};
    if(QDir(root).canonicalPath()!=QDir(root).absolutePath() ||
       ::lstat(QFile::encodeName(root).constData(),&st)!=0 || !S_ISDIR(st.st_mode) ||
       st.st_uid!=::geteuid() || (st.st_mode&0777)!=0700 || http<0 || http>599 ||
       !QRegularExpression("^[a-f0-9]{16}$").match(fingerprint).hasMatch())return;
    const auto path=root+"/provider-status.json";
    if(::lstat(QFile::encodeName(path).constData(),&st)==0 &&
       (!S_ISREG(st.st_mode) || st.st_uid!=::geteuid() || (st.st_mode&0777)!=0600 || st.st_nlink!=1))return;
    QJsonObject value{{"schema",1},{"keyId",fingerprint},{"httpStatus",http},
        {"code",allowedCode(code)},{"observedAt",QDateTime::currentDateTimeUtc().toString(Qt::ISODate)}};
    QSaveFile output(path);output.setDirectWriteFallback(false);
    if(!output.open(QIODevice::WriteOnly) || !output.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner))return;
    const auto bytes=QJsonDocument(value).toJson(QJsonDocument::Compact);
    if(output.write(bytes)==bytes.size())output.commit();
}
}
