/***************************************************************************
                          RTPEndsystemModule.cc  -  description
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

/** \file RTPEndsystemModule.cc
 * This file contains the implementation of member functions of the class RTPEndsystemModule.
 */

#include <omnetpp.h>

#include "IPAddress.h"
#include "UDPSocket.h"
#include "UDPControlInfo_m.h"

#include "RTPEndsystemModule.h"
#include "RTPInterfacePacket.h"
#include "RTPInnerPacket.h"
#include "RTPProfile.h"

#include "RTPSenderControlMessage.h"
#include "RTPSenderStatusMessage.h"

Define_Module(RTPEndsystemModule);


//
// methods inherited from cSimpleModule
//

void RTPEndsystemModule::initialize()
{
    _socketFdIn = -1;//UDPSocket::generateSocketId();
    _socketFdOut = -1;
    _leaveSession = false;
};


void RTPEndsystemModule::handleMessage(cMessage *msg)
{
    if (msg->getArrivalGateId() == findGate("fromApp")) {
        handleMessageFromApp(msg);
    }
    else if (msg->getArrivalGateId() == findGate("fromProfile")) {
        handleMessageFromProfile(msg);
    }
    else if (msg->getArrivalGateId() == findGate("fromRTCP")) {
        handleMessageFromRTCP(msg);
    }
    else if (msg->getArrivalGateId() == findGate("fromUDPLayer")) {
        handleMessagefromUDP(msg);
    }
    else {
        error("Message from unknown gate");
    }
};


//
// handle messages from different gates
//

void RTPEndsystemModule::handleMessageFromApp(cMessage *msg)
{
    RTPInterfacePacket *rifp = (RTPInterfacePacket *)msg;
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
};


