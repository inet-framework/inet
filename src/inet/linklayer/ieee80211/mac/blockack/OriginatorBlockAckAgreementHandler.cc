//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementHandler.h"

#include <vector>

#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mac/blockack/Ieee80211AddbaTransactionTag_m.h"
#include "inet/linklayer/ieee80211/mac/blockack/Ieee80211BlockAckAgreementTag_m.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/Ieee80211FragmentedActionContextTag.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"

namespace inet {
namespace ieee80211 {

void OriginatorBlockAckAgreementHandler::createAgreement(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, uint64_t transactionId)
{
    ASSERT(addbaRequest->getDialogToken() != 0);
    OriginatorBlockAckAgreement *blockAckAgreement = new OriginatorBlockAckAgreement(addbaRequest->getReceiverAddress(), addbaRequest->getTid(), addbaRequest->getStartingSequenceNumber(), addbaRequest->getBufferSize(), addbaRequest->getAMsduSupported(), addbaRequest->getBlockAckPolicy() == 0, addbaRequest->getDialogToken(), transactionId);
    auto agreementId = std::make_pair(addbaRequest->getReceiverAddress(), addbaRequest->getTid());
    blockAckAgreements[agreementId] = blockAckAgreement;
}

uint8_t OriginatorBlockAckAgreementHandler::allocateDialogToken()
{
    auto dialogToken = nextDialogToken;
    nextDialogToken = nextDialogToken == 255 ? 1 : nextDialogToken + 1;
    return dialogToken;
}

simtime_t OriginatorBlockAckAgreementHandler::computeEarliestExpirationTime()
{
    simtime_t earliestTime = SIMTIME_MAX;
    for (auto id : blockAckAgreements) {
        auto agreement = id.second;
        if (agreement->getIsAddbaResponseReceived() && !agreement->isInactivityExpired()) {
            ASSERT(earliestTime >= 0);
            ASSERT(agreement->getExpirationTime() >= 0);
            earliestTime = std::min(earliestTime, agreement->getExpirationTime());
        }
    }
    return earliestTime;
}

simtime_t OriginatorBlockAckAgreementHandler::computeEarliestAddbaResponseDeadline() const
{
    simtime_t earliestDeadline = SIMTIME_MAX;
    for (const auto& entry : blockAckAgreements) {
        auto agreement = entry.second;
        if (agreement->isPending() && agreement->getIsAddbaRequestSent()) {
            ASSERT(agreement->getAddbaResponseDeadline() >= 0);
            earliestDeadline = std::min(earliestDeadline, agreement->getAddbaResponseDeadline());
        }
    }
    return earliestDeadline;
}

simtime_t OriginatorBlockAckAgreementHandler::getAddbaResponseTimeout(IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy) const
{
    auto timeout = blockAckAgreementPolicy->getAddbaResponseTimeout();
    if (timeout <= 0)
        throw cRuntimeError("ADDBA response timeout must be greater than zero");
    return timeout;
}

void OriginatorBlockAckAgreementHandler::recordAddbaFailure(MacAddress receiverAddr, Tid tid, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy)
{
    auto retryBackoff = blockAckAgreementPolicy->computeAddbaRetryBackoff();
    if (retryBackoff < 0)
        throw cRuntimeError("ADDBA retry backoff must not be negative");
    addbaRetryDeadlines[std::make_pair(receiverAddr, tid)] = simTime() + retryBackoff;
}

void OriginatorBlockAckAgreementHandler::addbaResponseTimeoutExpired(IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback)
{
    auto now = simTime();
    std::vector<uint64_t> expiredTransactionIds;
    for (auto it = blockAckAgreements.begin(); it != blockAckAgreements.end();) {
        auto agreement = it->second;
        if (agreement->isPending() && agreement->getIsAddbaRequestSent() && agreement->getAddbaResponseDeadline() <= now) {
            EV_INFO << "ADDBA transaction timeout for receiver=" << agreement->getReceiverAddr() << " tid=" << (int)agreement->getTid() << endl;
            auto transactionId = agreement->getTransactionId();
            recordAddbaFailure(agreement->getReceiverAddr(), agreement->getTid(), blockAckAgreementPolicy);
            it = blockAckAgreements.erase(it);
            delete agreement;
            expiredTransactionIds.push_back(transactionId);
        }
        else
            it++;
    }
    for (auto transactionId : expiredTransactionIds)
        callback->cancelAddbaTransaction(transactionId, nullptr);
    scheduleAddbaResponseTimer(callback);
}

void OriginatorBlockAckAgreementHandler::blockAckAgreementExpired(IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback)
{
    // When a timeout of BlockAckTimeout is detected, the STA shall send a DELBA frame to the
    // peer STA with the Reason Code field set to TIMEOUT and shall issue a MLME-DELBA.indication
    // primitive with the ReasonCode parameter having a value of TIMEOUT.
    // The procedure is illustrated in Figure 10-14.
    simtime_t now = simTime();
    for (auto id : blockAckAgreements) {
        auto agreement = id.second;
        if (agreement->getIsAddbaResponseReceived() && !agreement->isInactivityExpired() && agreement->getExpirationTime() <= now) {
            agreement->markInactivityExpired();
            MacAddress receiverAddr = id.first.first;
            Tid tid = id.first.second;
            const auto& delba = buildDelba(receiverAddr, tid, 39);
            auto delbaPacket = new Packet("Delba", delba);
            delbaPacket->addTag<Ieee80211BlockAckAgreementTag>()->setGenerationId(agreement->getTransactionId());
            procedureCallback->processMgmtFrame(delbaPacket, delba); // 39 - TIMEOUT see: Table 8-36—Reason codes
        }
    }
    scheduleInactivityTimer(agreementHandlerCallback);
}

const Ptr<Ieee80211AddbaRequest> OriginatorBlockAckAgreementHandler::buildAddbaRequest(MacAddress receiverAddr, Tid tid, SequenceNumberCyclic startingSequenceNumber, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy)
{
    auto addbaRequest = makeShared<Ieee80211AddbaRequest>();
    addbaRequest->setReceiverAddress(receiverAddr);
    // IEEE Std 802.11-2024, 9.6.4.2: a solicited ADDBA Request uses a nonzero
    // Dialog Token, and the corresponding response copies that token.
    addbaRequest->setDialogToken(allocateDialogToken());
    addbaRequest->setTid(tid);
    addbaRequest->setAMsduSupported(blockAckAgreementPolicy->isMsduSupported());
    addbaRequest->setBlockAckTimeoutValue(blockAckAgreementPolicy->getBlockAckTimeoutValue());
    addbaRequest->setBufferSize(blockAckAgreementPolicy->getMaximumAllowedBufferSize());
    // The Block Ack Policy subfield is set to 1 for immediate Block Ack and 0 for delayed Block Ack.
    addbaRequest->setBlockAckPolicy(blockAckAgreementPolicy->isDelayedAckPolicySupported() ? 0 : 1);
    addbaRequest->setStartingSequenceNumber(startingSequenceNumber);
    return addbaRequest;
}

//
// The inactivity timer at the originator is reset when a BlockAck frame
// corresponding to the TID for which the Block Ack policy is set is received.
//
void OriginatorBlockAckAgreementHandler::processReceivedBlockAck(const Ptr<const Ieee80211BlockAck>& blockAck, IBlockAckAgreementHandlerCallback *callback)
{
    if (auto basicBlockAck = dynamicPtrCast<const Ieee80211BasicBlockAck>(blockAck)) {
        auto agreement = getAgreement(basicBlockAck->getTransmitterAddress(), basicBlockAck->getTidInfo());
        if (agreement && !agreement->isInactivityExpired()) {
            agreement->setStartingSequenceNumber(basicBlockAck->getStartingSequenceNumber());
            agreement->calculateExpirationTime();
            scheduleInactivityTimer(callback);
        }
    }
    else
        throw cRuntimeError("Unsupported BlockAck");
}

void OriginatorBlockAckAgreementHandler::scheduleInactivityTimer(IBlockAckAgreementHandlerCallback *callback)
{
    simtime_t earliestExpirationTime = computeEarliestExpirationTime();
    if (callback != nullptr)
        callback->scheduleInactivityTimer(BlockAckAgreementRole::ORIGINATOR, earliestExpirationTime);
}

void OriginatorBlockAckAgreementHandler::scheduleAddbaResponseTimer(IBlockAckAgreementHandlerCallback *callback)
{
    callback->scheduleAddbaResponseTimer(computeEarliestAddbaResponseDeadline());
}

OriginatorBlockAckAgreement *OriginatorBlockAckAgreementHandler::getAgreement(MacAddress receiverAddr, Tid tid)
{
    auto agreementId = std::make_pair(receiverAddr, tid);
    auto it = blockAckAgreements.find(agreementId);
    return it != blockAckAgreements.end() ? it->second : nullptr;
}

bool OriginatorBlockAckAgreementHandler::isAddbaResponsePending(MacAddress receiverAddr, Tid tid) const
{
    auto it = blockAckAgreements.find(std::make_pair(receiverAddr, tid));
    return it != blockAckAgreements.end() && it->second->isPending();
}

const Ptr<Ieee80211Delba> OriginatorBlockAckAgreementHandler::buildDelba(MacAddress receiverAddr, Tid tid, int reasonCode)
{
    auto delba = makeShared<Ieee80211Delba>();
    delba->setReceiverAddress(receiverAddr);
    delba->setTid(tid);
    delba->setReasonCode(reasonCode);
    // The Initiator subfield indicates if the originator or the recipient of the data is sending this frame.
    delba->setInitiator(true);
    return delba;
}

OriginatorBlockAckAgreement *OriginatorBlockAckAgreementHandler::removeAgreement(MacAddress originatorAddr, Tid tid)
{
    auto agreementId = std::make_pair(originatorAddr, tid);
    auto it = blockAckAgreements.find(agreementId);
    if (it != blockAckAgreements.end()) {
        auto agreement = it->second;
        blockAckAgreements.erase(it);
        return agreement;
    }
    return nullptr;
}

void OriginatorBlockAckAgreementHandler::terminateAgreement(MacAddress originatorAddr, Tid tid)
{
    delete removeAgreement(originatorAddr, tid);
}

uint64_t OriginatorBlockAckAgreementHandler::processAcknowledgedDataFrame(Packet *packet, const Ptr<const Ieee80211DataHeader>& dataHeader, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *procedureCallback)
{
    // IEEE Std 802.11-2024, Table 9-466: the Starting Sequence Number identifies
    // the first or next MSDU/A-MSDU sent under the agreement. Wait until the
    // final fragment is acknowledged so no remaining fragment precedes the SSN.
    if (dataHeader->getMoreFragments())
        return 0;
    auto receiverAddr = dataHeader->getReceiverAddress();
    auto tid = dataHeader->getTid();
    auto agreementId = std::make_pair(receiverAddr, tid);
    auto agreement = getAgreement(receiverAddr, tid);
    auto retryIt = addbaRetryDeadlines.find(agreementId);
    bool retryAllowed = retryIt == addbaRetryDeadlines.end() || retryIt->second <= simTime();
    uint64_t obsoleteTeardownTransactionId = 0;
    if (blockAckAgreementPolicy->isAddbaReqNeeded(packet, dataHeader) && agreement == nullptr && retryAllowed) {
        if (retryIt != addbaRetryDeadlines.end())
            addbaRetryDeadlines.erase(retryIt);
        // A replacement agreement makes an older best-effort DELBA stale. Drop
        // its eligibility instead of waiting indefinitely for a queue callback
        // that every custom packet provider may not deliver.
        auto teardownIt = pendingTeardownTransactionIds.find(agreementId);
        if (teardownIt != pendingTeardownTransactionIds.end()) {
            obsoleteTeardownTransactionId = teardownIt->second;
            pendingTeardownTransactionIds.erase(teardownIt);
        }
        // IEEE Std 802.11-2024, 10.25.2 and 11.5.2.2: Normal Ack data is
        // permitted before an agreement exists, and the requested SSN starts
        // after the acknowledged trigger MPDU.
        auto addbaReq = buildAddbaRequest(receiverAddr, tid, dataHeader->getSequenceNumber() + 1, blockAckAgreementPolicy);
        auto transactionId = nextTransactionId++;
        createAgreement(addbaReq, transactionId);
        auto addbaPacket = new Packet("AddbaReq", addbaReq);
        addbaPacket->addTag<Ieee80211AddbaTransactionTag>()->setTransactionId(transactionId);
        procedureCallback->processMgmtFrame(addbaPacket, addbaReq);
    }
    return obsoleteTeardownTransactionId;
}

OriginatorBlockAckAgreementResponse OriginatorBlockAckAgreementHandler::processReceivedAddbaResp(const Ptr<const Ieee80211AddbaResponse>& addbaResp, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback)
{
    OriginatorBlockAckAgreementResponse response;
    auto agreement = getAgreement(addbaResp->getTransmitterAddress(), addbaResp->getTid());
    // IEEE Std 802.11-2024, 11.5.2.2: only a successful response matching the
    // outstanding peer, TID, and Dialog Token establishes the agreement.
    if (agreement == nullptr || !agreement->isPending() || !agreement->getIsAddbaRequestSent() || agreement->getDialogToken() != addbaResp->getDialogToken())
        return response;
    bool acceptedByLocalPolicy = addbaResp->getStatusCode() == 0 && blockAckAgreementPolicy->isAddbaReqAccepted(addbaResp, agreement);
    if (addbaResp->getStatusCode() == 0) {
        auto transactionId = agreement->getTransactionId();
        updateAgreement(agreement, addbaResp);
        if (acceptedByLocalPolicy)
            addbaRetryDeadlines.erase(std::make_pair(addbaResp->getTransmitterAddress(), addbaResp->getTid()));
        else
            recordAddbaFailure(addbaResp->getTransmitterAddress(), addbaResp->getTid(), blockAckAgreementPolicy);
        scheduleInactivityTimer(callback);
        scheduleAddbaResponseTimer(callback);
        callback->cancelAddbaTransaction(transactionId, nullptr);
        if (!acceptedByLocalPolicy) {
            // IEEE Std 802.11-2024, 10.25.2 Note 3: delete a successful
            // agreement rejected by local policy and continue with Normal Ack.
            response.terminatedAgreement.reset(removeAgreement(addbaResp->getTransmitterAddress(), addbaResp->getTid()));
            if (response.terminatedAgreement == nullptr)
                throw cRuntimeError("Cannot terminate locally vetoed Block Ack agreement");
            response.teardownDelba = buildDelba(addbaResp->getTransmitterAddress(), addbaResp->getTid(), RC_END_BA);
            pendingTeardownTransactionIds[std::make_pair(addbaResp->getTransmitterAddress(), addbaResp->getTid())] = transactionId;
            response.teardownTransactionId = transactionId;
            scheduleInactivityTimer(callback);
        }
        else
            response.establishedAgreement = agreement;
        return response;
    }
    else {
        auto transactionId = agreement->getTransactionId();
        recordAddbaFailure(addbaResp->getTransmitterAddress(), addbaResp->getTid(), blockAckAgreementPolicy);
        terminateAgreement(addbaResp->getTransmitterAddress(), addbaResp->getTid());
        scheduleAddbaResponseTimer(callback);
        callback->cancelAddbaTransaction(transactionId, nullptr);
        return response;
    }
}

void OriginatorBlockAckAgreementHandler::updateAgreement(OriginatorBlockAckAgreement *agreement, const Ptr<const Ieee80211AddbaResponse>& addbaResp)
{
    agreement->setIsAddbaResponseReceived(true);
    agreement->setBufferSize(addbaResp->getBufferSize());
    agreement->setBlockAckTimeoutValue(addbaResp->getBlockAckTimeoutValue());
    agreement->calculateExpirationTime();
}

bool OriginatorBlockAckAgreementHandler::isAddbaRequestPending(const Packet *packet, const Ptr<const Ieee80211AddbaRequest>& addbaReq) const
{
    auto it = blockAckAgreements.find(std::make_pair(addbaReq->getReceiverAddress(), addbaReq->getTid()));
    auto transactionTag = packet->findTag<Ieee80211AddbaTransactionTag>();
    return it != blockAckAgreements.end() && it->second->isPending() && transactionTag != nullptr &&
            it->second->getDialogToken() == addbaReq->getDialogToken() && it->second->getTransactionId() == transactionTag->getTransactionId();
}

bool OriginatorBlockAckAgreementHandler::isDelbaPending(const Packet *packet, const Ptr<const Ieee80211Delba>& delba) const
{
    if (!delba->getInitiator())
        return true;
    auto agreementTag = packet->findTag<Ieee80211BlockAckAgreementTag>();
    if (agreementTag != nullptr) {
        auto generationId = agreementTag->getGenerationId();
        auto agreementIt = blockAckAgreements.find(std::make_pair(delba->getReceiverAddress(), delba->getTid()));
        if (agreementIt != blockAckAgreements.end())
            return agreementIt->second->getTransactionId() == generationId;
        auto teardownIt = pendingTeardownTransactionIds.find(std::make_pair(delba->getReceiverAddress(), delba->getTid()));
        return teardownIt != pendingTeardownTransactionIds.end() && teardownIt->second == generationId;
    }
    return true;
}

void OriginatorBlockAckAgreementHandler::processTransmittedAddbaReq(Packet *packet, const Ptr<const Ieee80211AddbaRequest>& addbaReq, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback)
{
    auto agreement = getAgreement(addbaReq->getReceiverAddress(), addbaReq->getTid());
    if (isAddbaRequestPending(packet, addbaReq) && !addbaReq->getMoreFragments() && !agreement->getIsAddbaRequestSent()) {
        agreement->setAddbaResponseDeadline(simTime() + getAddbaResponseTimeout(blockAckAgreementPolicy));
        agreement->setIsAddbaRequestSent(true);
        scheduleAddbaResponseTimer(callback);
    }
    else if (!isAddbaRequestPending(packet, addbaReq))
        EV_WARN << "Ignoring stale transmitted ADDBA Request for receiver=" << addbaReq->getReceiverAddress() << " tid=" << (int)addbaReq->getTid() << " dialogToken=" << (int)addbaReq->getDialogToken() << endl;
}

void OriginatorBlockAckAgreementHandler::processDroppedAddbaReq(Packet *packet, const Ptr<const Ieee80211AddbaRequest>& addbaReq, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback)
{
    if (isAddbaRequestPending(packet, addbaReq)) {
        auto transactionId = packet->getTag<Ieee80211AddbaTransactionTag>()->getTransactionId();
        recordAddbaFailure(addbaReq->getReceiverAddress(), addbaReq->getTid(), blockAckAgreementPolicy);
        terminateAgreement(addbaReq->getReceiverAddress(), addbaReq->getTid());
        scheduleAddbaResponseTimer(callback);
        callback->cancelAddbaTransaction(transactionId, packet);
    }
}

std::unique_ptr<OriginatorBlockAckAgreement> OriginatorBlockAckAgreementHandler::processTransmittedDelba(Packet *packet, IBlockAckAgreementHandlerCallback *callback)
{
    auto delba = findFragmentedActionContext<Ieee80211Delba>(packet);
    // IEEE Std 802.11-2024, 10.4 and 11.5.3.2: an untagged DELBA MMPDU
    // likewise cannot tear down the agreement before its final fragment.
    if (delba->getMoreFragments())
        return nullptr;
    auto agreementTag = packet->findTag<Ieee80211BlockAckAgreementTag>();
    if (agreementTag != nullptr) {
        auto generationId = agreementTag->getGenerationId();
        auto teardownIt = pendingTeardownTransactionIds.find(std::make_pair(delba->getReceiverAddress(), delba->getTid()));
        if (teardownIt != pendingTeardownTransactionIds.end() && teardownIt->second == generationId)
            return nullptr;
        auto agreement = getAgreement(delba->getReceiverAddress(), delba->getTid());
        if (agreement == nullptr || agreement->getTransactionId() != generationId)
            return nullptr;
        bool cancelPendingTransaction = agreement->isPending();
        std::unique_ptr<OriginatorBlockAckAgreement> terminatedAgreement(removeAgreement(delba->getReceiverAddress(), delba->getTid()));
        scheduleInactivityTimer(callback);
        pendingTeardownTransactionIds[std::make_pair(delba->getReceiverAddress(), delba->getTid())] = generationId;
        scheduleAddbaResponseTimer(callback);
        if (cancelPendingTransaction)
            callback->cancelAddbaTransaction(generationId, nullptr);
        callback->cancelBlockAckTeardown(true, delba->getReceiverAddress(), delba->getTid(), generationId, packet);
        return terminatedAgreement;
    }
    auto agreement = getAgreement(delba->getReceiverAddress(), delba->getTid());
    bool cancelPendingTransaction = agreement != nullptr && agreement->isPending();
    auto transactionId = cancelPendingTransaction ? agreement->getTransactionId() : 0;
    std::unique_ptr<OriginatorBlockAckAgreement> terminatedAgreement(removeAgreement(delba->getReceiverAddress(), delba->getTid()));
    scheduleInactivityTimer(callback);
    scheduleAddbaResponseTimer(callback);
    if (cancelPendingTransaction)
        callback->cancelAddbaTransaction(transactionId, nullptr);
    return terminatedAgreement;
}

bool OriginatorBlockAckAgreementHandler::processAcknowledgedDelba(Packet *packet, IBlockAckAgreementHandlerCallback *callback)
{
    auto delba = findFragmentedActionContext<Ieee80211Delba>(packet);
    if (!delba->getInitiator() || delba->getMoreFragments())
        return false;
    auto agreementTag = packet->findTag<Ieee80211BlockAckAgreementTag>();
    if (agreementTag == nullptr)
        return false;
    auto it = pendingTeardownTransactionIds.find(std::make_pair(delba->getReceiverAddress(), delba->getTid()));
    if (it == pendingTeardownTransactionIds.end() || it->second != agreementTag->getGenerationId())
        return false;
    auto transactionId = it->second;
    pendingTeardownTransactionIds.erase(it);
    callback->cancelBlockAckTeardown(true, delba->getReceiverAddress(), delba->getTid(), transactionId, packet);
    return true;
}

OriginatorBlockAckAgreementAbortResult OriginatorBlockAckAgreementHandler::processAbortedDelba(Packet *packet, IBlockAckAgreementHandlerCallback *callback)
{
    auto delba = findFragmentedActionContext<Ieee80211Delba>(packet);
    if (!delba->getInitiator())
        return {};
    auto agreementTag = packet->findTag<Ieee80211BlockAckAgreementTag>();
    if (agreementTag != nullptr) {
        auto agreementId = std::make_pair(delba->getReceiverAddress(), delba->getTid());
        auto it = pendingTeardownTransactionIds.find(agreementId);
        if (it != pendingTeardownTransactionIds.end() && it->second == agreementTag->getGenerationId()) {
            auto transactionId = it->second;
            pendingTeardownTransactionIds.erase(it);
            if (callback != nullptr)
                callback->cancelBlockAckTeardown(true, delba->getReceiverAddress(), delba->getTid(), transactionId, packet);
            return { true, nullptr };
        }
        auto agreement = getAgreement(delba->getReceiverAddress(), delba->getTid());
        if (agreement != nullptr && agreement->getTransactionId() == agreementTag->getGenerationId() && agreement->isInactivityExpired()) {
            OriginatorBlockAckAgreementAbortResult result;
            result.handled = true;
            result.terminatedAgreement.reset(removeAgreement(delba->getReceiverAddress(), delba->getTid()));
            scheduleInactivityTimer(callback);
            if (callback != nullptr)
                callback->cancelBlockAckTeardown(true, delba->getReceiverAddress(), delba->getTid(), agreementTag->getGenerationId(), packet);
            return result;
        }
    }
    return {};
}

std::unique_ptr<OriginatorBlockAckAgreement> OriginatorBlockAckAgreementHandler::processReceivedDelba(const Ptr<const Ieee80211Delba>& delba, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback)
{
    if (blockAckAgreementPolicy->isDelbaAccepted(delba)) {
        auto agreement = getAgreement(delba->getTransmitterAddress(), delba->getTid());
        bool cancelPendingTransaction = agreement != nullptr && agreement->isPending();
        auto transactionId = cancelPendingTransaction ? agreement->getTransactionId() : 0;
        auto agreementId = std::make_pair(delba->getTransmitterAddress(), delba->getTid());
        auto pendingTeardownIt = pendingTeardownTransactionIds.find(agreementId);
        auto pendingTeardownTransactionId = pendingTeardownIt == pendingTeardownTransactionIds.end() ? 0 : pendingTeardownIt->second;
        if (pendingTeardownIt != pendingTeardownTransactionIds.end())
            pendingTeardownTransactionIds.erase(pendingTeardownIt);
        std::unique_ptr<OriginatorBlockAckAgreement> terminatedAgreement(removeAgreement(delba->getTransmitterAddress(), delba->getTid()));
        scheduleInactivityTimer(callback);
        scheduleAddbaResponseTimer(callback);
        if (cancelPendingTransaction)
            callback->cancelAddbaTransaction(transactionId, nullptr);
        if (pendingTeardownTransactionId != 0)
            callback->cancelBlockAckTeardown(true, delba->getTransmitterAddress(), delba->getTid(), pendingTeardownTransactionId, nullptr);
        if (terminatedAgreement != nullptr)
            callback->cancelBlockAckTeardown(true, delba->getTransmitterAddress(), delba->getTid(), terminatedAgreement->getTransactionId(), nullptr);
        return terminatedAgreement;
    }
    return nullptr;
}

OriginatorBlockAckAgreementHandler::~OriginatorBlockAckAgreementHandler()
{
    for (auto it : blockAckAgreements)
        delete it.second;
}

} // namespace ieee80211
} // namespace inet
