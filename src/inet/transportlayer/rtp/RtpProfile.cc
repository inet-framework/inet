/***************************************************************************
                          RtpProfile.cc  -  description
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

#include "inet/transportlayer/rtp/RtpInnerPacket_m.h"
#include "inet/transportlayer/rtp/RtpPayloadReceiver.h"
#include "inet/transportlayer/rtp/RtpPayloadSender.h"
#include "inet/transportlayer/rtp/RtpProfile.h"

namespace inet {
namespace rtp {

Define_Module(RtpProfile);

RtpProfile::RtpProfile()
{
}

void RtpProfile::initialize()
{
    EV_TRACE << "initialize() Enter" << endl;
    _profileName = "Profile";
    _rtcpPercentage = 5;
    _preferredPort = PORT_UNDEF;

    // how many gates to payload receivers do we have
    _maxReceivers = gateSize("payloadReceiverOut");
    _ssrcGates.clear();
    _autoOutputFileNames = par("autoOutputFileNames");
    EV_TRACE << "initialize() Exit" << endl;
}

RtpProfile::~RtpProfile()
{
    for (auto & elem : _ssrcGates)
        delete elem.second;
}

void RtpProfile::handleMessage(cMessage *msg)
{
    if (msg->getArrivalGateId() == findGate("rtpIn")) {
        handleMessageFromRTP(msg);
    }
    else if (msg->getArrivalGateId() == findGate("payloadSenderIn")) {
        handleMessageFromPayloadSender(msg);
    }
    else if (msg->getArrivalGateId() >= findGate("payloadReceiverIn")
             && msg->getArrivalGateId() < findGate("payloadReceiverIn") + _maxReceivers)
    {
        handleMessageFromPayloadReceiver(msg);
    }
    else {
        throw cRuntimeError("message coming from unknown gate");
    }
}

void RtpProfile::handleMessageFromRTP(cMessage *msg)
{
    EV_TRACE << "handleMessageFromRTP Enter " << endl;

    RtpInnerPacket *rinpIn = check_and_cast<RtpInnerPacket *>(msg);

    switch (rinpIn->getType()) {
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
            throw cRuntimeError("RtpInnerPacket from RtpModule has wrong type: %d", rinpIn->getType());
            break;
    }

    EV_TRACE << "handleMessageFromRTP Exit " << endl;
}

void RtpProfile::handleMessageFromPayloadSender(cMessage *msg)
{
    RtpInnerPacket *rinpIn = check_and_cast<RtpInnerPacket *>(msg);

    switch (rinpIn->getType()) {
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
            throw cRuntimeError("Profile received RtpInnerPacket from sender module with wrong type: %d", rinpIn->getType());
            break;
    }
}

void RtpProfile::handleMessageFromPayloadReceiver(cMessage *msg)
{
    // currently payload receiver modules don't send messages
    delete msg;
}

void RtpProfile::initializeProfile(RtpInnerPacket *rinp)
{
    EV_TRACE << "initializeProfile Enter" << endl;
    _mtu = rinp->getMtu();
    delete rinp;
    RtpInnerPacket *rinpOut = new RtpInnerPacket("profileInitialized()");
    rinpOut->setProfileInitializedPkt(_rtcpPercentage, _preferredPort);
    send(rinpOut, "rtpOut");
    EV_TRACE << "initializeProfile Exit" << endl;
}

void RtpProfile::createSenderModule(RtpInnerPacket *rinp)
{
    EV_TRACE << "createSenderModule Enter" << endl;
    int ssrc = rinp->getSsrc();
    int payloadType = rinp->getPayloadType();
    char moduleName[100];

    EV_INFO << "ProfileName: " << _profileName << " payloadType: " << payloadType << endl;
    const char *pkgPrefix = "inet.transportlayer.rtp.";    //FIXME hardcoded string
    sprintf(moduleName, "%sRtp%sPayload%iSender", pkgPrefix, _profileName, payloadType);

    cModuleType *moduleType = cModuleType::find(moduleName);
    if (moduleType == nullptr)
        throw cRuntimeError("RtpProfile: payload sender module '%s' not found", moduleName);

    RtpPayloadSender *rtpPayloadSender = check_and_cast<RtpPayloadSender *>(moduleType->create(moduleName, this));
    rtpPayloadSender->finalizeParameters();

    gate("payloadSenderOut")->connectTo(rtpPayloadSender->gate("profileIn"));
    rtpPayloadSender->gate("profileOut")->connectTo(gate("payloadSenderIn"));

    rtpPayloadSender->callInitialize();
    rtpPayloadSender->scheduleStart(simTime());

    RtpInnerPacket *rinpOut1 = new RtpInnerPacket("senderModuleCreated()");
    rinpOut1->setSenderModuleCreatedPkt(ssrc);
    send(rinpOut1, "rtpOut");

    RtpInnerPacket *rinpOut2 = new RtpInnerPacket("initializeSenderModule()");
    rinpOut2->setInitializeSenderModulePkt(ssrc, rinp->getFileName(), _mtu);
    send(rinpOut2, "payloadSenderOut");

    delete rinp;
    EV_TRACE << "createSenderModule Exit" << endl;
}

void RtpProfile::deleteSenderModule(RtpInnerPacket *rinpIn)
{
    cModule *senderModule = gate("payloadSenderOut")->getNextGate()->getOwnerModule();
    senderModule->deleteModule();

    RtpInnerPacket *rinpOut = new RtpInnerPacket("senderModuleDeleted()");
    rinpOut->setSenderModuleDeletedPkt(rinpIn->getSsrc());
    delete rinpIn;

    send(rinpOut, "rtpOut");
}

void RtpProfile::senderModuleControl(RtpInnerPacket *rinp)
{
    send(rinp, "payloadSenderOut");
}

void RtpProfile::dataIn(RtpInnerPacket *rinp)
{
    EV_TRACE << "dataIn(RtpInnerPacket *rinp) Enter" << endl;
    processIncomingPacket(rinp);

    Packet *packet = check_and_cast<Packet *>(rinp->getEncapsulatedPacket());
    const auto& rtpHeader = packet->peekAtFront<RtpHeader>();

    uint32 ssrc = rtpHeader->getSsrc();

    SsrcGate *ssrcGate = findSSRCGate(ssrc);

    if (!ssrcGate) {
        ssrcGate = newSSRCGate(ssrc);
        char payloadReceiverName[100];
        const char *pkgPrefix = "inet.transportlayer.rtp.";    //FIXME hardcoded string
        sprintf(payloadReceiverName, "%sRtp%sPayload%iReceiver",
                pkgPrefix, _profileName, rtpHeader->getPayloadType());

        cModuleType *moduleType = cModuleType::find(payloadReceiverName);
        if (moduleType == nullptr)
            throw cRuntimeError("Receiver module type %s not found", payloadReceiverName);
        else {
            RtpPayloadReceiver *receiverModule =
                check_and_cast<RtpPayloadReceiver *>(moduleType->create(payloadReceiverName, this));
            if (_autoOutputFileNames) {
                char outputFileName[100];
                sprintf(outputFileName, "id%i.sim", receiverModule->getId());
                receiverModule->par("outputFileName") = outputFileName;
            }
            receiverModule->finalizeParameters();

            this->gate(ssrcGate->getGateId())->connectTo(receiverModule->gate("profileIn"));
            receiverModule->gate("profileOut")->connectTo(this->gate(ssrcGate->getGateId()
                            - findGate("payloadReceiverOut", 0) + findGate("payloadReceiverIn", 0)));

            for (int i = 0; receiverModule->callInitialize(i); i++)
                ;

            receiverModule->scheduleStart(simTime());
        }
    }

    send(rinp, ssrcGate->getGateId());
    EV_TRACE << "dataIn(RtpInnerPacket *rinp) Exit" << endl;
}

void RtpProfile::dataOut(RtpInnerPacket *rinp)
{
    processOutgoingPacket(rinp);
    send(rinp, "rtpOut");
}

void RtpProfile::senderModuleInitialized(RtpInnerPacket *rinp)
{
    EV_TRACE << "senderModuleInitialized" << endl;
    send(rinp, "rtpOut");
}

void RtpProfile::senderModuleStatus(RtpInnerPacket *rinp)
{
    EV_TRACE << "senderModuleStatus" << endl;
    send(rinp, "rtpOut");
}

void RtpProfile::processIncomingPacket(RtpInnerPacket *rinp)
{
    // do nothing with the packet
}

void RtpProfile::processOutgoingPacket(RtpInnerPacket *rinp)
{
    // do nothing with the packet
}

RtpProfile::SsrcGate *RtpProfile::findSSRCGate(uint32 ssrc)
{
    auto objectIndex = _ssrcGates.find(ssrc);
    if (objectIndex == _ssrcGates.end())
        return nullptr;
    return objectIndex->second;
}

RtpProfile::SsrcGate *RtpProfile::newSSRCGate(uint32 ssrc)
{
    SsrcGate *ssrcGate = new SsrcGate(ssrc);
    bool assigned = false;
    int receiverGateId = findGate("payloadReceiverOut", 0);
    for (int i = receiverGateId; i < receiverGateId + _maxReceivers && !assigned; i++) {
        if (!gate(i)->isConnected()) {
            ssrcGate->setGateId(i);
            assigned = true;
        }
    }

    if (!assigned)
        throw cRuntimeError("Can't manage more senders");

    _ssrcGates[ssrc] = ssrcGate;
    return ssrcGate;
}

} // namespace rtp
} // namespace inet