void RTPEndsystemModule::handleMessageFromProfile(cMessage *msg)
{
    RTPInnerPacket *rinp = (RTPInnerPacket *)msg;
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


void RTPEndsystemModule::handleMessageFromRTCP(cMessage *msg)
{
    RTPInnerPacket *rinp = (RTPInnerPacket *)msg;
    if (rinp->getType() == RTPInnerPacket::RTP_INP_RTCP_INITIALIZED) {
        rtcpInitialized(rinp);
    }
    else if (rinp->getType() == RTPInnerPacket::RTP_INP_SESSION_LEFT) {
        sessionLeft(rinp);
    }
    else {
        error("Unknown RTPInnerPacket type %d from rtcp", rinp->getType());
    }
};

void RTPEndsystemModule::handleMessagefromUDP(cMessage *msg)
{
    readRet(msg);
};


//
// methods for different messages
//

void RTPEndsystemModule::enterSession(RTPInterfacePacket *rifp)
{
    _profileName = rifp->profileName();
    _commonName = rifp->commonName();
    _bandwidth = rifp->bandwidth();
    _destinationAddress = rifp->getDestinationAddress();

    _port = rifp->port();
    if (_port % 2 != 0) {
        _port = _port - 1;
    }

    _mtu = resolveMTU();

    createProfile();
    initializeProfile();
    delete rifp;
};


void RTPEndsystemModule::leaveSession(RTPInterfacePacket *rifp)
{
    cModule *profileModule = gate("toProfile")->getToGate()->getOwnerModule();
    profileModule->deleteModule();
    _leaveSession = true;
    RTPInnerPacket *rinp = new RTPInnerPacket("leaveSession()");
    rinp->leaveSession();
    send(rinp,"toRTCP");

    delete rifp;
};


void RTPEndsystemModule::createSenderModule(RTPInterfacePacket *rifp)
{
    RTPInnerPacket *rinp = new RTPInnerPacket("createSenderModule()");
    ev << rifp->ssrc()<<endl;
    rinp->createSenderModule(rifp->ssrc(), rifp->payloadType(), rifp->getFileName());
    send(rinp, "toProfile");

    delete rifp;
};


void RTPEndsystemModule::deleteSenderModule(RTPInterfacePacket *rifp)
{
    RTPInnerPacket *rinp = new RTPInnerPacket("deleteSenderModule()");
    rinp->deleteSenderModule(rifp->ssrc());
    send(rinp, "toProfile");

    delete rifp;
};


void RTPEndsystemModule::senderModuleControl(RTPInterfacePacket *rifp)
{
    RTPInnerPacket *rinp = new RTPInnerPacket("senderModuleControl()");
    rinp->senderModuleControl(rinp->ssrc(), (RTPSenderControlMessage *)(rifp->decapsulate()));
    send(rinp, "toProfile");

    delete rifp;
}


void RTPEndsystemModule::profileInitialized(RTPInnerPacket *rinp)
{
    _rtcpPercentage = rinp->rtcpPercentage();
    if (_port == PORT_UNDEF) {
        _port = rinp->port();
        if (_port % 2 != 0) {
            _port = _port - 1;
        }
    }

    delete rinp;

    createSocket();
};


void RTPEndsystemModule::senderModuleCreated(RTPInnerPacket *rinp)
{
    RTPInterfacePacket *rifp = new RTPInterfacePacket("senderModuleCreated()");
    rifp->senderModuleCreated(rinp->ssrc());
    send(rifp, "toApp");

    delete rinp;
};


void RTPEndsystemModule::senderModuleDeleted(RTPInnerPacket *rinp)
{
    RTPInterfacePacket *rifp = new RTPInterfacePacket("senderModuleDeleted()");
    rifp->senderModuleDeleted(rinp->ssrc());
    send(rifp, "toApp");

    // perhaps we should send a message to rtcp module
    delete rinp;
};


void RTPEndsystemModule::senderModuleInitialized(RTPInnerPacket *rinp)
{
    send(rinp, "toRTCP");
};


void RTPEndsystemModule::senderModuleStatus(RTPInnerPacket *rinp)
{
    RTPInterfacePacket *rifp = new RTPInterfacePacket("senderModuleStatus()");
    rifp->senderModuleStatus(rinp->ssrc(), (RTPSenderStatusMessage *)(rinp->decapsulate()));
    send(rifp, "toApp");

    delete rinp;
};


void RTPEndsystemModule::dataOut(RTPInnerPacket *rinp)
{
    RTPPacket *msg = (RTPPacket *)(rinp->decapsulate());

    // send message to UDP, with the appropriate control info attached
    msg->setKind(UDP_C_DATA);

    UDPControlInfo *ctrl = new UDPControlInfo();
    ctrl->setDestAddr(_destinationAddress);
    ctrl->setDestPort(_port);
    msg->setControlInfo(ctrl);

//     ev << "Sending packet: ";msg->dump();
    send(msg, "toUDPLayer");

    // RTCP module must be informed about sent rtp data packet

    RTPInnerPacket *rinpOut = new RTPInnerPacket(*rinp);
    rinpOut->encapsulate(new RTPPacket(*msg));
    send(rinpOut, "toRTCP");

    delete rinp;
};


void RTPEndsystemModule::rtcpInitialized(RTPInnerPacket *rinp)
{
    RTPInterfacePacket *rifp = new RTPInterfacePacket("sessionEntered()");
    rifp->sessionEntered(rinp->ssrc());
    send(rifp, "toApp");

    delete rinp;
};


void RTPEndsystemModule::sessionLeft(RTPInnerPacket *rinp)
{
    RTPInterfacePacket *rifp = new RTPInterfacePacket("sessionLeft()");
    rifp->sessionLeft();
    send(rifp, "toApp");

    delete rinp;
};


void RTPEndsystemModule::socketRet()
{
};


void RTPEndsystemModule::connectRet()
{
    initializeRTCP();
};


void RTPEndsystemModule::readRet(cMessage *sifp)
{
    if ( ! _leaveSession)
    {
         RTPPacket *msg = (RTPPacket *)(sifp);

         msg->dump();
         RTPInnerPacket *rinp1 = new RTPInnerPacket("dataIn1()");
         rinp1->dataIn(new RTPPacket(*msg), IPAddress(_destinationAddress), _port);

         RTPInnerPacket *rinp2 = new RTPInnerPacket(*rinp1);
         send(rinp2, "toRTCP");
         //delete rinp2;
         send(rinp1, "toProfile");
         //delete rinp1;
    }

    delete sifp;
};


int RTPEndsystemModule::resolveMTU()
{
    // this is not what it should be
    // do something like mtu path discovery
    // for the simulation we can use this example value
    // it's 1500 bytes (ethernet) minus ip
    // and udp headers
    return 1500 - 20 - 8;
};


void RTPEndsystemModule::createProfile()
{
    cModuleType *moduleType = cModuleType::find(_profileName);
    if (moduleType == NULL)
        error("Profile type `%s' not found", _profileName);

    RTPProfile *profile = check_and_cast<RTPProfile *>(moduleType->create("Profile", this));

    profile->setGateSize("toPayloadReceiver", 30);
    profile->setGateSize("fromPayloadReceiver", 30);

    this->gate("toProfile")->connectTo(profile->gate("fromRTP"));
    profile->gate("toRTP")->connectTo(this->gate("fromProfile"));

    profile->callInitialize();
    profile->scheduleStart(simTime());
};


void RTPEndsystemModule::createSocket()
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
    send(msg,"toUDPLayer");

    connectRet();
    }
};

void RTPEndsystemModule::initializeProfile()
{
    RTPInnerPacket *rinp = new RTPInnerPacket("initializeProfile()");
    rinp->initializeProfile(_mtu);
    send(rinp, "toProfile");
};


void RTPEndsystemModule::initializeRTCP()
{
    RTPInnerPacket *rinp = new RTPInnerPacket("initializeRTCP()");
    int rtcpPort = _port + 1;
    rinp->initializeRTCP(opp_strdup(_commonName), _mtu, _bandwidth, _rtcpPercentage, _destinationAddress, rtcpPort);
    send(rinp, "toRTCP");
};
