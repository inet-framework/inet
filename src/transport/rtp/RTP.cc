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

/** \file RTP.cc
 * This file contains the implementation of member functions of the class RTP.
 */

#include "IPAddress.h"
#include "UDPSocket.h"
#include "UDPControlInfo_m.h"

#include "RTP.h"
#include "RTPInterfacePacket.h"
#include "RTPInnerPacket.h"
#include "RTPProfile.h"

#include "RTPSenderControlMessage_m.h"
#include "RTPSenderStatusMessage_m.h"

Define_Module(RTP);


//
// methods inherited from cSimpleModule
//

void RTP::initialize()
{
    _socketFdIn = -1;//UDPSocket::generateSocketId();
    _socketFdOut = -1;
    _leaveSession = false;
}


void RTP::handleMessage(cMessage *msg)
{
    if (msg->getArrivalGateId() == findGate("appIn")) {
        handleMessageFromApp(msg);
    }
    else if (msg->getArrivalGateId() == findGate("profileIn")) {
        handleMessageFromProfile(msg);
    }
    else if (msg->getArrivalGateId() == findGate("rtcpIn")) {
        handleMessageFromRTCP(msg);
    }
    else if (msg->getArrivalGateId() == findGate("udpIn")) {
        handleMessagefromUDP(msg);
    }
    else {
        error("Message from unknown gate");
    }
}


//
// handle messages from different gates
//

void RTP::handleMessageFromApp(cMessage *msg)
{
    RTPInterfacePacket *rifp = check_and_cast<RTPInterfacePacket *>(msg);
    if (rifp->getType() == RTPInterfacePacket::RTP_IFP_ENTER_SESSION) {
        enterSession(rifp);
    }
    else if (rifp->getType() == RTPInterfacePacket::RTP_IFP_CREATE_SENDER_MODULE) {
        createSenderModule(rifp);
    }
    else if (rifp->getType() == RTPInterfacePacket::RTP_IFP_DELETE_SENDER_MODULE) {
        deleteSenderModule(rifp);
    }
    else if (rifp->getType() == RTPInterfacePacket::RTP_IFP_SENDER_CONTROL) {
        senderModuleControl(rifp);
    }
    else if (rifp->getType() == RTPInterfacePacket::RTP_IFP_LEAVE_SESSION) {
        leaveSession(rifp);
    }
    else {
        error("unknown RTPInterfacePacket type from application");
    }
}


void RTP::handleMessageFromProfile(cMessage *msg)
{
    RTPInnerPacket *rinp = check_and_cast<RTPInnerPacket *>(msg);
    if (rinp->getType() == RTPInnerPacket::RTP_INP_PROFILE_INITIALIZED) {
        profileInitialized(rinp);
    }
    else if (rinp->getType() == RTPInnerPacket::RTP_INP_SENDER_MODULE_CREATED) {
        senderModuleCreated(rinp);
    }
    else if (rinp->getType() == RTPInnerPacket::RTP_INP_SENDER_MODULE_DELETED) {
        senderModuleDeleted(rinp);
    }
    else if (rinp->getType() == RTPInnerPacket::RTP_INP_SENDER_MODULE_INITIALIZED) {
        senderModuleInitialized(rinp);
    }
    else if (rinp->getType() == RTPInnerPacket::RTP_INP_SENDER_MODULE_STATUS) {
        senderModuleStatus(rinp);
    }
    else if (rinp->getType() == RTPInnerPacket::RTP_INP_DATA_OUT) {
        dataOut(rinp);
    }
    else {
        delete msg;
    }
    ev << "handleMessageFromProfile(cMessage *msg) Exit"<<endl;
}


void RTP::handleMessageFromRTCP(cMessage *msg)
{
    RTPInnerPacket *rinp = check_and_cast<RTPInnerPacket *>(msg);
    if (rinp->getType() == RTPInnerPacket::RTP_INP_RTCP_INITIALIZED) {
        rtcpInitialized(rinp);
    }
    else if (rinp->getType() == RTPInnerPacket::RTP_INP_SESSION_LEFT) {
        sessionLeft(rinp);
    }
    else {
        error("Unknown RTPInnerPacket type %d from rtcp", rinp->getType());
    }
}

void RTP::handleMessagefromUDP(cMessage *msg)
{
    readRet(msg);
}


//
// methods for different messages
//

void RTP::enterSession(RTPInterfacePacket *rifp)
{
    _profileName = rifp->getProfileName();
    _commonName = rifp->getCommonName();
    _bandwidth = rifp->getBandwidth();
    _destinationAddress = rifp->getDestinationAddress();

    _port = rifp->getPort();
    if (_port % 2 != 0) {
        _port = _port - 1;
    }

    _mtu = resolveMTU();

    createProfile();
    initializeProfile();
    delete rifp;
}


void RTP::leaveSession(RTPInterfacePacket *rifp)
{
    cModule *profileModule = gate("profileOut")->getNextGate()->getOwnerModule();
    profileModule->deleteModule();
    _leaveSession = true;
    RTPInnerPacket *rinp = new RTPInnerPacket("leaveSession()");
    rinp->leaveSession();
    send(rinp,"rtcpOut");

    delete rifp;
}


void RTP::createSenderModule(RTPInterfacePacket *rifp)
{
    RTPInnerPacket *rinp = new RTPInnerPacket("createSenderModule()");
    ev << rifp->getSSRC()<<endl;
    rinp->createSenderModule(rifp->getSSRC(), rifp->getPayloadType(), rifp->getFileName());
    send(rinp, "profileOut");

    delete rifp;
}


void RTP::deleteSenderModule(RTPInterfacePacket *rifp)
{
    RTPInnerPacket *rinp = new RTPInnerPacket("deleteSenderModule()");
    rinp->deleteSenderModule(rifp->getSSRC());
    send(rinp, "profileOut");

    delete rifp;
}


