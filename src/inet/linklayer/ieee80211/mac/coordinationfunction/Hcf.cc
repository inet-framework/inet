//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mac/blockack/Ieee80211AddbaTransactionTag_m.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckProcedure.h"
#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/Ieee80211FragmentedActionContextTag.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceStep.h"
#include "inet/linklayer/ieee80211/mac/framesequence/HcfFs.h"
#include "inet/linklayer/ieee80211/mac/recipient/RecipientAckProcedure.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtTransactionTag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

simsignal_t Hcf::edcaCollisionDetectedSignal = cComponent::registerSignal("edcaCollisionDetected");
simsignal_t Hcf::blockAckAgreementAddedSignal = cComponent::registerSignal("blockAckAgreementAdded");
simsignal_t Hcf::blockAckAgreementDeletedSignal = cComponent::registerSignal("blockAckAgreementDeleted");
simsignal_t Hcf::blockAckAgreementChangedSignal = cComponent::registerSignal("blockAckAgreementChanged");

Define_Module(Hcf);

void Hcf::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mac = check_and_cast<Ieee80211Mac *>(getContainingNicModule(this)->getSubmodule("mac"));
        startRxTimer = new cMessage("startRxTimeout");
        inactivityTimer = new cMessage("blockAckInactivityTimer");
        addbaResponseTimer = new cMessage("addbaResponseTimer");
        edca = check_and_cast<Edca *>(getSubmodule("edca"));
        hcca = check_and_cast<Hcca *>(getSubmodule("hcca"));
        tx = check_and_cast<ITx *>(getModuleByPath(par("txModule")));
        rx = check_and_cast<IRx *>(getModuleByPath(par("rxModule")));
        dataAndMgmtRateControl = dynamic_cast<IRateControl *>(getSubmodule("rateControl"));
        originatorBlockAckAgreementPolicy = dynamic_cast<IOriginatorBlockAckAgreementPolicy *>(getSubmodule("originatorBlockAckAgreementPolicy"));
        recipientBlockAckAgreementPolicy = dynamic_cast<IRecipientBlockAckAgreementPolicy *>(getSubmodule("recipientBlockAckAgreementPolicy"));
        rateSelection = check_and_cast<IQosRateSelection *>(getSubmodule("rateSelection"));
        frameSequenceHandler = new FrameSequenceHandler();
        WATCH_EXPR("frameSequenceInfo", getFrameSequenceInfo());
        originatorDataService = check_and_cast<IOriginatorMacDataService *>(getSubmodule(("originatorMacDataService")));
        recipientDataService = check_and_cast<IRecipientQosMacDataService *>(getSubmodule("recipientMacDataService"));
        originatorAckPolicy = check_and_cast<IOriginatorQoSAckPolicy *>(getSubmodule("originatorAckPolicy"));
        recipientAckPolicy = check_and_cast<IRecipientQosAckPolicy *>(getSubmodule("recipientAckPolicy"));
        singleProtectionMechanism = check_and_cast<SingleProtectionMechanism *>(getSubmodule("singleProtectionMechanism"));
        rtsProcedure = new RtsProcedure();
        rtsPolicy = check_and_cast<IRtsPolicy *>(getSubmodule("rtsPolicy"));
        recipientAckProcedure = new RecipientAckProcedure();
        ctsProcedure = new CtsProcedure();
        ctsPolicy = check_and_cast<ICtsPolicy *>(getSubmodule("ctsPolicy"));
        if (originatorBlockAckAgreementPolicy && recipientBlockAckAgreementPolicy) {
            recipientBlockAckAgreementHandler = new RecipientBlockAckAgreementHandler();
            originatorBlockAckAgreementHandler = new OriginatorBlockAckAgreementHandler();
            originatorBlockAckProcedure = new OriginatorBlockAckProcedure();
            recipientBlockAckProcedure = new RecipientBlockAckProcedure();
            originatorDataService->setFrameEligibilityFunction([this](const Packet *packet) {
                if (auto addbaReq = findFragmentedActionContext<Ieee80211AddbaRequest>(packet))
                    return originatorBlockAckAgreementHandler->isAddbaRequestPending(packet, addbaReq);
                if (auto delba = findFragmentedActionContext<Ieee80211Delba>(packet))
                    return originatorBlockAckAgreementHandler->isDelbaPending(packet, delba);
                auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(packet->peekAtFront<Ieee80211MacHeader>());
                // Hold this peer/TID while its ADDBA response is pending so no
                // already-sequenced MPDU can precede the advertised SSN. The
                // response timeout starts only after the request is transmitted.
                return dataHeader == nullptr || dataHeader->getType() != ST_DATA_WITH_QOS || (!originatorBlockAckAgreementHandler->isAddbaResponsePending(dataHeader->getReceiverAddress(), dataHeader->getTid()));
            });
        }
    }
    else if (stage == INITSTAGE_LAST) {
        // Edca resolves its Edcaf array at the link-layer stage. Install the
        // queue callbacks after all child initialization has completed.
        for (int ac = 0; ac < edca->getNumEdcafs(); ac++) {
            auto pendingQueue = edca->getEdcaf(AccessCategory(ac))->getPendingQueue();
            pendingQueue->addPacketCallback(this);
        }
        rebuildPendingFrameEligibility();
    }
}

void Hcf::trackPendingFrame(Packet *packet, AccessCategory accessCategory)
{
    untrackPendingFrame(packet);
    bool eligible = originatorDataService->isFrameEligible(packet);
    pendingFrameEligibility.emplace(packet, PendingFrameEligibility { accessCategory, eligible });
    if (eligible)
        numEligiblePendingFrames[accessCategory]++;
}

void Hcf::untrackPendingFrame(const Packet *packet)
{
    auto it = pendingFrameEligibility.find(packet);
    if (it != pendingFrameEligibility.end()) {
        if (it->second.eligible) {
            ASSERT(numEligiblePendingFrames[it->second.accessCategory] > 0);
            numEligiblePendingFrames[it->second.accessCategory]--;
        }
        pendingFrameEligibility.erase(it);
    }
}

void Hcf::rebuildPendingFrameEligibility()
{
    pendingFrameEligibility.clear();
    numEligiblePendingFrames.fill(0);
    int numPendingFrames = 0;
    for (int ac = 0; ac < edca->getNumEdcafs(); ac++) {
        auto accessCategory = AccessCategory(ac);
        auto pendingQueue = edca->getEdcaf(accessCategory)->getPendingQueue();
        for (int i = 0; i < pendingQueue->getNumPackets(); i++) {
            trackPendingFrame(pendingQueue->getPacket(i), accessCategory);
            numPendingFrames++;
        }
    }
    ASSERT((int)pendingFrameEligibility.size() == numPendingFrames);
}

