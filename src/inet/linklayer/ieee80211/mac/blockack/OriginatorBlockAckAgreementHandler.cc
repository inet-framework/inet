//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementHandler.h"

#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtTransactionTag_m.h"

namespace inet {
namespace ieee80211 {

void OriginatorBlockAckAgreementHandler::createAgreement(const Ptr<const Ieee80211AddbaRequest>& addbaRequest)
{
    OriginatorBlockAckAgreement *blockAckAgreement = new OriginatorBlockAckAgreement(addbaRequest->getReceiverAddress(), addbaRequest->getTid(), addbaRequest->getStartingSequenceNumber(), addbaRequest->getBufferSize(), addbaRequest->getAMsduSupported(), addbaRequest->getBlockAckPolicy() == 0);
    auto agreementId = std::make_pair(addbaRequest->getReceiverAddress(), addbaRequest->getTid());
    blockAckAgreements[agreementId] = blockAckAgreement;
}

simtime_t OriginatorBlockAckAgreementHandler::computeEarliestExpirationTime()
{
    simtime_t earliestTime = SIMTIME_MAX;
    for (auto id : blockAckAgreements) {
        auto agreement = id.second;
        if (agreement->getIsAddbaResponseReceived()) {
            ASSERT(earliestTime >= 0);
            ASSERT(agreement->getExpirationTime() >= 0);
            earliestTime = std::min(earliestTime, agreement->getExpirationTime());
        }
    }
    return earliestTime;
}

void OriginatorBlockAckAgreementHandler::blockAckAgreementExpired(IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback)
{
    // IEEE Std 802.11-2024, 11.5.4: inactivity expiry requires DELBA with TIMEOUT.
    // Remove all due agreements before callbacks can restart HCF or alter the map.
    simtime_t now = simTime();
    std::vector<Packet *> expiredAgreements;
    for (auto it = blockAckAgreements.begin(); it != blockAckAgreements.end(); ) {
        auto agreement = it->second;
        if (agreement->getIsAddbaResponseReceived() && agreement->getExpirationTime() != SIMTIME_MAX && agreement->getExpirationTime() <= now) {
            auto packet = new Packet("Delba", buildDelba(it->first.first, it->first.second, 39));
            auto transactionId = packet->getTreeId();
            packet->addTag<Ieee80211MgmtTransactionTag>()->setTransactionId(transactionId);
            pendingTeardowns.emplace(it->first, PendingTeardown{static_cast<uint64_t>(transactionId), nullptr});
            expiredAgreements.push_back(packet);
            it = blockAckAgreements.erase(it);
            delete agreement;
        }
        else
            ++it;
    }
    agreementHandlerCallback->scheduleInactivityTimer();
    for (auto packet : expiredAgreements)
        procedureCallback->processMgmtFrame(packet, packet->peekAtFront<Ieee80211Delba>());
}

const Ptr<Ieee80211AddbaRequest> OriginatorBlockAckAgreementHandler::buildAddbaRequest(MacAddress receiverAddr, Tid tid, SequenceNumberCyclic startingSequenceNumber, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy)
{
    auto addbaRequest = makeShared<Ieee80211AddbaRequest>();
    addbaRequest->setReceiverAddress(receiverAddr);
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
        if (agreement) {
            agreement->setStartingSequenceNumber(basicBlockAck->getStartingSequenceNumber());
            agreement->calculateExpirationTime();
            callback->scheduleInactivityTimer();
        }
    }
    else
        throw cRuntimeError("Unsupported BlockAck");
}

OriginatorBlockAckAgreement *OriginatorBlockAckAgreementHandler::getAgreement(MacAddress receiverAddr, Tid tid)
{
    auto agreementId = std::make_pair(receiverAddr, tid);
    auto it = blockAckAgreements.find(agreementId);
    return it != blockAckAgreements.end() ? it->second : nullptr;
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

void OriginatorBlockAckAgreementHandler::terminateAgreement(MacAddress originatorAddr, Tid tid)
{
    auto agreementId = std::make_pair(originatorAddr, tid);
    auto it = blockAckAgreements.find(agreementId);
    if (it != blockAckAgreements.end()) {
        OriginatorBlockAckAgreement *agreement = it->second;
        blockAckAgreements.erase(it);
        delete agreement;
    }
}

void OriginatorBlockAckAgreementHandler::processTransmittedDataFrame(Packet *packet, const Ptr<const Ieee80211DataHeader>& dataHeader, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *callback)
{
    auto agreement = getAgreement(dataHeader->getReceiverAddress(), dataHeader->getTid());
    if (blockAckAgreementPolicy->isAddbaReqNeeded(packet, dataHeader) && agreement == nullptr) {
        auto addbaReq = buildAddbaRequest(dataHeader->getReceiverAddress(), dataHeader->getTid(), dataHeader->getSequenceNumber() + 1, blockAckAgreementPolicy);
        auto pending = pendingTeardowns.find({dataHeader->getReceiverAddress(), dataHeader->getTid()});
        if (pending != pendingTeardowns.end()) {
            if (pending->second.deferredRequest == nullptr)
                pending->second.deferredRequest = addbaReq;
            return;
        }
        createAgreement(addbaReq);
        auto addbaPacket = new Packet("AddbaReq", addbaReq);
        callback->processMgmtFrame(addbaPacket, addbaReq);
    }
}

void OriginatorBlockAckAgreementHandler::processReceivedAddbaResp(const Ptr<const Ieee80211AddbaResponse>& addbaResp, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback)
{
    auto agreement = getAgreement(addbaResp->getTransmitterAddress(), addbaResp->getTid());
    if (agreement != nullptr && blockAckAgreementPolicy->isAddbaReqAccepted(addbaResp, agreement)) {
        updateAgreement(agreement, addbaResp);
        callback->scheduleInactivityTimer();
    }
    else {
        // TODO send a new one?
    }
}

void OriginatorBlockAckAgreementHandler::updateAgreement(OriginatorBlockAckAgreement *agreement, const Ptr<const Ieee80211AddbaResponse>& addbaResp)
{
    agreement->setIsAddbaResponseReceived(true);
    agreement->setBufferSize(addbaResp->getBufferSize());
    agreement->setBlockAckTimeoutValue(addbaResp->getBlockAckTimeoutValue());
    agreement->calculateExpirationTime();
}

void OriginatorBlockAckAgreementHandler::processTransmittedAddbaReq(const Ptr<const Ieee80211AddbaRequest>& addbaReq)
{
    // The agreement exists before the request enters the queue.
    // A late completion must not require it or change a replacement agreement.
}

void OriginatorBlockAckAgreementHandler::processTransmittedDelba(const Ptr<const Ieee80211Delba>& delba)
{
    // Expiry already removed the agreement. Individual attempts cannot remove a replacement.
}

void OriginatorBlockAckAgreementHandler::processDelbaFrameFinished(const Packet *packet, IProcedureCallback *callback)
{
    auto delba = packet->peekAtFront<Ieee80211Delba>();
    auto pending = pendingTeardowns.find({delba->getReceiverAddress(), delba->getTid()});
    auto tag = packet->findTag<Ieee80211MgmtTransactionTag>();
    if (pending == pendingTeardowns.end() || tag == nullptr || pending->second.transactionId != tag->getTransactionId())
        return;
    auto request = pending->second.deferredRequest;
    pendingTeardowns.erase(pending);
    if (request != nullptr) {
        createAgreement(request);
        callback->processMgmtFrame(new Packet("AddbaReq", request), request);
    }
}

void OriginatorBlockAckAgreementHandler::processReceivedDelba(const Ptr<const Ieee80211Delba>& delba, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy)
{
    auto agreement = getAgreement(delba->getTransmitterAddress(), delba->getTid());
    // A peer's old DELBA can precede the response to a new setup request.
    if (agreement != nullptr && agreement->getIsAddbaResponseReceived() && blockAckAgreementPolicy->isDelbaAccepted(delba))
        terminateAgreement(delba->getTransmitterAddress(), delba->getTid());
}

OriginatorBlockAckAgreementHandler::~OriginatorBlockAckAgreementHandler()
{
    for (auto it : blockAckAgreements)
        delete it.second;
}

} // namespace ieee80211
} // namespace inet
