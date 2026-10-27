// Local/SSH-terminal setup only. No network, broker, arming, or insertion code.
#include "config.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSaveFile>
#include <iostream>
#include <signal.h>
#include <sys/resource.h>
#include <termios.h>

namespace {
volatile sig_atomic_t interrupted=0;
void interrupt(int) { interrupted=1; }
struct Terminal {
    int fd=-1;
    termios original{};
    bool changed=false;
    Terminal() {
        fd=::open("/dev/tty",O_RDWR|O_CLOEXEC|O_NOCTTY);
        if(fd<0 || ::tcgetattr(fd,&original)!=0) {
            if(fd>=0) ::close(fd);
            throw std::runtime_error("A private interactive terminal is required");
        }
        auto hidden=original;
        hidden.c_lflag &= ~(ECHO|ECHONL);
        if(::tcsetattr(fd,TCSAFLUSH,&hidden)!=0) {
            ::close(fd); fd=-1;
            throw std::runtime_error("Cannot hide terminal input");
        }
        changed=true;
    }
    ~Terminal() {
        if(changed) ::tcsetattr(fd,TCSAFLUSH,&original);
        if(fd>=0) ::close(fd);
    }
    QByteArray readKey() {
        const char prompt[]="OpenAI API key (hidden; GPT-6 Luna test): ";
        if(::write(fd,prompt,sizeof(prompt)-1)<0) throw std::runtime_error("Terminal unavailable");
        QByteArray key;
        char c=0;
        while(!interrupted) {
            auto n=::read(fd,&c,1);
            if(n!=1) break;
            if(c=='\n') { ::write(fd,"\n",1); return key; }
            key.append(c);
            if(key.size()>512) break;
        }
        key.fill('\0');
        throw std::runtime_error("Key entry interrupted or too long; nothing saved");
    }
};
void requirePrivateParent(const QString &path) {
    QFileInfo parent(QFileInfo(path).absolutePath());
    struct stat st{};
    if(parent.canonicalFilePath()!=parent.absoluteFilePath() ||
       ::lstat(QFile::encodeName(parent.absoluteFilePath()).constData(),&st)!=0 ||
       !S_ISDIR(st.st_mode) || st.st_uid!=::geteuid() || (st.st_mode&0777)!=0700)
        throw std::runtime_error("Configuration parent must be an existing owned 0700 directory without symlink ancestors");
}
}

int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);
    rlimit noCore{0,0}; ::setrlimit(RLIMIT_CORE,&noCore);
    ::umask(077);
    const auto args=app.arguments();
    if((args.size()!=2 && args.size()!=4) ||
       (args[1]!="--configure" && args[1]!="--check") ||
       (args.size()==4 && args[2]!="--config")) {
        std::cerr << "Usage: configure-move-live --configure|--check [--config PATH]\n";
        return 2;
    }
    const auto path=QDir::cleanPath(args.size()==4 ? args[3] :
        QString("/home/root/.config/ink-diary/config.json"));
    try {
        if(!QDir::isAbsolutePath(path)) throw std::runtime_error("Configuration path must be absolute");
        requirePrivateParent(path);
        if(args[1]=="--check") {
            moveLive::loadKey(path);
            std::cout << "Private configuration valid for GPT-6 Luna. No API request made.\n";
            return 0;
        }
        struct stat before{};
        bool exists=::lstat(QFile::encodeName(path).constData(),&before)==0;
        if(!exists && errno!=ENOENT) throw std::runtime_error("Cannot inspect configuration");
        QJsonObject config=exists ? moveLive::readPrivateConfig(path) : QJsonObject{};
        if(config.value("demo_mode").toBool(false))
            throw std::runtime_error("Demo configuration left unchanged; use a separately reviewed live configuration");
        struct sigaction handler{};
        handler.sa_handler=interrupt; ::sigemptyset(&handler.sa_mask);
        for(int sig: {SIGINT,SIGTERM,SIGHUP,SIGQUIT}) ::sigaction(sig,&handler,nullptr);
        QByteArray key;
        { Terminal terminal; key=terminal.readKey(); }
        if(interrupted || !QRegularExpression("^[A-Za-z0-9_-]{1,512}$")
                .match(QString::fromLatin1(key)).hasMatch()) {
            key.fill('\0'); throw std::runtime_error("Invalid or interrupted key entry; nothing saved");
        }
        config.insert("api_key",QString::fromLatin1(key)); key.fill('\0');
        config.insert("model",moveLive::model);
        requirePrivateParent(path);
        struct stat now{};
        bool present=::lstat(QFile::encodeName(path).constData(),&now)==0;
        if(exists!=present || (exists && (now.st_dev!=before.st_dev || now.st_ino!=before.st_ino ||
           now.st_size!=before.st_size || now.st_mtim.tv_sec!=before.st_mtim.tv_sec ||
           now.st_mtim.tv_nsec!=before.st_mtim.tv_nsec || now.st_mode!=before.st_mode || now.st_nlink!=1)))
            throw std::runtime_error("Configuration changed during entry; nothing saved");
        auto bytes=QJsonDocument(config).toJson(QJsonDocument::Compact);
        if(bytes.size()>16384) {
            bytes.fill('\0');
            throw std::runtime_error("Configuration would exceed live-client limit; nothing saved");
        }
        QSaveFile output(path); output.setDirectWriteFallback(false);
        if(interrupted || !output.open(QIODevice::WriteOnly) ||
           !output.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner))
            throw std::runtime_error("Cannot securely stage configuration");
        bool written=output.write(bytes)==bytes.size(); bytes.fill('\0');
        if(!written || interrupted || !output.commit())
            throw std::runtime_error("Cannot commit configuration; original preserved");
        moveLive::loadKey(path);
        std::cout << "Private configuration saved; other fields preserved. No API request or arming performed.\n";
        return 0;
    } catch(const std::exception &e) {
        std::cerr << e.what() << '\n';
        return interrupted ? 130 : 1;
    }
}
