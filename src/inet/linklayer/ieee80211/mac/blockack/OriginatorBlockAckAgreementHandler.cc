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
    OriginatorBlockAckAgreement *blockAckAgreement = new OriginatorBlockAckAgreement(addbaRequest->getReceiverAddress(), addbaRequest->getTid(), addbaRequest->getStartingSequenceNumber(), addbaRequest->getBufferSize(), addbaRequest->getAMsduSupported(), addbaRequest->getBlockAckPolicy() == 0);
    auto agreementId = std::make_pair(addbaRequest->getReceiverAddress(), addbaRequest->getTid());
    terminateAgreement(addbaRequest->getReceiverAddress(), addbaRequest->getTid());
    blockAckAgreement->setOriginatorAddr(addbaRequest->getTransmitterAddress());
    blockAckAgreement->setDialogToken(addbaRequest->getDialogToken());
    blockAckAgreement->setBlockAckTimeoutValue(addbaRequest->getBlockAckTimeoutValue());
    blockAckAgreements[agreementId] = blockAckAgreement;
}

simtime_t OriginatorBlockAckAgreementHandler::getEarliestExpirationTime() const
{
    simtime_t earliestTime = SIMTIME_MAX;
    for (const auto& entry : blockAckAgreements)
        if (entry.second->getIsAddbaResponseReceived())
            earliestTime = std::min(earliestTime, entry.second->getExpirationTime());
    return earliestTime;
}

void OriginatorBlockAckAgreementHandler::blockAckAgreementExpired(IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback)
{
    // IEEE Std 802.11-2024, 11.5.4: remove all due state before callbacks.
    std::vector<std::pair<std::unique_ptr<OriginatorBlockAckAgreement>, Ptr<Ieee80211Delba>>> expired;
    for (auto it = blockAckAgreements.begin(); it != blockAckAgreements.end(); ) {
        auto agreement = it->second;
        if (agreement->getIsAddbaResponseReceived() && agreement->getExpirationTime() != SIMTIME_MAX &&
                agreement->getExpirationTime() <= simTime()) {
            auto delba = buildDelba(it->first.first, it->first.second, 39); // TIMEOUT
            expired.emplace_back(std::unique_ptr<OriginatorBlockAckAgreement>(agreement), delba);
            it = blockAckAgreements.erase(it);
        }
        else
            ++it;
    }
    for (const auto& entry : expired) {
        const auto& delba = entry.second;
        agreementHandlerCallback->blockAckAgreementTerminated(entry.first.get());
        procedureCallback->processMgmtFrame(new Packet("Delba", delba), delba);
    }
    scheduleInactivityTimer(agreementHandlerCallback);
}

const Ptr<Ieee80211AddbaRequest> OriginatorBlockAckAgreementHandler::buildAddbaRequest(MacAddress receiverAddr, Tid tid, SequenceNumberCyclic startingSequenceNumber, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy)
{
    auto addbaRequest = makeShared<Ieee80211AddbaRequest>();
    addbaRequest->setReceiverAddress(receiverAddr);
    addbaRequest->setDialogToken(nextDialogToken++);
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
    MacAddress peer;
    Tid tid;
    if (auto compressed = dynamicPtrCast<const Ieee80211CompressedBlockAck>(blockAck)) {
        if (compressed->getMultiTid() || !compressed->getCompressedBitmap() ||
                compressed->getFragmentNumber() != 0 || compressed->getBlockAckBitmap().getSize() != 64 ||
                compressed->getChunkLength() != LENGTH_COMPRESSED_BLOCKACK - B(4) || blockAck->isIncorrect())
            return;
        peer = compressed->getTransmitterAddress();
        tid = compressed->getTidInfo();
    }
    else if (auto basic = dynamicPtrCast<const Ieee80211BasicBlockAck>(blockAck)) {
        peer = basic->getTransmitterAddress();
        tid = basic->getTidInfo();
    }
    else
        return;
    auto agreement = getAgreement(peer, tid);
    if (agreement && agreement->getIsAddbaResponseReceived() && !blockAck->isIncorrect() &&
            blockAck->getReceiverAddress() == agreement->getOriginatorAddr()) {
        // IEEE Std 802.11-2024, 11.5.4. Bitmap retirement owns window movement.
        agreement->calculateExpirationTime();
        scheduleInactivityTimer(callback);
    }
}

void OriginatorBlockAckAgreementHandler::scheduleInactivityTimer(IBlockAckAgreementHandlerCallback *callback)
{
    callback->rescheduleInactivityTimer();
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
        createAgreement(addbaReq);
        auto addbaPacket = new Packet("AddbaReq", addbaReq);
        callback->processMgmtFrame(addbaPacket, addbaReq);
    }
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
    agreement->setIsAMsduSupported(addbaResp->getAMsduSupported());
    agreement->setIsDelayedBlockAckPolicySupported(!addbaResp->getBlockAckPolicy());
    agreement->setIsAddbaResponseReceived(true);
    agreement->setBufferSize(addbaResp->getBufferSize());
    agreement->setBlockAckTimeoutValue(addbaResp->getBlockAckTimeoutValue());
    agreement->calculateExpirationTime();
}

void OriginatorBlockAckAgreementHandler::processTransmittedAddbaReq(const Ptr<const Ieee80211AddbaRequest>& addbaReq)
{
    auto agreement = getAgreement(addbaReq->getReceiverAddress(), addbaReq->getTid());
    if (agreement && agreement->getDialogToken() == addbaReq->getDialogToken()) {
        agreement->setOriginatorAddr(addbaReq->getTransmitterAddress());
        agreement->setIsAddbaRequestSent(true);
    }
    else
        throw cRuntimeError("Block Ack Agreement should have already been added");
}

void OriginatorBlockAckAgreementHandler::processTransmittedDelba(const Ptr<const Ieee80211Delba>& delba)
{
    if (delba->getInitiator())
        terminateAgreement(delba->getReceiverAddress(), delba->getTid());
}

void OriginatorBlockAckAgreementHandler::processReceivedDelba(const Ptr<const Ieee80211Delba>& delba, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy)
{
    if (!delba->getInitiator() && blockAckAgreementPolicy->isDelbaAccepted(delba))
        terminateAgreement(delba->getTransmitterAddress(), delba->getTid());
}

OriginatorBlockAckAgreementHandler::~OriginatorBlockAckAgreementHandler()
{
    for (auto it : blockAckAgreements)
        delete it.second;
}

} // namespace ieee80211
} // namespace inet

