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
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckProcedure.h"
#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"
#include "inet/linklayer/ieee80211/mac/framesequence/HcfFs.h"
#include "inet/linklayer/ieee80211/mac/recipient/RecipientAckProcedure.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

simsignal_t Hcf::edcaCollisionDetectedSignal = cComponent::registerSignal("edcaCollisionDetected");
simsignal_t Hcf::blockAckAgreementAddedSignal = cComponent::registerSignal("blockAckAgreementAdded");
simsignal_t Hcf::blockAckAgreementDeletedSignal = cComponent::registerSignal("blockAckAgreementDeleted");

Define_Module(Hcf);

void Hcf::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mac = check_and_cast<Ieee80211Mac *>(getContainingNicModule(this)->getSubmodule("mac"));
        startRxTimer = new cMessage("startRxTimeout");
        inactivityTimer = new cMessage("blockAckInactivityTimer");
        edca = check_and_cast<Edca *>(getSubmodule("edca"));
        hcca = check_and_cast<Hcca *>(getSubmodule("hcca"));
        tx = check_and_cast<ITx *>(getModuleByPath(par("txModule")));
        rx = check_and_cast<IRx *>(getModuleByPath(par("rxModule")));
        dataAndMgmtRateControl = dynamic_cast<IRateControl*>(getSubmodule("rateControl"));
        originatorBlockAckAgreementPolicy = dynamic_cast<IOriginatorBlockAckAgreementPolicy*>(getSubmodule("originatorBlockAckAgreementPolicy"));
        recipientBlockAckAgreementPolicy = dynamic_cast<IRecipientBlockAckAgreementPolicy*>(getSubmodule("recipientBlockAckAgreementPolicy"));
        rateSelection = check_and_cast<IQosRateSelection *>(getSubmodule("rateSelection"));
        frameSequenceHandler = new FrameSequenceHandler();
        originatorDataService = check_and_cast<IOriginatorMacDataService *>(getSubmodule(("originatorMacDataService")));
        recipientDataService = check_and_cast<IRecipientQosMacDataService*>(getSubmodule("recipientMacDataService"));
        originatorAckPolicy = check_and_cast<IOriginatorQoSAckPolicy*>(getSubmodule("originatorAckPolicy"));
        recipientAckPolicy = check_and_cast<IRecipientQosAckPolicy*>(getSubmodule("recipientAckPolicy"));
        singleProtectionMechanism = check_and_cast<SingleProtectionMechanism*>(getSubmodule("singleProtectionMechanism"));
        rtsProcedure = new RtsProcedure();
        rtsPolicy = check_and_cast<IRtsPolicy*>(getSubmodule("rtsPolicy"));
        recipientAckProcedure = new RecipientAckProcedure();
        ctsProcedure = new CtsProcedure();
        ctsPolicy = check_and_cast<ICtsPolicy*>(getSubmodule("ctsPolicy"));
        if (originatorBlockAckAgreementPolicy && recipientBlockAckAgreementPolicy) {
            recipientBlockAckAgreementHandler = new RecipientBlockAckAgreementHandler();
            originatorBlockAckAgreementHandler = new OriginatorBlockAckAgreementHandler();
            originatorBlockAckProcedure = new OriginatorBlockAckProcedure();
            recipientBlockAckProcedure = new RecipientBlockAckProcedure();
        }
    }
}

void Hcf::forEachChild(cVisitor *v)
{
    cSimpleModule::forEachChild(v);
    if (frameSequenceHandler != nullptr && frameSequenceHandler->getContext() != nullptr)
        v->visit(const_cast<FrameSequenceContext *>(frameSequenceHandler->getContext()));
}

