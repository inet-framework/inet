/***************************************************************************
                          RTP.cc  -  description
                             -------------------
    (C) 2007 Ahmed Ayadi  <ahmed.ayadi@sophia.inria.fr>
    (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>

***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "inet/transportlayer/rtp/RTP.h"

#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/transportlayer/rtp/RTPInnerPacket.h"
#include "inet/transportlayer/rtp/RTPInterfacePacket_m.h"
#include "inet/transportlayer/rtp/RTPProfile.h"
#include "inet/transportlayer/rtp/RTPSenderControlMessage_m.h"
#include "inet/transportlayer/rtp/RTPSenderStatusMessage_m.h"
#include "inet/transportlayer/contract/udp/UDPControlInfo_m.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"

namespace inet {

namespace rtp {

Define_Module(RTP);

simsignal_t RTP::rcvdPkSignal = registerSignal("rcvdPk");

//
// methods inherited from cSimpleModule
//

void RTP::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        _leaveSession = false;
        appInGate = findGate("appIn");
        profileInGate = findGate("profileIn");
        rtcpInGate = findGate("rtcpIn");
        udpInGate = findGate("udpIn");
        _udpSocket.setOutputGate(gate("udpOut"));
    }
    else if (stage == INITSTAGE_TRANSPORT_LAYER) {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}

void RTP::handleMessage(cMessage *msg)
{
    if (msg->getArrivalGateId() == appInGate) {
        handleMessageFromApp(msg);
    }
    else if (msg->getArrivalGateId() == profileInGate) {
        handleMessageFromProfile(msg);
    }
    else if (msg->getArrivalGateId() == rtcpInGate) {
        handleMessageFromRTCP(msg);
    }
    else if (msg->getArrivalGateId() == udpInGate) {
        handleMessagefromUDP(msg);
    }
    else {
        throw cRuntimeError("Message from unknown gate");
    }
}

//
// handle messages from different gates
//
void RTP::handleMessageFromApp(cMessage *msg)
{
    RTPControlInfo *ci = check_and_cast<RTPControlInfo *>(msg->removeControlInfo());
    delete msg;

    switch (ci->getType()) {
        case RTP_IFP_ENTER_SESSION:
            enterSession(check_and_cast<RTPCIEnterSession *>(ci));
            break;

        case RTP_IFP_CREATE_SENDER_MODULE:
            createSenderModule(check_and_cast<RTPCICreateSenderModule *>(ci));
            break;

        case RTP_IFP_DELETE_SENDER_MODULE:
            deleteSenderModule(check_and_cast<RTPCIDeleteSenderModule *>(ci));
            break;

        case RTP_IFP_SENDER_CONTROL:
            senderModuleControl(check_and_cast<RTPCISenderControl *>(ci));
            break;

        case RTP_IFP_LEAVE_SESSION:
            leaveSession(check_and_cast<RTPCILeaveSession *>(ci));
            break;

        default:
            throw cRuntimeError("Unknown RTPControlInfo type from application");
    }
}

void RTP::handleMessageFromProfile(cMessage *msg)
{
    RTPInnerPacket *rinp = check_and_cast<RTPInnerPacket *>(msg);

    switch (rinp->getType()) {
        case RTP_INP_PROFILE_INITIALIZED:
            profileInitialized(rinp);
            break;

        case RTP_INP_SENDER_MODULE_CREATED:
            senderModuleCreated(rinp);
            break;

        case RTP_INP_SENDER_MODULE_DELETED:
            senderModuleDeleted(rinp);
            break;

        case RTP_INP_SENDER_MODULE_INITIALIZED:
            senderModuleInitialized(rinp);
            break;

        case RTP_INP_SENDER_MODULE_STATUS:
            senderModuleStatus(rinp);
            break;

        case RTP_INP_DATA_OUT:
            dataOut(rinp);
            break;

        default:
            throw cRuntimeError("Unknown RTPInnerPacket type %d from profile", rinp->getType());
    }
    EV_DEBUG << "handleMessageFromProfile(cMessage *msg) Exit" << endl;
}

void RTP::handleMessageFromRTCP(cMessage *msg)
{
    RTPInnerPacket *rinp = check_and_cast<RTPInnerPacket *>(msg);

    switch (rinp->getType()) {
        case RTP_INP_RTCP_INITIALIZED:
            rtcpInitialized(rinp);
            break;

        case RTP_INP_SESSION_LEFT:
            sessionLeft(rinp);
            break;

        default:
            throw cRuntimeError("Unknown RTPInnerPacket type %d from rtcp", rinp->getType());
    }
}

void RTP::handleMessagefromUDP(cMessage *msg)
{
    readRet(msg);
}

//
// methods for different messages
//

void RTP::enterSession(RTPCIEnterSession *rifp)
{
    const char *profileName = rifp->getProfileName();
    _commonName = rifp->getCommonName();
    _bandwidth = rifp->getBandwidth();
    _destinationAddress = rifp->getDestinationAddress();

    _port = rifp->getPort();
    if (_port & 1)
        _port--;

    _mtu = resolveMTU();

    createProfile(profileName);
    initializeProfile();
    delete rifp;
}

void RTP::leaveSession(RTPCILeaveSession *rifp)
{
    if (!_leaveSession) {
        _leaveSession = true;
        cModule *profileModule = gate("profileOut")->getNextGate()->getOwnerModule();
        profileModule->deleteModule();
        RTPInnerPacket *rinp = new RTPInnerPacket("leaveSession()");
        rinp->setLeaveSessionPkt();
        send(rinp, "rtcpOut");
    }
    delete rifp;
}

void RTP::createSenderModule(RTPCICreateSenderModule *rifp)
{
    RTPInnerPacket *rinp = new RTPInnerPacket("createSenderModule()");
    EV_INFO << rifp->getSsrc() << endl;
    rinp->setCreateSenderModulePkt(rifp->getSsrc(), rifp->getPayloadType(), rifp->getFileName());
    send(rinp, "profileOut");

    delete rifp;
}

void RTP::deleteSenderModule(RTPCIDeleteSenderModule *rifp)
{
    RTPInnerPacket *rinp = new RTPInnerPacket("deleteSenderModule()");
    rinp->setDeleteSenderModulePkt(rifp->getSsrc());
    send(rinp, "profileOut");

    delete rifp;
}

void RTP::senderModuleControl(RTPCISenderControl *rifp)
{
    RTPInnerPacket *rinp = new RTPInnerPacket("senderModuleControl()");
    RTPSenderControlMessage *scm = new RTPSenderControlMessage();
    scm->setCommand(rifp->getCommand());
    scm->setCommandParameter1(rifp->getCommandParameter1());
    scm->setCommandParameter2(rifp->getCommandParameter2());
    rinp->setSenderModuleControlPkt(rinp->getSsrc(), scm);
    send(rinp, "profileOut");

    delete rifp;
}

void RTP::profileInitialized(RTPInnerPacket *rinp)
{
    _rtcpPercentage = rinp->getRtcpPercentage();
    if (_port == PORT_UNDEF) {
        _port = rinp->getPort();
        if (_port & 1)
            _port--;
    }

    delete rinp;

    createSocket();
}

void RTP::senderModuleCreated(RTPInnerPacket *rinp)
{
    RTPCISenderModuleCreated *ci = new RTPCISenderModuleCreated();
    ci->setSsrc(rinp->getSsrc());
    cMessage *msg = new RTPControlMsg("senderModuleCreated()");
    msg->setControlInfo(ci);
    send(msg, "appOut");

    delete rinp;
}

void RTP::senderModuleDeleted(RTPInnerPacket *rinp)
{
    RTPCISenderModuleDeleted *ci = new RTPCISenderModuleDeleted();
    ci->setSsrc(rinp->getSsrc());
    cMessage *msg = new RTPControlMsg("senderModuleDeleted()");
    msg->setControlInfo(ci);
    send(msg, "appOut");
    // perhaps we should send a message to rtcp module
    delete rinp;
}

void RTP::senderModuleInitialized(RTPInnerPacket *rinp)
{
    send(rinp, "rtcpOut");
}

void RTP::senderModuleStatus(RTPInnerPacket *rinp)
{
    RTPSenderStatusMessage *ssm = (RTPSenderStatusMessage *)(rinp->decapsulate());
    RTPCISenderStatus *ci = new RTPCISenderStatus();
    ci->setSsrc(rinp->getSsrc());
    ci->setStatus(ssm->getStatus());
    ci->setTimeStamp(ssm->getTimeStamp());
    cMessage *msg = new RTPControlMsg("senderModuleStatus()");
    msg->setControlInfo(ci);
    send(msg, "appOut");
    delete ssm;
    delete rinp;
}

void RTP::dataOut(RTPInnerPacket *rinp)
{
    RTPPacket *msg = check_and_cast<RTPPacket *>(rinp->getEncapsulatedPacket()->dup());

    _udpSocket.sendTo(msg, _destinationAddress, _port);

    // RTCP module must be informed about sent rtp data packet
    send(rinp, "rtcpOut");
}

void RTP::rtcpInitialized(RTPInnerPacket *rinp)
{
    RTPCISessionEntered *ci = new RTPCISessionEntered();
    ci->setSsrc(rinp->getSsrc());
    cMessage *msg = new RTPControlMsg("sessionEntered()");
    msg->setControlInfo(ci);
    send(msg, "appOut");

    delete rinp;
}

void RTP::sessionLeft(RTPInnerPacket *rinp)
{
    RTPCISessionLeft *ci = new RTPCISessionLeft();
    cMessage *msg = new RTPControlMsg("sessionLeft()");
    msg->setControlInfo(ci);
    send(msg, "appOut");

    delete rinp;
}

void RTP::socketRet()
{
}

void RTP::connectRet()
{
    initializeRTCP();
}

void RTP::readRet(cMessage *sifp)
{
    if (!_leaveSession) {
        RTPPacket *msg = check_and_cast<RTPPacket *>(sifp);

        emit(rcvdPkSignal, msg);

        msg->dump();
        RTPInnerPacket *rinp1 = new RTPInnerPacket("dataIn1()");
        rinp1->setDataInPkt(new RTPPacket(*msg), IPv4Address(_destinationAddress), _port);
        RTPInnerPacket *rinp2 = new RTPInnerPacket(*rinp1);
        send(rinp2, "rtcpOut");
        send(rinp1, "profileOut");
    }

    delete sifp;
}

int RTP::resolveMTU()
{
    // it returns MTU bytelength (ethernet) minus ip
    // and udp headers
    // TODO: How to do get the valid length of IP and ETHERNET header?
    IIPv4RoutingTable *rt = getModuleFromPar<IIPv4RoutingTable>(par("routingTableModule"), this);
    const InterfaceEntry *rtie = rt->getInterfaceForDestAddr(_destinationAddress);

    if (rtie == nullptr)
        throw cRuntimeError("No interface for remote address %s found!", _destinationAddress.str().c_str());

    int pmtu = rtie->getMTU();
    return pmtu - 20 - 8;
}

void RTP::createProfile(const char *profileName)
{
    cModuleType *moduleType = cModuleType::find(profileName);
    if (moduleType == nullptr)
        throw cRuntimeError("Profile type `%s' not found", profileName);

    RTPProfile *profile = check_and_cast<RTPProfile *>(moduleType->create("Profile", this));
    profile->finalizeParameters();

    profile->setGateSize("payloadReceiverOut", 30);
    profile->setGateSize("payloadReceiverIn", 30);

    this->gate("profileOut")->connectTo(profile->gate("rtpIn"));
    profile->gate("rtpOut")->connectTo(this->gate("profileIn"));

    profile->callInitialize();
    profile->scheduleStart(simTime());
}

void RTP::createSocket()
{
    _udpSocket.bind(_port);
    MulticastGroupList mgl = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)->collectMulticastGroups();
    _udpSocket.joinLocalMulticastGroups(mgl);    //TODO make it parameter-dependent
    connectRet();
}

void RTP::initializeProfile()
{
    RTPInnerPacket *rinp = new RTPInnerPacket("initializeProfile()");
    rinp->setInitializeProfilePkt(_mtu);
    send(rinp, "profileOut");
}

void RTP::initializeRTCP()
{
    RTPInnerPacket *rinp = new RTPInnerPacket("initializeRTCP()");
    int rtcpPort = _port + 1;
    rinp->setInitializeRTCPPkt(_commonName, _mtu, _bandwidth, _rtcpPercentage, _destinationAddress, rtcpPort);
    send(rinp, "rtcpOut");
}

bool RTP::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

} // namespace rtp

} // namespace inet

