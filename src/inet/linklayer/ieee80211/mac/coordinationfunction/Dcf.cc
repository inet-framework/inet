//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Dcf.h"
#include "inet/linklayer/ieee80211/mac/framesequence/DcfFs.h"
#include "inet/linklayer/ieee80211/mac/rateselection/RateSelection.h"
#include "inet/linklayer/ieee80211/mac/recipient/RecipientAckProcedure.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

Define_Module(Dcf);

void Dcf::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LINK_LAYER) {
        startRxTimer = new cMessage("startRxTimeout");
        mac = check_and_cast<Ieee80211Mac *>(getContainingNicModule(this)->getSubmodule("mac"));
        dataAndMgmtRateControl = dynamic_cast<IRateControl *>(getSubmodule(("rateControl")));
        tx = check_and_cast<ITx *>(getModuleByPath(par("txModule")));
        rx = check_and_cast<IRx *>(getModuleByPath(par("rxModule")));
        channelAccess = check_and_cast<Dcaf *>(getSubmodule("channelAccess"));
        originatorDataService = check_and_cast<IOriginatorMacDataService *>(getSubmodule(("originatorMacDataService")));
        recipientDataService = check_and_cast<IRecipientMacDataService*>(getSubmodule("recipientMacDataService"));
        recoveryProcedure = check_and_cast<NonQosRecoveryProcedure *>(getSubmodule("recoveryProcedure"));
        rateSelection = check_and_cast<IRateSelection*>(getSubmodule("rateSelection"));
        rtsProcedure = new RtsProcedure();
        rtsPolicy = check_and_cast<IRtsPolicy *>(getSubmodule("rtsPolicy"));
        recipientAckProcedure = new RecipientAckProcedure();
        recipientAckPolicy = check_and_cast<IRecipientAckPolicy *>(getSubmodule("recipientAckPolicy"));
        originatorAckPolicy = check_and_cast<IOriginatorAckPolicy *>(getSubmodule("originatorAckPolicy"));
        frameSequenceHandler = new FrameSequenceHandler();
        ackHandler = check_and_cast<AckHandler *>(getSubmodule("ackHandler"));
        ctsProcedure = new CtsProcedure();
        ctsPolicy = check_and_cast<ICtsPolicy *>(getSubmodule("ctsPolicy"));
        stationRetryCounters = new StationRetryCounters();
        originatorProtectionMechanism = check_and_cast<OriginatorProtectionMechanism*>(getSubmodule("originatorProtectionMechanism"));
    }
}

void Dcf::forEachChild(cVisitor *v)
{
    cSimpleModule::forEachChild(v);
    if (frameSequenceHandler != nullptr && frameSequenceHandler->getContext() != nullptr)
        v->visit(const_cast<FrameSequenceContext *>(frameSequenceHandler->getContext()));
}

void Dcf::handleMessage(cMessage* msg)
{
    if (msg == startRxTimer) {
        if (!isReceptionInProgress()) {
            frameSequenceHandler->handleStartRxTimeout();
            updateDisplayString();
        }
    }
    else
        throw cRuntimeError("Unknown msg type");
}

void Dcf::updateDisplayString()
{
    if (frameSequenceHandler->isSequenceRunning()) {
        auto history = frameSequenceHandler->getFrameSequence()->getHistory();
        getDisplayString().setTagArg("t", 0, ("Fs: " + history).c_str());
    }
    else
        getDisplayString().removeTag("t");
}

void Dcf::channelGranted(IChannelAccess *channelAccess)
{
    Enter_Method_Silent("channelGranted");
    ASSERT(this->channelAccess == channelAccess);
    if (!frameSequenceHandler->isSequenceRunning()) {
        frameSequenceHandler->startFrameSequence(new DcfFs(), buildContext(), this);
        emit(IFrameSequenceHandler::frameSequenceStartedSignal, frameSequenceHandler->getContext());
        updateDisplayString();
    }
}