bool Hcf::processDroppedBlockAckSetupFrame(Packet *packet)
{
    if (originatorBlockAckAgreementHandler) {
        auto addbaReq = findFragmentedActionContext<Ieee80211AddbaRequest>(packet);
        if (addbaReq != nullptr && originatorBlockAckAgreementHandler->isAddbaRequestPending(packet, addbaReq)) {
            originatorBlockAckAgreementHandler->processDroppedAddbaReq(packet, addbaReq, originatorBlockAckAgreementPolicy, this);
            rebuildPendingFrameEligibility();
            return true;
        }
    }
    return false;
}

bool Hcf::processDroppedBlockAckTeardownFrame(Packet *packet)
{
    if (originatorBlockAckAgreementHandler) {
        auto delba = findFragmentedActionContext<Ieee80211Delba>(packet);
        if (delba != nullptr && originatorBlockAckAgreementHandler->processAbortedDelba(packet, this)) {
            rebuildPendingFrameEligibility();
            return true;
        }
    }
    return false;
}

void Hcf::handlePacketRemoved(Packet *packet, queueing::IPacketQueue::PacketRemovalReason reason)
{
    Enter_Method("handlePacketRemoved");
    untrackPendingFrame(packet);
    bool shouldResume = false;
    // HCF treats explicit REMOVED notifications as terminal transaction
    // disposal; code relocating a packet must use dequeuePacket().
    if (reason == queueing::IPacketQueue::PacketRemovalReason::DROPPED || reason == queueing::IPacketQueue::PacketRemovalReason::REMOVED) {
        if (reason == queueing::IPacketQueue::PacketRemovalReason::DROPPED && packet->findTag<Ieee80211MgmtTransactionTag>() != nullptr)
            mac->notifyFrameTransmission(packet, IFrameTransmissionCallback::Status::DROPPED_BEFORE_TRANSMISSION);
        shouldResume |= processDroppedBlockAckSetupFrame(packet);
        shouldResume |= processDroppedBlockAckTeardownFrame(packet);
    }
    if (shouldResume)
        resumeEligibleChannelAccess();
}

std::string Hcf::getFrameSequenceInfo() const
{
    if (!frameSequenceHandler->isSequenceRunning())
        return "";
    auto history = frameSequenceHandler->getFrameSequence()->getHistory();
    if (history.length() > 32) {
        history.erase(history.begin(), history.end() - 32);
        history = "..." + history;
    }
    return "Fs: " + history;
}

void Hcf::forEachChild(cVisitor *v)
{
    SimpleModule::forEachChild(v);
    if (frameSequenceHandler != nullptr && frameSequenceHandler->getContext() != nullptr)
        v->visit(const_cast<FrameSequenceContext *>(frameSequenceHandler->getContext()));
}

void Hcf::handleMessage(cMessage *msg)
{
    if (msg == startRxTimer) {
        if (!isReceptionInProgress()) {
            frameSequenceHandler->handleStartRxTimeout();
        }
    }
    else if (msg == inactivityTimer) {
        if (originatorBlockAckAgreementHandler && recipientBlockAckAgreementHandler) {
            originatorBlockAckAgreementHandler->blockAckAgreementExpired(this, this);
            recipientBlockAckAgreementHandler->blockAckAgreementExpired(this, this);
        }
        else
            throw cRuntimeError("Unknown event");
    }
    else if (msg == addbaResponseTimer) {
        if (originatorBlockAckAgreementHandler) {
            originatorBlockAckAgreementHandler->addbaResponseTimeoutExpired(originatorBlockAckAgreementPolicy, this);
            rebuildPendingFrameEligibility();
            resumeEligibleChannelAccess();
        }
        else
            throw cRuntimeError("Unknown event");
    }
    else
        throw cRuntimeError("Unknown msg type");
}

void Hcf::refreshDisplay() const
{
    ModeSetListener::refreshDisplay();
    if (frameSequenceHandler->isSequenceRunning()) {
        auto history = frameSequenceHandler->getFrameSequence()->getHistory();
        getDisplayString().setTagArg("tt", 0, ("Fs: " + history).c_str());
    }
    else {
        getDisplayString().removeTag("tt");
    }
}

void Hcf::processUpperFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    Enter_Method("processUpperFrame(%s)", packet->getName());
    take(packet);
    EV_INFO << "Processing upper frame: " << packet->getName() << endl;
    // TODO
    // A QoS STA should send individually addressed Management frames that are addressed to a non-QoS STA
    // using the access category AC_BE and shall send all other management frames using the access category
    // AC_VO. A QoS STA that does not send individually addressed Management frames that are addressed to a
    // non-QoS STA using the access category AC_BE shall send them using the access category AC_VO.
    // Management frames are exempted from any and all restrictions on transmissions arising from admission
    // control procedures.
    AccessCategory ac = AccessCategory(-1);
    if (dynamicPtrCast<const Ieee80211MgmtHeader>(header)) // TODO + non-QoS frames
        ac = AccessCategory::AC_VO;
    else if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header))
        ac = edca->classifyFrame(dataHeader);
    else
        throw cRuntimeError("Unknown message type");
    EV_INFO << "The upper frame has been classified as a " << printAccessCategory(ac) << " frame." << endl;
    auto pendingQueue = edca->getEdcaf(ac)->getPendingQueue();
    trackPendingFrame(packet, ac);
    pendingQueue->enqueuePacket(packet);
    if (hasFrameToTransmit(ac)) {
        auto edcaf = edca->getChannelOwner();
        if (edcaf == nullptr || edcaf->getAccessCategory() != ac) {
            EV_DETAIL << "Requesting channel for access category " << printAccessCategory(ac) << endl;
            edca->requestChannelAccess(ac, this);
        }
    }
}

bool Hcf::isPacketReferencedByCurrentFrameSequence(const Packet *packet) const
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

void Hcf::scheduleStartRxTimer(simtime_t timeout)
{
    Enter_Method("scheduleStartRxTimer");
    scheduleAfter(timeout, startRxTimer);
}

void Hcf::scheduleInactivityTimer(simtime_t timeout)
{
    Enter_Method("scheduleInactivityTimer");
    rescheduleAfter(timeout, inactivityTimer);
}

void Hcf::scheduleAddbaResponseTimer(simtime_t deadline)
{
    Enter_Method("scheduleAddbaResponseTimer");
    if (deadline == SIMTIME_MAX) {
        if (addbaResponseTimer->isScheduled())
            cancelEvent(addbaResponseTimer);
    }
    else
        rescheduleAt(deadline, addbaResponseTimer);
}

