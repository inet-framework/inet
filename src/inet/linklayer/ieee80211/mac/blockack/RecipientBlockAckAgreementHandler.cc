//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.h"

#include <algorithm>

#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreement.h"

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

void RecipientBlockAckAgreementHandler::scheduleInactivityTimer(IBlockAckAgreementHandlerCallback *callback)
{
    simtime_t earliestExpirationTime = computeEarliestExpirationTime();
    if (earliestExpirationTime != SIMTIME_MAX)
        callback->scheduleInactivityTimer(earliestExpirationTime);
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
        if (agreement)
            scheduleInactivityTimer(callback);
    }
}

void RecipientBlockAckAgreementHandler::blockAckAgreementExpired(IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback)
{
    // When a timeout of BlockAckTimeout is detected, the STA shall send a DELBA frame to the
    // peer STA with the Reason Code field set to TIMEOUT and shall issue a MLME-DELBA.indication
    // primitive with the ReasonCode parameter having a value of TIMEOUT.
    // The procedure is illustrated in Figure 10-14.
    simtime_t now = simTime();
    for (auto id : blockAckAgreements) {
        auto agreement = id.second;
        if (agreement->getExpirationTime() == now) {
            MacAddress receiverAddr = id.first.first;
            Tid tid = id.first.second;
            const auto& delba = buildDelba(receiverAddr, tid, 39);
            auto delbaPacket = new Packet("Delba", delba);
            procedureCallback->processMgmtFrame(delbaPacket, delba); // 39 - TIMEOUT see: Table 8-36—Reason codes
        }
    }
    scheduleInactivityTimer(agreementHandlerCallback);
}

//
// Keep the accepted parameters staged until the exact corresponding successful
// response packet is actually transmitted. Packet identity survives mutable
// header copy-on-write during sequence-number assignment and keeps overlapping
// transactions independent even when the originator reuses a Dialog Token.
//
void RecipientBlockAckAgreementHandler::clearPendingAgreements(MacAddress originatorAddr, Tid tid)
{
    for (auto it = pendingBlockAckAgreements.begin(); it != pendingBlockAckAgreements.end();) {
        if (it->originatorAddress == originatorAddr && it->tid == tid) {
            delete it->agreement;
            it = pendingBlockAckAgreements.erase(it);
        }
        else
            it++;
    }
}

void RecipientBlockAckAgreementHandler::stageAgreement(Packet *addbaResponsePacket, const Ptr<const Ieee80211AddbaRequest>& addbaRequest, const Ptr<const Ieee80211AddbaResponse>& addbaResponse)
{
    auto originatorAddr = addbaRequest->getTransmitterAddress();
    Tid tid = addbaRequest->getTid();
    auto agreement = new RecipientBlockAckAgreement(originatorAddr, tid, addbaRequest->getStartingSequenceNumber(), addbaResponse->getBufferSize(), addbaResponse->getBlockAckTimeoutValue());
    pendingBlockAckAgreements.push_back({ addbaResponsePacket->getId(), originatorAddr, tid, addbaRequest->getDialogToken(), agreement });
    EV_DETAIL << "Block Ack Agreement is staged with the following parameters: " << *agreement << endl;
}

RecipientBlockAckAgreement *RecipientBlockAckAgreementHandler::activateAgreement(Packet *addbaResponsePacket, const Ptr<const Ieee80211AddbaResponse>& addbaResponse)
{
    auto pendingIt = std::find_if(pendingBlockAckAgreements.begin(), pendingBlockAckAgreements.end(), [addbaResponsePacket](const PendingAgreement& pendingAgreement) {
        return pendingAgreement.addbaResponsePacketId == addbaResponsePacket->getId();
    });
    if (pendingIt == pendingBlockAckAgreements.end())
        return nullptr;
    if (addbaResponse->getReceiverAddress() != pendingIt->originatorAddress || addbaResponse->getTid() != pendingIt->tid || addbaResponse->getDialogToken() != pendingIt->dialogToken || addbaResponse->getStatusCode() != 0) {
        EV_WARN << "Ignoring transmitted ADDBA Response whose fields do not match its staged transaction" << endl;
        return nullptr;
    }
    auto agreement = pendingIt->agreement;
    pendingBlockAckAgreements.erase(pendingIt);
    auto id = std::make_pair(addbaResponse->getReceiverAddress(), addbaResponse->getTid());
    auto activeIt = blockAckAgreements.find(id);
    if (activeIt != blockAckAgreements.end()) {
        delete activeIt->second;
        activeIt->second = agreement;
    }
    else
        blockAckAgreements[id] = agreement;
    agreement->addbaResposneSent();
    agreement->calculateExpirationTime();
    return agreement;
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

void RecipientBlockAckAgreementHandler::terminateAgreement(MacAddress originatorAddr, Tid tid)
{
    auto agreementId = std::make_pair(originatorAddr, tid);
    clearPendingAgreements(originatorAddr, tid);
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

RecipientBlockAckAgreement *RecipientBlockAckAgreementHandler::processTransmittedAddbaResp(Packet *addbaRespPacket, const Ptr<const Ieee80211AddbaResponse>& addbaResp, IBlockAckAgreementHandlerCallback *callback)
{
    // IEEE Std 802.11-2024, 11.5.2.3: the recipient agreement becomes active
    // only when the matching successful ADDBA Response is transmitted.
    auto agreement = activateAgreement(addbaRespPacket, addbaResp);
    if (agreement != nullptr)
        scheduleInactivityTimer(callback);
    return agreement;
}

void RecipientBlockAckAgreementHandler::processReceivedAddbaRequest(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *callback)
{
    EV_INFO << "Processing Addba Request from " << addbaRequest->getTransmitterAddress() << endl;
    bool accepted = addbaRequest->getDialogToken() != 0 && blockAckAgreementPolicy->isAddbaReqAccepted(addbaRequest);
    EV_DETAIL << "Building Addba Response" << endl;
    auto addbaResponse = buildAddbaResponse(addbaRequest, blockAckAgreementPolicy, accepted);
    auto addbaResponsePacket = new Packet("AddbaResponse", addbaResponse);
    if (accepted)
        stageAgreement(addbaResponsePacket, addbaRequest, addbaResponse);
    callback->processMgmtFrame(addbaResponsePacket, addbaResponse);
}

void RecipientBlockAckAgreementHandler::processTransmittedDelba(const Ptr<const Ieee80211Delba>& delba)
{
    terminateAgreement(delba->getReceiverAddress(), delba->getTid());
}

void RecipientBlockAckAgreementHandler::processReceivedDelba(const Ptr<const Ieee80211Delba>& delba, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy)
{
    if (blockAckAgreementPolicy->isDelbaAccepted(delba))
        terminateAgreement(delba->getReceiverAddress(), delba->getTid());
}

RecipientBlockAckAgreementHandler::~RecipientBlockAckAgreementHandler()
{
    for (auto it : blockAckAgreements)
        delete it.second;
    for (auto it : pendingBlockAckAgreements)
        delete it.agreement;
}

} // namespace ieee80211
} // namespace inet
