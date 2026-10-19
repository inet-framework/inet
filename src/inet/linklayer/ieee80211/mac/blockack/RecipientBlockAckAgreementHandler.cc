//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.h"

#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"

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
    callback->scheduleInactivityTimer();
}

// The inactivity timer at a recipient is reset when MPDUs corresponding to the TID for which the Block Ack
// policy is set are received and the Ack Policy subfield in the QoS Control field of that MPDU header is
// Block Ack or Implicit Block Ack Request.
//
void RecipientBlockAckAgreementHandler::qosFrameReceived(const Ptr<const Ieee80211DataHeader>& qosHeader, IBlockAckAgreementHandlerCallback *callback)
{
    // IEEE Std 802.11-2024, 11.5.4: matching Block Ack-policy DATA refreshes inactivity, including a duplicate.
    if (qosHeader->getAckPolicy() == AckPolicy::BLOCK_ACK) {
        Tid tid = qosHeader->getTid();
        MacAddress originatorAddr = qosHeader->getTransmitterAddress();
        auto agreement = getAgreement(tid, originatorAddr);
        if (agreement) {
            agreement->calculateExpirationTime();
            scheduleInactivityTimer(callback);
        }
    }
}

void RecipientBlockAckAgreementHandler::implicitBlockAckRequested(const Ptr<const Ieee80211DataHeader>& qosHeader,
        IBlockAckAgreementHandlerCallback *callback)
{
    // IEEE Std 802.11-2024, 11.5.4: valid Implicit BAR DATA also refreshes inactivity, including duplicates.
    auto agreement = getAgreement(qosHeader->getTid(), qosHeader->getTransmitterAddress());
    if (agreement && agreement->getVariant() == BlockAckVariant::COMPRESSED && qosHeader->getAckPolicy() == NORMAL_ACK) {
        agreement->calculateExpirationTime();
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
    std::vector<std::pair<MacAddress, Tid>> expired;
    for (auto id : blockAckAgreements) {
        auto agreement = id.second;
        if (agreement->getExpirationTime() <= now)
            expired.push_back(id.first);
    }
    for (const auto& id : expired) {
        const auto& delba = buildDelba(id.first, id.second, 39);
        auto delbaPacket = new Packet("Delba", delba);
        procedureCallback->processMgmtFrame(delbaPacket, delba);
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
    addbaResponse->setDialogToken(addbaRequest->getDialogToken());
    // IEEE Std 802.11-2024, 10.25.2: the request allocation and timeout are advisory.
    addbaResponse->setBufferSize(std::min(64, blockAckAgreementPolicy->getMaximumAllowedBufferSize()));
    addbaResponse->setBlockAckTimeoutValue(blockAckAgreementPolicy->getBlockAckTimeoutValue());
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
    EV_INFO << "Processing Addba Request from " << addbaRequest->getTransmitterAddress() << endl;
    auto id = std::make_pair(addbaRequest->getTransmitterAddress(), addbaRequest->getTid());
    auto addbaResponse = buildAddbaResponse(addbaRequest, blockAckAgreementPolicy);
    bool accepted = blockAckAgreementPolicy->isAddbaReqAccepted(addbaRequest) &&
        blockAckAgreements.count(id) == 0 && pendingTeardowns.count(id) == 0;
    addbaResponse->setStatusCode(accepted ? SC_SUCCESSFUL : SC_UNSPECIFIED);
    if (accepted) {
        EV_DETAIL << "Addba Request has been accepted. Creating a new Block Ack Agreement." << endl;
        auto parameters = makeShared<Ieee80211AddbaRequest>(*addbaRequest);
        parameters->setBufferSize(addbaResponse->getBufferSize());
        parameters->setBlockAckTimeoutValue(addbaResponse->getBlockAckTimeoutValue());
        auto agreement = addAgreement(parameters);
        EV_DETAIL << "Agreement is added with the following parameters: " << *agreement << endl;
        EV_DETAIL << "Building Addba Response" << endl;
    }
    callback->processMgmtFrame(new Packet("AddbaResponse", addbaResponse), addbaResponse);
}

RecipientBlockAckAgreement *RecipientBlockAckAgreementHandler::detachAgreement(MacAddress peer, Tid tid)
{
    auto it = blockAckAgreements.find({peer, tid});
    if (it == blockAckAgreements.end())
        return nullptr;
    auto agreement = it->second;
    blockAckAgreements.erase(it);
    return agreement;
}

std::vector<RecipientBlockAckAgreement *> RecipientBlockAckAgreementHandler::detachPeer(MacAddress peer)
{
    std::vector<RecipientBlockAckAgreement *> removed;
    for (auto it = blockAckAgreements.begin(); it != blockAckAgreements.end();) {
        if (it->first.first == peer) {
            removed.push_back(it->second);
            it = blockAckAgreements.erase(it);
        }
        else
            ++it;
    }
    for (auto it = pendingTeardowns.begin(); it != pendingTeardowns.end();) {
        if (it->first == peer)
            it = pendingTeardowns.erase(it);
        else
            ++it;
    }
    return removed;
}

void RecipientBlockAckAgreementHandler::processTransmittedDelba(const Ptr<const Ieee80211Delba>& delba)
{
    // HCF retains teardown state until the Action reaches a terminal result.
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