void Hcf::cancelAddbaTransaction(uint64_t transactionId, Packet *excludedPacket)
{
    Enter_Method("cancelAddbaTransaction");
    // Frames borrowed by the active sequence cannot be removed here. The
    // sequence's failure paths detect their now-stale transaction and discard them.
    auto belongsToTransaction = [this, transactionId, excludedPacket](Packet *packet) {
        auto transactionTag = packet->findTag<Ieee80211AddbaTransactionTag>();
        return packet != excludedPacket && !isPacketReferencedByCurrentFrameSequence(packet) && transactionTag != nullptr && transactionTag->getTransactionId() == transactionId;
    };
    for (int ac = 0; ac < edca->getNumEdcafs(); ac++) {
        auto edcaf = edca->getEdcaf(AccessCategory(ac));
        auto pendingQueue = edcaf->getPendingQueue();
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
        auto inProgressFrames = edcaf->getInProgressFrames();
        for (int i = inProgressFrames->getLength() - 1; i >= 0; i--) {
            auto packet = inProgressFrames->getFrames(i);
            if (belongsToTransaction(packet)) {
                auto header = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
                auto extractedPacket = inProgressFrames->extractFrame(packet);
                ASSERT(extractedPacket == packet);
                take(packet);
                edcaf->getAckHandler()->dropFrame(header);
                PacketDropDetails details;
                details.setReason(OTHER_PACKET_DROP);
                emit(packetDroppedSignal, packet, &details);
                delete packet;
            }
        }
    }
}

void Hcf::processLowerFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    Enter_Method("processLowerFrame(%s)", packet->getName());
    take(packet);
    EV_INFO << "Processing lower frame: " << packet->getName() << endl;
    auto edcaf = edca->getChannelOwner();
    if (edcaf && frameSequenceHandler->isSequenceRunning()) {
        // TODO always call processResponse?
        if ((!isForUs(header) && !startRxTimer->isScheduled()) || isForUs(header)) {
            frameSequenceHandler->processResponse(packet);
            // Only cancel RxTimer when the current running sequence has been handled by frameSequenceHandler->processResponse().
            // If the received frame is not for us, we are still waiting to receive our ACK. In that case, don't cancel the timer.
            // Otherwise, current frame sequence stucks in RX step and runs longer than intendeed, preventing sequence from
            // another access category (AC) to start running (RuntimeError("Channel access granted while a frame sequence is running")).
            cancelEvent(startRxTimer);
        }
        else {
            EV_INFO << "This frame is not for us" << std::endl;
            PacketDropDetails details;
            details.setReason(NOT_ADDRESSED_TO_US);
            emit(packetDroppedSignal, packet, &details);
            delete packet;
        }
    }
    else if (hcca->isOwning())
        throw cRuntimeError("Hcca is unimplemented!");
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

void Hcf::channelGranted(IChannelAccess *channelAccess)
{
    Enter_Method("channelGranted");
    auto edcaf = check_and_cast<Edcaf *>(channelAccess);
    if (edcaf) {
        AccessCategory ac = edcaf->getAccessCategory();
        EV_DETAIL << "Channel access granted to the " << printAccessCategory(ac) << " queue" << std::endl;
        auto internallyCollidedEdcafs = edca->getInternallyCollidedEdcafs();
        if (internallyCollidedEdcafs.size() > 0) {
            EV_INFO << "Internal collision happened with the following queues:" << std::endl;
            // IEEE Std 802.11-2024, 10.23.2.4: an EDCAF with no eligible
            // frame has no collision recovery action to perform.
            auto handledCollisions = handleInternalCollision(internallyCollidedEdcafs);
            if (handledCollisions > 0)
                emit(edcaCollisionDetectedSignal, (unsigned long)handledCollisions);
        }
        if (!hasFrameToTransmit(ac)) {
            EV_DETAIL << "Releasing channel because no eligible frame is available.\n";
            edcaf->releaseChannel(this);
            mac->sendDownPendingRadioConfigMsg();
            return;
        }
        edcaf->getTxopProcedure()->startTxop(ac);
        startFrameSequence(ac);
    }
    else
        throw cRuntimeError("Channel access granted but channel owner not found!");
}

FrameSequenceContext *Hcf::buildContext(AccessCategory ac)
{
    auto edcaf = edca->getEdcaf(ac);
    auto qosContext = new QoSContext(originatorAckPolicy, originatorBlockAckProcedure, originatorBlockAckAgreementHandler, edcaf->getTxopProcedure());
    return new FrameSequenceContext(mac->getAddress(), modeSet, edcaf->getInProgressFrames(), rtsProcedure, rtsPolicy, nullptr, qosContext);
}

void Hcf::startFrameSequence(AccessCategory ac)
{
    frameSequenceHandler->startFrameSequence(new HcfFs(), buildContext(ac), this);
    emit(IFrameSequenceHandler::frameSequenceStartedSignal, frameSequenceHandler->getContext());
}

int Hcf::handleInternalCollision(std::vector<Edcaf *> internallyCollidedEdcafs)
{
    int handledCollisions = 0;
    for (auto edcaf : internallyCollidedEdcafs) {
        AccessCategory ac = edcaf->getAccessCategory();
        auto dataRecoveryProcedure = edcaf->getRecoveryProcedure();
        Packet *internallyCollidedFrame = edcaf->getInProgressFrames()->getFrameToTransmit();
        if (internallyCollidedFrame == nullptr) {
            EV_DETAIL << "Ignoring internal collision because no eligible frame is available for " << printAccessCategory(ac) << ".\n";
            continue;
        }
        handledCollisions++;
        auto internallyCollidedHeader = internallyCollidedFrame->peekAtFront<Ieee80211DataOrMgmtHeader>();
        EV_INFO << printAccessCategory(ac) << " (" << internallyCollidedFrame->getName() << ")" << endl;
        bool retryLimitReached = false;
        if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(internallyCollidedHeader)) { // TODO QoSDataFrame
            dataRecoveryProcedure->dataFrameTransmissionFailed(internallyCollidedFrame, dataHeader);
            retryLimitReached = dataRecoveryProcedure->isRetryLimitReached(internallyCollidedFrame, dataHeader);
        }
        else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(internallyCollidedHeader)) {
            ASSERT(ac == AccessCategory::AC_BE);
            edca->getMgmtAndNonQoSRecoveryProcedure()->dataOrMgmtFrameTransmissionFailed(internallyCollidedFrame, mgmtHeader, edca->getEdcaf(AccessCategory::AC_BE)->getStationRetryCounters());
            retryLimitReached = edca->getMgmtAndNonQoSRecoveryProcedure()->isRetryLimitReached(internallyCollidedFrame, mgmtHeader);
        }
        else // TODO + NonQoSDataFrame
            throw cRuntimeError("Unknown frame");
        if (retryLimitReached) {
            EV_DETAIL << "The frame has reached its retry limit. Dropping it" << std::endl;
            if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(internallyCollidedHeader))
                dataRecoveryProcedure->retryLimitReached(internallyCollidedFrame, dataHeader);
            else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(internallyCollidedHeader))
                edca->getMgmtAndNonQoSRecoveryProcedure()->retryLimitReached(internallyCollidedFrame, mgmtHeader);
            else ; // TODO + NonQoSDataFrame
            edcaf->getInProgressFrames()->dropFrame(internallyCollidedFrame);
            processDroppedBlockAckSetupFrame(internallyCollidedFrame);
            processDroppedBlockAckTeardownFrame(internallyCollidedFrame);
            edcaf->getAckHandler()->dropFrame(internallyCollidedHeader);
            PacketDropDetails details;
            details.setReason(RETRY_LIMIT_REACHED);
            details.setLimit(-1); // TODO
            emit(packetDroppedSignal, internallyCollidedFrame, &details);
            emit(linkBrokenSignal, internallyCollidedFrame);
            if (dynamicPtrCast<const Ieee80211MgmtHeader>(internallyCollidedHeader))
                mac->notifyFrameTransmission(internallyCollidedFrame, IFrameTransmissionCallback::Status::RETRY_LIMIT_REACHED);
            if (hasFrameToTransmit(ac))
                edcaf->requestChannel(this);
        }
        else
            edcaf->requestChannel(this);
    }
    return handledCollisions;
}