void Hcf::handleMessage(cMessage* msg)
{
    if (msg == startRxTimer) {
        if (!isReceptionInProgress()) {
            frameSequenceHandler->handleStartRxTimeout();
            updateDisplayString();
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
    else
        throw cRuntimeError("Unknown msg type");
}

void Hcf::updateDisplayString()
{
    if (frameSequenceHandler->isSequenceRunning()) {
        auto history = frameSequenceHandler->getFrameSequence()->getHistory();
        getDisplayString().setTagArg("tt", 0, ("Fs: " + history).c_str());
        if (history.length() > 32) {
            history.erase(history.begin(), history.end() - 32);
            history = "..." + history;
        }
        getDisplayString().setTagArg("t", 0, ("Fs: " + history).c_str());
    }
    else {
        getDisplayString().removeTag("t");
        getDisplayString().removeTag("tt");
    }
}

void Hcf::processUpperFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    Enter_Method_Silent("processUpperFrame(%s)", packet->getName());
    EV_INFO << "Processing upper frame: " << packet->getName() << endl;
    // TODO:
    // A QoS STA should send individually addressed Management frames that are addressed to a non-QoS STA
    // using the access category AC_BE and shall send all other management frames using the access category
    // AC_VO. A QoS STA that does not send individually addressed Management frames that are addressed to a
    // non-QoS STA using the access category AC_BE shall send them using the access category AC_VO.
    // Management frames are exempted from any and all restrictions on transmissions arising from admission
    // control procedures.
    AccessCategory ac = AccessCategory(-1);
    if (dynamicPtrCast<const Ieee80211MgmtHeader>(header)) // TODO: + non-QoS frames
        ac = AccessCategory::AC_VO;
    else if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header))
        ac = edca->classifyFrame(dataHeader);
    else
        throw cRuntimeError("Unknown message type");
    EV_INFO << "The upper frame has been classified as a " << printAccessCategory(ac) << " frame." << endl;
    auto pendingQueue = edca->getEdcaf(ac)->getPendingQueue();
    pendingQueue->pushPacket(packet);
    if (!pendingQueue->isEmpty()) {
        auto edcaf = edca->getChannelOwner();
        if (edcaf == nullptr || edcaf->getAccessCategory() != ac) {
            EV_DETAIL << "Requesting channel for access category " << printAccessCategory(ac) << endl;
            edca->requestChannelAccess(ac, this);
        }
    }
}

void Hcf::scheduleStartRxTimer(simtime_t timeout)
{
    Enter_Method_Silent();
    scheduleAfter(timeout, startRxTimer);
}

void Hcf::scheduleInactivityTimer(simtime_t timeout)
{
    Enter_Method_Silent();
    if (inactivityTimer->isScheduled())
        cancelEvent(inactivityTimer);
    scheduleAfter(timeout, inactivityTimer);
}

void Hcf::processLowerFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    Enter_Method_Silent("processLowerFrame(%s)", packet->getName());
    EV_INFO << "Processing lower frame: " << packet->getName() << endl;
    auto edcaf = edca->getChannelOwner();
    if (edcaf && frameSequenceHandler->isSequenceRunning()) {
        // TODO: always call processResponse?
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

void Hcf::channelGranted(IChannelAccess* channelAccess)
{
    Enter_Method_Silent("channelGranted");
    auto edcaf = check_and_cast<Edcaf*>(channelAccess);
    if (edcaf) {
        AccessCategory ac = edcaf->getAccessCategory();
        EV_DETAIL << "Channel access granted to the " << printAccessCategory(ac) << " queue" << std::endl;
        edcaf->getTxopProcedure()->startTxop(ac);
        auto internallyCollidedEdcafs = edca->getInternallyCollidedEdcafs();
        if (internallyCollidedEdcafs.size() > 0) {
            EV_INFO << "Internal collision happened with the following queues:" << std::endl;
            handleInternalCollision(internallyCollidedEdcafs);
            emit(edcaCollisionDetectedSignal, (unsigned long)internallyCollidedEdcafs.size());
        }
        startFrameSequence(ac);
    }
    else
        throw cRuntimeError("Channel access granted but channel owner not found!");
}

FrameSequenceContext* Hcf::buildContext(AccessCategory ac)
{
    auto edcaf = edca->getEdcaf(ac);
    auto qosContext = new QoSContext(originatorAckPolicy, originatorBlockAckProcedure, originatorBlockAckAgreementHandler, edcaf->getTxopProcedure());
    return new FrameSequenceContext(mac->getAddress(), modeSet, edcaf->getInProgressFrames(), rtsProcedure, rtsPolicy, nullptr, qosContext);
}

void Hcf::startFrameSequence(AccessCategory ac)
{
    frameSequenceHandler->startFrameSequence(new HcfFs(), buildContext(ac), this);
    emit(IFrameSequenceHandler::frameSequenceStartedSignal, frameSequenceHandler->getContext());
    updateDisplayString();
}

void Hcf::handleInternalCollision(std::vector<Edcaf*> internallyCollidedEdcafs)
{
    for (auto edcaf : internallyCollidedEdcafs) {
        AccessCategory ac = edcaf->getAccessCategory();
        auto dataRecoveryProcedure = edcaf->getRecoveryProcedure();
        Packet *internallyCollidedFrame = edcaf->getInProgressFrames()->getFrameToTransmit();
        auto internallyCollidedHeader = internallyCollidedFrame->peekAtFront<Ieee80211DataOrMgmtHeader>();
        EV_INFO << printAccessCategory(ac) << " (" << internallyCollidedFrame->getName() << ")" << endl;
        bool retryLimitReached = false;
        if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(internallyCollidedHeader)) { // TODO: QoSDataFrame
            dataRecoveryProcedure->dataFrameTransmissionFailed(internallyCollidedFrame, dataHeader);
            retryLimitReached = dataRecoveryProcedure->isRetryLimitReached(internallyCollidedFrame, dataHeader);
        }
        else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(internallyCollidedHeader)) {
            ASSERT(ac == AccessCategory::AC_BE);
            edca->getMgmtAndNonQoSRecoveryProcedure()->dataOrMgmtFrameTransmissionFailed(internallyCollidedFrame, mgmtHeader, edca->getEdcaf(AccessCategory::AC_BE)->getStationRetryCounters());
            retryLimitReached = edca->getMgmtAndNonQoSRecoveryProcedure()->isRetryLimitReached(internallyCollidedFrame, mgmtHeader);
        }
        else // TODO: + NonQoSDataFrame
            throw cRuntimeError("Unknown frame");
        if (retryLimitReached) {
            EV_DETAIL << "The frame has reached its retry limit. Dropping it" << std::endl;
            if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(internallyCollidedHeader))
                dataRecoveryProcedure->retryLimitReached(internallyCollidedFrame, dataHeader);
            else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(internallyCollidedHeader))
                edca->getMgmtAndNonQoSRecoveryProcedure()->retryLimitReached(internallyCollidedFrame, mgmtHeader);
            else ; // TODO: + NonQoSDataFrame
            edcaf->getInProgressFrames()->dropFrame(internallyCollidedFrame);
            edcaf->getAckHandler()->dropFrame(internallyCollidedHeader);
            PacketDropDetails details;
            details.setReason(RETRY_LIMIT_REACHED);
            details.setLimit(-1); // TODO:
            emit(packetDroppedSignal, internallyCollidedFrame, &details);
            emit(linkBrokenSignal, internallyCollidedFrame);
            if (hasFrameToTransmit(ac))
                edcaf->requestChannel(this);
        }
        else
            edcaf->requestChannel(this);
    }
}

