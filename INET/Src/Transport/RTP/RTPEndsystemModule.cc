/***************************************************************************
                          RTPEndsystemModule.cc  -  description
                             -------------------
    begin                : Fri Oct 19 2001
    copyright            : (C) 2001 by Matthias Oppitz
    email                : Matthias.Oppitz@gmx.de
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

//XXX #include "SocketInterfacePacket.h"
//XXX #include "in_port.h"
#include "tmp/defs.h"

#include "IPAddress.h"

#include "RTPEndsystemModule.h"
#include "RTPInterfacePacket.h"
#include "RTPInnerPacket.h"
#include "RTPProfile.h"

#include "RTPSenderControlMessage.h"
#include "RTPSenderStatusMessage.h"

Define_Module_Like(RTPEndsystemModule, RTPModule);


//
// methods inherited from cSimpleModule
//

void RTPEndsystemModule::initialize() {

    _socketFdIn = Socket::FILEDESC_UNDEF;
    _socketFdOut = Socket::FILEDESC_UNDEF;
};


void RTPEndsystemModule::handleMessage(cMessage *msg) {

    if (msg->arrivalGateId() == findGate("fromApp")) {
        handleMessageFromApp(msg);
    }

    else if (msg->arrivalGateId() == findGate("fromProfile")) {
        handleMessageFromProfile(msg);
    }

    else if (msg->arrivalGateId() == findGate("fromRTCP")) {
        handleMessageFromRTCP(msg);
    }

    else if (msg->arrivalGateId() == findGate("fromSocketLayer")) {
        handleMessageFromSocketLayer(msg);
    }

    else {
        EV << "RTPEndsystemModule: Message from unknown Gate !" << endl;
    }
};


//
// handle messages from different gates
//

void RTPEndsystemModule::handleMessageFromApp(cMessage *msg) {
    RTPInterfacePacket *rifp = (RTPInterfacePacket *)msg;
    if (rifp->type() == RTPInterfacePacket::RTP_IFP_ENTER_SESSION) {
        enterSession(rifp);
    }
    else if (rifp->type() == RTPInterfacePacket::RTP_IFP_CREATE_SENDER_MODULE) {
        createSenderModule(rifp);
    }
    else if (rifp->type() == RTPInterfacePacket::RTP_IFP_DELETE_SENDER_MODULE) {
        deleteSenderModule(rifp);
    }
    else if (rifp->type() == RTPInterfacePacket::RTP_IFP_SENDER_CONTROL) {
        senderModuleControl(rifp);
    }
    else if (rifp->type() == RTPInterfacePacket::RTP_IFP_LEAVE_SESSION) {
        leaveSession(rifp);
    }
    else {
        EV << "RTPEndsystemModule: unknown RTPInterfacePacket type from application !" << endl;
    }
};


void RTPEndsystemModule::handleMessageFromProfile(cMessage *msg) {
    RTPInnerPacket *rinp = (RTPInnerPacket *)msg;
    if (rinp->type() == RTPInnerPacket::RTP_INP_PROFILE_INITIALIZED) {
        profileInitialized(rinp);
    }
    else if (rinp->type() == RTPInnerPacket::RTP_INP_SENDER_MODULE_CREATED) {
        senderModuleCreated(rinp);
    }
    else if (rinp->type() == RTPInnerPacket::RTP_INP_SENDER_MODULE_DELETED) {
        senderModuleDeleted(rinp);
    }
    else if (rinp->type() == RTPInnerPacket::RTP_INP_SENDER_MODULE_INITIALIZED) {
        senderModuleInitialized(rinp);
    }
    else if (rinp->type() == RTPInnerPacket::RTP_INP_SENDER_MODULE_STATUS) {
        senderModuleStatus(rinp);
    }
    else if (rinp->type() == RTPInnerPacket::RTP_INP_DATA_OUT) {
        dataOut(rinp);
    }
    else {
        EV << "RTPEndsystemModule: Unknown RTPInnerPacket type from profile !" << endl;
    }
}


void RTPEndsystemModule::handleMessageFromRTCP(cMessage *msg) {
    RTPInnerPacket *rinp = (RTPInnerPacket *)msg;
    if (rinp->type() == RTPInnerPacket::RTP_INP_RTCP_INITIALIZED) {
        rtcpInitialized(rinp);
    }
    else if (rinp->type() == RTPInnerPacket::RTP_INP_SESSION_LEFT) {
        sessionLeft(rinp);
    }
    else {
        EV << "RTPEndsystemModule: Unknown RTPInnerPacket type " << rinp->type() << " from rtcp !" << endl;
    }
};


void RTPEndsystemModule::handleMessageFromSocketLayer(cMessage *msg) {
    SocketInterfacePacket *sifpIn = (SocketInterfacePacket *)msg;
    if (sifpIn->action() == SocketInterfacePacket::SA_SOCKET_RET) {
        socketRet(sifpIn);
    }
    else if (sifpIn->action() == SocketInterfacePacket::SA_CONNECT_RET) {
        connectRet(sifpIn);
    }
    else if (sifpIn->action() == SocketInterfacePacket::SA_READ_RET) {
        readRet(sifpIn);
    }
    else {
        EV << "RTPEndsystemModule: Unknown SocketInterfacePacket type " << sifpIn->action() << " !" << endl;
    }
};


//
// methods for different messages
//

void RTPEndsystemModule::enterSession(RTPInterfacePacket *rifp) {
    _profileName = rifp->profileName();
    _commonName = rifp->commonName();
    _bandwidth = rifp->bandwidth();
    _destinationAddress = rifp->destinationAddress();
    _port = rifp->port();
    if (_port % 2 != 0) {
        _port = _port - 1;
    }

    _mtu = resolveMTU();

    createProfile();
    initializeProfile();

    delete rifp;
};


void RTPEndsystemModule::leaveSession(RTPInterfacePacket *rifp) {
    cModule *profileModule = gate("toProfile")->toGate()->ownerModule();
    profileModule->deleteModule();
    RTPInnerPacket *rinp = new RTPInnerPacket("leaveSession()");
    rinp->leaveSession();
    send(rinp,"toRTCP");

    delete rifp;
};


void RTPEndsystemModule::createSenderModule(RTPInterfacePacket *rifp) {
    RTPInnerPacket *rinp = new RTPInnerPacket("createSenderModule()");
    rinp->createSenderModule(rifp->ssrc(), rifp->payloadType(), rifp->fileName());
    send(rinp, "toProfile");

    delete rifp;
};


void RTPEndsystemModule::deleteSenderModule(RTPInterfacePacket *rifp) {
    RTPInnerPacket *rinp = new RTPInnerPacket("deleteSenderModule()");
    rinp->deleteSenderModule(rifp->ssrc());
    send(rinp, "toProfile");

    delete rifp;
};


void RTPEndsystemModule::senderModuleControl(RTPInterfacePacket *rifp) {
    RTPInnerPacket *rinp = new RTPInnerPacket("senderModuleControl()");
    rinp->senderModuleControl(rinp->ssrc(), (RTPSenderControlMessage *)(rifp->decapsulate()));
    send(rinp, "toProfile");

    delete rifp;
}


void RTPEndsystemModule::profileInitialized(RTPInnerPacket *rinp) {
    _rtcpPercentage = rinp->rtcpPercentage();
    if (_port == IPSuite_PORT_UNDEF) {
        _port = rinp->port();
        if (_port % 2 != 0) {
            _port = _port - 1;
        }
    }

    delete rinp;

    createServerSocket();
};


void RTPEndsystemModule::senderModuleCreated(RTPInnerPacket *rinp) {
    RTPInterfacePacket *rifp = new RTPInterfacePacket("senderModuleCreated()");
    rifp->senderModuleCreated(rinp->ssrc());
    send(rifp, "toApp");

    delete rinp;
};


void RTPEndsystemModule::senderModuleDeleted(RTPInnerPacket *rinp) {
    RTPInterfacePacket *rifp = new RTPInterfacePacket("senderModuleDeleted()");
    rifp->senderModuleDeleted(rinp->ssrc());
    send(rifp, "toApp");

    // perhaps we should send a message to rtcp module
    delete rinp;
};


void RTPEndsystemModule::senderModuleInitialized(RTPInnerPacket *rinp) {
    send(rinp, "toRTCP");
};


void RTPEndsystemModule::senderModuleStatus(RTPInnerPacket *rinp) {
    RTPInterfacePacket *rifp = new RTPInterfacePacket("senderModuleStatus()");
    rifp->senderModuleStatus(rinp->ssrc(), (RTPSenderStatusMessage *)(rinp->decapsulate()));
    send(rifp, "toApp");

    delete rinp;
};


void RTPEndsystemModule::dataOut(RTPInnerPacket *rinp) {

    RTPPacket *encapsulatedMsg = (RTPPacket *)(rinp->decapsulate());
    SocketInterfacePacket *sifpOut = new SocketInterfacePacket("write()");
    sifpOut->write(_socketFdOut, encapsulatedMsg);
    send(sifpOut, "toSocketLayer");

    // RTCP module must be informed about sent rtp data packet

    RTPInnerPacket *rinpOut = new RTPInnerPacket(*rinp);
    rinpOut->encapsulate(new RTPPacket(*encapsulatedMsg));
    send(rinpOut, "toRTCP");

    delete rinp;
};


void RTPEndsystemModule::rtcpInitialized(RTPInnerPacket *rinp) {
    RTPInterfacePacket *rifp = new RTPInterfacePacket("sessionEntered()");
    rifp->sessionEntered(rinp->ssrc());
    send(rifp, "toApp");

    delete rinp;
};


void RTPEndsystemModule::sessionLeft(RTPInnerPacket *rinp) {

    RTPInterfacePacket *rifp = new RTPInterfacePacket("sessionLeft()");
    rifp->sessionLeft();
    send(rifp, "toApp");

    delete rinp;
};


void RTPEndsystemModule::socketRet(SocketInterfacePacket *sifp) {
    if (_socketFdIn == Socket::FILEDESC_UNDEF) {
        _socketFdIn = sifp->filedesc();

        SocketInterfacePacket *sifpOut = new SocketInterfacePacket("bind()");

        IPAddress ipaddr(_destinationAddress);

        if (ipaddr.isMulticast()) {
            sifpOut->bind(_socketFdIn, IN_Addr(_destinationAddress), IN_Port(_port));
        }
        else {
            sifpOut->bind(_socketFdIn, IPADDRESS_UNDEF, IN_Port(_port));
        }
        send(sifpOut, "toSocketLayer");

        createClientSocket();
    }
    else if (_socketFdOut == Socket::FILEDESC_UNDEF) {
        _socketFdOut = sifp->filedesc();

        SocketInterfacePacket *sifpOut = new SocketInterfacePacket("connect()");
        sifpOut->connect(_socketFdOut, _destinationAddress, _port);
        send(sifpOut, "toSocketLayer");

    }
    else {
        EV << "RTPEndsystemModule: SA_SOCKET_RET from socket layer but no socket requested !" << endl;
    }

    delete sifp;
};


void RTPEndsystemModule::connectRet(SocketInterfacePacket *sifp) {
    initializeRTCP();

    delete sifp;
};


void RTPEndsystemModule::readRet(SocketInterfacePacket *sifp) {

    cMessage *msg = sifp->decapsulate();

    RTPPacket *packet1 = (RTPPacket *)msg;
    RTPInnerPacket *rinp1 = new RTPInnerPacket("dataIn1()");
    rinp1->dataIn(packet1, sifp->fAddr(), sifp->fPort());

    //RTPPacket *packet2 = new RTPPacket(*packet1);

    RTPInnerPacket *rinp2 = new RTPInnerPacket(*rinp1);

    //rinp2->dataIn(packet2, IN_Addr(sifp->fAddr()), IN_Port(sifp->fPort()));

    send(rinp2, "toRTCP");
    send(rinp1, "toProfile");

    delete sifp;
};


int RTPEndsystemModule::resolveMTU() {
    // this is not what it should be
    // do something like mtu path discovery
    // for the simulation we can use this example value
    // it's 1500 bytes (ethernet) minus ip
    // and udp headers
    return 1500 - 20 - 8;
};


void RTPEndsystemModule::createProfile() {
    cModuleType *moduleType = findModuleType(_profileName);
    if (moduleType == NULL) {
        error("Profile type not found !");
    };
    RTPProfile *profile = (RTPProfile *)(moduleType->create("Profile", this));

    profile->setGateSize("toPayloadReceiver", 4);
    profile->setGateSize("fromPayloadReceiver", 4);

    connect(this, findGate("toProfile"), NULL, profile, profile->findGate("fromRTP"));
    connect(profile, profile->findGate("toRTP"), NULL, this, findGate("fromProfile"));
    profile->initialize();
    profile->scheduleStart(simTime());
};


void RTPEndsystemModule::createServerSocket() {
    SocketInterfacePacket *sifp = new SocketInterfacePacket("socket()");
    sifp->socket(Socket::IPSuite_AF_INET, Socket::IPSuite_SOCK_DGRAM, Socket::UDP);
    send(sifp, "toSocketLayer");
};


void RTPEndsystemModule::createClientSocket() {
    SocketInterfacePacket *sifp = new SocketInterfacePacket("socket()");
    sifp->socket(Socket::IPSuite_AF_INET, Socket::IPSuite_SOCK_DGRAM, Socket::UDP);
    send(sifp, "toSocketLayer");
};


void RTPEndsystemModule::initializeProfile() {
    RTPInnerPacket *rinp = new RTPInnerPacket("initializeProfile()");
    rinp->initializeProfile(_mtu);
    send(rinp, "toProfile");
};


void RTPEndsystemModule::initializeRTCP() {
    RTPInnerPacket *rinp = new RTPInnerPacket("initializeRTCP()");
    IN_Port rtcpPort = _port + 1;
    rinp->initializeRTCP(opp_strdup(_commonName), _mtu, _bandwidth, _rtcpPercentage, _destinationAddress, rtcpPort);
    send(rinp, "toRTCP");
};