/*
 * TODO  If a PHY-RXSTART.indication primitive does not occur during the ACKTimeout interval,
 * the STA concludes that the transmission of the MPDU has failed, and this STA shall invoke its
 * backoff procedure **upon expiration of the ACKTimeout interval**.
 */

void Hcf::frameSequenceFinished()
{
    Enter_Method("frameSequenceFinished");
    emit(IFrameSequenceHandler::frameSequenceFinishedSignal, frameSequenceHandler->getContext());
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        edcaf->releaseChannel(this);
        mac->sendDownPendingRadioConfigMsg(); // TODO review
        edcaf->getTxopProcedure()->endTxop();
        // Agreement transitions may have made frames in any AC eligible.
        requestEligibleChannelAccess();
    }
    else if (hcca->isOwning()) {
        hcca->releaseChannel(this);
        mac->sendDownPendingRadioConfigMsg(); // TODO review
        throw cRuntimeError("Hcca is unimplemented!");
    }
    else
        throw cRuntimeError("Frame sequence finished but channel owner not found!");
}

void Hcf::recipientProcessReceivedFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    EV_INFO << "Processing received frame " << packet->getName() << " as recipient.\n";
    emit(packetReceivedFromPeerSignal, packet);
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header))
        recipientAckProcedure->processReceivedFrame(packet, dataOrMgmtHeader, check_and_cast<IRecipientAckPolicy *>(recipientAckPolicy), this);
    if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header)) {
        if (dataHeader->getType() == ST_DATA_WITH_QOS && recipientBlockAckAgreementHandler)
            recipientBlockAckAgreementHandler->qosFrameReceived(dataHeader, this);
        sendUp(recipientDataService->dataFrameReceived(packet, dataHeader, recipientBlockAckAgreementHandler));
    }
    else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(header)) {
        auto receptionResult = recipientDataService->managementFrameReceived(packet, mgmtHeader);
        sendUp(receptionResult.completeFrames);
        if (receptionResult.completeHeader != nullptr)
            recipientProcessReceivedManagementFrame(receptionResult.completeHeader, receptionResult.duplicate);
    }
    else { // TODO else if (auto ctrlFrame = dynamic_cast<Ieee80211ControlFrame*>(frame))
        sendUp(recipientDataService->controlFrameReceived(packet, header, recipientBlockAckAgreementHandler));
        recipientProcessReceivedControlFrame(packet, header);
        delete packet;
    }
}

void Hcf::recipientProcessReceivedControlFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    if (auto rtsFrame = dynamicPtrCast<const Ieee80211RtsFrame>(header))
        ctsProcedure->processReceivedRts(packet, rtsFrame, ctsPolicy, this);
    else if (auto blockAckRequest = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(header)) {
        if (recipientBlockAckProcedure)
            recipientBlockAckProcedure->processReceivedBlockAckReq(packet, blockAckRequest, recipientAckPolicy, recipientBlockAckAgreementHandler, this);
    }
    else if (dynamicPtrCast<const Ieee80211AckFrame>(header))
        EV_WARN << "ACK frame received after timeout, ignoring it.\n"; // drop it, it is an ACK frame that is received after the ACKTimeout
    else
        throw cRuntimeError("Unknown control frame");
}

void Hcf::recipientProcessReceivedManagementFrame(const Ptr<const Ieee80211MgmtHeader>& header, bool duplicate)
{
    if (duplicate) {
        if (recipientBlockAckAgreementHandler) {
            if (auto addbaRequest = dynamicPtrCast<const Ieee80211AddbaRequest>(header))
                recipientBlockAckAgreementHandler->processDuplicateAddbaRequest(addbaRequest, this);
        }
        return;
    }
    if (recipientBlockAckAgreementHandler && originatorBlockAckAgreementHandler) {
        if (auto addbaRequest = dynamicPtrCast<const Ieee80211AddbaRequest>(header)) {
            bool hadAgreement = recipientBlockAckAgreementHandler->getAgreement(addbaRequest->getTid(), addbaRequest->getTransmitterAddress()) != nullptr;
            auto agreement = recipientBlockAckAgreementHandler->processReceivedAddbaRequest(addbaRequest, recipientBlockAckAgreementPolicy, this, this);
            if (agreement != nullptr) {
                if (hadAgreement) {
                    recipientDataService->resetBlockAckReordering(addbaRequest->getTid(), addbaRequest->getTransmitterAddress());
                    emit(blockAckAgreementChangedSignal, agreement);
                }
                else
                    emit(blockAckAgreementAddedSignal, agreement);
            }
        }
        else if (auto addbaResp = dynamicPtrCast<const Ieee80211AddbaResponse>(header)) {
            bool wasPending = originatorBlockAckAgreementHandler->isAddbaResponsePending(addbaResp->getTransmitterAddress(), addbaResp->getTid());
            auto response = originatorBlockAckAgreementHandler->processReceivedAddbaResp(addbaResp, originatorBlockAckAgreementPolicy, this);
            if (wasPending && !originatorBlockAckAgreementHandler->isAddbaResponsePending(addbaResp->getTransmitterAddress(), addbaResp->getTid()))
                rebuildPendingFrameEligibility();
            if (response.teardownDelba != nullptr && (response.terminatedAgreement == nullptr || response.teardownTransactionId == 0))
                throw cRuntimeError("Invalid locally vetoed ADDBA response outcome");
            if (response.establishedAgreement != nullptr)
                emit(blockAckAgreementAddedSignal, response.establishedAgreement);
            if (response.terminatedAgreement != nullptr) {
                emit(blockAckAgreementAddedSignal, response.terminatedAgreement.get());
                emit(blockAckAgreementDeletedSignal, response.terminatedAgreement.get());
            }
            if (response.teardownDelba != nullptr) {
                auto delbaPacket = new Packet("Delba", response.teardownDelba);
                delbaPacket->addTag<Ieee80211AddbaTransactionTag>()->setTransactionId(response.teardownTransactionId);
                processMgmtFrame(delbaPacket, response.teardownDelba);
            }
            resumeEligibleChannelAccess();
        }
        else if (auto delba = dynamicPtrCast<const Ieee80211Delba>(header)) {
            // IEEE Std 802.11-2024, 9.4.1.16, 10.25.4, and 11.5.3.3:
            // Initiator selects the agreement direction; the transmitter is the peer.
            if (delba->getInitiator()) {
                auto agreement = recipientBlockAckAgreementHandler->processReceivedDelba(delba, recipientBlockAckAgreementPolicy);
                if (agreement != nullptr) {
                    recipientDataService->resetBlockAckReordering(delba->getTid(), delba->getTransmitterAddress());
                    emit(blockAckAgreementDeletedSignal, agreement.get());
                }
            }
            else {
                bool wasPending = originatorBlockAckAgreementHandler->isAddbaResponsePending(delba->getTransmitterAddress(), delba->getTid());
                auto agreement = originatorBlockAckAgreementHandler->processReceivedDelba(delba, originatorBlockAckAgreementPolicy, this);
                if (wasPending && !originatorBlockAckAgreementHandler->isAddbaResponsePending(delba->getTransmitterAddress(), delba->getTid()))
                    rebuildPendingFrameEligibility();
                if (agreement != nullptr && agreement->getIsAddbaResponseReceived())
                    emit(blockAckAgreementDeletedSignal, agreement.get());
                resumeEligibleChannelAccess();
            }
        }
        else
            ; // Beacon, etc
    }
    else
        ; // Optional modules
}

