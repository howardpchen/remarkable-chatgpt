#include "policy.h"
#include <iostream>
int main() {
    using diagnostic::permitted;
    const auto machine=QByteArray(diagnostic::machineName);
    const auto otherMachine=machine=="reMarkable Chiappa" ? "reMarkable Ferrari" : "reMarkable Chiappa";
    const auto os=QByteArray("NAME=reMarkable\nIMG_VERSION=\"3.28.0.172\"\n");
    if(!permitted(machine+"\n",os,"6.10.3",true,true) ||
        permitted(otherMachine,os,"6.10.3",true,true) ||
        permitted("reMarkable 2",os,"6.10.3",true,true) ||
        permitted(machine,os,"6.10.2",true,true) ||
        permitted(machine,"IMG_VERSION=3.28.0.171\n","6.10.3",true,true) ||
        permitted(machine,os+"IMG_VERSION=3.28.0.172\n","6.10.3",true,true) ||
        permitted(machine,"VERSION_ID=3.28.0.172\n","6.10.3",true,true))return 1;
    for(bool gui:{false,true})for(bool deviceLock:{false,true}) {
        if(permitted(machine,os,"6.10.3",gui,deviceLock)!=(gui&&deviceLock))return 1;
    }
    std::cout<<"Diagnostic runtime policy tests passed: "<<diagnostic::model<<"\n";
}
