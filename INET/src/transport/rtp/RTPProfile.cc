/***************************************************************************
                          RTPProfile.cc  -  description
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

/** \file RTPProfile.cc
 * This file contains the implementaion of member functions of the class RTPProfile.
 */

#include <string.h>

#include <omnetpp.h>

#include "RTPProfile.h"
#include "RTPInnerPacket.h"
#include "RTPPayloadSender.h"
#include "RTPPayloadReceiver.h"
#include "RTPSSRCGate.h"
#include "RTPParticipantInfo.h"


Define_Module(RTPProfile);

void RTPProfile::initialize()
{
    ev << "initialize() Enter"<<endl;
    _profileName = "Profile";
    _rtcpPercentage = 5;
    _preferredPort = IPSuite_PORT_UNDEF;

    // how many gates to payload receivers do we have
    _maxReceivers = gateSize("toPayloadReceiver");
    _ssrcGates = new cArray("SSRCGates");
    _autoOutputFileNames = par("autoOutputFileNames").boolValue();
    ev << "initialize() Exit"<<endl;
}

RTPProfile::~RTPProfile()
{
    delete _ssrcGates;
}


void RTPProfile::handleMessage(cMessage *msg)
{
    if (msg->getArrivalGateId() == findGate("fromRTP")) {
        handleMessageFromRTP(msg);
    }

    else if (msg->getArrivalGateId() == findGate("fromPayloadSender")) {
        handleMessageFromPayloadSender(msg);
    }

    else if (msg->getArrivalGateId() >= findGate("fromPayloadReceiver") && msg->getArrivalGateId() < findGate("fromPayloadReceiver") + _maxReceivers) {
        handleMessageFromPayloadReceiver(msg);
    }

    else {
        // this shouldn't happen
        ev << "message coming from unknown gate" << endl;
    }

};


void RTPProfile::handleMessageFromRTP(cMessage *msg)
{
    ev << "handleMessageFromRTP Enter "<<endl;
    if (opp_strcmp(msg->getClassName(), "RTPInnerPacket"))
        error("RTPProfile: message received from RTPModule is not an RTPInnerPacket !");

        RTPInnerPacket *rinpIn = (RTPInnerPacket *)msg;

        if (rinpIn->getType() == RTPInnerPacket::RTP_INP_INITIALIZE_PROFILE) {
            initializeProfile(rinpIn);
        }

        else if (rinpIn->getType() == RTPInnerPacket::RTP_INP_CREATE_SENDER_MODULE) {
            createSenderModule(rinpIn);
        }

        else if (rinpIn->getType() == RTPInnerPacket::RTP_INP_DELETE_SENDER_MODULE) {
            deleteSenderModule(rinpIn);
        }

        else if (rinpIn->getType() == RTPInnerPacket::RTP_INP_SENDER_MODULE_CONTROL) {
            senderModuleControl(rinpIn);
        }

        else if (rinpIn->getType() == RTPInnerPacket::RTP_INP_DATA_IN) {
            dataIn(rinpIn);
        }

        else {
            ev << "RTPInnerPacket from RTPModule has wrong type !"<< endl;
            delete msg;
        }

    ev << "handleMessageFromRTP Exit "<<endl;
}


void RTPProfile::handleMessageFromPayloadSender(cMessage *msg) {

    RTPInnerPacket *rinpIn = (RTPInnerPacket *)msg;

    if (rinpIn->getType() == RTPInnerPacket::RTP_INP_DATA_OUT) {
        dataOut(rinpIn);
    }

    else if (rinpIn->getType() == RTPInnerPacket::RTP_INP_SENDER_MODULE_INITIALIZED) {
        senderModuleInitialized(rinpIn);
    }

    else if (rinpIn->getType() == RTPInnerPacket::RTP_INP_SENDER_MODULE_STATUS) {
        senderModuleStatus(rinpIn);
    }

    else {
        ev << "Profile received RTPInnerPacket from sender module with wrong type!" << endl;
    }

}


void RTPProfile::handleMessageFromPayloadReceiver(cMessage *msg) {
    // currently payload receiver modules don't send messages
    delete msg;
}


void RTPProfile::initializeProfile(RTPInnerPacket *rinp)
{
    ev << "initializeProfile Enter"<<endl;
    _mtu = rinp->mtu();
    delete rinp;
    RTPInnerPacket *rinpOut = new RTPInnerPacket("profileInitialized()");
    rinpOut->profileInitialized(_rtcpPercentage, _preferredPort);
    send(rinpOut, "toRTP");
    ev << "initializeProfile Exit"<<endl;
}