void Hcf::transmissionComplete(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    Enter_Method("transmissionComplete");
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        frameSequenceHandler->transmissionComplete();
    }
    else if (hcca->isOwning())
        throw cRuntimeError("Hcca is unimplemented!");
    else
        recipientProcessTransmittedControlResponseFrame(packet, header);
}

void Hcf::originatorProcessRtsProtectionFailed(Packet *packet)
{
    Enter_Method("originatorProcessRtsProtectionFailed");
    auto protectedHeader = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        EV_INFO << "RTS frame transmission failed\n";
        bool retryLimitReached = false;
        if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(protectedHeader)) {
            edcaf->getRecoveryProcedure()->rtsFrameTransmissionFailed(dataHeader);
            retryLimitReached = edcaf->getRecoveryProcedure()->isRtsFrameRetryLimitReached(packet, dataHeader);
        }
        else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(protectedHeader)) {
            edca->getMgmtAndNonQoSRecoveryProcedure()->rtsFrameTransmissionFailed(mgmtHeader, edcaf->getStationRetryCounters());
            retryLimitReached = edca->getMgmtAndNonQoSRecoveryProcedure()->isRtsFrameRetryLimitReached(packet, mgmtHeader);
        }
        else
            throw cRuntimeError("Unknown frame"); // TODO QoSDataFrame, NonQoSDataFrame
        auto addbaRequest = findFragmentedActionContext<Ieee80211AddbaRequest>(packet);
        bool staleAddbaRequest = addbaRequest != nullptr && originatorBlockAckAgreementHandler != nullptr && !originatorBlockAckAgreementHandler->isAddbaRequestPending(packet, addbaRequest);
        if (retryLimitReached || staleAddbaRequest) {
            if (retryLimitReached) {
                if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(protectedHeader))
                    edcaf->getRecoveryProcedure()->retryLimitReached(packet, dataHeader);
                else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(protectedHeader))
                    edca->getMgmtAndNonQoSRecoveryProcedure()->rtsFrameRetryLimitReached(packet, mgmtHeader);
                else ; // TODO nonqos data
            }
            else
                edca->getMgmtAndNonQoSRecoveryProcedure()->discardRtsFrame(addbaRequest);
            edcaf->getInProgressFrames()->dropFrame(packet);
            processDroppedBlockAckSetupFrame(packet);
            processDroppedBlockAckTeardownFrame(packet);
            edcaf->getAckHandler()->dropFrame(protectedHeader);
            EV_INFO << "Dropping RTS/CTS protected frame " << packet->getName() << (retryLimitReached ? ", because retry limit is reached.\n" : ", because its ADDBA transaction is no longer pending.\n");
            PacketDropDetails details;
            details.setReason(retryLimitReached ? RETRY_LIMIT_REACHED : OTHER_PACKET_DROP);
            if (retryLimitReached)
                details.setLimit(-1); // TODO
            emit(packetDroppedSignal, packet, &details);
            if (retryLimitReached) {
                emit(linkBrokenSignal, packet);
                if (dynamicPtrCast<const Ieee80211MgmtHeader>(protectedHeader))
                    mac->notifyFrameTransmission(packet, IFrameTransmissionCallback::Status::RETRY_LIMIT_REACHED);
            }
        }
    }
    else
        throw cRuntimeError("Hcca is unimplemented!");
}

void Hcf::originatorProcessTransmittedFrame(Packet *packet)
{
    Enter_Method("originatorProcessTransmittedFrame");
    EV_INFO << "Processing transmitted frame " << packet->getName() << " as originator in frame sequence.\n";
    auto transmittedHeader = packet->peekAtFront<Ieee80211MacHeader>();
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        edcaf->emit(packetSentToPeerSignal, packet);
        AccessCategory ac = edcaf->getAccessCategory();
        if (transmittedHeader->getReceiverAddress().isMulticast()) {
            edcaf->getRecoveryProcedure()->multicastFrameTransmitted();
            if (auto transmittedDataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(transmittedHeader))
                edcaf->getInProgressFrames()->dropFrame(packet);
        }
        else if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(transmittedHeader))
            originatorProcessTransmittedDataFrame(packet, dataHeader, ac);
        else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(transmittedHeader))
            originatorProcessTransmittedManagementFrame(packet, mgmtHeader, ac);
        else // TODO Ieee80211ControlFrame
            originatorProcessTransmittedControlFrame(transmittedHeader, ac);
    }
    else if (hcca->isOwning())
        throw cRuntimeError("Hcca is unimplemented");
    else
        throw cRuntimeError("Frame transmitted but channel owner not found");
}

void Hcf::originatorProcessTransmittedDataFrame(Packet *packet, const Ptr<const Ieee80211DataHeader>& dataHeader, AccessCategory ac)
{
    auto edcaf = edca->getEdcaf(ac);
    edcaf->getAckHandler()->processTransmittedDataOrMgmtFrame(dataHeader);
    if (dataHeader->getAckPolicy() == NO_ACK)
        edcaf->getInProgressFrames()->dropFrame(packet);
}

void Hcf::originatorProcessTransmittedManagementFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& mgmtHeader, AccessCategory ac)
{
    auto edcaf = edca->getEdcaf(ac);
    if (originatorAckPolicy->isAckNeeded(mgmtHeader))
        edcaf->getAckHandler()->processTransmittedDataOrMgmtFrame(mgmtHeader);
    if (auto addbaReq = findFragmentedActionContext<Ieee80211AddbaRequest>(packet)) {
        if (originatorBlockAckAgreementHandler)
            originatorBlockAckAgreementHandler->processTransmittedAddbaReq(packet, addbaReq, originatorBlockAckAgreementPolicy, this);
    }
    else if (findFragmentedActionContext<Ieee80211AddbaResponse>(packet))
        ; // Recipient agreement was established when the successful response was formed.
    else if (auto delba = findFragmentedActionContext<Ieee80211Delba>(packet)) {
        if (delba->getInitiator()) {
            bool wasPending = originatorBlockAckAgreementHandler->isAddbaResponsePending(delba->getReceiverAddress(), delba->getTid());
            auto agreement = originatorBlockAckAgreementHandler->processTransmittedDelba(packet, this);
            if (wasPending)
                rebuildPendingFrameEligibility();
            if (agreement != nullptr && agreement->getIsAddbaResponseReceived())
                emit(blockAckAgreementDeletedSignal, agreement.get());
        }
        else {
            auto agreement = recipientBlockAckAgreementHandler->processTransmittedDelba(delba);
            if (agreement != nullptr) {
                // IEEE Std 802.11-2024, 10.25.4 and 11.5.3.5: recipient
                // resources are released whether the recipient transmitted or
                // received DELBA. The reorder window is such a resource.
                recipientDataService->resetBlockAckReordering(delba->getTid(), delba->getReceiverAddress());
                emit(blockAckAgreementDeletedSignal, agreement.get());
            }
        }
    }
    else ; // TODO other mgmt frames if needed
}

void Hcf::originatorProcessTransmittedControlFrame(const Ptr<const Ieee80211MacHeader>& controlHeader, AccessCategory ac)
{
    auto edcaf = edca->getEdcaf(ac);
    if (auto blockAckReq = dynamicPtrCast<const Ieee80211BlockAckReq>(controlHeader))
        edcaf->getAckHandler()->processTransmittedBlockAckReq(blockAckReq);
    else if (auto rtsFrame = dynamicPtrCast<const Ieee80211RtsFrame>(controlHeader))
        rtsProcedure->processTransmittedRts(rtsFrame);
    else
        throw cRuntimeError("Unknown control frame");
}

void Hcf::originatorProcessFailedFrame(Packet *failedPacket)
{
    Enter_Method("originatorProcessFailedFrame");
    auto failedHeader = failedPacket->peekAtFront<Ieee80211MacHeader>();
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        bool retryLimitReached = false;
        if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(failedHeader)) {
            ASSERT(dataHeader->getAckPolicy() == NORMAL_ACK);
            EV_INFO << "Data frame transmission failed\n";
            edcaf->getRecoveryProcedure()->dataFrameTransmissionFailed(failedPacket, dataHeader);
            retryLimitReached = edcaf->getRecoveryProcedure()->isRetryLimitReached(failedPacket, dataHeader);
            if (dataAndMgmtRateControl) {
                int retryCount = edcaf->getRecoveryProcedure()->getRetryCount(failedPacket, dataHeader);
                dataAndMgmtRateControl->frameTransmitted(failedPacket, retryCount, false, retryLimitReached);
            }
            edcaf->getAckHandler()->processFailedFrame(dataHeader);
        }
        else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(failedHeader)) { // TODO + NonQoS frames
            EV_INFO << "Management frame transmission failed\n";
            edca->getMgmtAndNonQoSRecoveryProcedure()->dataOrMgmtFrameTransmissionFailed(failedPacket, mgmtHeader, edcaf->getStationRetryCounters());
            retryLimitReached = edca->getMgmtAndNonQoSRecoveryProcedure()->isRetryLimitReached(failedPacket, mgmtHeader);
            if (dataAndMgmtRateControl) {
                int retryCount = edca->getMgmtAndNonQoSRecoveryProcedure()->getRetryCount(failedPacket, mgmtHeader);
                dataAndMgmtRateControl->frameTransmitted(failedPacket, retryCount, false, retryLimitReached);
            }
            edcaf->getAckHandler()->processFailedFrame(mgmtHeader);
        }
        else if (auto blockAckReq = dynamicPtrCast<const Ieee80211BlockAckReq>(failedHeader)) {
            edcaf->getAckHandler()->processFailedBlockAckReq(blockAckReq);
            return;
        }
        else
            throw cRuntimeError("Unknown frame"); // TODO qos, nonqos
        auto addbaRequest = findFragmentedActionContext<Ieee80211AddbaRequest>(failedPacket);
        bool staleAddbaRequest = addbaRequest != nullptr && originatorBlockAckAgreementHandler != nullptr && !originatorBlockAckAgreementHandler->isAddbaRequestPending(failedPacket, addbaRequest);
        if (retryLimitReached || staleAddbaRequest) {
            if (retryLimitReached) {
                if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(failedHeader))
                    edcaf->getRecoveryProcedure()->retryLimitReached(failedPacket, dataHeader);
                else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(failedHeader))
                    edca->getMgmtAndNonQoSRecoveryProcedure()->retryLimitReached(failedPacket, mgmtHeader);
            }
            else
                edca->getMgmtAndNonQoSRecoveryProcedure()->discardFrame(failedPacket, addbaRequest);
            edcaf->getInProgressFrames()->dropFrame(failedPacket);
            processDroppedBlockAckSetupFrame(failedPacket);
            processDroppedBlockAckTeardownFrame(failedPacket);
            edcaf->getAckHandler()->dropFrame(dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(failedHeader));
            EV_INFO << "Dropping frame " << failedPacket->getName() << (retryLimitReached ? ", because retry limit is reached.\n" : ", because its ADDBA transaction is no longer pending.\n");
            PacketDropDetails details;
            details.setReason(retryLimitReached ? RETRY_LIMIT_REACHED : OTHER_PACKET_DROP);
            if (retryLimitReached)
                details.setLimit(-1); // TODO
            emit(packetDroppedSignal, failedPacket, &details);
            if (retryLimitReached) {
                emit(linkBrokenSignal, failedPacket);
                if (dynamicPtrCast<const Ieee80211MgmtHeader>(failedHeader))
                    mac->notifyFrameTransmission(failedPacket, IFrameTransmissionCallback::Status::RETRY_LIMIT_REACHED);
            }
        }
        else {
            EV_INFO << "Retrying frame " << failedPacket->getName() << ".\n";
            auto h = failedPacket->removeAtFront<Ieee80211DataOrMgmtHeader>();
            h->setRetry(true);
            failedPacket->insertAtFront(h);
        }
    }
    else
        throw cRuntimeError("Hcca is unimplemented!");
}

void Hcf::originatorProcessReceivedFrame(Packet *receivedPacket, Packet *lastTransmittedPacket)
{
    Enter_Method("originatorProcessReceivedFrame");
    EV_INFO << "Processing received frame " << receivedPacket->getName() << " as originator in frame sequence.\n";
    emit(packetReceivedFromPeerSignal, receivedPacket);
    auto receivedHeader = receivedPacket->peekAtFront<Ieee80211MacHeader>();
    auto lastTransmittedHeader = lastTransmittedPacket->peekAtFront<Ieee80211MacHeader>();
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        AccessCategory ac = edcaf->getAccessCategory();
        if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(receivedHeader))
            originatorProcessReceivedDataFrame(dataHeader, lastTransmittedHeader, ac);
        else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(receivedHeader))
            originatorProcessReceivedManagementFrame(mgmtHeader, lastTransmittedHeader, ac);
        else
            originatorProcessReceivedControlFrame(receivedPacket, receivedHeader, lastTransmittedPacket, lastTransmittedHeader, ac);
    }
    else
        throw cRuntimeError("Hcca is unimplemented!");
}