/*
 * TODO:  If a PHY-RXSTART.indication primitive does not occur during the ACKTimeout interval,
 * the STA concludes that the transmission of the MPDU has failed, and this STA shall invoke its
 * backoff procedure **upon expiration of the ACKTimeout interval**.
 */

void Hcf::frameSequenceFinished()
{
    Enter_Method_Silent("frameSequenceFinished");
    emit(IFrameSequenceHandler::frameSequenceFinishedSignal, frameSequenceHandler->getContext());
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        bool startContention = hasFrameToTransmit(); // TODO: outstanding frame
        edcaf->releaseChannel(this);
        mac->sendDownPendingRadioConfigMsg(); // TODO: review
        edcaf->getTxopProcedure()->endTxop();
        if (startContention)
            edcaf->requestChannel(this);
    }
    else if (hcca->isOwning()) {
        hcca->releaseChannel(this);
        mac->sendDownPendingRadioConfigMsg(); // TODO: review
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
        recipientAckProcedure->processReceivedFrame(packet, dataOrMgmtHeader, check_and_cast<IRecipientAckPolicy*>(recipientAckPolicy), this);
    if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header)) {
        if (dataHeader->getType() == ST_DATA_WITH_QOS && recipientBlockAckAgreementHandler)
            recipientBlockAckAgreementHandler->qosFrameReceived(dataHeader, this);
        sendUp(recipientDataService->dataFrameReceived(packet, dataHeader, recipientBlockAckAgreementHandler));
    }
    else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(header)) {
        sendUp(recipientDataService->managementFrameReceived(packet, mgmtHeader));
        recipientProcessReceivedManagementFrame(mgmtHeader);
        if (dynamicPtrCast<const Ieee80211ActionFrame>(mgmtHeader))
            delete packet;
    }
    else { // TODO: else if (auto ctrlFrame = dynamic_cast<Ieee80211ControlFrame*>(frame))
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

void Hcf::recipientProcessReceivedManagementFrame(const Ptr<const Ieee80211MgmtHeader>& header)
{
    if (recipientBlockAckAgreementHandler && originatorBlockAckAgreementHandler) {
        if (auto addbaRequest = dynamicPtrCast<const Ieee80211AddbaRequest>(header)) {
            recipientBlockAckAgreementHandler->processReceivedAddbaRequest(addbaRequest, recipientBlockAckAgreementPolicy, this);
            auto agreement = recipientBlockAckAgreementHandler->getAgreement(addbaRequest->getTid(), addbaRequest->getTransmitterAddress());
            emit(blockAckAgreementAddedSignal, agreement);
        }
        else if (auto addbaResp = dynamicPtrCast<const Ieee80211AddbaResponse>(header)) {
            originatorBlockAckAgreementHandler->processReceivedAddbaResp(addbaResp, originatorBlockAckAgreementPolicy, this);
            auto agreement = originatorBlockAckAgreementHandler->getAgreement(addbaResp->getTransmitterAddress(), addbaResp->getTid());
            emit(blockAckAgreementAddedSignal, agreement);
        }
        else if (auto delba = dynamicPtrCast<const Ieee80211Delba>(header)) {
            if (delba->getInitiator()) {
                auto agreement = recipientBlockAckAgreementHandler->getAgreement(delba->getTid(), delba->getReceiverAddress());
                emit(blockAckAgreementDeletedSignal, agreement);
                recipientBlockAckAgreementHandler->processReceivedDelba(delba, recipientBlockAckAgreementPolicy);
            }
            else {
                auto agreement = originatorBlockAckAgreementHandler->getAgreement(delba->getReceiverAddress(), delba->getTid());
                emit(blockAckAgreementDeletedSignal, agreement);
                originatorBlockAckAgreementHandler->processReceivedDelba(delba, originatorBlockAckAgreementPolicy);
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
    Enter_Method_Silent("transmissionComplete");
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        frameSequenceHandler->transmissionComplete();
        updateDisplayString();
    }
    else if (hcca->isOwning())
        throw cRuntimeError("Hcca is unimplemented!");
    else
        recipientProcessTransmittedControlResponseFrame(packet, header);
    delete packet;
}

void Hcf::originatorProcessRtsProtectionFailed(Packet *packet)
{
    Enter_Method_Silent("originatorProcessRtsProtectionFailed");
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
            retryLimitReached = edca->getMgmtAndNonQoSRecoveryProcedure()->isRtsFrameRetryLimitReached(packet, dataHeader);
        }
        else
            throw cRuntimeError("Unknown frame"); // TODO: QoSDataFrame, NonQoSDataFrame
        if (retryLimitReached) {
            if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(protectedHeader))
                edcaf->getRecoveryProcedure()->retryLimitReached(packet, dataHeader);
            else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(protectedHeader))
                edca->getMgmtAndNonQoSRecoveryProcedure()->retryLimitReached(packet, mgmtHeader);
            else ; // TODO: nonqos data
            edcaf->getInProgressFrames()->dropFrame(packet);
            edcaf->getAckHandler()->dropFrame(protectedHeader);
            EV_INFO << "Dropping RTS/CTS protected frame " << packet->getName() << ", because retry limit is reached.\n";
            PacketDropDetails details;
            details.setReason(RETRY_LIMIT_REACHED);
            details.setLimit(-1); // TODO:
            emit(packetDroppedSignal, packet, &details);
            emit(linkBrokenSignal, packet);
        }
    }
    else
        throw cRuntimeError("Hcca is unimplemented!");
}

