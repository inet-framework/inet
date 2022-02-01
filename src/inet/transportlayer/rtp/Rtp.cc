//
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
// Copyright (C) 2007 Ahmed Ayadi <ahmed.ayadi@sophia.inria.fr>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/rtp/Rtp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/transportlayer/rtp/RtpInnerPacket_m.h"
#include "inet/transportlayer/rtp/RtpInterfacePacket_m.h"
#include "inet/transportlayer/rtp/RtpProfile.h"
#include "inet/transportlayer/rtp/RtpSenderControlMessage_m.h"
#include "inet/transportlayer/rtp/RtpSenderStatusMessage_m.h"

namespace inet {

namespace rtp {

Define_Module(Rtp);

//
// methods inherited from cSimpleModule
//

void Rtp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        _commonName = "";
        _leaveSession = false;
        appInGate = findGate("appIn");
        profileInGate = findGate("profileIn");
        rtcpInGate = findGate("rtcpIn");
        udpInGate = findGate("udpIn");
        _udpSocket.setOutputGate(gate("udpOut"));
    }
    else if (stage == INITSTAGE_TRANSPORT_LAYER) {
        cModule *node = findContainingNode(this);
        NodeStatus *nodeStatus = node ? check_and_cast_nullable<NodeStatus *>(node->getSubmodule("status")) : nullptr;
        bool isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}

void Rtp::handleMessage(cMessage *msg)
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
void Rtp::handleMessageFromApp(cMessage *msg)
{
    RtpControlInfo *ci = check_and_cast<RtpControlInfo *>(msg->removeControlInfo());
    delete msg;

    switch (ci->getType()) {
        case RTP_IFP_ENTER_SESSION:
            enterSession(check_and_cast<RtpCiEnterSession *>(ci));
            break;

        case RTP_IFP_CREATE_SENDER_MODULE:
            createSenderModule(check_and_cast<RtpCiCreateSenderModule *>(ci));
            break;

        case RTP_IFP_DELETE_SENDER_MODULE:
            deleteSenderModule(check_and_cast<RtpCiDeleteSenderModule *>(ci));
            break;

        case RTP_IFP_SENDER_CONTROL:
            senderModuleControl(check_and_cast<RtpCiSenderControl *>(ci));
            break;

        case RTP_IFP_LEAVE_SESSION:
            leaveSession(check_and_cast<RtpCiLeaveSession *>(ci));
            break;

        default:
            throw cRuntimeError("Unknown RtpControlInfo type from application");
    }
}

void Rtp::handleMessageFromProfile(cMessage *msg)
{
    RtpInnerPacket *rinp = check_and_cast<RtpInnerPacket *>(msg);

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
            throw cRuntimeError("Unknown RtpInnerPacket type %d from profile", rinp->getType());
    }
    EV_DEBUG << "handleMessageFromProfile(cMessage *msg) Exit" << endl;
}

void Rtp::handleMessageFromRTCP(cMessage *msg)
{
    RtpInnerPacket *rinp = check_and_cast<RtpInnerPacket *>(msg);

    switch (rinp->getType()) {
        case RTP_INP_RTCP_INITIALIZED:
            rtcpInitialized(rinp);
            break;

        case RTP_INP_SESSION_LEFT:
            sessionLeft(rinp);
            break;

        default:
            throw cRuntimeError("Unknown RtpInnerPacket type %d from rtcp", rinp->getType());
    }
}

void Rtp::handleMessagefromUDP(cMessage *msg)
{
    readRet(msg);
}

//
// methods for different messages
//

void Rtp::enterSession(RtpCiEnterSession *rifp)
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

void Rtp::leaveSession(RtpCiLeaveSession *rifp)
{
    if (!_leaveSession) {
        _leaveSession = true;
        cModule *profileModule = gate("profileOut")->getNextGate()->getOwnerModule();
        profileModule->deleteModule();
        RtpInnerPacket *rinp = new RtpInnerPacket("leaveSession()");
        rinp->setLeaveSessionPkt();
        send(rinp, "rtcpOut");
    }
    delete rifp;
}

void Rtp::createSenderModule(RtpCiCreateSenderModule *rifp)
{
    RtpInnerPacket *rinp = new RtpInnerPacket("createSenderModule()");
    EV_INFO << rifp->getSsrc() << endl;
    rinp->setCreateSenderModulePkt(rifp->getSsrc(), rifp->getPayloadType(), rifp->getFileName());
    send(rinp, "profileOut");

    delete rifp;
}

void Rtp::deleteSenderModule(RtpCiDeleteSenderModule *rifp)
{
    RtpInnerPacket *rinp = new RtpInnerPacket("deleteSenderModule()");
    rinp->setDeleteSenderModulePkt(rifp->getSsrc());
    send(rinp, "profileOut");

    delete rifp;
}

void Rtp::senderModuleControl(RtpCiSenderControl *rifp)
{
    RtpInnerPacket *rinp = new RtpInnerPacket("senderModuleControl()");
    RtpSenderControlMessage *scm = new RtpSenderControlMessage();
    scm->setCommand(rifp->getCommand());
    scm->setCommandParameter1(rifp->getCommandParameter1());
    scm->setCommandParameter2(rifp->getCommandParameter2());
    rinp->setSenderModuleControlPkt(rinp->getSsrc(), scm);
    send(rinp, "profileOut");

    delete rifp;
}

void Rtp::profileInitialized(RtpInnerPacket *rinp)
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

void Rtp::senderModuleCreated(RtpInnerPacket *rinp)
{
    RtpCiSenderModuleCreated *ci = new RtpCiSenderModuleCreated();
    ci->setSsrc(rinp->getSsrc());
    cMessage *msg = new RtpControlMsg("senderModuleCreated()");
    msg->setControlInfo(ci);
    send(msg, "appOut");

    delete rinp;
}