void Hcf::originatorProcessReceivedManagementFrame(const Ptr<const Ieee80211MgmtHeader>& header, const Ptr<const Ieee80211MacHeader>& lastTransmittedHeader, AccessCategory ac)
{
    throw cRuntimeError("Unknown management frame");
}

void Hcf::originatorProcessReceivedControlFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, Packet *lastTransmittedPacket, const Ptr<const Ieee80211MacHeader>& lastTransmittedHeader, AccessCategory ac)
{
    auto edcaf = edca->getEdcaf(ac);
    if (auto ackFrame = dynamicPtrCast<const Ieee80211AckFrame>(header)) {
        if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(lastTransmittedHeader)) {
            if (dataAndMgmtRateControl) {
                int retryCount;
                if (dataHeader->getRetry())
                    retryCount = edcaf->getRecoveryProcedure()->getRetryCount(lastTransmittedPacket, dataHeader);
                else
                    retryCount = 0;
                dataAndMgmtRateControl->frameTransmitted(lastTransmittedPacket, retryCount, true, false);
            }
            edcaf->getRecoveryProcedure()->ackFrameReceived(lastTransmittedPacket, dataHeader);
        }
        else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(lastTransmittedHeader)) {
            if (dataAndMgmtRateControl) {
                int retryCount = edca->getMgmtAndNonQoSRecoveryProcedure()->getRetryCount(lastTransmittedPacket, mgmtHeader);
                dataAndMgmtRateControl->frameTransmitted(lastTransmittedPacket, retryCount, true, false);
            }
            edca->getMgmtAndNonQoSRecoveryProcedure()->ackFrameReceived(lastTransmittedPacket, mgmtHeader, edcaf->getStationRetryCounters());
        }
        else
            throw cRuntimeError("Unknown frame"); // TODO qos, nonqos frame
        auto lastTransmittedDataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(lastTransmittedHeader);
        edcaf->getAckHandler()->processReceivedAck(ackFrame, lastTransmittedDataOrMgmtHeader);
        if (auto delba = findFragmentedActionContext<Ieee80211Delba>(lastTransmittedPacket)) {
            if (delba->getInitiator() && originatorBlockAckAgreementHandler != nullptr && originatorBlockAckAgreementHandler->processAcknowledgedDelba(lastTransmittedPacket, this))
                rebuildPendingFrameEligibility();
        }
        edcaf->getInProgressFrames()->dropFrame(lastTransmittedPacket);
        edcaf->getAckHandler()->dropFrame(lastTransmittedDataOrMgmtHeader);
        if (dynamicPtrCast<const Ieee80211MgmtHeader>(lastTransmittedHeader))
            mac->notifyFrameTransmission(lastTransmittedPacket, IFrameTransmissionCallback::Status::ACKNOWLEDGED);
        if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(lastTransmittedHeader)) {
            if (originatorBlockAckAgreementHandler) {
                bool wasPending = originatorBlockAckAgreementHandler->isAddbaResponsePending(dataHeader->getReceiverAddress(), dataHeader->getTid());
                auto obsoleteTeardownTransactionId = originatorBlockAckAgreementHandler->processAcknowledgedDataFrame(lastTransmittedPacket, dataHeader, originatorBlockAckAgreementPolicy, this);
                if (obsoleteTeardownTransactionId != 0)
                    cancelAddbaTransaction(obsoleteTeardownTransactionId, nullptr);
                if (!wasPending && originatorBlockAckAgreementHandler->isAddbaResponsePending(dataHeader->getReceiverAddress(), dataHeader->getTid()))
                    rebuildPendingFrameEligibility();
            }
        }
    }
    else if (auto blockAck = dynamicPtrCast<const Ieee80211BasicBlockAck>(header)) {
        EV_INFO << "BasicBlockAck has arrived" << std::endl;
        edcaf->getRecoveryProcedure()->blockAckFrameReceived();
        auto ackedSeqAndFragNums = edcaf->getAckHandler()->processReceivedBlockAck(blockAck);
        if (originatorBlockAckAgreementHandler)
            originatorBlockAckAgreementHandler->processReceivedBlockAck(blockAck, this);
        EV_TRACE << "It has acknowledged the following frames:" << std::endl;
        for (auto it : ackedSeqAndFragNums)
            EV_TRACE << "   sequenceNumber = " << it.second.second.getSequenceNumber() << ", fragmentNumber = " << (int)it.second.second.getFragmentNumber() << std::endl;
        edcaf->getInProgressFrames()->dropFrames(ackedSeqAndFragNums);
        edcaf->getAckHandler()->dropFrames(ackedSeqAndFragNums);
    }
    else if (dynamicPtrCast<const Ieee80211RtsFrame>(header))
        ; // void
    else if (dynamicPtrCast<const Ieee80211CtsFrame>(header))
        edcaf->getRecoveryProcedure()->ctsFrameReceived();
    else if (header->getType() == ST_DATA_WITH_QOS)
        ; // void
    else if (dynamicPtrCast<const Ieee80211BasicBlockAckReq>(header))
        ; // void
    else
        throw cRuntimeError("Unknown control frame");
}

void Hcf::originatorProcessReceivedDataFrame(const Ptr<const Ieee80211DataHeader>& header, const Ptr<const Ieee80211MacHeader>& lastTransmittedHeader, AccessCategory ac)
{
    throw cRuntimeError("Unknown data frame");
}

bool Hcf::hasFrameToTransmit(AccessCategory ac)
{
    auto edcaf = edca->getEdcaf(ac);
    if (edcaf)
        return numEligiblePendingFrames[ac] != 0 || edcaf->getInProgressFrames()->hasEligibleInProgressFrames();
    else
        throw cRuntimeError("Hcca is unimplemented");
}

bool Hcf::hasFrameToTransmit()
{
    auto edcaf = edca->getChannelOwner();
    if (edcaf)
        return numEligiblePendingFrames[edcaf->getAccessCategory()] != 0 || edcaf->getInProgressFrames()->hasEligibleInProgressFrames();
    else
        throw cRuntimeError("Hcca is unimplemented");
}

void Hcf::requestEligibleChannelAccess()
{
    for (int ac = 0; ac < edca->getNumEdcafs(); ac++) {
        auto accessCategory = AccessCategory(ac);
        if (hasFrameToTransmit(accessCategory))
            edca->requestChannelAccess(accessCategory, this);
    }
}

void Hcf::resumeEligibleChannelAccess()
{
    if (edca->getChannelOwner() == nullptr && !frameSequenceHandler->isSequenceRunning())
        requestEligibleChannelAccess();
}

void Hcf::sendUp(const std::vector<Packet *>& completeFrames)
{
    for (auto frame : completeFrames)
        mac->sendUpFrame(frame);
}

