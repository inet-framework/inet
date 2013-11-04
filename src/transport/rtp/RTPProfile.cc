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


#include <string.h>

#include "RTPProfile.h"

#include "RTPInnerPacket.h"
#include "RTPPayloadReceiver.h"
#include "RTPPayloadSender.h"


Define_Module(RTPProfile);

RTPProfile::RTPProfile()
{
}

void RTPProfile::initialize()
{
    EV << "initialize() Enter"<<endl;
    _profileName = "Profile";
    _rtcpPercentage = 5;
    _preferredPort = PORT_UNDEF;

    // how many gates to payload receivers do we have
    _maxReceivers = gateSize("payloadReceiverOut");
    _ssrcGates.clear();
    _autoOutputFileNames = par("autoOutputFileNames").boolValue();
    EV << "initialize() Exit"<<endl;
}

RTPProfile::~RTPProfile()
{
    SSRCGateMap::iterator i;
    for (i = _ssrcGates.begin(); i != _ssrcGates.end(); i++)
        delete i->second;
}

void RTPProfile::handleMessage(cMessage *msg)
{
    if (msg->getArrivalGateId() == findGate("rtpIn"))
    {
        handleMessageFromRTP(msg);
    }
    else if (msg->getArrivalGateId() == findGate("payloadSenderIn"))
    {
        handleMessageFromPayloadSender(msg);
    }
    else if (msg->getArrivalGateId() >= findGate("payloadReceiverIn")
            && msg->getArrivalGateId() < findGate("payloadReceiverIn") + _maxReceivers)
    {
        handleMessageFromPayloadReceiver(msg);
    }
    else
    {
        error("message coming from unknown gate");
    }
}

void RTPProfile::handleMessageFromRTP(cMessage *msg)
{
    EV << "handleMessageFromRTP Enter "<<endl;

    RTPInnerPacket *rinpIn = check_and_cast<RTPInnerPacket *>(msg);

    switch (rinpIn->getType())
    {
    case RTP_INP_INITIALIZE_PROFILE:
        initializeProfile(rinpIn);
        break;

    case RTP_INP_CREATE_SENDER_MODULE:
        createSenderModule(rinpIn);
        break;

    case RTP_INP_DELETE_SENDER_MODULE:
        deleteSenderModule(rinpIn);
        break;

    case RTP_INP_SENDER_MODULE_CONTROL:
        senderModuleControl(rinpIn);
        break;

    case RTP_INP_DATA_IN:
        dataIn(rinpIn);
        break;

   default:
        error("RTPInnerPacket from RTPModule has wrong type: %d", rinpIn->getType());
        break;
    }

    EV << "handleMessageFromRTP Exit "<<endl;
}


void RTPProfile::handleMessageFromPayloadSender(cMessage *msg)
{
    RTPInnerPacket *rinpIn = check_and_cast<RTPInnerPacket *>(msg);

    switch (rinpIn->getType())
    {
    case RTP_INP_DATA_OUT:
        dataOut(rinpIn);
        break;

    case RTP_INP_SENDER_MODULE_INITIALIZED:
        senderModuleInitialized(rinpIn);
        break;

    case RTP_INP_SENDER_MODULE_STATUS:
        senderModuleStatus(rinpIn);
        break;

    default:
        error("Profile received RTPInnerPacket from sender module with wrong type: %d", rinpIn->getType());
        break;
    }
}

void RTPProfile::handleMessageFromPayloadReceiver(cMessage *msg)
{
    // currently payload receiver modules don't send messages
    delete msg;
}

void RTPProfile::initializeProfile(RTPInnerPacket *rinp)
{
    EV << "initializeProfile Enter"<<endl;
    _mtu = rinp->getMTU();
    delete rinp;
    RTPInnerPacket *rinpOut = new RTPInnerPacket("profileInitialized()");
    rinpOut->setProfileInitializedPkt(_rtcpPercentage, _preferredPort);
    sendSync(rinpOut, "rtpOut");
    EV << "initializeProfile Exit"<<endl;
}

void RTPProfile::createSenderModule(RTPInnerPacket *rinp)
{
    EV << "createSenderModule Enter"<<endl;
    int ssrc = rinp->getSsrc();
    int payloadType = rinp->getPayloadType();
    char moduleName[100];

    EV << "ProfileName: " << _profileName << " payloadType: " << payloadType<<endl;
    const char *pkgPrefix = "inet.transport.rtp."; //FIXME hardcoded string
    sprintf(moduleName, "%sRTP%sPayload%iSender", pkgPrefix, _profileName, payloadType);

    cModuleType *moduleType = cModuleType::find(moduleName);
    if (moduleType == NULL)
        throw cRuntimeError("RTPProfile: payload sender module '%s' not found", moduleName);

    RTPPayloadSender *rtpPayloadSender = (RTPPayloadSender *)(moduleType->create(moduleName, this));
    rtpPayloadSender->finalizeParameters();

    gate("payloadSenderOut")->connectTo(rtpPayloadSender->gate("profileIn"));
    rtpPayloadSender->gate("profileOut")->connectTo(gate("payloadSenderIn"));

    rtpPayloadSender->callInitialize();
    rtpPayloadSender->scheduleStart(simTime());

    RTPInnerPacket *rinpOut1 = new RTPInnerPacket("senderModuleCreated()");
    rinpOut1->setSenderModuleCreatedPkt(ssrc);
    sendSync(rinpOut1, "rtpOut");

    RTPInnerPacket *rinpOut2 = new RTPInnerPacket("initializeSenderModule()");
    rinpOut2->setInitializeSenderModulePkt(ssrc, rinp->getFileName(), _mtu);
    sendSync(rinpOut2, "payloadSenderOut");

    delete rinp;
    EV << "createSenderModule Exit"<<endl;
}

