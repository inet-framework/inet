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
    for (auto id : blockAckAgreements) {
        auto agreement = id.second;
        earliestTime = std::min(earliestTime, agreement->getExpirationTime());
    }
    return earliestTime;
}

void RecipientBlockAckAgreementHandler::scheduleInactivityTimer(IBlockAckAgreementHandlerCallback *callback)
{
    callback->scheduleInactivityTimer();
}

// The inactivity timer at a recipient is reset when MPDUs corresponding to the TID for which the Block Ack
// policy is set are received and the Ack Policy subfield in the QoS Control field of that MPDU header is
// Block Ack or Implicit Block Ack Request.
//
bool RecipientBlockAckAgreementHandler::qosFrameReceived(const Ptr<const Ieee80211DataHeader>& qosHeader, IBlockAckAgreementHandlerCallback *callback)
{
    if (qosHeader->getAckPolicy() == AckPolicy::BLOCK_ACK) { // TODO + Implicit Block Ack
        Tid tid = qosHeader->getTid();
        MacAddress originatorAddr = qosHeader->getTransmitterAddress();
        auto agreement = getAgreement(tid, originatorAddr);
        if (agreement) {
            // IEEE Std 802.11-2024, 11.5.4: late Block Ack data requires retirement and discard.
            if (agreement->getExpirationTime() <= simTime()) {
                callback->expireBlockAckAgreements();
                return false;
            }
            // IEEE Std 802.11-2024, 11.5.4: matching Block Ack data resets inactivity.
            agreement->calculateExpirationTime();
            scheduleInactivityTimer(callback);
        }
    }
    return true;
}

std::vector<Ptr<Ieee80211Delba>> RecipientBlockAckAgreementHandler::blockAckAgreementExpired(IBlockAckAgreementHandlerCallback *callback)
{
    // IEEE Std 802.11-2024, 11.5.4: inactivity expiry ends the agreement and requires timeout DELBA.
    simtime_t now = simTime();
    std::vector<Ptr<Ieee80211Delba>> delbas;
    std::vector<std::unique_ptr<RecipientBlockAckAgreement>> expiredAgreements;
    for (auto it = blockAckAgreements.begin(); it != blockAckAgreements.end();) {
        auto agreement = it->second;
        if (agreement->getExpirationTime() <= now) {
            delbas.push_back(buildDelba(it->first.first, it->first.second, 39));
            expiredAgreements.emplace_back(agreement);
            it = blockAckAgreements.erase(it);
        }
        else
            ++it;
    }
    // Detach the entire batch before a callback can enter the handler again.
    for (const auto& agreement : expiredAgreements)
        callback->recipientBlockAckAgreementDeleted(agreement.get());
    return delbas;
}

//
// An originator that intends to use the Block Ack mechanism for the transmission of QoS data frames to an
// intended recipient should first check whether the intended recipient STA is capable of participating in Block
// Ack mechanism by discovering and examining its Delayed Block Ack and Immediate Block Ack capability
// bits. If the intended recipient STA is capable of participating, the originator sends an ADDBA Request frame
// indicating the TID for which the Block Ack is being set up.
//
RecipientBlockAckAgreement *RecipientBlockAckAgreementHandler::addAgreement(const Ptr<const Ieee80211AddbaRequest>& addbaReq)
{
    MacAddress originatorAddr = addbaReq->getTransmitterAddress();
    auto id = std::make_pair(originatorAddr, addbaReq->getTid());
    auto it = blockAckAgreements.find(id);
    if (it == blockAckAgreements.end()) {
        RecipientBlockAckAgreement *agreement = new RecipientBlockAckAgreement(originatorAddr, addbaReq->getTid(), addbaReq->getStartingSequenceNumber(), addbaReq->getBufferSize(), addbaReq->getBlockAckTimeoutValue());
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

void RecipientBlockAckAgreementHandler::updateAgreement(const Ptr<const Ieee80211AddbaResponse>& addbaResponse)
{
    auto id = std::make_pair(addbaResponse->getReceiverAddress(), addbaResponse->getTid());
    auto it = blockAckAgreements.find(id);
    if (it != blockAckAgreements.end()) {
        RecipientBlockAckAgreement *agreement = it->second;
        agreement->addbaResposneSent();
    }
    else
        throw cRuntimeError("Agreement is not found");
}

void RecipientBlockAckAgreementHandler::terminateAgreement(MacAddress originatorAddr, Tid tid, IBlockAckAgreementHandlerCallback *callback)
{
    auto agreementId = std::make_pair(originatorAddr, tid);
    auto it = blockAckAgreements.find(agreementId);
    if (it != blockAckAgreements.end()) {
        std::unique_ptr<RecipientBlockAckAgreement> agreement(it->second);
        blockAckAgreements.erase(it);
        callback->recipientBlockAckAgreementDeleted(agreement.get());
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
    EV_INFO << "Processing Addba Request from " << addbaRequest->getTransmitterAddress() << endl;
    if (blockAckAgreementPolicy->isAddbaReqAccepted(addbaRequest)) {
        EV_DETAIL << "Addba Request has been accepted. Creating a new Block Ack Agreement." << endl;
        auto agreement = addAgreement(addbaRequest);
        EV_DETAIL << "Agreement is added with the following parameters: " << *agreement << endl;
        EV_DETAIL << "Building Addba Response" << endl;
        auto addbaResponse = buildAddbaResponse(addbaRequest, blockAckAgreementPolicy);
        auto addbaResponsePacket = new Packet("AddbaResponse", addbaResponse);
        callback->processMgmtFrame(addbaResponsePacket, addbaResponse);
    }
}

void RecipientBlockAckAgreementHandler::processTransmittedDelba(const Ptr<const Ieee80211Delba>& delba, IBlockAckAgreementHandlerCallback *callback)
{
    // Timeout DELBA already retired its agreement before the frame entered the queue.
    // Its completion must preserve any replacement agreement for the same peer and TID.
    if (delba->getReasonCode() != 39)
        terminateAgreement(delba->getReceiverAddress(), delba->getTid(), callback);
}

void RecipientBlockAckAgreementHandler::processReceivedDelba(const Ptr<const Ieee80211Delba>& delba, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback)
{
    if (blockAckAgreementPolicy->isDelbaAccepted(delba))
        terminateAgreement(delba->getTransmitterAddress(), delba->getTid(), callback);
}

RecipientBlockAckAgreementHandler::~RecipientBlockAckAgreementHandler()
{
    for (auto it : blockAckAgreements)
        delete it.second;
}

} // namespace ieee80211
} // namespace inet

