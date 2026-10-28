#include <atomic>
#include <cassert>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <thread>
#include <string>
#include <iostream>
#include "pipes.h"
std::atomic<int> calls{0};
extern "C" char *broadcastToNative(const char *,const char *value,int *hits) {
    ++calls;*hits=1;
    return strdup(std::string(strcmp(value,"large")==0?150000:2,'x').c_str());
}
extern "C" char *broadcast(const char *,const char *) {return nullptr;}
void send(const std::string &s) {int fd=open(IN_CMD_PIPE,O_WRONLY);assert(fd>=0);assert(write(fd,s.data(),s.size())==ssize_t(s.size()));close(fd);}
int main() {
    initializePipes();
    int fd=open(OUT_PIPE,O_RDONLY|O_NONBLOCK);assert(fd>=0);
    send(">etest:large\n");
    std::string response;
    auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);
    while(response.size()<150000 && std::chrono::steady_clock::now()<deadline) {
        char bytes[512];ssize_t n=read(fd,bytes,sizeof bytes);
        if(n>0)response.append(bytes,n);else {pollfd f={fd,POLLIN,0};poll(&f,1,10);}
    }
    close(fd);assert(response.size()==150000 && calls==1);
    send(std::string(1200,'a')+"\n");send("etest:ok\n");
    while(calls<2 && std::chrono::steady_clock::now()<deadline)std::this_thread::yield();
    assert(calls==2);
    // An abandoned output reader never hangs exit. Shutdown interrupts its wait.
    send(">etest:ok\n");while(calls<3)std::this_thread::yield();
    auto start=std::chrono::steady_clock::now();
    stopPipes();shutdownPipes();
    assert(std::chrono::steady_clock::now()-start<std::chrono::seconds(1));
    assert(access(IN_CMD_PIPE,F_OK)!=0 && access(OUT_PIPE,F_OK)!=0);
    // Existing endpoint must be preserved, not replaced/deleted.
    int sentinel=open(IN_CMD_PIPE,O_WRONLY|O_CREAT|O_EXCL,0600);assert(sentinel>=0);close(sentinel);
    initializePipes();shutdownPipes();assert(access(IN_CMD_PIPE,F_OK)==0);unlink(IN_CMD_PIPE);
    std::cout<<"Broker worker checks passed: large partial response, oversized-input rejection, no-reader cancellation, join, cleanup and existing-endpoint preservation\n";
}