void Dcf::processUpperFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    Enter_Method_Silent("processUpperFrame(%s)", packet->getName());
    EV_INFO << "Processing upper frame: " << packet->getName() << endl;
    auto pendingQueue = channelAccess->getPendingQueue();
    pendingQueue->pushPacket(packet);
    if (!pendingQueue->isEmpty()) {
        EV_DETAIL << "Requesting channel" << endl;
        channelAccess->requestChannel(this);
    }
}

void Dcf::transmitControlResponseFrame(Packet *responsePacket, const Ptr<const Ieee80211MacHeader>& responseHeader, Packet *receivedPacket, const Ptr<const Ieee80211MacHeader>& receivedHeader)
{
    Enter_Method_Silent("transmitControlResponseFrame");
    responsePacket->insertAtBack(makeShared<Ieee80211MacTrailer>());
    const IIeee80211Mode *responseMode = nullptr;
    if (auto rtsFrame = dynamicPtrCast<const Ieee80211RtsFrame>(receivedHeader))
        responseMode = rateSelection->computeResponseCtsFrameMode(receivedPacket, rtsFrame);
    else if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(receivedHeader))
        responseMode = rateSelection->computeResponseAckFrameMode(receivedPacket, dataOrMgmtHeader);
    else
        throw cRuntimeError("Unknown received frame type");
    RateSelection::setFrameMode(responsePacket, responseHeader, responseMode);
    emit(IRateSelection::datarateSelectedSignal, responseMode->getDataMode()->getNetBitrate().get(), responsePacket);
    EV_DEBUG << "Datarate for " << responsePacket->getName() << " is set to " << responseMode->getDataMode()->getNetBitrate() << ".\n";
    tx->transmitFrame(responsePacket, responseHeader, modeSet->getSifsTime(), this);
    delete responsePacket;
}

void Dcf::processMgmtFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& mgmtHeader)
{
    throw cRuntimeError("Unknown management frame");
}

void Dcf::recipientProcessTransmittedControlResponseFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    emit(packetSentToPeerSignal, packet);
    if (auto ctsFrame = dynamicPtrCast<const Ieee80211CtsFrame>(header))
        ctsProcedure->processTransmittedCts(ctsFrame);
    else if (auto ackFrame = dynamicPtrCast<const Ieee80211AckFrame>(header))
        recipientAckProcedure->processTransmittedAck(ackFrame);
    else
        throw cRuntimeError("Unknown control response frame");
}

void Dcf::scheduleStartRxTimer(simtime_t timeout)
{
    Enter_Method_Silent();
    scheduleAfter(timeout, startRxTimer);
}

void Dcf::processLowerFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    Enter_Method_Silent("processLowerFrame(%s)", packet->getName());
    EV_INFO << "Processing lower frame: " << packet->getName() << endl;
    if (frameSequenceHandler->isSequenceRunning()) {
        // TODO: always call processResponses
        if ((!isForUs(header) && !startRxTimer->isScheduled()) || isForUs(header)) {
            frameSequenceHandler->processResponse(packet);
            updateDisplayString();
        }
        else {
            EV_INFO << "This frame is not for us" << std::endl;
            PacketDropDetails details;
            details.setReason(NOT_ADDRESSED_TO_US);
            emit(packetDroppedSignal, packet, &details);
            delete packet;
        }
        cancelEvent(startRxTimer);
    }
    else if (isForUs(header))
        recipientProcessReceivedFrame(packet, header);
    else {
        EV_INFO << "This frame is not for us" << std::endl;
        PacketDropDetails details;
        details.setReason(NOT_ADDRESSED_TO_US);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
}

void Dcf::transmitFrame(Packet *packet, simtime_t ifs)
{
    Enter_Method_Silent("transmitFrame");
    const auto& header = packet->peekAtFront<Ieee80211MacHeader>();
    auto mode = rateSelection->computeMode(packet, header);
    RateSelection::setFrameMode(packet, header, mode);
    emit(IRateSelection::datarateSelectedSignal, mode->getDataMode()->getNetBitrate().get(), packet);
    EV_DEBUG << "Datarate for " << packet->getName() << " is set to " << mode->getDataMode()->getNetBitrate() << ".\n";
    auto pendingPacket = channelAccess->getInProgressFrames()->getPendingFrameFor(packet);
    auto duration = originatorProtectionMechanism->computeDurationField(packet, header, pendingPacket, pendingPacket == nullptr ? nullptr : pendingPacket->peekAtFront<Ieee80211DataOrMgmtHeader>());
    const auto& updatedHeader = packet->removeAtFront<Ieee80211MacHeader>();
    updatedHeader->setDurationField(duration);
    EV_DEBUG << "Duration for " << packet->getName() << " is set to " << duration << " s.\n";
    packet->insertAtFront(updatedHeader);
    tx->transmitFrame(packet, packet->peekAtFront<Ieee80211MacHeader>(), ifs, this);
}

