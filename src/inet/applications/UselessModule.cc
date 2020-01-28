#include "inet/applications/base/ApplicationPacket_m.h"
//#include "inet/applications/udpapp/UdpBasicApp.h"
#include "inet/applications/UselessModule.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
//#include "inet/networklayer/common/FragmentationTag_m.h"
//#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

namespace inet {

//Define_Module(UselessUdpApp);

UselessUdpApp::~UselessUdpApp()
{
    cancelAndDelete(selfMsg);
}

void UselessUdpApp::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
//        numSent = 0;
//        numReceived = 0;
//        WATCH(numSent);
//        WATCH(numReceived);
//
//        localPort = par("localPort");
//        destPort = par("destPort");
//        startTime = par("startTime");
//        stopTime = par("stopTime");
//        packetName = par("packetName");
//        dontFragment = par("dontFragment");
//        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
//            throw cRuntimeError("Invalid startTime/stopTime parameters");
        selfMsg = new cMessage("uselessTimer");
        scheduleAt(simTime() + 1E-6, selfMsg);
    }
}

void UselessUdpApp::handleSelfMessage(cMessage *message)
{
            std::cout << "USELESS APP WORKING!" << endl;
//            char buf[20];
//            strcpy(buf, "Sent %d useless msg's", x)
//            getDisplayString().setTagArg("t", 1, "r");
//            getDisplayString().setTagArg("t", 0, buf);
            delete message;
}

//void UselessUdpApp::finish()
//{
//    recordScalar("packets sent", numSent);
//    recordScalar("packets received", numReceived);
//    ApplicationBase::finish();
//}

}
