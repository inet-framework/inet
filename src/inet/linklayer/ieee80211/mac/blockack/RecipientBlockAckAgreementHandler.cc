//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.h"

#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mac/blockack/Ieee80211BlockAckAgreementTag_m.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/Ieee80211FragmentedActionContextTag.h"

namespace inet {
namespace ieee80211 {

simtime_t RecipientBlockAckAgreementHandler::computeEarliestExpirationTime()
{
    simtime_t earliestTime = SIMTIME_MAX;
    for (auto id : blockAckAgreements) {
        auto agreement = id.second;
        if (!agreement->isInactivityExpired())
            earliestTime = std::min(earliestTime, agreement->getExpirationTime());
    }
    return earliestTime;
}

void RecipientBlockAckAgreementHandler::scheduleInactivityTimer(IBlockAckAgreementHandlerCallback *callback)
{
    simtime_t earliestExpirationTime = computeEarliestExpirationTime();
    if (callback != nullptr)
        callback->scheduleInactivityTimer(BlockAckAgreementRole::RECIPIENT, earliestExpirationTime);
}

// The inactivity timer at a recipient is reset when MPDUs corresponding to the TID for which the Block Ack
// policy is set are received and the Ack Policy subfield in the QoS Control field of that MPDU header is
// Block Ack or Implicit Block Ack Request.
//
void RecipientBlockAckAgreementHandler::qosFrameReceived(const Ptr<const Ieee80211DataHeader>& qosHeader, IBlockAckAgreementHandlerCallback *callback)
{
    if (qosHeader->getAckPolicy() == AckPolicy::BLOCK_ACK) { // TODO + Implicit Block Ack
        Tid tid = qosHeader->getTid();
        MacAddress originatorAddr = qosHeader->getTransmitterAddress();
        auto agreement = getActiveAgreement(tid, originatorAddr);
        if (agreement != nullptr) {
            agreement->calculateExpirationTime();
            scheduleInactivityTimer(callback);
        }
    }
}

// IEEE Std 802.11-2024, 11.5.4: a Basic BlockAckReq for an agreement's TID
// also resets the recipient inactivity timer.
void RecipientBlockAckAgreementHandler::blockAckReqReceived(const Ptr<const Ieee80211BasicBlockAckReq>& blockAckReq, IBlockAckAgreementHandlerCallback *callback)
{
    auto agreement = getActiveAgreement(blockAckReq->getTidInfo(), blockAckReq->getTransmitterAddress());
    if (agreement != nullptr) {
        agreement->calculateExpirationTime();
        scheduleInactivityTimer(callback);
    }
}

bool RecipientBlockAckAgreementHandler::blockAckAgreementExpired(IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback)
{
    // When a timeout of BlockAckTimeout is detected, the STA shall send a DELBA frame to the
    // peer STA with the Reason Code field set to TIMEOUT and shall issue a MLME-DELBA.indication
    // primitive with the ReasonCode parameter having a value of TIMEOUT.
    // The procedure is illustrated in Figure 10-14.
    simtime_t now = simTime();
    bool expired = false;
    for (auto id : blockAckAgreements) {
        auto agreement = id.second;
        if (!agreement->isInactivityExpired() && agreement->getExpirationTime() <= now) {
            agreement->markInactivityExpired();
            MacAddress receiverAddr = id.first.first;
            Tid tid = id.first.second;
            const auto& delba = buildDelba(receiverAddr, tid, 39);
            auto delbaPacket = new Packet("Delba", delba);
            delbaPacket->addTag<Ieee80211BlockAckAgreementTag>()->setGenerationId(agreement->getGenerationId());
            procedureCallback->processMgmtFrame(delbaPacket, delba); // 39 - TIMEOUT see: Table 8-36—Reason codes
            expired = true;
        }
    }
    scheduleInactivityTimer(agreementHandlerCallback);
    return expired;
}

//
// When a timeout of BlockAckTimeout is detected, the STA shall send a DELBA frame to the peer STA with the Reason Code
// field set to TIMEOUT and shall issue a MLME-DELBA.indication primitive with the ReasonCode
// parameter having a value of TIMEOUT. The procedure is illustrated in Figure 10-14.
//
const Ptr<Ieee80211Delba> RecipientBlockAckAgreementHandler::buildDelba(MacAddress receiverAddr, Tid tid, int reasonCode)
{
    auto delba = makeShared<Ieee80211Delba>();
    delba->setReceiverAddress(receiverAddr);
    delba->setInitiator(false);
    delba->setTid(tid);
    delba->setReasonCode(reasonCode);
    return delba;
}

uint64_t RecipientBlockAckAgreementHandler::allocateAgreementGenerationId()
{
    if (nextAgreementGenerationId == 0)
        throw cRuntimeError("Block Ack agreement generation ID exhausted");
    return nextAgreementGenerationId++;
}

const Ptr<Ieee80211AddbaResponse> RecipientBlockAckAgreementHandler::buildAddbaResponse(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, bool accepted)
{
    auto addbaResponse = makeShared<Ieee80211AddbaResponse>();
    addbaResponse->setReceiverAddress(addbaRequest->getTransmitterAddress());
    // IEEE Std 802.11-2024, 9.6.4.2 and 11.5.2.3: the response copies the
    // request's Dialog Token and reports whether the agreement was accepted.
    addbaResponse->setDialogToken(addbaRequest->getDialogToken());
    addbaResponse->setStatusCode(accepted ? 0 : 1); // 1: REFUSED_REASON_UNSPECIFIED
    // The Block Ack Policy subfield is set to 1 for immediate Block Ack and 0 for delayed Block Ack.
    Tid tid = addbaRequest->getTid();
    addbaResponse->setTid(tid);
    addbaResponse->setBlockAckPolicy(!addbaRequest->getBlockAckPolicy() && blockAckAgreementPolicy->delayedBlockAckPolicySupported() ? false : true);
    addbaResponse->setBufferSize(addbaRequest->getBufferSize() <= blockAckAgreementPolicy->getMaximumAllowedBufferSize() ? addbaRequest->getBufferSize() : blockAckAgreementPolicy->getMaximumAllowedBufferSize());
    addbaResponse->setBlockAckTimeoutValue(blockAckAgreementPolicy->getBlockAckTimeoutValue() == 0 ? blockAckAgreementPolicy->getBlockAckTimeoutValue() : addbaRequest->getBlockAckTimeoutValue());
    addbaResponse->setAMsduSupported(blockAckAgreementPolicy->aMsduSupported());
    return addbaResponse;
}

RecipientBlockAckAgreement *RecipientBlockAckAgreementHandler::removeAgreement(MacAddress originatorAddr, Tid tid)
{
    auto agreementId = std::make_pair(originatorAddr, tid);
    lastAddbaResponses.erase(agreementId);
    auto it = blockAckAgreements.find(agreementId);
    if (it != blockAckAgreements.end()) {
        auto agreement = it->second;
        blockAckAgreements.erase(it);
        return agreement;
    }
    return nullptr;
}

RecipientBlockAckAgreement *RecipientBlockAckAgreementHandler::getAgreement(Tid tid, MacAddress originatorAddr)
{
    auto agreementId = std::make_pair(originatorAddr, tid);
    auto it = blockAckAgreements.find(agreementId);
    return it != blockAckAgreements.end() ? it->second : nullptr;
}

RecipientBlockAckAgreement *RecipientBlockAckAgreementHandler::getActiveAgreement(Tid tid, MacAddress originatorAddr)
{
    auto agreement = getAgreement(tid, originatorAddr);
    return agreement != nullptr && !agreement->isInactivityExpired() ? agreement : nullptr;
}

RecipientBlockAckAgreement *RecipientBlockAckAgreementHandler::processReceivedAddbaRequest(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback)
{
    EV_INFO << "Processing Addba Request from " << addbaRequest->getTransmitterAddress() << endl;
    bool accepted = addbaRequest->getDialogToken() != 0 && blockAckAgreementPolicy->isAddbaReqAccepted(addbaRequest);
    EV_DETAIL << "Building Addba Response" << endl;
    auto addbaResponse = buildAddbaResponse(addbaRequest, blockAckAgreementPolicy, accepted);
    auto id = std::make_pair(addbaRequest->getTransmitterAddress(), addbaRequest->getTid());
    bool hadAgreement = blockAckAgreements.find(id) != blockAckAgreements.end();
    // Keep the immutable response body that corresponds to the most recently
    // processed request identity. This is response replay state, not a second
    // duplicate detector; RecipientQosMacDataService remains authoritative.
    if (accepted || hadAgreement)
        lastAddbaResponses[id] = addbaResponse;
    else
        lastAddbaResponses.erase(id);
    auto addbaResponsePacket = new Packet("AddbaResponse", addbaResponse);
    RecipientBlockAckAgreement *agreement = nullptr;
    if (accepted) {
        // IEEE Std 802.11-2024, 10.25.2 and 11.5.2.3: accepting the
        // request establishes or modifies the recipient agreement when the
        // successful response is formed; transmission is not a state gate.
        auto pendingTeardownIt = pendingTeardownGenerationIds.find(id);
        auto pendingTeardownGenerationId = pendingTeardownIt == pendingTeardownGenerationIds.end() ? 0 : pendingTeardownIt->second;
        if (pendingTeardownIt != pendingTeardownGenerationIds.end())
            pendingTeardownGenerationIds.erase(pendingTeardownIt);
        if (pendingTeardownGenerationId != 0 && agreementHandlerCallback != nullptr)
            agreementHandlerCallback->cancelBlockAckTeardown(false, id.first, id.second, pendingTeardownGenerationId, nullptr);
        auto generationId = allocateAgreementGenerationId();
        agreement = new RecipientBlockAckAgreement(addbaRequest->getTransmitterAddress(), addbaRequest->getTid(), addbaRequest->getStartingSequenceNumber(), addbaResponse->getBufferSize(), addbaResponse->getBlockAckTimeoutValue(), generationId);
        auto it = blockAckAgreements.find(id);
        if (it != blockAckAgreements.end()) {
            if (agreementHandlerCallback != nullptr && (pendingTeardownGenerationId == 0 || pendingTeardownGenerationId != it->second->getGenerationId()))
                agreementHandlerCallback->cancelBlockAckTeardown(false, id.first, id.second, it->second->getGenerationId(), nullptr);
            delete it->second;
            it->second = agreement;
        }
        else
            blockAckAgreements[id] = agreement;
        scheduleInactivityTimer(agreementHandlerCallback);
    }
    procedureCallback->processMgmtFrame(addbaResponsePacket, addbaResponse);
    return agreement;
}

void RecipientBlockAckAgreementHandler::processDuplicateAddbaRequest(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, IProcedureCallback *procedureCallback)
{
    auto agreement = getAgreement(addbaRequest->getTid(), addbaRequest->getTransmitterAddress());
    if (agreement != nullptr) {
        // IEEE Std 802.11-2024, 10.3.2.14.3 normally discards duplicate management bodies.
        // Replaying the already generated response is an explicit robustness/model
        // extension; it does not modify the agreement, reorder window, or inactivity timer.
        auto id = std::make_pair(addbaRequest->getTransmitterAddress(), addbaRequest->getTid());
        auto it = lastAddbaResponses.find(id);
        if (it == lastAddbaResponses.end())
            return;
        // Copy the immutable snapshot so outbound sequence assignment uses COW
        // and cannot modify the cached body used by a later retransmission.
        auto addbaResponse = staticPtrCast<Ieee80211AddbaResponse>(it->second->dupShared());
        procedureCallback->processMgmtFrame(new Packet("AddbaResponse", addbaResponse), addbaResponse);
    }
}

bool RecipientBlockAckAgreementHandler::isDelbaPending(const Packet *packet, const Ptr<const Ieee80211Delba>& delba) const
{
    if (delba->getInitiator())
        return false;
    auto agreementTag = packet->findTag<Ieee80211BlockAckAgreementTag>();
    if (agreementTag == nullptr)
        return true;
    auto it = blockAckAgreements.find(std::make_pair(delba->getReceiverAddress(), delba->getTid()));
    if (it != blockAckAgreements.end())
        return it->second->getGenerationId() == agreementTag->getGenerationId();
    auto teardownIt = pendingTeardownGenerationIds.find(std::make_pair(delba->getReceiverAddress(), delba->getTid()));
    return teardownIt != pendingTeardownGenerationIds.end() && teardownIt->second == agreementTag->getGenerationId();
}

std::unique_ptr<RecipientBlockAckAgreement> RecipientBlockAckAgreementHandler::processTransmittedDelba(Packet *packet, IBlockAckAgreementHandlerCallback *callback)
{
    auto delba = findFragmentedActionContext<Ieee80211Delba>(packet);
    if (delba->getInitiator())
        return nullptr;
    // IEEE Std 802.11-2024, 10.4 and 11.5.3.5: the DELBA MMPDU has not
    // been transmitted while a later fragment is still outstanding.
    if (delba->getMoreFragments())
        return nullptr;
    auto agreementTag = packet->findTag<Ieee80211BlockAckAgreementTag>();
    if (agreementTag != nullptr) {
        auto agreement = getAgreement(delba->getTid(), delba->getReceiverAddress());
        auto agreementId = std::make_pair(delba->getReceiverAddress(), delba->getTid());
        if (agreement != nullptr) {
            if (agreement->getGenerationId() != agreementTag->getGenerationId())
                return nullptr;
            auto terminatedAgreement = std::unique_ptr<RecipientBlockAckAgreement>(removeAgreement(delba->getReceiverAddress(), delba->getTid()));
            scheduleInactivityTimer(callback);
            pendingTeardownGenerationIds[agreementId] = agreementTag->getGenerationId();
            return terminatedAgreement;
        }
        auto teardownIt = pendingTeardownGenerationIds.find(agreementId);
        if (teardownIt == pendingTeardownGenerationIds.end() || teardownIt->second != agreementTag->getGenerationId())
            return nullptr;
        return nullptr;
    }
    auto terminatedAgreement = std::unique_ptr<RecipientBlockAckAgreement>(removeAgreement(delba->getReceiverAddress(), delba->getTid()));
    scheduleInactivityTimer(callback);
    return terminatedAgreement;
}

bool RecipientBlockAckAgreementHandler::processAcknowledgedDelba(Packet *packet, IBlockAckAgreementHandlerCallback *callback)
{
    auto delba = findFragmentedActionContext<Ieee80211Delba>(packet);
    if (delba->getInitiator() || delba->getMoreFragments())
        return false;
    auto agreementTag = packet->findTag<Ieee80211BlockAckAgreementTag>();
    if (agreementTag == nullptr)
        return false;
    auto agreementId = std::make_pair(delba->getReceiverAddress(), delba->getTid());
    auto it = pendingTeardownGenerationIds.find(agreementId);
    if (it == pendingTeardownGenerationIds.end() || it->second != agreementTag->getGenerationId())
        return false;
    auto generationId = it->second;
    pendingTeardownGenerationIds.erase(it);
    if (callback != nullptr)
        callback->cancelBlockAckTeardown(false, delba->getReceiverAddress(), delba->getTid(), generationId, packet);
    return true;
}

RecipientBlockAckAgreementAbortResult RecipientBlockAckAgreementHandler::processAbortedDelba(Packet *packet, IBlockAckAgreementHandlerCallback *callback)
{
    auto delba = findFragmentedActionContext<Ieee80211Delba>(packet);
    if (delba->getInitiator())
        return {};
    auto agreementTag = packet->findTag<Ieee80211BlockAckAgreementTag>();
    if (agreementTag == nullptr)
        return {};
    auto agreementId = std::make_pair(delba->getReceiverAddress(), delba->getTid());
    auto it = pendingTeardownGenerationIds.find(agreementId);
    if (it != pendingTeardownGenerationIds.end() && it->second == agreementTag->getGenerationId()) {
        auto generationId = it->second;
        pendingTeardownGenerationIds.erase(it);
        if (callback != nullptr)
            callback->cancelBlockAckTeardown(false, delba->getReceiverAddress(), delba->getTid(), generationId, packet);
        return { true, nullptr };
    }
    auto agreement = getAgreement(delba->getTid(), delba->getReceiverAddress());
    if (agreement != nullptr && agreement->getGenerationId() == agreementTag->getGenerationId() && agreement->isInactivityExpired()) {
        RecipientBlockAckAgreementAbortResult result;
        result.handled = true;
        result.terminatedAgreement.reset(removeAgreement(delba->getReceiverAddress(), delba->getTid()));
        scheduleInactivityTimer(callback);
        if (callback != nullptr)
            callback->cancelBlockAckTeardown(false, delba->getReceiverAddress(), delba->getTid(), agreementTag->getGenerationId(), packet);
        return result;
    }
    return {};
}

uint64_t RecipientBlockAckAgreementHandler::getPendingTeardownGenerationId(Tid tid, MacAddress originatorAddr) const
{
    auto it = pendingTeardownGenerationIds.find(std::make_pair(originatorAddr, tid));
    return it == pendingTeardownGenerationIds.end() ? 0 : it->second;
}

std::unique_ptr<RecipientBlockAckAgreement> RecipientBlockAckAgreementHandler::processReceivedDelba(const Ptr<const Ieee80211Delba>& delba, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback)
{
    if (blockAckAgreementPolicy->isDelbaAccepted(delba)) {
        auto agreementId = std::make_pair(delba->getTransmitterAddress(), delba->getTid());
        auto pendingTeardownIt = pendingTeardownGenerationIds.find(agreementId);
        if (pendingTeardownIt != pendingTeardownGenerationIds.end())
            pendingTeardownGenerationIds.erase(pendingTeardownIt);
        std::unique_ptr<RecipientBlockAckAgreement> terminatedAgreement(removeAgreement(delba->getTransmitterAddress(), delba->getTid()));
        scheduleInactivityTimer(callback);
        return terminatedAgreement;
    }
    return nullptr;
}

RecipientBlockAckAgreementHandler::~RecipientBlockAckAgreementHandler()
{
    for (auto it : blockAckAgreements)
        delete it.second;
}

} // namespace ieee80211
} // namespace inet