/*
 * TODO:  If a PHY-RXSTART.indication primitive does not occur during the ACKTimeout interval,
 * the STA concludes that the transmission of the MPDU has failed, and this STA shall invoke its
 * backoff procedure **upon expiration of the ACKTimeout interval**.
 */

void Dcf::frameSequenceFinished()
{
    Enter_Method_Silent("frameSequenceFinished");
    emit(IFrameSequenceHandler::frameSequenceFinishedSignal, frameSequenceHandler->getContext());
    channelAccess->releaseChannel(this);
    if (hasFrameToTransmit())
        channelAccess->requestChannel(this);
    mac->sendDownPendingRadioConfigMsg(); // TODO: review
}

bool Dcf::isReceptionInProgress()
{
    return rx->isReceptionInProgress();
}

void Dcf::recipientProcessReceivedFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    EV_INFO << "Processing received frame " << packet->getName() << " as recipient.\n";
    emit(packetReceivedFromPeerSignal, packet);
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header))
        recipientAckProcedure->processReceivedFrame(packet, dataOrMgmtHeader, recipientAckPolicy, this);
    if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header))
        sendUp(recipientDataService->dataFrameReceived(packet, dataHeader));
    else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(header))
        sendUp(recipientDataService->managementFrameReceived(packet, mgmtHeader));
    else { // TODO: else if (auto ctrlFrame = dynamic_cast<Ieee80211ControlFrame*>(frame))
        sendUp(recipientDataService->controlFrameReceived(packet, header));
        recipientProcessReceivedControlFrame(packet, header);
        delete packet;
    }
}

void Dcf::sendUp(const std::vector<Packet *>& completeFrames)
{
    for (auto frame : completeFrames)
        mac->sendUpFrame(frame);
}

void Dcf::recipientProcessReceivedControlFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    if (auto rtsFrame = dynamicPtrCast<const Ieee80211RtsFrame>(header))
        ctsProcedure->processReceivedRts(packet, rtsFrame, ctsPolicy, this);
    else
        throw cRuntimeError("Unknown control frame");
}

FrameSequenceContext* Dcf::buildContext()
{
    auto nonQoSContext = new NonQoSContext(originatorAckPolicy);
    return new FrameSequenceContext(mac->getAddress(), modeSet, channelAccess->getInProgressFrames(), rtsProcedure, rtsPolicy, nonQoSContext, nullptr);
}

void Dcf::transmissionComplete(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    Enter_Method_Silent("transmissionComplete");
    if (frameSequenceHandler->isSequenceRunning()) {
        frameSequenceHandler->transmissionComplete();
        updateDisplayString();
    }
    else
        recipientProcessTransmittedControlResponseFrame(packet, header);
    delete packet;
}

bool Dcf::hasFrameToTransmit()
{
    return !channelAccess->getPendingQueue()->isEmpty() || channelAccess->getInProgressFrames()->hasInProgressFrames();
}