void RTPProfile::deleteSenderModule(RTPInnerPacket *rinpIn)
{
    cModule *senderModule = gate("payloadSenderOut")->getNextGate()->getOwnerModule();
    senderModule->deleteModule();

    RTPInnerPacket *rinpOut = new RTPInnerPacket("senderModuleDeleted()");
    rinpOut->setSenderModuleDeletedPkt(rinpIn->getSsrc());
    delete rinpIn;

    sendSync(rinpOut, "rtpOut");
}

void RTPProfile::senderModuleControl(RTPInnerPacket *rinp)
{
    sendSync(rinp, "payloadSenderOut");
}


void RTPProfile::dataIn(RTPInnerPacket *rinp)
{
    EV << "dataIn(RTPInnerPacket *rinp) Enter"<<endl;
    processIncomingPacket(rinp);

    RTPPacket *packet = check_and_cast<RTPPacket *>(rinp->getEncapsulatedPacket());

    uint32 ssrc = packet->getSsrc();

    SSRCGate *ssrcGate = findSSRCGate(ssrc);

    if (!ssrcGate)
    {
        ssrcGate = newSSRCGate(ssrc);
        char payloadReceiverName[100];
        const char *pkgPrefix = "inet.transport.rtp."; //FIXME hardcoded string
        sprintf(payloadReceiverName, "%sRTP%sPayload%iReceiver",
                pkgPrefix, _profileName, packet->getPayloadType());

        cModuleType *moduleType = cModuleType::find(payloadReceiverName);
        if (moduleType == NULL)
            throw cRuntimeError("Receiver module type %s not found", payloadReceiverName);
        else
        {
            RTPPayloadReceiver *receiverModule =
                    (RTPPayloadReceiver *)(moduleType->create(payloadReceiverName, this));
            if (_autoOutputFileNames)
            {
                char outputFileName[100];
                sprintf(outputFileName, "id%i.sim", receiverModule->getId());
                receiverModule->par("outputFileName") = outputFileName;
            }
            receiverModule->finalizeParameters();

            this->gate(ssrcGate->getGateId())->connectTo(receiverModule->gate("profileIn"));
            receiverModule->gate("profileOut")->connectTo(this->gate(ssrcGate->getGateId() -
                    findGate("payloadReceiverOut", 0) + findGate("payloadReceiverIn", 0)));

            for (int i=0; receiverModule->callInitialize(i); i++)
                ;

            receiverModule->scheduleStart(simTime());
        }
    }

    sendSync(rinp, ssrcGate->getGateId());
    EV << "dataIn(RTPInnerPacket *rinp) Exit"<<endl;
}

void RTPProfile::dataOut(RTPInnerPacket *rinp)
{
    processOutgoingPacket(rinp);
    sendSync(rinp, "rtpOut");
}

void RTPProfile::senderModuleInitialized(RTPInnerPacket *rinp)
{
    EV << "senderModuleInitialized"<<endl;
    sendSync(rinp, "rtpOut");
}

void RTPProfile::senderModuleStatus(RTPInnerPacket *rinp)
{
    EV << "senderModuleStatus"<<endl;
    sendSync(rinp, "rtpOut");
}

void RTPProfile::processIncomingPacket(RTPInnerPacket *rinp)
{
    // do nothing with the packet
}

void RTPProfile::processOutgoingPacket(RTPInnerPacket *rinp)
{
    // do nothing with the packet
}

RTPProfile::SSRCGate* RTPProfile::findSSRCGate(uint32 ssrc)
{
    SSRCGateMap::iterator objectIndex = _ssrcGates.find(ssrc);
    if (objectIndex == _ssrcGates.end())
        return NULL;
    return objectIndex->second;
}

RTPProfile::SSRCGate *RTPProfile::newSSRCGate(uint32 ssrc)
{
    SSRCGate *ssrcGate = new SSRCGate(ssrc);
    bool assigned = false;
    int receiverGateId = findGate("payloadReceiverOut", 0);
    for (int i = receiverGateId; i < receiverGateId + _maxReceivers && !assigned; i++)
    {
        if (!gate(i)->isConnected())
        {
            ssrcGate->setGateId(i);
            assigned = true;
        }
    }

    if (!assigned)
        throw cRuntimeError("Can't manage more senders");

    _ssrcGates[ssrc] = ssrcGate;
    return ssrcGate;
}
