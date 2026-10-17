//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementHandler.h"

#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211DataSequenceTag_m.h"

namespace inet {
namespace ieee80211 {

void OriginatorBlockAckAgreementHandler::noteEligibleDataTransmission(MacAddress peer, Tid tid)
{
    Procedure procedure;
    procedure.state = ProcedureState::WAITING_FOR_SUCCESSOR;
    procedures.emplace(AgreementId(peer, tid), procedure);
}

void OriginatorBlockAckAgreementHandler::beginSetupIntent(MacAddress peer, Tid tid)
{
    auto result = procedures.emplace(AgreementId(peer, tid), Procedure());
    if (result.first->second.state == ProcedureState::WAITING_FOR_SUCCESSOR)
        result.first->second.state = ProcedureState::SETUP_INTENT;
}

void OriginatorBlockAckAgreementHandler::beginTeardown(MacAddress peer, Tid tid)
{
    Procedure procedure;
    procedure.state = ProcedureState::TEARDOWN_PENDING;
    procedures[{peer, tid}] = procedure;
}

void OriginatorBlockAckAgreementHandler::failTeardown(MacAddress peer, Tid tid)
{
    Procedure procedure;
    procedure.state = ProcedureState::RECOVERY_NEEDED;
    procedures[{peer, tid}] = procedure;
}

void OriginatorBlockAckAgreementHandler::finishProcedure(MacAddress peer, Tid tid)
{
    procedures.erase({peer, tid});
}

void OriginatorBlockAckAgreementHandler::markRequestAttempted(MacAddress peer, Tid tid)
{
    auto it = procedures.find({peer, tid});
    if (it != procedures.end() && it->second.state == ProcedureState::REQUEST_PENDING)
        it->second.requestAttempted = true;
}

void OriginatorBlockAckAgreementHandler::acknowledgeRequest(MacAddress peer, Tid tid, simtime_t timeout)
{
    auto it = procedures.find({peer, tid});
    if (it != procedures.end() && it->second.state == ProcedureState::REQUEST_PENDING)
        it->second.responseDeadline = simTime() + timeout;
}

void OriginatorBlockAckAgreementHandler::createAgreement(const Ptr<const Ieee80211AddbaRequest>& addbaRequest)
{
    OriginatorBlockAckAgreement *blockAckAgreement = new OriginatorBlockAckAgreement(addbaRequest->getReceiverAddress(), addbaRequest->getTid(), addbaRequest->getStartingSequenceNumber(), addbaRequest->getBufferSize(), addbaRequest->getAMsduSupported(), addbaRequest->getBlockAckPolicy() == 0);
    auto agreementId = std::make_pair(addbaRequest->getReceiverAddress(), addbaRequest->getTid());
    blockAckAgreements[agreementId] = blockAckAgreement;
    blockAckAgreement->setDialogToken(addbaRequest->getDialogToken());
    procedures[agreementId].state = ProcedureState::REQUEST_PENDING;
}

OriginatorBlockAckAgreement *OriginatorBlockAckAgreementHandler::detachAgreement(MacAddress peer, Tid tid)
{
    auto it = blockAckAgreements.find({peer, tid});
    if (it == blockAckAgreements.end())
        return nullptr;
    auto agreement = it->second;
    blockAckAgreements.erase(it);
    return agreement;
}

std::vector<OriginatorBlockAckAgreement *> OriginatorBlockAckAgreementHandler::detachPeer(MacAddress peer)
{
    std::vector<OriginatorBlockAckAgreement *> removed;
    for (auto it = blockAckAgreements.begin(); it != blockAckAgreements.end();) {
        if (it->first.first == peer) {
            removed.push_back(it->second);
            it = blockAckAgreements.erase(it);
        }
        else
            ++it;
    }
    for (auto it = procedures.begin(); it != procedures.end();) {
        if (it->first.first == peer)
            it = procedures.erase(it);
        else
            ++it;
    }
    return removed;
}

void OriginatorBlockAckAgreementHandler::startSetup(MacAddress peer, Tid tid, SequenceNumberCyclic sequence,
        IOriginatorBlockAckAgreementPolicy *policy, IProcedureCallback *callback, bool aMsdu)
{
    auto request = buildAddbaRequest(peer, tid, sequence, policy);
    request->setAMsduSupported(aMsdu && policy->isMsduSupported());
    // IEEE Std 802.11-2024, Table 9-466: ordinary setup uses a nonzero dialog token.
    auto& token = dialogTokens[{peer, tid}];
    token = token % 255 + 1;
    request->setDialogToken(token);
    createAgreement(request);
    callback->processMgmtFrame(new Packet("AddbaReq", request), request);
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
    // When a timeout of BlockAckTimeout is detected, the STA shall send a DELBA frame to the
    // peer STA with the Reason Code field set to TIMEOUT and shall issue a MLME-DELBA.indication
    // primitive with the ReasonCode parameter having a value of TIMEOUT.
    // The procedure is illustrated in Figure 10-14.
    simtime_t now = simTime();
    std::vector<AgreementId> expired;
    for (auto id : blockAckAgreements) {
        auto agreement = id.second;
        if (agreement->getIsAddbaResponseReceived() && agreement->getExpirationTime() <= now)
            expired.push_back(id.first);
    }
    for (const auto& id : expired) {
        const auto& delba = buildDelba(id.first, id.second, 39);
        auto delbaPacket = new Packet("Delba", delba);
        procedureCallback->processMgmtFrame(delbaPacket, delba);
    }
    scheduleInactivityTimer(agreementHandlerCallback);
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
            agreement->calculateExpirationTime();
            scheduleInactivityTimer(callback);
        }
    }
    else
        throw cRuntimeError("Unsupported BlockAck");
}

void OriginatorBlockAckAgreementHandler::scheduleInactivityTimer(IBlockAckAgreementHandlerCallback *callback)
{
    callback->scheduleInactivityTimer();
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
    if (agreement && agreement->getIsAddbaResponseReceived())
        agreement->recordTransmittedData(packet->getTag<Ieee80211DataSequenceTag>()->getAllocationIndex());
}

void OriginatorBlockAckAgreementHandler::processReceivedAddbaResp(const Ptr<const Ieee80211AddbaResponse>& addbaResp, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback)
{
    auto agreement = getAgreement(addbaResp->getTransmitterAddress(), addbaResp->getTid());
    if (blockAckAgreementPolicy->isAddbaReqAccepted(addbaResp, agreement)) {
        updateAgreement(agreement, addbaResp);
        scheduleInactivityTimer(callback);
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
    agreement->setResponseAMsduSupported(addbaResp->getAMsduSupported());
    agreement->calculateExpirationTime();
}

void OriginatorBlockAckAgreementHandler::processTransmittedAddbaReq(const Ptr<const Ieee80211AddbaRequest>& addbaReq)
{
    auto agreement = getAgreement(addbaReq->getReceiverAddress(), addbaReq->getTid());
    if (agreement)
        agreement->setIsAddbaRequestSent(true);
}

void OriginatorBlockAckAgreementHandler::processTransmittedDelba(const Ptr<const Ieee80211Delba>& delba)
{
    // HCF retires the agreement before it queues DELBA. ACK completes teardown.
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