void Dcf::originatorProcessRtsProtectionFailed(Packet *packet)
{
    Enter_Method_Silent("originatorProcessRtsProtectionFailed");
    EV_INFO << "RTS frame transmission failed\n";
    auto protectedHeader = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
    recoveryProcedure->rtsFrameTransmissionFailed(protectedHeader, stationRetryCounters);
    EV_INFO << "For the current frame exchange, we have CW = " << channelAccess->getCw() << " SRC = " << recoveryProcedure->getShortRetryCount(packet, protectedHeader) << " LRC = " << recoveryProcedure->getLongRetryCount(packet, protectedHeader) << " SSRC = " << stationRetryCounters->getStationShortRetryCount() << " and SLRC = " << stationRetryCounters->getStationLongRetryCount() << std::endl;
    if (recoveryProcedure->isRtsFrameRetryLimitReached(packet, protectedHeader)) {
        recoveryProcedure->retryLimitReached(packet, protectedHeader);
        channelAccess->getInProgressFrames()->dropFrame(packet);
        ackHandler->dropFrame(protectedHeader);
        EV_INFO << "Dropping RTS/CTS protected frame " << packet->getName() << ", because retry limit is reached.\n";
        PacketDropDetails details;
        details.setReason(RETRY_LIMIT_REACHED);
        details.setLimit(recoveryProcedure->getShortRetryLimit());
        emit(packetDroppedSignal, packet, &details);
        emit(linkBrokenSignal, packet);
    }
}

void Dcf::originatorProcessTransmittedFrame(Packet *packet)
{
    Enter_Method_Silent("originatorProcessTransmittedFrame");
    EV_INFO << "Processing transmitted frame " << packet->getName() << " as originator in frame sequence.\n";
    emit(packetSentToPeerSignal, packet);
    auto transmittedHeader = packet->peekAtFront<Ieee80211MacHeader>();
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(transmittedHeader)) {
        EV_INFO << "For the current frame exchange, we have CW = " << channelAccess->getCw() << " SRC = " << recoveryProcedure->getShortRetryCount(packet, dataOrMgmtHeader) << " LRC = " << recoveryProcedure->getLongRetryCount(packet, dataOrMgmtHeader) << " SSRC = " << stationRetryCounters->getStationShortRetryCount() << " and SLRC = " << stationRetryCounters->getStationLongRetryCount() << std::endl;
        if (originatorAckPolicy->isAckNeeded(dataOrMgmtHeader)) {
            ackHandler->processTransmittedDataOrMgmtFrame(dataOrMgmtHeader);
        }
        else if (dataOrMgmtHeader->getReceiverAddress().isMulticast()) {
            recoveryProcedure->multicastFrameTransmitted(stationRetryCounters);
            channelAccess->getInProgressFrames()->dropFrame(packet);
        }
    }
    else if (auto rtsFrame = dynamicPtrCast<const Ieee80211RtsFrame>(transmittedHeader)) {
        auto protectedFrame = channelAccess->getInProgressFrames()->getFrameToTransmit(); // KLUDGE:
        auto protectedHeader = protectedFrame->peekAtFront<Ieee80211DataOrMgmtHeader>();
        EV_INFO << "For the current frame exchange, we have CW = " << channelAccess->getCw() << " SRC = " << recoveryProcedure->getShortRetryCount(protectedFrame, protectedHeader) << " LRC = " << recoveryProcedure->getLongRetryCount(protectedFrame, protectedHeader) << " SSRC = " << stationRetryCounters->getStationShortRetryCount() << " and SLRC = " << stationRetryCounters->getStationLongRetryCount() << std::endl;
        rtsProcedure->processTransmittedRts(rtsFrame);
    }
}

void Dcf::originatorProcessReceivedFrame(Packet *receivedPacket, Packet *lastTransmittedPacket)
{
    Enter_Method_Silent("originatorProcessReceivedFrame");
    EV_INFO << "Processing received frame " << receivedPacket->getName() << " as originator in frame sequence.\n";
    emit(packetReceivedFromPeerSignal, receivedPacket);
    auto receivedHeader = receivedPacket->peekAtFront<Ieee80211MacHeader>();
    auto lastTransmittedHeader = lastTransmittedPacket->peekAtFront<Ieee80211MacHeader>();
    if (receivedHeader->getType() == ST_ACK) {
        auto lastTransmittedDataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(lastTransmittedHeader);
        if (dataAndMgmtRateControl) {
            int retryCount = lastTransmittedHeader->getRetry() ? recoveryProcedure->getRetryCount(lastTransmittedPacket, lastTransmittedDataOrMgmtHeader) : 0;
            dataAndMgmtRateControl->frameTransmitted(lastTransmittedPacket, retryCount, true, false);
        }
        recoveryProcedure->ackFrameReceived(lastTransmittedPacket, lastTransmittedDataOrMgmtHeader, stationRetryCounters);
        ackHandler->processReceivedAck(dynamicPtrCast<const Ieee80211AckFrame>(receivedHeader), lastTransmittedDataOrMgmtHeader);
        channelAccess->getInProgressFrames()->dropFrame(lastTransmittedPacket);
        ackHandler->dropFrame(lastTransmittedDataOrMgmtHeader);
    }
    else if (receivedHeader->getType() == ST_RTS)
        ; // void
    else if (receivedHeader->getType() == ST_CTS)
        recoveryProcedure->ctsFrameReceived(stationRetryCounters);
    else
        throw cRuntimeError("Unknown frame type");
}

