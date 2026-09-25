//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.h"

#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtTransactionTag_m.h"

namespace inet {
namespace ieee80211 {

simtime_t RecipientBlockAckAgreementHandler::computeEarliestExpirationTime()
{
    simtime_t earliestTime = SIMTIME_MAX;
    for (auto id : blockAckAgreements) {
        auto agreement = id.second;
        earliestTime = std::min(earliestTime, agreement->getExpirationTime());
    }
    return earliestTime;
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
        auto agreement = getAgreement(tid, originatorAddr);
        if (agreement) {
            agreement->calculateExpirationTime();
            callback->scheduleInactivityTimer();
        }
    }
}

void RecipientBlockAckAgreementHandler::blockAckRequestReceived(const Ptr<const Ieee80211BasicBlockAckReq>& request, IBlockAckAgreementHandlerCallback *callback)
{
    auto agreement = getAgreement(request->getTidInfo(), request->getTransmitterAddress());
    if (agreement) {
        agreement->calculateExpirationTime();
        callback->scheduleInactivityTimer();
    }
}

void RecipientBlockAckAgreementHandler::blockAckAgreementExpired(IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback)
{
    // IEEE Std 802.11-2024, 11.5.4: inactivity expiry requires DELBA with TIMEOUT.
    // Remove all due agreements before callbacks can restart HCF or alter the map.
    simtime_t now = simTime();
    std::vector<Packet *> expiredAgreements;
    for (auto it = blockAckAgreements.begin(); it != blockAckAgreements.end(); ) {
        auto agreement = it->second;
        if (agreement->getExpirationTime() != SIMTIME_MAX && agreement->getExpirationTime() <= now) {
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

//
// An originator that intends to use the Block Ack mechanism for the transmission of QoS data frames to an
// intended recipient should first check whether the intended recipient STA is capable of participating in Block
// Ack mechanism by discovering and examining its Delayed Block Ack and Immediate Block Ack capability
// bits. If the intended recipient STA is capable of participating, the originator sends an ADDBA Request frame
// indicating the TID for which the Block Ack is being set up.
//
RecipientBlockAckAgreement *RecipientBlockAckAgreementHandler::addAgreement(const Ptr<const Ieee80211AddbaRequest>& addbaReq, simtime_t acceptedTimeout)
{
    MacAddress originatorAddr = addbaReq->getTransmitterAddress();
    auto id = std::make_pair(originatorAddr, addbaReq->getTid());
    auto it = blockAckAgreements.find(id);
    if (it == blockAckAgreements.end()) {
        RecipientBlockAckAgreement *agreement = new RecipientBlockAckAgreement(originatorAddr, addbaReq->getTid(), addbaReq->getStartingSequenceNumber(), addbaReq->getBufferSize(), acceptedTimeout);
        blockAckAgreements[id] = agreement;
        return agreement;
    }
    else
        // TODO update?
        return it->second;
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

const Ptr<Ieee80211AddbaResponse> RecipientBlockAckAgreementHandler::buildAddbaResponse(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy)
{
    auto addbaResponse = makeShared<Ieee80211AddbaResponse>();
    addbaResponse->setReceiverAddress(addbaRequest->getTransmitterAddress());
    // The Block Ack Policy subfield is set to 1 for immediate Block Ack and 0 for delayed Block Ack.
    Tid tid = addbaRequest->getTid();
    addbaResponse->setTid(tid);
    addbaResponse->setBlockAckPolicy(!addbaRequest->getBlockAckPolicy() && blockAckAgreementPolicy->delayedBlockAckPolicySupported() ? false : true);
    addbaResponse->setBufferSize(addbaRequest->getBufferSize() <= blockAckAgreementPolicy->getMaximumAllowedBufferSize() ? addbaRequest->getBufferSize() : blockAckAgreementPolicy->getMaximumAllowedBufferSize());
    addbaResponse->setBlockAckTimeoutValue(blockAckAgreementPolicy->getBlockAckTimeoutValue() == 0 ? blockAckAgreementPolicy->getBlockAckTimeoutValue() : addbaRequest->getBlockAckTimeoutValue());
    addbaResponse->setAMsduSupported(blockAckAgreementPolicy->aMsduSupported());
    return addbaResponse;
}

void RecipientBlockAckAgreementHandler::terminateAgreement(MacAddress originatorAddr, Tid tid)
{
    auto agreementId = std::make_pair(originatorAddr, tid);
    auto it = blockAckAgreements.find(agreementId);
    if (it != blockAckAgreements.end()) {
        RecipientBlockAckAgreement *agreement = it->second;
        blockAckAgreements.erase(it);
        delete agreement;
    }
}

RecipientBlockAckAgreement *RecipientBlockAckAgreementHandler::getAgreement(Tid tid, MacAddress originatorAddr)
{
    auto agreementId = std::make_pair(originatorAddr, tid);
    auto it = blockAckAgreements.find(agreementId);
    return it != blockAckAgreements.end() ? it->second : nullptr;
}

void RecipientBlockAckAgreementHandler::processTransmittedAddbaResp(const Ptr<const Ieee80211AddbaResponse>& addbaResp, IBlockAckAgreementHandlerCallback *callback)
{
    // The response can complete after expiry or replacement of its agreement.
    // Refresh the shared timer from current agreements without a response-based update.
    callback->scheduleInactivityTimer();
}

void RecipientBlockAckAgreementHandler::processReceivedAddbaRequest(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *callback)
{
    auto pending = pendingTeardowns.find({addbaRequest->getTransmitterAddress(), addbaRequest->getTid()});
    if (pending != pendingTeardowns.end()) {
        pending->second.deferredRequest = addbaRequest;
        return;
    }
    EV_INFO << "Processing Addba Request from " << addbaRequest->getTransmitterAddress() << endl;
    if (blockAckAgreementPolicy->isAddbaReqAccepted(addbaRequest)) {
        EV_DETAIL << "Addba Request has been accepted. Creating a new Block Ack Agreement." << endl;
        auto addbaResponse = buildAddbaResponse(addbaRequest, blockAckAgreementPolicy);
        auto existingAgreement = getAgreement(addbaRequest->getTid(), addbaRequest->getTransmitterAddress());
        auto agreement = addAgreement(addbaRequest, addbaResponse->getBlockAckTimeoutValue());
        if (existingAgreement == nullptr)
            agreement->setNegotiatedParameters(addbaResponse->getBufferSize(), addbaResponse->getBlockAckPolicy(), addbaResponse->getAMsduSupported());
        EV_DETAIL << "Agreement is added with the following parameters: " << *agreement << endl;
        EV_DETAIL << "Building Addba Response" << endl;
        // IEEE Std 802.11-2024, 11.5.2.3: the response describes the established agreement.
        // A duplicate request does not refresh its deadline or change its parameters.
        addbaResponse->setBufferSize(agreement->getBufferSize());
        addbaResponse->setBlockAckPolicy(agreement->getBlockAckPolicy());
        addbaResponse->setAMsduSupported(agreement->getAMsduSupported());
        addbaResponse->setBlockAckTimeoutValue(agreement->getBlockAckTimeoutValue());
        auto addbaResponsePacket = new Packet("AddbaResponse", addbaResponse);
        callback->processMgmtFrame(addbaResponsePacket, addbaResponse);
    }
}

void RecipientBlockAckAgreementHandler::processTransmittedDelba(const Ptr<const Ieee80211Delba>& delba)
{
    // Expiry already removed the agreement. Individual attempts cannot remove a replacement.
}

void RecipientBlockAckAgreementHandler::processDelbaFrameFinished(const Packet *packet, IRecipientBlockAckAgreementPolicy *policy, IProcedureCallback *callback)
{
    auto delba = packet->peekAtFront<Ieee80211Delba>();
    auto pending = pendingTeardowns.find({delba->getReceiverAddress(), delba->getTid()});
    auto tag = packet->findTag<Ieee80211MgmtTransactionTag>();
    if (pending == pendingTeardowns.end() || tag == nullptr || pending->second.transactionId != tag->getTransactionId())
        return;
    auto request = pending->second.deferredRequest;
    pendingTeardowns.erase(pending);
    if (request != nullptr)
        processReceivedAddbaRequest(request, policy, callback);
}

void RecipientBlockAckAgreementHandler::processReceivedDelba(const Ptr<const Ieee80211Delba>& delba, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy)
{
    if (blockAckAgreementPolicy->isDelbaAccepted(delba))
        terminateAgreement(delba->getTransmitterAddress(), delba->getTid());
}

RecipientBlockAckAgreementHandler::~RecipientBlockAckAgreementHandler()
{
    for (auto it : blockAckAgreements)
        delete it.second;
}

} // namespace ieee80211
} // namespace inet