void RTP::senderModuleControl(RTPInterfacePacket *rifp)
{
    RTPInnerPacket *rinp = new RTPInnerPacket("senderModuleControl()");
    rinp->senderModuleControl(rinp->getSSRC(), (RTPSenderControlMessage *)(rifp->decapsulate()));
    send(rinp, "profileOut");

    delete rifp;
}


void RTP::profileInitialized(RTPInnerPacket *rinp)
{
    _rtcpPercentage = rinp->getRtcpPercentage();
    if (_port == PORT_UNDEF) {
        _port = rinp->getPort();
        if (_port % 2 != 0) {
            _port = _port - 1;
        }
    }

    delete rinp;

    createSocket();
}


void RTP::senderModuleCreated(RTPInnerPacket *rinp)
{
    RTPInterfacePacket *rifp = new RTPInterfacePacket("senderModuleCreated()");
    rifp->senderModuleCreated(rinp->getSSRC());
    send(rifp, "appOut");

    delete rinp;
}


void RTP::senderModuleDeleted(RTPInnerPacket *rinp)
{
    RTPInterfacePacket *rifp = new RTPInterfacePacket("senderModuleDeleted()");
    rifp->senderModuleDeleted(rinp->getSSRC());
    send(rifp, "appOut");

    // perhaps we should send a message to rtcp module
    delete rinp;
}


void RTP::senderModuleInitialized(RTPInnerPacket *rinp)
{
    send(rinp, "rtcpOut");
}


void RTP::senderModuleStatus(RTPInnerPacket *rinp)
{
    RTPInterfacePacket *rifp = new RTPInterfacePacket("senderModuleStatus()");
    rifp->senderModuleStatus(rinp->getSSRC(), (RTPSenderStatusMessage *)(rinp->decapsulate()));
    send(rifp, "appOut");

    delete rinp;
}


void RTP::dataOut(RTPInnerPacket *rinp)
{
    RTPPacket *msg = check_and_cast<RTPPacket *>(rinp->decapsulate());

    // send message to UDP, with the appropriate control info attached
    msg->setKind(UDP_C_DATA);

    UDPControlInfo *ctrl = new UDPControlInfo();
    ctrl->setDestAddr(_destinationAddress);
    ctrl->setDestPort(_port);
    msg->setControlInfo(ctrl);

//     ev << "Sending packet: ";msg->dump();
    send(msg, "udpOut");

    // RTCP module must be informed about sent rtp data packet

    RTPInnerPacket *rinpOut = new RTPInnerPacket(*rinp);
    rinpOut->encapsulate(new RTPPacket(*msg));
    send(rinpOut, "rtcpOut");

    delete rinp;
}


void RTP::rtcpInitialized(RTPInnerPacket *rinp)
{
    RTPInterfacePacket *rifp = new RTPInterfacePacket("sessionEntered()");
    rifp->sessionEntered(rinp->getSSRC());
    send(rifp, "appOut");

    delete rinp;
}


void RTP::sessionLeft(RTPInnerPacket *rinp)
{
    RTPInterfacePacket *rifp = new RTPInterfacePacket("sessionLeft()");
    rifp->sessionLeft();
    send(rifp, "appOut");

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
    if ( ! _leaveSession)
    {
         RTPPacket *msg = check_and_cast<RTPPacket *>(sifp);

         msg->dump();
         RTPInnerPacket *rinp1 = new RTPInnerPacket("dataIn1()");
         rinp1->dataIn(new RTPPacket(*msg), IPAddress(_destinationAddress), _port);

         RTPInnerPacket *rinp2 = new RTPInnerPacket(*rinp1);
         send(rinp2, "rtcpOut");
         //delete rinp2;
         send(rinp1, "profileOut");
         //delete rinp1;
    }

    delete sifp;
}


int RTP::resolveMTU()
{
    // this is not what it should be
    // do something like mtu path discovery
    // for the simulation we can use this example value
    // it's 1500 bytes (ethernet) minus ip
    // and udp headers
    return 1500 - 20 - 8;
}


void RTP::createProfile()
{
    cModuleType *moduleType = cModuleType::find(_profileName);
    if (moduleType == NULL)
        error("Profile type `%s' not found", _profileName);

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
    // TODO UDPAppBase should be ported to use UDPSocket sometime, but for now
    // we just manage the UDP socket by hand...
    if (_socketFdIn == -1) {
        _socketFdIn = UDPSocket::generateSocketId();

        UDPControlInfo *ctrl = new UDPControlInfo();

        IPAddress ipaddr(_destinationAddress);

        if (ipaddr.isMulticast()) {
            ctrl->setSrcAddr(IPAddress(_destinationAddress));
            ctrl->setSrcPort(_port);
        }
        else {
             ctrl->setSrcPort(_port);
             ctrl->setSockId(_socketFdOut);
        }
        ctrl->setSockId((int)_socketFdIn);
        cMessage *msg = new cMessage("UDP_C_BIND", UDP_C_BIND);
        msg->setControlInfo(ctrl);
        send(msg,"udpOut");

        connectRet();
    }
}

void RTP::initializeProfile()
{
    RTPInnerPacket *rinp = new RTPInnerPacket("initializeProfile()");
    rinp->initializeProfile(_mtu);
    send(rinp, "profileOut");
}


void RTP::initializeRTCP()
{
    RTPInnerPacket *rinp = new RTPInnerPacket("initializeRTCP()");
    int rtcpPort = _port + 1;
    rinp->initializeRTCP(opp_strdup(_commonName), _mtu, _bandwidth, _rtcpPercentage, _destinationAddress, rtcpPort);
    send(rinp, "rtcpOut");
}


