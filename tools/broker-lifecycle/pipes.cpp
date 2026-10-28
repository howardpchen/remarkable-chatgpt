// Lifecycle overlay for upstream rm-xovi-extensions broker ABI 0.2.0.
// Preserve the FIFO wire protocol; all descriptors belong to a joinable worker.
#include "pipes.h"
#include "messagebroker.h"
#include <atomic>
#include <pthread.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <string>
#include <cstdio>

namespace {
std::atomic<bool> stopping{false};
pthread_t worker;
bool started=false;
int wakeFds[2]={-1,-1};
ino_t inInode=0,outInode=0;
bool ownedFifo(const char *path,ino_t &inode) {
    if(mkfifo(path,0660)!=0)return false; // Never replace somebody else's endpoint.
    struct stat st;
    if(lstat(path,&st)!=0 || !S_ISFIFO(st.st_mode))return false;
    inode=st.st_ino; return true;
}
void removeOwned(const char *path,ino_t inode) {
    struct stat st;
    if(inode && lstat(path,&st)==0 && S_ISFIFO(st.st_mode) && st.st_ino==inode)unlink(path);
}
bool waitFd(int fd,short events,int timeout) {
    struct pollfd fds[2]={{fd,events,0},{wakeFds[0],POLLIN,0}};
    int n;
    do {n=poll(fds,2,timeout);}while(n<0 && errno==EINTR && !stopping.load());
    return n>0 && !stopping.load() && !(fds[0].revents&(POLLERR|POLLHUP|POLLNVAL)) && (fds[0].revents&events);
}
void reply(const char *output) {
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);
    int fd=-1;
    while(!stopping.load() && std::chrono::steady_clock::now()<deadline) {
        fd=open(OUT_PIPE,O_WRONLY|O_NONBLOCK|O_CLOEXEC);
        if(fd>=0)break;
        if(errno!=ENXIO && errno!=EINTR)return;
        struct pollfd wake={wakeFds[0],POLLIN,0}; poll(&wake,1,20);
    }
    if(fd<0)return;
    size_t sent=0,length=output?strlen(output):0;
    while(sent<length && !stopping.load() && std::chrono::steady_clock::now()<deadline) {
        ssize_t n=write(fd,output+sent,length-sent);
        if(n>0)sent+=size_t(n);
        else if(n<0 && errno==EINTR)continue;
        else if(n<0 && (errno==EAGAIN || errno==EWOULDBLOCK))waitFd(fd,POLLOUT,20);
        else break;
    }
    close(fd);
}
void execute(std::string command) {
    bool output=false;
    size_t pos=0;
    if(command[pos]=='>'){output=true;++pos;}
    if(pos>=command.size())return;
    char route=command[pos++];
    if(route!='e' && route!='u')return;
    const auto colon=command.find(':',pos);
    if(colon==std::string::npos)return;
    command[colon]='\0';
    int hits=1;
    char *result=nullptr;
    if(!stopping.load()) {
        if(route=='e')result=broadcastToNative(command.c_str()+pos,command.c_str()+colon+1,&hits);
        else result=broadcast(command.c_str()+pos,command.c_str()+colon+1);
    }
    if(output && hits==1 && !stopping.load())reply(result);
    free(result);
}
void *loop(void *) {
    // A vanished reader must not deliver SIGPIPE to the application process.
    sigset_t signals;sigemptyset(&signals);sigaddset(&signals,SIGPIPE);
    pthread_sigmask(SIG_BLOCK,&signals,nullptr);
    int fd=open(IN_CMD_PIPE,O_RDWR|O_NONBLOCK|O_CLOEXEC);
    if(fd<0)return nullptr;
    std::string command;bool overflow=false;
    while(!stopping.load()) {
        if(!waitFd(fd,POLLIN,100))continue;
        char bytes[256];ssize_t n=read(fd,bytes,sizeof(bytes));
        if(n<=0)continue;
        for(ssize_t i=0;i<n && !stopping.load();++i) {
            if(bytes[i]=='\n') {
                if(!overflow && !command.empty())execute(std::move(command));
                command.clear();overflow=false;
            } else if(!overflow) {
                if(command.size()>=MAX_COMMAND_SIZE-1 || bytes[i]=='\0') {overflow=true;command.clear();}
                else command+=bytes[i];
            }
        }
    }
    close(fd); return nullptr;
}
}
void stopPipes() {
    stopping.store(true);
    if(wakeFds[1]>=0) {char byte=1; ssize_t ignored=write(wakeFds[1],&byte,1); (void)ignored;}
}
void shutdownPipes() {
    stopPipes();
    if(started) {pthread_join(worker,nullptr);started=false;}
    for(int &fd:wakeFds)if(fd>=0){close(fd);fd=-1;}
    removeOwned(IN_CMD_PIPE,inInode);removeOwned(OUT_PIPE,outInode);
    inInode=outInode=0;
}
void initializePipes() {
    if(started)return;
    stopping.store(false);
    if(pipe2(wakeFds,O_CLOEXEC|O_NONBLOCK)!=0 || !ownedFifo(IN_CMD_PIPE,inInode) || !ownedFifo(OUT_PIPE,outInode)) {
        shutdownPipes();return;
    }
    if(pthread_create(&worker,nullptr,loop,nullptr)!=0) {shutdownPipes();return;}
    started=true;
}
// Not a UI-thread join: aboutToQuit only closes intake and wakes the worker.
// App-context destruction/ELF teardown happens after all quit callbacks run.
__attribute__((destructor)) static void finishBrokerWorker() {shutdownPipes();}
