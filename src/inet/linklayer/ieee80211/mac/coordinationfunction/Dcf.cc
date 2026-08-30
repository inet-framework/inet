//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/coordinationfunction/Dcf.h"

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mac/framesequence/DcfFs.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceStep.h"
#include "inet/linklayer/ieee80211/mac/rateselection/RateSelection.h"
#include "inet/linklayer/ieee80211/mac/recipient/RecipientAckProcedure.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtTransactionTag_m.h"

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
        recipientDataService = check_and_cast<IRecipientMacDataService *>(getSubmodule("recipientMacDataService"));
        recoveryProcedure = check_and_cast<NonQosRecoveryProcedure *>(getSubmodule("recoveryProcedure"));
        rateSelection = check_and_cast<IRateSelection *>(getSubmodule("rateSelection"));
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
        originatorProtectionMechanism = check_and_cast<OriginatorProtectionMechanism *>(getSubmodule("originatorProtectionMechanism"));
        WATCH_EXPR("frameSequenceInfo", frameSequenceHandler->isSequenceRunning() ? "Fs: " + frameSequenceHandler->getFrameSequence()->getHistory() : "");
    }
    else if (stage == INITSTAGE_LAST)
        // Dcaf resolves its pending queue at the link-layer stage. Install
        // this callback after all child initialization has completed.
        channelAccess->getPendingQueue()->addPacketCallback(this);
}

void Dcf::forEachChild(cVisitor *v)
{
    SimpleModule::forEachChild(v);
    if (frameSequenceHandler != nullptr && frameSequenceHandler->getContext() != nullptr)
        v->visit(const_cast<FrameSequenceContext *>(frameSequenceHandler->getContext()));
}

void Dcf::handleMessage(cMessage *msg)
{
    if (msg == startRxTimer) {
        if (!isReceptionInProgress()) {
            frameSequenceHandler->handleStartRxTimeout();
        }
    }
    else
        throw cRuntimeError("Unknown msg type");
}

void Dcf::channelGranted(IChannelAccess *channelAccess)
{
    Enter_Method("channelGranted");
    ASSERT(this->channelAccess == channelAccess);
    if (!frameSequenceHandler->isSequenceRunning()) {
        if (this->channelAccess->getInProgressFrames()->getFrameToTransmit() == nullptr) {
            EV_DETAIL << "Releasing channel because no frame is available.\n";
            channelAccess->releaseChannel(this);
            mac->sendDownPendingRadioConfigMsg();
            return;
        }
        frameSequenceHandler->startFrameSequence(new DcfFs(), buildContext(), this);
        emit(IFrameSequenceHandler::frameSequenceStartedSignal, frameSequenceHandler->getContext());
    }
}

void Dcf::processUpperFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    Enter_Method("processUpperFrame(%s)", packet->getName());
    take(packet);
    EV_INFO << "Processing upper frame: " << packet->getName() << endl;
    auto pendingQueue = channelAccess->getPendingQueue();
    pendingQueue->enqueuePacket(packet);
    if (!pendingQueue->isEmpty()) {
        EV_DETAIL << "Requesting channel" << endl;
        channelAccess->requestChannel(this);
    }
}

void Dcf::transmitControlResponseFrame(Packet *responsePacket, const Ptr<const Ieee80211MacHeader>& responseHeader, Packet *receivedPacket, const Ptr<const Ieee80211MacHeader>& receivedHeader)
{
    Enter_Method("transmitControlResponseFrame");
    responsePacket->insertAtBack(makeShared<Ieee80211MacTrailer>());
    const IIeee80211Mode *responseMode = nullptr;
    if (auto rtsFrame = dynamicPtrCast<const Ieee80211RtsFrame>(receivedHeader))
        responseMode = rateSelection->computeResponseCtsFrameMode(receivedPacket, rtsFrame);
    else if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(receivedHeader))
        responseMode = rateSelection->computeResponseAckFrameMode(receivedPacket, dataOrMgmtHeader);
    else
        throw cRuntimeError("Unknown received frame type");
    RateSelection::setFrameMode(responsePacket, responseHeader, responseMode);
    emit(IRateSelection::datarateSelectedSignal, responseMode->getDataMode()->getNetBitrate().get<bps>(), responsePacket);
    EV_DEBUG << "Datarate for " << responsePacket->getName() << " is set to " << responseMode->getDataMode()->getNetBitrate() << ".\n";
    tx->transmitFrame(responsePacket, responseHeader, modeSet->getSifsTime(), this);
    delete responsePacket;
}

