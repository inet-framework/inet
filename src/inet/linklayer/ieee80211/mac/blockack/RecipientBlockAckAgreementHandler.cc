//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.h"

#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreement.h"

namespace inet {
namespace ieee80211 {

simtime_t RecipientBlockAckAgreementHandler::getEarliestExpirationTime() const
{
    simtime_t earliestTime = SIMTIME_MAX;
    for (const auto& entry : blockAckAgreements)
        if (entry.second->getIsAddbaResponseSent())
            earliestTime = std::min(earliestTime, entry.second->getExpirationTime());
    return earliestTime;
}

void RecipientBlockAckAgreementHandler::scheduleInactivityTimer(IBlockAckAgreementHandlerCallback *callback)
{
    callback->rescheduleInactivityTimer();
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
        if (agreement && agreement->getIsAddbaResponseSent() && !qosHeader->isIncorrect() &&
                qosHeader->getReceiverAddress() == agreement->getRecipientAddr() &&
                qosHeader->getFragmentNumber() == 0 && !qosHeader->getMoreFragments()) {
            agreement->calculateExpirationTime();
            scheduleInactivityTimer(callback);
        }
    }
}

void RecipientBlockAckAgreementHandler::processReceivedBlockAckReq(const Ptr<const Ieee80211BlockAckReq>& blockAckReq, IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback)
{
    auto compressed = dynamicPtrCast<const Ieee80211CompressedBlockAckReq>(blockAckReq);
    if (!compressed || compressed->getFragmentNumber() != 0 || compressed->getMultiTid() ||
            !compressed->getCompressedBitmap() || compressed->isIncorrect())
        return;
    auto agreement = getAgreement(compressed->getTidInfo(), compressed->getTransmitterAddress());
    if (agreement && agreement->getIsAddbaResponseSent() &&
            blockAckReq->getReceiverAddress() == agreement->getRecipientAddr()) {
        agreement->getBlockAckRecord()->blockAckReqReceived(compressed->getStartingSequenceNumber());
        agreement->calculateExpirationTime();
        scheduleInactivityTimer(agreementHandlerCallback);
    }
    else {
        // IEEE Std 802.11-2024, 11.5.4: no implicit agreement, no BlockAck response.
        auto delba = buildDelba(compressed->getTransmitterAddress(), compressed->getTidInfo(), 38); // UNKNOWN_BA
        procedureCallback->processMgmtFrame(new Packet("Delba", delba), delba);
    }
}

void RecipientBlockAckAgreementHandler::blockAckAgreementExpired(IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback)
{
    // IEEE Std 802.11-2024, 11.5.4: remove all due state before callbacks.
    std::vector<std::pair<std::unique_ptr<RecipientBlockAckAgreement>, Ptr<Ieee80211Delba>>> expired;
    for (auto it = blockAckAgreements.begin(); it != blockAckAgreements.end(); ) {
        auto agreement = it->second;
        if (agreement->getIsAddbaResponseSent() && agreement->getExpirationTime() != SIMTIME_MAX &&
                agreement->getExpirationTime() <= simTime()) {
            auto delba = buildDelba(it->first.first, it->first.second, 39); // TIMEOUT
            expired.emplace_back(std::unique_ptr<RecipientBlockAckAgreement>(agreement), delba);
            it = blockAckAgreements.erase(it);
        }
        else
            ++it;
    }
    for (const auto& entry : expired) {
        const auto& delba = entry.second;
        agreementHandlerCallback->recipientAgreementTerminated(delba->getReceiverAddress(), delba->getTid());
        agreementHandlerCallback->blockAckAgreementTerminated(entry.first.get());
        procedureCallback->processMgmtFrame(new Packet("Delba", delba), delba);
    }
    scheduleInactivityTimer(agreementHandlerCallback);
}

//
// An originator that intends to use the Block Ack mechanism for the transmission of QoS data frames to an
// intended recipient should first check whether the intended recipient STA is capable of participating in Block
// Ack mechanism by discovering and examining its Delayed Block Ack and Immediate Block Ack capability
// bits. If the intended recipient STA is capable of participating, the originator sends an ADDBA Request frame
// indicating the TID for which the Block Ack is being set up.
//
RecipientBlockAckAgreement *RecipientBlockAckAgreementHandler::addAgreement(const Ptr<const Ieee80211AddbaRequest>& addbaReq, const Ptr<const Ieee80211AddbaResponse>& addbaResponse)
{
    MacAddress originatorAddr = addbaReq->getTransmitterAddress();
    auto id = std::make_pair(originatorAddr, addbaReq->getTid());
    auto it = blockAckAgreements.find(id);
    if (it == blockAckAgreements.end()) {
        RecipientBlockAckAgreement *agreement = new RecipientBlockAckAgreement(originatorAddr, addbaReq->getTid(), addbaReq->getStartingSequenceNumber(), addbaResponse->getBufferSize(), addbaResponse->getBlockAckTimeoutValue());
        agreement->setRecipientAddr(addbaReq->getReceiverAddress());
        agreement->setDialogToken(addbaResponse->getDialogToken());
        agreement->setIsAMsduSupported(addbaResponse->getAMsduSupported());
        blockAckAgreements[id] = agreement;
        EV_DETAIL << "Block Ack Agreement is added with the following parameters: " << *agreement << endl;
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
    auto response = makeShared<Ieee80211AddbaResponse>();
    response->setReceiverAddress(addbaRequest->getTransmitterAddress());
    response->setDialogToken(addbaRequest->getDialogToken());
    response->setTid(addbaRequest->getTid());
    response->setBlockAckPolicy(true);
    response->setBufferSize(std::min(64, std::min<int>(addbaRequest->getBufferSize(), blockAckAgreementPolicy->getMaximumAllowedBufferSize())));
    response->setBlockAckTimeoutValue(blockAckAgreementPolicy->getBlockAckTimeoutValue() == 0 ?
            addbaRequest->getBlockAckTimeoutValue() : blockAckAgreementPolicy->getBlockAckTimeoutValue());
    response->setAMsduSupported(addbaRequest->getAMsduSupported() && blockAckAgreementPolicy->aMsduSupported());
    return response;
}

void RecipientBlockAckAgreementHandler::updateAgreement(const Ptr<const Ieee80211AddbaResponse>& addbaResponse)
{
    auto agreement = getAgreement(addbaResponse->getTid(), addbaResponse->getReceiverAddress());
    if (agreement && !agreement->getIsAddbaResponseSent() && addbaResponse->getStatusCode() == 0 &&
            agreement->getDialogToken() == addbaResponse->getDialogToken() &&
            agreement->getBufferSize() == addbaResponse->getBufferSize() &&
            agreement->getBlockAckTimeoutValue() == addbaResponse->getBlockAckTimeoutValue() &&
            agreement->getIsAMsduSupported() == addbaResponse->getAMsduSupported() && addbaResponse->getBlockAckPolicy())
        agreement->addbaResposneSent();
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
    updateAgreement(addbaResp);
    scheduleInactivityTimer(callback);
}

void RecipientBlockAckAgreementHandler::processReceivedAddbaRequest(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *callback)
{
    auto response = buildAddbaResponse(addbaRequest, blockAckAgreementPolicy);
    // IEEE Std 802.11-2024, 10.25.2: a parsed request always receives a response.
    auto agreement = getAgreement(addbaRequest->getTid(), addbaRequest->getTransmitterAddress());
    if (!blockAckAgreementPolicy->isAddbaReqAccepted(addbaRequest) || (agreement && agreement->getIsAddbaResponseSent()))
        response->setStatusCode(37); // REFUSED
    else if (agreement && agreement->getDialogToken() == addbaRequest->getDialogToken()) {
        // A retry reuses the same pending transaction; it cannot change its receive window.
        if (agreement->getStartingSequenceNumber() != addbaRequest->getStartingSequenceNumber() ||
                agreement->getBufferSize() != response->getBufferSize() ||
                agreement->getBlockAckTimeoutValue() != response->getBlockAckTimeoutValue() ||
                agreement->getIsAMsduSupported() != response->getAMsduSupported())
            response->setStatusCode(37); // REFUSED
    }
    else {
        // Replace inactive state after acceptance. Old response tokens cannot activate this agreement.
        terminateAgreement(addbaRequest->getTransmitterAddress(), addbaRequest->getTid());
        addAgreement(addbaRequest, response);
    }
    callback->processMgmtFrame(new Packet("AddbaResponse", response), response);
}

void RecipientBlockAckAgreementHandler::processTransmittedDelba(const Ptr<const Ieee80211Delba>& delba)
{
    // TIMEOUT expiry already completed local teardown before it queued this frame.
    if (!delba->getInitiator() && delba->getReasonCode() != 39)
        terminateAgreement(delba->getReceiverAddress(), delba->getTid());
}

void RecipientBlockAckAgreementHandler::processReceivedDelba(const Ptr<const Ieee80211Delba>& delba, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy)
{
    if (delba->getInitiator() && blockAckAgreementPolicy->isDelbaAccepted(delba))
        terminateAgreement(delba->getTransmitterAddress(), delba->getTid());
}

RecipientBlockAckAgreementHandler::~RecipientBlockAckAgreementHandler()
{
    for (auto it : blockAckAgreements)
        delete it.second;
}

} // namespace ieee80211
} // namespace inet