void Hcf::originatorProcessTransmittedFrame(Packet *packet)
{
    Enter_Method_Silent("originatorProcessTransmittedFrame");
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
            originatorProcessTransmittedManagementFrame(mgmtHeader, ac);
        else // TODO: Ieee80211ControlFrame
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
    if (originatorBlockAckAgreementHandler)
        originatorBlockAckAgreementHandler->processTransmittedDataFrame(packet, dataHeader, originatorBlockAckAgreementPolicy, this);
    if (dataHeader->getAckPolicy() == NO_ACK)
        edcaf->getInProgressFrames()->dropFrame(packet);
}

void Hcf::originatorProcessTransmittedManagementFrame(const Ptr<const Ieee80211MgmtHeader>& mgmtHeader, AccessCategory ac)
{
    auto edcaf = edca->getEdcaf(ac);
    if (originatorAckPolicy->isAckNeeded(mgmtHeader))
        edcaf->getAckHandler()->processTransmittedDataOrMgmtFrame(mgmtHeader);
    if (auto addbaReq = dynamicPtrCast<const Ieee80211AddbaRequest>(mgmtHeader)) {
        if (originatorBlockAckAgreementHandler)
            originatorBlockAckAgreementHandler->processTransmittedAddbaReq(addbaReq);
    }
    else if (auto addbaResp = dynamicPtrCast<const Ieee80211AddbaResponse>(mgmtHeader))
        recipientBlockAckAgreementHandler->processTransmittedAddbaResp(addbaResp, this);
    else if (auto delba = dynamicPtrCast<const Ieee80211Delba>(mgmtHeader)) {
        if (delba->getInitiator())
            originatorBlockAckAgreementHandler->processTransmittedDelba(delba);
        else
            recipientBlockAckAgreementHandler->processTransmittedDelba(delba);
    }
    else ; // TODO: other mgmt frames if needed
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
    Enter_Method_Silent("originatorProcessFailedFrame");
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
        else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(failedHeader)) { // TODO: + NonQoS frames
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
            throw cRuntimeError("Unknown frame"); // TODO: qos, nonqos
        if (retryLimitReached) {
            if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(failedHeader))
                edcaf->getRecoveryProcedure()->retryLimitReached(failedPacket, dataHeader);
            else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(failedHeader))
                edca->getMgmtAndNonQoSRecoveryProcedure()->retryLimitReached(failedPacket, mgmtHeader);
            edcaf->getInProgressFrames()->dropFrame(failedPacket);
            edcaf->getAckHandler()->dropFrame(dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(failedHeader));
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
    else
        throw cRuntimeError("Hcca is unimplemented!");
}