void Dcf::processMgmtFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& mgmtHeader)
{
    throw cRuntimeError("Unknown management frame");
}

void Dcf::handlePacketRemoved(Packet *packet, queueing::IPacketQueue::PacketRemovalReason reason)
{
    Enter_Method("handlePacketRemoved");
    auto transactionTag = packet->findTag<Ieee80211MgmtTransactionTag>();
    if ((reason == queueing::IPacketQueue::PacketRemovalReason::DROPPED || reason == queueing::IPacketQueue::PacketRemovalReason::REMOVED) && transactionTag != nullptr) {
        if (cancelManagementTransaction(transactionTag->getTransactionId(), packet))
            mac->notifyFrameTransmission(packet, IFrameTransmissionCallback::Status::DROPPED_BEFORE_TRANSMISSION);
    }
}

void Dcf::cancelManagementTransaction(uint64_t transactionId)
{
    Enter_Method("cancelManagementTransaction");
    cancelManagementTransaction(transactionId, nullptr);
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

bool Dcf::isPacketReferencedByCurrentFrameSequence(const Packet *packet) const
{
    if (frameSequenceHandler == nullptr || !frameSequenceHandler->isSequenceRunning())
        return false;
    auto context = frameSequenceHandler->getContext();
    if (context == nullptr)
        return false;
    for (int i = 0; i < context->getNumSteps(); i++) {
        auto transmitStep = dynamic_cast<ITransmitStep *>(context->getStep(i));
        if (transmitStep != nullptr && transmitStep->getFrameToTransmit() == packet)
            return true;
        auto rtsTransmitStep = dynamic_cast<RtsTransmitStep *>(transmitStep);
        if (rtsTransmitStep != nullptr && rtsTransmitStep->getProtectedFrame() == packet)
            return true;
    }
    return false;
}

bool Dcf::isManagementTransactionCancelled(const Packet *packet) const
{
    if (packet == nullptr)
        return false;
    auto transactionTag = packet->findTag<Ieee80211MgmtTransactionTag>();
    return transactionTag != nullptr && cancelledManagementTransactions.find(transactionTag->getTransactionId()) != cancelledManagementTransactions.end();
}

bool Dcf::isCurrentFrameSequenceCancelled(const Packet *packet) const
{
    if (packet == nullptr || frameSequenceHandler == nullptr || !frameSequenceHandler->isSequenceRunning())
        return false;
    auto context = frameSequenceHandler->getContext();
    if (context == nullptr)
        return false;
    auto transmitStep = dynamic_cast<ITransmitStep *>(context->getLastStep());
    if (transmitStep == nullptr)
        transmitStep = dynamic_cast<ITransmitStep *>(context->getStepBeforeLast());
    if (transmitStep == nullptr)
        return false;
    if (transmitStep->getFrameToTransmit() == packet)
        return isManagementTransactionCancelled(packet) || (dynamic_cast<RtsTransmitStep *>(transmitStep) != nullptr &&
                isManagementTransactionCancelled(dynamic_cast<RtsTransmitStep *>(transmitStep)->getProtectedFrame()));
    auto rtsTransmitStep = dynamic_cast<RtsTransmitStep *>(transmitStep);
    return rtsTransmitStep != nullptr && rtsTransmitStep->getProtectedFrame() == packet && isManagementTransactionCancelled(packet);
}

bool Dcf::cancelManagementTransaction(uint64_t transactionId, Packet *excludedPacket)
{
    Enter_Method("cancelManagementTransaction");
    auto eventNumber = cSimulation::getActiveSimulation()->getEventNumber();
    if (completedManagementTransactionsEventNumber != eventNumber) {
        completedManagementTransactions.clear();
        completedManagementTransactionsEventNumber = eventNumber;
    }
    if (completedManagementTransactions.find(transactionId) != completedManagementTransactions.end())
        return false;
    if (!managementTransactionsBeingCancelled.insert(transactionId).second)
        return false;

    // IEEE Std 802.11-2024, 10.3.4.4 and 10.4: terminal retry/lifetime
    // failure discards the MMPDU and all remaining fragments. The callback's
    // packet is still borrowed by the active frame sequence, so it is left
    // for the caller to retire after this helper returns.

    auto belongsToTransaction = [transactionId, excludedPacket](Packet *packet) {
        auto transactionTag = packet->findTag<Ieee80211MgmtTransactionTag>();
        return packet != excludedPacket && transactionTag != nullptr && transactionTag->getTransactionId() == transactionId;
    };

    auto pendingQueue = channelAccess->getPendingQueue();
    for (int i = pendingQueue->getNumPackets() - 1; i >= 0; i--) {
        auto packet = pendingQueue->getPacket(i);
        if (belongsToTransaction(packet)) {
            pendingQueue->removePacket(packet);
            take(packet);
            PacketDropDetails details;
            details.setReason(OTHER_PACKET_DROP);
            emit(packetDroppedSignal, packet, &details);
            delete packet;
        }
    }

    auto inProgressFrames = channelAccess->getInProgressFrames();
    bool frameSequenceCancellationRequested = false;
    bool pendingTransmissionCancelled = false;
    for (int i = inProgressFrames->getLength() - 1; i >= 0; i--) {
        auto packet = inProgressFrames->getFrames(i);
        if (belongsToTransaction(packet)) {
            if (isPacketReferencedByCurrentFrameSequence(packet)) {
                // Keep the packet alive for raw pointers held by the active
                // sequence, but remove it from eligibility immediately. The
                // sequence retires it at its next safe boundary.
                inProgressFrames->dropFrame(packet);
                cancelledManagementTransactions.insert(transactionId);
                PacketDropDetails details;
                details.setReason(OTHER_PACKET_DROP);
                emit(packetDroppedSignal, packet, &details);
                bool currentFrameSequenceCancelled = isCurrentFrameSequenceCancelled(packet);
                frameSequenceCancellationRequested |= currentFrameSequenceCancelled;
                // A sequence can retain completed steps in its context. Only
                // cancel Tx when the current transmit/protected step belongs
                // to this transaction; the callback owner alone is not enough
                // to identify a historical frame.
                if (currentFrameSequenceCancelled && tx != nullptr)
                    pendingTransmissionCancelled |= tx->cancelPendingTransmission(this);
                continue;
            }
            auto header = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
            auto extractedPacket = inProgressFrames->extractFrame(packet);
            ASSERT(extractedPacket == packet);
            take(packet);
            if (recoveryProcedure != nullptr)
                recoveryProcedure->discardFrame(packet, header);
            ackHandler->dropFrame(header);
            PacketDropDetails details;
            details.setReason(OTHER_PACKET_DROP);
            emit(packetDroppedSignal, packet, &details);
            delete packet;
        }
    }
    if (frameSequenceCancellationRequested && frameSequenceHandler != nullptr) {
        frameSequenceHandler->cancelFrameSequence();
        if (pendingTransmissionCancelled)
            frameSequenceHandler->abortFrameSequence();
    }
    managementTransactionsBeingCancelled.erase(transactionId);
    completedManagementTransactions.insert(transactionId);
    return true;
}

void Dcf::scheduleStartRxTimer(simtime_t timeout)
{
    Enter_Method("scheduleStartRxTimer");
    scheduleAfter(timeout, startRxTimer);
}

void Dcf::processLowerFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    Enter_Method("processLowerFrame(%s)", packet->getName());
    take(packet);
    EV_INFO << "Processing lower frame: " << packet->getName() << endl;
    if (frameSequenceHandler->isSequenceRunning()) {
        // TODO always call processResponses
        if ((!isForUs(header) && !startRxTimer->isScheduled()) || isForUs(header)) {
            frameSequenceHandler->processResponse(packet);
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
    Enter_Method("transmitFrame");
    const auto& header = packet->peekAtFront<Ieee80211MacHeader>();
    auto mode = rateSelection->computeMode(packet, header);
    RateSelection::setFrameMode(packet, header, mode);
    emit(IRateSelection::datarateSelectedSignal, mode->getDataMode()->getNetBitrate().get<bps>(), packet);
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
 * TODO  If a PHY-RXSTART.indication primitive does not occur during the ACKTimeout interval,
 * the STA concludes that the transmission of the MPDU has failed, and this STA shall invoke its
 * backoff procedure **upon expiration of the ACKTimeout interval**.
 */

void Dcf::frameSequenceFinished()
{
    Enter_Method("frameSequenceFinished");
    emit(IFrameSequenceHandler::frameSequenceFinishedSignal, frameSequenceHandler->getContext());
    channelAccess->releaseChannel(this);
    if (hasFrameToTransmit())
        channelAccess->requestChannel(this);
    mac->sendDownPendingRadioConfigMsg(); // TODO review
    cancelledManagementTransactions.clear();
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
    else { // TODO else if (auto ctrlFrame = dynamic_cast<Ieee80211ControlFrame*>(frame))
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

FrameSequenceContext *Dcf::buildContext()
{
    auto nonQoSContext = new NonQoSContext(originatorAckPolicy);
    return new FrameSequenceContext(mac->getAddress(), modeSet, channelAccess->getInProgressFrames(), rtsProcedure, rtsPolicy, nonQoSContext, nullptr);
}

void Dcf::transmissionComplete(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    Enter_Method("transmissionComplete");
    if (frameSequenceHandler->isSequenceRunning()) {
        frameSequenceHandler->transmissionComplete();
    }
    else
        recipientProcessTransmittedControlResponseFrame(packet, header);
}

bool Dcf::hasFrameToTransmit()
{
    return !channelAccess->getPendingQueue()->isEmpty() || channelAccess->getInProgressFrames()->hasInProgressFrames();
}

void Dcf::originatorProcessRtsProtectionFailed(Packet *packet)
{
    Enter_Method("originatorProcessRtsProtectionFailed");
    if (isManagementTransactionCancelled(packet)) {
        auto protectedHeader = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
        if (recoveryProcedure != nullptr)
            recoveryProcedure->discardRtsFrame(protectedHeader);
        channelAccess->getInProgressFrames()->dropFrame(packet);
        if (ackHandler != nullptr)
            ackHandler->dropFrame(protectedHeader);
        return;
    }
    EV_INFO << "RTS frame transmission failed\n";
    auto protectedHeader = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
    recoveryProcedure->rtsFrameTransmissionFailed(protectedHeader, stationRetryCounters);
    EV_INFO << "For the current frame exchange, we have CW = " << channelAccess->getCw() << " SRC = " << recoveryProcedure->getShortRetryCount(packet, protectedHeader) << " LRC = " << recoveryProcedure->getLongRetryCount(packet, protectedHeader) << " SSRC = " << stationRetryCounters->getStationShortRetryCount() << " and SLRC = " << stationRetryCounters->getStationLongRetryCount() << std::endl;
    if (recoveryProcedure->isRtsFrameRetryLimitReached(packet, protectedHeader)) {
        recoveryProcedure->retryLimitReached(packet, protectedHeader);
        auto transactionTag = packet->findTag<Ieee80211MgmtTransactionTag>();
        bool notifyManagement = dynamicPtrCast<const Ieee80211MgmtHeader>(protectedHeader) != nullptr &&
                (transactionTag == nullptr || cancelManagementTransaction(transactionTag->getTransactionId(), packet));
        channelAccess->getInProgressFrames()->dropFrame(packet);
        ackHandler->dropFrame(protectedHeader);
        EV_INFO << "Dropping RTS/CTS protected frame " << packet->getName() << ", because retry limit is reached.\n";
        PacketDropDetails details;
        details.setReason(RETRY_LIMIT_REACHED);
        details.setLimit(recoveryProcedure->getShortRetryLimit());
        emit(packetDroppedSignal, packet, &details);
        emit(linkBrokenSignal, packet);
        if (notifyManagement)
            mac->notifyFrameTransmission(packet, IFrameTransmissionCallback::Status::RETRY_LIMIT_REACHED);
    }
}

void Dcf::originatorProcessTransmittedFrame(Packet *packet)
{
    Enter_Method("originatorProcessTransmittedFrame");
    if (isCurrentFrameSequenceCancelled(packet))
        return;
    EV_INFO << "Processing transmitted frame " << packet->getName() << " as originator in frame sequence.\n";
    emit(packetSentToPeerSignal, packet);
    if (isCurrentFrameSequenceCancelled(packet))
        return;
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
        auto protectedFrame = channelAccess->getInProgressFrames()->getFrameToTransmit(); // KLUDGE
        auto protectedHeader = protectedFrame->peekAtFront<Ieee80211DataOrMgmtHeader>();
        EV_INFO << "For the current frame exchange, we have CW = " << channelAccess->getCw() << " SRC = " << recoveryProcedure->getShortRetryCount(protectedFrame, protectedHeader) << " LRC = " << recoveryProcedure->getLongRetryCount(protectedFrame, protectedHeader) << " SSRC = " << stationRetryCounters->getStationShortRetryCount() << " and SLRC = " << stationRetryCounters->getStationLongRetryCount() << std::endl;
        rtsProcedure->processTransmittedRts(rtsFrame);
    }
}

void Dcf::originatorProcessReceivedFrame(Packet *receivedPacket, Packet *lastTransmittedPacket)
{
    Enter_Method("originatorProcessReceivedFrame");
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
        if (dynamicPtrCast<const Ieee80211MgmtHeader>(lastTransmittedDataOrMgmtHeader))
            mac->notifyFrameTransmission(lastTransmittedPacket, IFrameTransmissionCallback::Status::ACKNOWLEDGED);
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
    Enter_Method("originatorProcessFailedFrame");
    if (isManagementTransactionCancelled(failedPacket)) {
        auto failedHeader = failedPacket->peekAtFront<Ieee80211DataOrMgmtHeader>();
        if (recoveryProcedure != nullptr)
            recoveryProcedure->discardFrame(failedPacket, failedHeader);
        channelAccess->getInProgressFrames()->dropFrame(failedPacket);
        if (ackHandler != nullptr)
            ackHandler->dropFrame(failedHeader);
        return;
    }
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
        auto transactionTag = failedPacket->findTag<Ieee80211MgmtTransactionTag>();
        bool notifyManagement = dynamicPtrCast<const Ieee80211MgmtHeader>(failedHeader) != nullptr &&
                (transactionTag == nullptr || cancelManagementTransaction(transactionTag->getTransactionId(), failedPacket));
        channelAccess->getInProgressFrames()->dropFrame(failedPacket);
        ackHandler->dropFrame(failedHeader);
        EV_INFO << "Dropping frame " << failedPacket->getName() << ", because retry limit is reached.\n";
        PacketDropDetails details;
        details.setReason(RETRY_LIMIT_REACHED);
        details.setLimit(-1); // TODO
        emit(packetDroppedSignal, failedPacket, &details);
        emit(linkBrokenSignal, failedPacket);
        if (notifyManagement)
            mac->notifyFrameTransmission(failedPacket, IFrameTransmissionCallback::Status::RETRY_LIMIT_REACHED);
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
    // FIXME
    // Check the roles of the Addr3 field when aggregation is applied
    // Table 8-19—Address field contents
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header))
        return dataOrMgmtHeader->getAddress3() == mac->getAddress();
    else
        return false;
}

void Dcf::corruptedFrameReceived()
{
    Enter_Method("corruptedFrameReceived");
    if (frameSequenceHandler->isSequenceRunning() && !startRxTimer->isScheduled()) {
        frameSequenceHandler->handleStartRxTimeout();
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
