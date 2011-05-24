
#include "Ieee802154TestApp.h"

//#undef EV
//#define EV (ev.isDisabled() || !m_debug) ? std::cout : ev ==> EV is now part of <omnetpp.h>

Define_Module(Ieee802154TestApp);

void Ieee802154TestApp::initialize(int aStage)
{
    TrafGenPar::initialize(aStage);
    EV << getParentModule()->getFullName() << ": initializing Ieee802154TestApp, stage=" << aStage << endl;
    if (0 == aStage)
    {
        m_debug             = par("debug");
        mLowergateIn        = findGate("lowergateIn");
        mLowergateOut       = findGate("lowergateOut");
        m_moduleName        = getParentModule()->getFullName();
        sumE2EDelay         = 0;
        numReceived         = 0;
        mNumTrafficMsgs     = 0;
        totalByteRecv           = 0;
        e2eDelayVec.setName("End-to-end delay");
        meanE2EDelayVec.setName("Mean end-to-end delay");
    }
}

void Ieee802154TestApp::finish()
{
    recordScalar("trafficSent", mNumTrafficMsgs);
    recordScalar("total bytes received", totalByteRecv);
    //recordScalar("total time", simTime() - FirstPacketTime());
    //recordScalar("goodput (Bytes/s)", totalByteRecv / (simTime() - FirstPacketTime()));
}

void Ieee802154TestApp::handleLowerMsg(cMessage* apMsg)
{
    simtime_t e2eDelay;
    Ieee802154AppPkt* tmpPkt = check_and_cast<Ieee802154AppPkt *>(apMsg);
    e2eDelay = simTime() - tmpPkt->getCreationTime();
    totalByteRecv += tmpPkt->getByteLength();
    e2eDelayVec.record(SIMTIME_DBL(e2eDelay));
    numReceived++;
    sumE2EDelay += e2eDelay;
    meanE2EDelayVec.record(sumE2EDelay/numReceived);
    EV << "[APP]: a message sent by " << tmpPkt->getSourceName() << " arrived at application with delay " << e2eDelay << " s" << endl;
    delete apMsg;
}

//***************************************************************
// Reimplement this function and use msg type Ieee802154AppPkt for app pkts
//***************************************************************
void Ieee802154TestApp::handleSelfMsg(cMessage *apMsg)
{
    TrafGenPar::handleSelfMsg(apMsg);
}

/** this function has to be redefined in every application derived from the
    TrafGen class.
    Its purpose is to translate the destination (given, for example, as "host[5]")
    to a valid address (MAC, IP, ...) that can be understood by the next lower
    layer.
    It also constructs an appropriate control info block that might be needed by
    the lower layer to process the message.
    In the example, the messages are sent directly to a mac 802.11 layer, address
    and control info are selected accordingly.
*/
void Ieee802154TestApp::SendTraf(cPacket* apMsg, const char* apDest)
{
    delete apMsg;
    // create a new app pkt
    Ieee802154AppPkt* appPkt = new Ieee802154AppPkt("Ieee802154AppPkt");

    appPkt->setBitLength(PacketSize()*8);
    appPkt->setSourceName(m_moduleName);
    appPkt->setDestName(apDest);
    appPkt->setCreationTime(simTime());

    /*Ieee802154UpperCtrlInfo *control_info = new Ieee802154UpperCtrlInfo();
    control_info->setDestName(apDest);
    appPkt->setControlInfo(control_info);*/

    mNumTrafficMsgs++;
    send(appPkt, mLowergateOut);

}