void Hcf::originatorProcessReceivedFrame(Packet *receivedPacket, Packet *lastTransmittedPacket)
{
    Enter_Method_Silent("originatorProcessReceivedFrame");
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
                int retryCount = edca->getMgmtAndNonQoSRecoveryProcedure()->getRetryCount(lastTransmittedPacket, dataHeader);
                dataAndMgmtRateControl->frameTransmitted(lastTransmittedPacket, retryCount, true, false);
            }
            edca->getMgmtAndNonQoSRecoveryProcedure()->ackFrameReceived(lastTransmittedPacket, mgmtHeader, edcaf->getStationRetryCounters());
        }
        else
            throw cRuntimeError("Unknown frame"); // TODO: qos, nonqos frame
        auto lastTransmittedDataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(lastTransmittedHeader);
        edcaf->getAckHandler()->processReceivedAck(ackFrame, lastTransmittedDataOrMgmtHeader);
        edcaf->getInProgressFrames()->dropFrame(lastTransmittedPacket);
        edcaf->getAckHandler()->dropFrame(lastTransmittedDataOrMgmtHeader);
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
    auto edcaf = edca->getChannelOwner();
    if (edcaf)
        return !edcaf->getPendingQueue()->isEmpty() || edcaf->getInProgressFrames()->hasInProgressFrames();
    else
        throw cRuntimeError("Hcca is unimplemented");
}

bool Hcf::hasFrameToTransmit()
{
    auto edcaf = edca->getChannelOwner();
    if (edcaf)
        return !edcaf->getPendingQueue()->isEmpty() || edcaf->getInProgressFrames()->hasInProgressFrames();
    else
        throw cRuntimeError("Hcca is unimplemented");
}

void Hcf::sendUp(const std::vector<Packet *>& completeFrames)
{
    for (auto frame : completeFrames) {
        // FIXME: mgmt module does not handle addba req ..
        const auto& header = frame->peekAtFront<Ieee80211DataOrMgmtHeader>();
        if (!dynamicPtrCast<const Ieee80211ActionFrame>(header))
            mac->sendUpFrame(frame);
    }
}

void Hcf::transmitFrame(Packet *packet, simtime_t ifs)
{
    Enter_Method_Silent("transmitFrame");
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
        emit(IRateSelection::datarateSelectedSignal, mode->getDataMode()->getNetBitrate().get(), packet);
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
    Enter_Method_Silent("transmitControlResponseFrame");
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
    emit(IRateSelection::datarateSelectedSignal, responseMode->getDataMode()->getNetBitrate().get(), responsePacket);
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
    Enter_Method_Silent("processMgmtFrame");
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
    // FIXME:
    // Check the roles of the Addr3 field when aggregation is applied
    // Table 8-19—Address field contents
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header))
        return dataOrMgmtHeader->getAddress3() == mac->getAddress();
    else
        return false;
}

void Hcf::corruptedFrameReceived()
{
    Enter_Method_Silent("corruptedFrameReceived");
    if (frameSequenceHandler->isSequenceRunning() && !startRxTimer->isScheduled()) {
        frameSequenceHandler->handleStartRxTimeout();
        updateDisplayString();
    }
    else
        EV_DEBUG << "Ignoring received corrupt frame.\n";
}

Hcf::~Hcf()
{
    cancelAndDelete(startRxTimer);
    cancelAndDelete(inactivityTimer);
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