void RTPProfile::createSenderModule(RTPInnerPacket *rinp)
{
    ev << "createSenderModule Enter"<<endl;
    int ssrc = rinp->ssrc();
    int payloadType = rinp->payloadType();
    char moduleName[100];

    ev<<"ProfileName"<<_profileName<<"payloadType"<<payloadType<<endl;
    sprintf(moduleName, "RTP%sPayload%iSender", _profileName, payloadType);

    cModuleType *moduleType = cModuleType::find(moduleName);

    if (moduleType == NULL) {
        opp_error("RTPProfile: payload sender module \"", moduleName, "\" not found !");
    };
    RTPPayloadSender *rtpPayloadSender = (RTPPayloadSender *)(moduleType->create(moduleName, this));

    gate("toPayloadSender")->connectTo(rtpPayloadSender->gate("fromProfile"));
    rtpPayloadSender->gate("toProfile")->connectTo(gate("fromPayloadSender"));

    rtpPayloadSender->initialize();
    rtpPayloadSender->scheduleStart(simTime());

    RTPInnerPacket *rinpOut1 = new RTPInnerPacket("senderModuleCreated()");
    rinpOut1->senderModuleCreated(ssrc);
    send(rinpOut1, "toRTP");

    RTPInnerPacket *rinpOut2 = new RTPInnerPacket("initializeSenderModule()");
    rinpOut2->initializeSenderModule(ssrc, rinp->getFileName(), _mtu);
    send(rinpOut2, "toPayloadSender");

    delete rinp;
    ev << "createSenderModule Exit"<<endl;
};


void RTPProfile::deleteSenderModule(RTPInnerPacket *rinpIn) {
    cModule *senderModule = gate("toPayloadSender")->getToGate()->getOwnerModule();
    senderModule->deleteModule();

    RTPInnerPacket *rinpOut = new RTPInnerPacket("senderModuleDeleted()");
    rinpOut->senderModuleDeleted(rinpIn->ssrc());
    delete rinpIn;

    send(rinpOut, "toRTP");
};


void RTPProfile::senderModuleControl(RTPInnerPacket *rinp) {
    send(rinp, "toPayloadSender");
};


void RTPProfile::dataIn(RTPInnerPacket *rinp)
{
    ev << "dataIn(RTPInnerPacket *rinp) Enter"<<endl;
    processIncomingPacket(rinp);

    RTPPacket *packet = (RTPPacket *)(rinp->getEncapsulatedMsg());

    u_int32 ssrc = packet->ssrc();

    RTPSSRCGate *ssrcGate = findSSRCGate(ssrc);

    if (!ssrcGate) {
        ssrcGate = newSSRCGate(ssrc);
        char payloadReceiverName[100];
        sprintf(payloadReceiverName, "RTP%sPayload%iReceiver", _profileName, packet->payloadType());

        cModuleType *moduleType = cModuleType::find(payloadReceiverName);
        if (moduleType == NULL) {
            opp_error("Receiver module %s not found !", payloadReceiverName);
        }
        else {
            RTPPayloadReceiver *receiverModule = (RTPPayloadReceiver *)(moduleType->create(payloadReceiverName, this));
            if (_autoOutputFileNames) {
                char outputFileName[100];
                sprintf(outputFileName, "id%i.sim", receiverModule->getId());
                receiverModule->par("outputFileName") = outputFileName;
            }
            this->gate(ssrcGate->gateId())->connectTo(receiverModule->gate("fromProfile"));
            receiverModule->gate("toProfile")->connectTo(this->gate(ssrcGate->gateId() - findGate("toPayloadReceiver") + findGate("fromPayloadReceiver")));

            receiverModule->callInitialize(0);
            receiverModule->scheduleStart(simTime());
        }
    };

    send(rinp, ssrcGate->gateId());
    ev << "dataIn(RTPInnerPacket *rinp) Exit"<<endl;
};


void RTPProfile::dataOut(RTPInnerPacket *rinp) {
    processOutgoingPacket(rinp);
    send(rinp, "toRTP");
};


void RTPProfile::senderModuleInitialized(RTPInnerPacket *rinp)
{
    ev << "senderModuleInitialized"<<endl;
    send(rinp, "toRTP");
};


void RTPProfile::senderModuleStatus(RTPInnerPacket *rinp)
{
    ev << "senderModuleStatus"<<endl;
    send(rinp, "toRTP");
};


void RTPProfile::processIncomingPacket(RTPInnerPacket *rinp)
{
    // do nothing with the packet
};


void RTPProfile::processOutgoingPacket(RTPInnerPacket *rinp)
{
    // do nothing with the packet
};


RTPSSRCGate *RTPProfile::findSSRCGate(u_int32 ssrc)
{
    const char *name = RTPParticipantInfo::ssrcToName(ssrc);
    int objectIndex = _ssrcGates->find(name);
    if (objectIndex == -1) {
        return NULL;
    }
    else {
        cObject *co = (_ssrcGates->get(objectIndex));
        return (RTPSSRCGate *)co;
    };
};


RTPSSRCGate *RTPProfile::newSSRCGate(u_int32 ssrc) {
    RTPSSRCGate *ssrcGate = new RTPSSRCGate(ssrc);
    bool assigned = false;
    int receiverGateId = findGate("toPayloadReceiver");
    for (int i = receiverGateId; i < receiverGateId + _maxReceivers && !assigned; i++) {
        if (!gate(i)->isConnected()) {
            ssrcGate->setGateId(i);
            assigned = true;
        };
    };
    if (!assigned) {
        opp_error("Can't manage more senders !");
    };
    _ssrcGates->add(ssrcGate);
    return ssrcGate;
};