void Hcf::transmitFrame(Packet *packet, simtime_t ifs)
{
    Enter_Method("transmitFrame");
    auto channelOwner = edca->getChannelOwner();
    if (channelOwner) {
        auto header = packet->peekAtFront<Ieee80211MacHeader>();
        auto txop = channelOwner->getTxopProcedure();
        if (auto dataFrame = dynamicPtrCast<const Ieee80211DataHeader>(header)) {
            OriginatorBlockAckAgreement *agreement = nullptr;
            if (originatorBlockAckAgreementHandler)
                agreement = originatorBlockAckAgreementHandler->getAgreement(dataFrame->getReceiverAddress(), dataFrame->getTid());
            auto ackPolicy = originatorAckPolicy->computeAckPolicy(packet, dataFrame, agreement);
            auto dataHeader = packet->removeAtFront<Ieee80211DataHeader>();
            dataHeader->setAckPolicy(ackPolicy);
            packet->insertAtFront(dataHeader);
        }
        auto mode = rateSelection->computeMode(packet, header, txop);
        setFrameMode(packet, header, mode);
        emit(IRateSelection::datarateSelectedSignal, mode->getDataMode()->getNetBitrate().get<bps>(), packet);
        EV_DEBUG << "Datarate for " << packet->getName() << " is set to " << mode->getDataMode()->getNetBitrate() << ".\n";
        if (txop->getProtectionMechanism() == TxopProcedure::ProtectionMechanism::SINGLE_PROTECTION) {
            auto pendingPacket = channelOwner->getInProgressFrames()->getPendingFrameFor(packet);
            const auto& pendingHeader = pendingPacket == nullptr ? nullptr : pendingPacket->peekAtFront<Ieee80211DataOrMgmtHeader>();
            auto duration = singleProtectionMechanism->computeDurationField(packet, header, pendingPacket, pendingHeader, txop, recipientAckPolicy);
            auto header = packet->removeAtFront<Ieee80211MacHeader>();
            header->setDurationField(duration);
            EV_DEBUG << "Duration for " << packet->getName() << " is set to " << duration << " s.\n";
            packet->insertAtFront(header);
        }
        else if (txop->getProtectionMechanism() == TxopProcedure::ProtectionMechanism::MULTIPLE_PROTECTION)
            throw cRuntimeError("Multiple protection is unsupported");
        else
            throw cRuntimeError("Undefined protection mechanism");
        tx->transmitFrame(packet, packet->peekAtFront<Ieee80211MacHeader>(), ifs, this);
    }
    else
        throw cRuntimeError("Hcca is unimplemented");
}

void Hcf::transmitControlResponseFrame(Packet *responsePacket, const Ptr<const Ieee80211MacHeader>& responseHeader, Packet *receivedPacket, const Ptr<const Ieee80211MacHeader>& receivedHeader)
{
    Enter_Method("transmitControlResponseFrame");
    responsePacket->insertAtBack(makeShared<Ieee80211MacTrailer>());
    const IIeee80211Mode *responseMode = nullptr;
    if (auto rtsFrame = dynamicPtrCast<const Ieee80211RtsFrame>(receivedHeader))
        responseMode = rateSelection->computeResponseCtsFrameMode(receivedPacket, rtsFrame);
    else if (auto blockAckReq = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(receivedHeader))
        responseMode = rateSelection->computeResponseBlockAckFrameMode(receivedPacket, blockAckReq);
    else if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(receivedHeader))
        responseMode = rateSelection->computeResponseAckFrameMode(receivedPacket, dataOrMgmtHeader);
    else
        throw cRuntimeError("Unknown received frame type");
    setFrameMode(responsePacket, responseHeader, responseMode);
    emit(IRateSelection::datarateSelectedSignal, responseMode->getDataMode()->getNetBitrate().get<bps>(), responsePacket);
    EV_DEBUG << "Datarate for " << responsePacket->getName() << " is set to " << responseMode->getDataMode()->getNetBitrate() << ".\n";
    tx->transmitFrame(responsePacket, responseHeader, modeSet->getSifsTime(), this);
    delete responsePacket;
}

void Hcf::recipientProcessTransmittedControlResponseFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    emit(packetSentToPeerSignal, packet);
    if (auto ctsFrame = dynamicPtrCast<const Ieee80211CtsFrame>(header))
        ctsProcedure->processTransmittedCts(ctsFrame);
    else if (auto blockAck = dynamicPtrCast<const Ieee80211BlockAck>(header)) {
        if (recipientBlockAckProcedure)
            recipientBlockAckProcedure->processTransmittedBlockAck(blockAck);
    }
    else if (auto ackFrame = dynamicPtrCast<const Ieee80211AckFrame>(header))
        recipientAckProcedure->processTransmittedAck(ackFrame);
    else
        throw cRuntimeError("Unknown control response frame");
}

void Hcf::processMgmtFrame(Packet *mgmtPacket, const Ptr<const Ieee80211MgmtHeader>& mgmtHeader)
{
    Enter_Method("processMgmtFrame");
    mgmtPacket->insertAtBack(makeShared<Ieee80211MacTrailer>());
    processUpperFrame(mgmtPacket, mgmtHeader);
}

void Hcf::setFrameMode(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, const IIeee80211Mode *mode) const
{
    ASSERT(mode != nullptr);
    packet->addTagIfAbsent<Ieee80211ModeReq>()->setMode(mode);
}

bool Hcf::isReceptionInProgress()
{
    return rx->isReceptionInProgress();
}

bool Hcf::isForUs(const Ptr<const Ieee80211MacHeader>& header) const
{
    return header->getReceiverAddress() == mac->getAddress() || (header->getReceiverAddress().isMulticast() && !isSentByUs(header));
}

bool Hcf::isSentByUs(const Ptr<const Ieee80211MacHeader>& header) const
{
    // FIXME
    // Check the roles of the Addr3 field when aggregation is applied
    // Table 8-19—Address field contents
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header))
        return dataOrMgmtHeader->getAddress3() == mac->getAddress();
    else
        return false;
}

void Hcf::corruptedFrameReceived()
{
    Enter_Method("corruptedFrameReceived");
    if (frameSequenceHandler->isSequenceRunning() && !startRxTimer->isScheduled()) {
        frameSequenceHandler->handleStartRxTimeout();
    }
    else
        EV_DEBUG << "Ignoring received corrupt frame.\n";
}

Hcf::~Hcf()
{
    // Callback pointers are stored by child queues, which are destroyed with
    // this compound module. Traversing edca here may reach deleted children.
    cancelAndDelete(startRxTimer);
    cancelAndDelete(inactivityTimer);
    cancelAndDelete(addbaResponseTimer);
    delete recipientAckProcedure;
    delete ctsProcedure;
    delete rtsProcedure;
    delete originatorBlockAckAgreementHandler;
    delete recipientBlockAckAgreementHandler;
    delete originatorBlockAckProcedure;
    delete recipientBlockAckProcedure;
    delete frameSequenceHandler;
}

} // namespace ieee80211
} // namespace inet
