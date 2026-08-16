//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementHandler.h"

#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreement.h"

namespace inet {
namespace ieee80211 {

void OriginatorBlockAckAgreementHandler::createAgreement(const Ptr<const Ieee80211AddbaRequest>& addbaRequest)
{
    ASSERT(addbaRequest->getDialogToken() != 0);
    OriginatorBlockAckAgreement *blockAckAgreement = new OriginatorBlockAckAgreement(addbaRequest->getReceiverAddress(), addbaRequest->getTid(), addbaRequest->getStartingSequenceNumber(), addbaRequest->getBufferSize(), addbaRequest->getAMsduSupported(), addbaRequest->getBlockAckPolicy() == 0, addbaRequest->getDialogToken());
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
        if (agreement->getIsAddbaResponseReceived()) {
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

void OriginatorBlockAckAgreementHandler::addbaResponseTimeoutExpired(IBlockAckAgreementHandlerCallback *callback)
{
    auto now = simTime();
    for (auto it = blockAckAgreements.begin(); it != blockAckAgreements.end();) {
        auto agreement = it->second;
        if (agreement->isPending() && agreement->getIsAddbaRequestSent() && agreement->getAddbaResponseDeadline() <= now) {
            EV_INFO << "ADDBA Response timeout for receiver=" << agreement->getReceiverAddr() << " tid=" << (int)agreement->getTid() << endl;
            it = blockAckAgreements.erase(it);
            delete agreement;
        }
        else
            it++;
    }
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
        if (agreement) {
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
    if (earliestExpirationTime != SIMTIME_MAX)
        callback->scheduleInactivityTimer(earliestExpirationTime);
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
        createAgreement(addbaReq);
        auto addbaPacket = new Packet("AddbaReq", addbaReq);
        callback->processMgmtFrame(addbaPacket, addbaReq);
    }
}

OriginatorBlockAckAgreement *OriginatorBlockAckAgreementHandler::processReceivedAddbaResp(const Ptr<const Ieee80211AddbaResponse>& addbaResp, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback)
{
    auto agreement = getAgreement(addbaResp->getTransmitterAddress(), addbaResp->getTid());
    // IEEE Std 802.11-2024, 11.5.2.2: only a successful response matching the
    // outstanding peer, TID, and Dialog Token establishes the agreement.
    if (agreement == nullptr || !agreement->isPending() || !agreement->getIsAddbaRequestSent() || agreement->getDialogToken() != addbaResp->getDialogToken())
        return nullptr;
    if (addbaResp->getStatusCode() == 0 && blockAckAgreementPolicy->isAddbaReqAccepted(addbaResp, agreement)) {
        updateAgreement(agreement, addbaResp);
        scheduleInactivityTimer(callback);
        scheduleAddbaResponseTimer(callback);
        return agreement;
    }
    else {
        terminateAgreement(addbaResp->getTransmitterAddress(), addbaResp->getTid());
        scheduleAddbaResponseTimer(callback);
        return nullptr;
    }
}

void OriginatorBlockAckAgreementHandler::updateAgreement(OriginatorBlockAckAgreement *agreement, const Ptr<const Ieee80211AddbaResponse>& addbaResp)
{
    agreement->setIsAddbaResponseReceived(true);
    agreement->setBufferSize(addbaResp->getBufferSize());
    agreement->setBlockAckTimeoutValue(addbaResp->getBlockAckTimeoutValue());
    agreement->calculateExpirationTime();
}

void OriginatorBlockAckAgreementHandler::processTransmittedAddbaReq(const Ptr<const Ieee80211AddbaRequest>& addbaReq, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback)
{
    auto agreement = getAgreement(addbaReq->getReceiverAddress(), addbaReq->getTid());
    if (agreement && agreement->isPending() && agreement->getDialogToken() == addbaReq->getDialogToken()) {
        if (agreement->getAddbaResponseDeadline() < 0) {
            auto addbaFailureTimeout = blockAckAgreementPolicy->computeAddbaFailureTimeout();
            if (addbaFailureTimeout <= 0)
                throw cRuntimeError("ADDBA failure timeout must be greater than zero");
            agreement->setAddbaResponseDeadline(simTime() + addbaFailureTimeout);
        }
        agreement->setIsAddbaRequestSent(true);
        scheduleAddbaResponseTimer(callback);
    }
    else
        EV_WARN << "Ignoring stale transmitted ADDBA Request for receiver=" << addbaReq->getReceiverAddress() << " tid=" << (int)addbaReq->getTid() << " dialogToken=" << (int)addbaReq->getDialogToken() << endl;
}

void OriginatorBlockAckAgreementHandler::processTransmittedDelba(const Ptr<const Ieee80211Delba>& delba)
{
    terminateAgreement(delba->getReceiverAddress(), delba->getTid());
}

void OriginatorBlockAckAgreementHandler::processReceivedDelba(const Ptr<const Ieee80211Delba>& delba, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy)
{
    if (blockAckAgreementPolicy->isDelbaAccepted(delba))
        terminateAgreement(delba->getTransmitterAddress(), delba->getTid());
}

OriginatorBlockAckAgreementHandler::~OriginatorBlockAckAgreementHandler()
{
    for (auto it : blockAckAgreements)
        delete it.second;
}

} // namespace ieee80211
} // namespace inet