void Rtp::senderModuleDeleted(RtpInnerPacket *rinp)
{
    RtpCiSenderModuleDeleted *ci = new RtpCiSenderModuleDeleted();
    ci->setSsrc(rinp->getSsrc());
    cMessage *msg = new RtpControlMsg("senderModuleDeleted()");
    msg->setControlInfo(ci);
    send(msg, "appOut");
    // perhaps we should send a message to rtcp module
    delete rinp;
}

void Rtp::senderModuleInitialized(RtpInnerPacket *rinp)
{
    send(rinp, "rtcpOut");
}

void Rtp::senderModuleStatus(RtpInnerPacket *rinp)
{
    RtpSenderStatusMessage *ssm = check_and_cast<RtpSenderStatusMessage *>(rinp->decapsulate());
    RtpCiSenderStatus *ci = new RtpCiSenderStatus();
    ci->setSsrc(rinp->getSsrc());
    ci->setStatus(ssm->getStatus());
    ci->setTimeStamp(ssm->getTimeStamp());
    cMessage *msg = new RtpControlMsg("senderModuleStatus()");
    msg->setControlInfo(ci);
    send(msg, "appOut");
    delete ssm;
    delete rinp;
}

void Rtp::dataOut(RtpInnerPacket *rinp)
{
    Packet *packet = check_and_cast<Packet *>(rinp->getEncapsulatedPacket()->dup());
//    RtpPacket *msg = check_and_cast<RtpPacket *>(rinp->getEncapsulatedPacket()->dup());      //FIXME kell itt az RtpPacket?

    _udpSocket.sendTo(packet, _destinationAddress, _port);

    // Rtcp module must be informed about sent rtp data packet
    send(rinp, "rtcpOut");
}

void Rtp::rtcpInitialized(RtpInnerPacket *rinp)
{
    RtpCiSessionEntered *ci = new RtpCiSessionEntered();
    ci->setSsrc(rinp->getSsrc());
    cMessage *msg = new RtpControlMsg("sessionEntered()");
    msg->setControlInfo(ci);
    send(msg, "appOut");

    delete rinp;
}

void Rtp::sessionLeft(RtpInnerPacket *rinp)
{
    RtpCiSessionLeft *ci = new RtpCiSessionLeft();
    cMessage *msg = new RtpControlMsg("sessionLeft()");
    msg->setControlInfo(ci);
    send(msg, "appOut");

    delete rinp;
}

void Rtp::socketRet()
{
}

void Rtp::connectRet()
{
    initializeRTCP();
}

void Rtp::readRet(cMessage *sifp)
{
    if (!_leaveSession) {
        Packet *pk = check_and_cast<Packet *>(sifp);
        const auto& rtpHeader = pk->peekAtFront<RtpHeader>();

        emit(packetReceivedSignal, pk);

        rtpHeader->dump();
        RtpInnerPacket *rinp1 = new RtpInnerPacket("dataIn1()");
        rinp1->setDataInPkt(pk->dup(), Ipv4Address(_destinationAddress), _port);
        RtpInnerPacket *rinp2 = new RtpInnerPacket(*rinp1);
        send(rinp2, "rtcpOut");
        send(rinp1, "profileOut");
    }

    delete sifp;
}

int Rtp::resolveMTU()
{
    // it returns MTU bytelength (ethernet) minus ip
    // and udp headers
    // TODO How to do get the valid length of IP and ETHERNET header?
    IIpv4RoutingTable *rt = getModuleFromPar<IIpv4RoutingTable>(par("routingTableModule"), this);
    const NetworkInterface *rtie = rt->getInterfaceForDestAddr(_destinationAddress);

    if (rtie == nullptr)
        throw cRuntimeError("No interface for remote address %s found!", _destinationAddress.str().c_str());

    int pmtu = rtie->getMtu();
    return pmtu - 20 - 8;
}

void Rtp::createProfile(const char *profileName)
{
    cModuleType *moduleType = cModuleType::find(profileName);
    if (moduleType == nullptr)
        throw cRuntimeError("Profile type `%s' not found", profileName);

    RtpProfile *profile = check_and_cast<RtpProfile *>(moduleType->create("Profile", this));
    profile->finalizeParameters();

    profile->setGateSize("payloadReceiverOut", 30);
    profile->setGateSize("payloadReceiverIn", 30);

    this->gate("profileOut")->connectTo(profile->gate("rtpIn"));
    profile->gate("rtpOut")->connectTo(this->gate("profileIn"));

    profile->callInitialize();
    profile->scheduleStart(simTime());
}

void Rtp::createSocket()
{
    _udpSocket.bind(_port);
    MulticastGroupList mgl = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)->collectMulticastGroups();
    _udpSocket.joinLocalMulticastGroups(mgl); // TODO make it parameter-dependent
    connectRet();
}

void Rtp::initializeProfile()
{
    RtpInnerPacket *rinp = new RtpInnerPacket("initializeProfile()");
    rinp->setInitializeProfilePkt(_mtu);
    send(rinp, "profileOut");
}

void Rtp::initializeRTCP()
{
    RtpInnerPacket *rinp = new RtpInnerPacket("initializeRTCP()");
    int rtcpPort = _port + 1;
    rinp->setInitializeRTCPPkt(_commonName.c_str(), _mtu, _bandwidth, _rtcpPercentage, _destinationAddress, rtcpPort);
    send(rinp, "rtcpOut");
}

} // namespace rtp

} // namespace inet

