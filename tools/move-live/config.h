#pragma once
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <stdexcept>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace moveLive {
inline constexpr auto model="gpt-6-luna";
// Open only the expected private regular file, without following a symlink.
// No credential contents enter status, exceptions, evidence, or logs.
inline QJsonObject readPrivateConfig(const QString &path) {
    int fd=::open(QFile::encodeName(path).constData(),O_RDONLY|O_NOFOLLOW|O_CLOEXEC|O_NONBLOCK);
    if(fd<0) throw std::runtime_error("Private API configuration missing");
    struct stat st{};
    if(::fstat(fd,&st)!=0 || !S_ISREG(st.st_mode) || st.st_uid!=::geteuid() ||
       (st.st_mode&0777)!=0600 || st.st_nlink!=1 || st.st_size<1 || st.st_size>16384) {
        ::close(fd); throw std::runtime_error("API configuration must be owner-only regular file (0600)");
    }
    QFile file;
    if(!file.open(fd,QIODevice::ReadOnly,QFileDevice::AutoCloseHandle)) {
        ::close(fd);throw std::runtime_error("Cannot read API configuration");
    }
    QJsonParseError error;
    auto doc=QJsonDocument::fromJson(file.read(16385),&error);
    if(error.error!=QJsonParseError::NoError || !doc.isObject())
        throw std::runtime_error("Invalid private configuration JSON");
    return doc.object();
}
inline QString loadKey(const QString &path) {
    auto config=readPrivateConfig(path);
    const auto key=config.value("api_key").toString();
    if(!QRegularExpression("^[A-Za-z0-9_-]{1,512}$").match(key).hasMatch() ||
       config.value("model").toString()!=model || config.value("demo_mode").toBool(false))
        throw std::runtime_error("Live test requires API key, gpt-6-luna, and no demo mode");
    return key;
}
} // namespace moveLive