void Dcf::originatorProcessFailedFrame(Packet *failedPacket)
{
    Enter_Method_Silent("originatorProcessFailedFrame");
    EV_INFO << "Data/Mgmt frame transmission failed\n";
    const auto& failedHeader = failedPacket->peekAtFront<Ieee80211DataOrMgmtHeader>();
    ASSERT(failedHeader->getType() != ST_DATA_WITH_QOS);
    ASSERT(ackHandler->getAckStatus(failedHeader) == AckHandler::Status::WAITING_FOR_ACK || ackHandler->getAckStatus(failedHeader) == AckHandler::Status::NO_ACK_REQUIRED);
    recoveryProcedure->dataOrMgmtFrameTransmissionFailed(failedPacket, failedHeader, stationRetryCounters);
    bool retryLimitReached = recoveryProcedure->isRetryLimitReached(failedPacket, failedHeader);
    if (dataAndMgmtRateControl) {
        int retryCount = recoveryProcedure->getRetryCount(failedPacket, failedHeader);
        dataAndMgmtRateControl->frameTransmitted(failedPacket, retryCount, false, retryLimitReached);
    }
    ackHandler->processFailedFrame(failedHeader);
    if (retryLimitReached) {
        recoveryProcedure->retryLimitReached(failedPacket, failedHeader);
        channelAccess->getInProgressFrames()->dropFrame(failedPacket);
        ackHandler->dropFrame(failedHeader);
        EV_INFO << "Dropping frame " << failedPacket->getName() << ", because retry limit is reached.\n";
        PacketDropDetails details;
        details.setReason(RETRY_LIMIT_REACHED);
        details.setLimit(-1); // TODO:
        emit(packetDroppedSignal, failedPacket, &details);
        emit(linkBrokenSignal, failedPacket);
    }
    else {
        EV_INFO << "Retrying frame " << failedPacket->getName() << ".\n";
        auto h = failedPacket->removeAtFront<Ieee80211DataOrMgmtHeader>();
        h->setRetry(true);
        failedPacket->insertAtFront(h);
    }
}

bool Dcf::isForUs(const Ptr<const Ieee80211MacHeader>& header) const
{
    return header->getReceiverAddress() == mac->getAddress() || (header->getReceiverAddress().isMulticast() && !isSentByUs(header));
}

bool Dcf::isSentByUs(const Ptr<const Ieee80211MacHeader>& header) const
{
    // FIXME:
    // Check the roles of the Addr3 field when aggregation is applied
    // Table 8-19—Address field contents
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header))
        return dataOrMgmtHeader->getAddress3() == mac->getAddress();
    else
        return false;
}

void Dcf::corruptedFrameReceived()
{
    Enter_Method_Silent("corruptedFrameReceived");
    if (frameSequenceHandler->isSequenceRunning() && !startRxTimer->isScheduled()) {
        frameSequenceHandler->handleStartRxTimeout();
        updateDisplayString();
    }
    else
        EV_DEBUG << "Ignoring received corrupt frame.\n";
}

Dcf::~Dcf()
{
    cancelAndDelete(startRxTimer);
    delete rtsProcedure;
    delete recipientAckProcedure;
    delete stationRetryCounters;
    delete ctsProcedure;
    delete frameSequenceHandler;
}

} // namespace ieee80211
} // namespace inet

