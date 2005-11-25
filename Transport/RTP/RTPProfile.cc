/***************************************************************************
                          RTPProfile.cc  -  description
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

void RTPProfile::initialize() {

    _profileName = "Profile";
    _rtcpPercentage = 5;
    _preferredPort = IPSuite_PORT_UNDEF;

    // how many gates to payload receivers do we have
    _maxReceivers = gate("toPayloadReceiver")->size();
    _ssrcGates = new cArray("SSRCGates");
    _autoOutputFileNames = par("autoOutputFileNames").boolValue();
}


void RTPProfile::handleMessage(cMessage *msg) {

    if (msg->arrivalGateId() == findGate("fromRTP")) {
        handleMessageFromRTP(msg);
    }

    else if (msg->arrivalGateId() == findGate("fromPayloadSender")) {
        handleMessageFromPayloadSender(msg);
    }

    else if (msg->arrivalGateId() >= findGate("fromPayloadReceiver") && msg->arrivalGateId() < findGate("fromPayloadReceiver") + _maxReceivers) {
        handleMessageFromPayloadReceiver(msg);
    }

    else {
        // this shouldn't happen
        EV << "RTPProfile: message coming from unknown gate" << endl;
    }

};


void RTPProfile::handleMessageFromRTP(cMessage *msg) {

    if (opp_strcmp(msg->className(), "RTPInnerPacket"))
        error("RTPProfile: message received from RTPModule is not an RTPInnerPacket !");

        RTPInnerPacket *rinpIn = (RTPInnerPacket *)msg;

        if (rinpIn->type() == RTPInnerPacket::RTP_INP_INITIALIZE_PROFILE) {
            initializeProfile(rinpIn);
        }

        else if (rinpIn->type() == RTPInnerPacket::RTP_INP_CREATE_SENDER_MODULE) {
            createSenderModule(rinpIn);
        }

        else if (rinpIn->type() == RTPInnerPacket::RTP_INP_DELETE_SENDER_MODULE) {
            deleteSenderModule(rinpIn);
        }

        else if (rinpIn->type() == RTPInnerPacket::RTP_INP_SENDER_MODULE_CONTROL) {
            senderModuleControl(rinpIn);
        }

        else if (rinpIn->type() == RTPInnerPacket::RTP_INP_DATA_IN) {
            dataIn(rinpIn);
        }

        else {
            EV << "RTPProfile: RTPInnerPacket from RTPModule has wrong type !"<< endl;
        }

}


void RTPProfile::handleMessageFromPayloadSender(cMessage *msg) {

    RTPInnerPacket *rinpIn = (RTPInnerPacket *)msg;

    if (rinpIn->type() == RTPInnerPacket::RTP_INP_DATA_OUT) {
        dataOut(rinpIn);
    }

    else if (rinpIn->type() == RTPInnerPacket::RTP_INP_SENDER_MODULE_INITIALIZED) {
        senderModuleInitialized(rinpIn);
    }

    else if (rinpIn->type() == RTPInnerPacket::RTP_INP_SENDER_MODULE_STATUS) {
        senderModuleStatus(rinpIn);
    }

    else {
        EV << "Profile received RTPInnerPacket from sender module with wrong type!" << endl;
    }

}


void RTPProfile::handleMessageFromPayloadReceiver(cMessage *msg) {
    // currently payload receiver modules don't send messages
    delete msg;
}


void RTPProfile::initializeProfile(RTPInnerPacket *rinp) {
    _mtu = rinp->mtu();
    delete rinp;
    RTPInnerPacket *rinpOut = new RTPInnerPacket("profileInitialized()");
    rinpOut->profileInitialized(_rtcpPercentage, _preferredPort);
    send(rinpOut, "toRTP");
}


void RTPProfile::createSenderModule(RTPInnerPacket *rinp) {

    int ssrc = rinp->ssrc();
    int payloadType = rinp->payloadType();
    char moduleName[100];
    sprintf(moduleName, "RTP%sPayload%iSender", _profileName, payloadType);

    cModuleType *moduleType = findModuleType(moduleName);

    if (moduleType == NULL) {
        opp_error("RTPProfile: payload sender module \"", moduleName, "\" not found !");
    };
    RTPPayloadSender *rtpPayloadSender = (RTPPayloadSender *)(moduleType->create(moduleName, this));

    connect(this, findGate("toPayloadSender"), NULL, rtpPayloadSender, rtpPayloadSender->findGate("fromProfile"));
    connect(rtpPayloadSender, rtpPayloadSender->findGate("toProfile"), NULL, this, findGate("fromPayloadSender"));

    rtpPayloadSender->initialize();
    rtpPayloadSender->scheduleStart(simTime());

    RTPInnerPacket *rinpOut1 = new RTPInnerPacket("senderModuleCreated()");
    rinpOut1->senderModuleCreated(ssrc);
    send(rinpOut1, "toRTP");

    RTPInnerPacket *rinpOut2 = new RTPInnerPacket("initializeSenderModule()");
    rinpOut2->initializeSenderModule(ssrc, rinp->fileName(), _mtu);
    send(rinpOut2, "toPayloadSender");

    delete rinp;
};


void RTPProfile::deleteSenderModule(RTPInnerPacket *rinpIn) {
    cModule *senderModule = gate("toPayloadSender")->toGate()->ownerModule();
    senderModule->deleteModule();

    RTPInnerPacket *rinpOut = new RTPInnerPacket("senderModuleDeleted()");
    rinpOut->senderModuleDeleted(rinpIn->ssrc());
    delete rinpIn;

    send(rinpOut, "toRTP");
};


void RTPProfile::senderModuleControl(RTPInnerPacket *rinp) {
    send(rinp, "toPayloadSender");
};


void RTPProfile::dataIn(RTPInnerPacket *rinp) {

    processIncomingPacket(rinp);

    RTPPacket *packet = (RTPPacket *)(rinp->encapsulatedMsg());

    u_int32 ssrc = packet->ssrc();

    RTPSSRCGate *ssrcGate = findSSRCGate(ssrc);

    if (!ssrcGate) {
        ssrcGate = newSSRCGate(ssrc);
        char payloadReceiverName[100];
        sprintf(payloadReceiverName, "RTP%sPayload%iReceiver", _profileName, packet->payloadType());

        cModuleType *moduleType = findModuleType(payloadReceiverName);
        if (moduleType == NULL) {
            opp_error("Receiver module %s not found !", payloadReceiverName);
        }
        else {
            RTPPayloadReceiver *receiverModule = (RTPPayloadReceiver *)(moduleType->create(payloadReceiverName, this));
            if (_autoOutputFileNames) {
                char outputFileName[100];
                sprintf(outputFileName, "id%i.sim", receiverModule->id());
                receiverModule->par("outputFileName") = outputFileName;
            }
            connect(this, ssrcGate->gateId(), NULL, receiverModule, receiverModule->findGate("fromProfile"));
            connect(receiverModule, receiverModule->findGate("toProfile"), NULL, this,  ssrcGate->gateId() - findGate("toPayloadReceiver") + findGate("fromPayloadReceiver"));
            receiverModule->callInitialize(0);
            receiverModule->scheduleStart(simTime());
        }
    };

    send(rinp, ssrcGate->gateId());
};


void RTPProfile::dataOut(RTPInnerPacket *rinp) {
    processOutgoingPacket(rinp);
    send(rinp, "toRTP");
};


void RTPProfile::senderModuleInitialized(RTPInnerPacket *rinp) {
    send(rinp, "toRTP");
};


void RTPProfile::senderModuleStatus(RTPInnerPacket *rinp) {
    send(rinp, "toRTP");
};


void RTPProfile::processIncomingPacket(RTPInnerPacket *rinp) {
    // do nothing with the packet
};


void RTPProfile::processOutgoingPacket(RTPInnerPacket *rinp) {
    // do nothing with the packet
};


RTPSSRCGate *RTPProfile::findSSRCGate(u_int32 ssrc) {
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
