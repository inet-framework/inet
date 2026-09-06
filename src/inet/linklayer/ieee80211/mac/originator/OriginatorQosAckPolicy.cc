//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/originator/OriginatorQosAckPolicy.h"

#include "inet/linklayer/ieee80211/mac/blockack/BlockAckWindow.h"

#include <tuple>

#include "inet/linklayer/ieee80211/mac/contract/IOriginatorBlockAckAgreementHandler.h"

namespace inet {
namespace ieee80211 {

Define_Module(OriginatorQosAckPolicy);

void OriginatorQosAckPolicy::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        rateSelection = check_and_cast<IQosRateSelection *>(getModuleByPath(par("rateSelectionModule")));
        maxBlockAckPolicyFrameLength = par("maxBlockAckPolicyFrameLength");
        blockAckReqThreshold = par("blockAckReqThreshold");
        blockAckTimeout = par("blockAckTimeout");
        ackTimeout = par("ackTimeout");
    }
}

bool OriginatorQosAckPolicy::isAckNeeded(const Ptr<const Ieee80211MgmtHeader>& header) const
{
    return !header->getReceiverAddress().isMulticast();
}

std::map<std::pair<MacAddress, Tid>, std::vector<Packet *>> OriginatorQosAckPolicy::getOutstandingFramesPerAgreement(InProgressFrames *inProgressFrames, IOriginatorBlockAckAgreementHandler *blockAckAgreementHandler) const
{
    auto outstandingFrames = inProgressFrames->getOutstandingFrames();
    std::map<std::pair<MacAddress, Tid>, std::vector<Packet *>> outstandingFramesPerAgreement;
    if (blockAckAgreementHandler == nullptr)
        return outstandingFramesPerAgreement;
    for (auto frame : outstandingFrames) {
        auto dataHeader = frame->peekAtFront<Ieee80211DataHeader>();
        auto receiverAddress = dataHeader->getReceiverAddress();
        auto tid = dataHeader->getTid();
        if (blockAckAgreementHandler->getActiveAgreement(receiverAddress, tid) != nullptr)
            outstandingFramesPerAgreement[std::make_pair(receiverAddress, tid)].push_back(frame);
    }
    return outstandingFramesPerAgreement;
}

SequenceNumberCyclic OriginatorQosAckPolicy::computeStartingSequenceNumber(const std::vector<Packet *>& outstandingFrames) const
{
    ASSERT(outstandingFrames.size() > 0);
    auto startingSequenceNumber = outstandingFrames[0]->peekAtFront<Ieee80211DataHeader>()->getSequenceNumber();
    for (size_t i = 1; i < outstandingFrames.size(); i++) {
        auto seqNum = outstandingFrames[i]->peekAtFront<Ieee80211DataHeader>()->getSequenceNumber();
        if (seqNum < startingSequenceNumber)
            startingSequenceNumber = seqNum;
    }
    return startingSequenceNumber;
}

bool OriginatorQosAckPolicy::isCompressedBlockAckReq(const std::vector<Packet *>& outstandingFrames, OriginatorBlockAckAgreement *agreement) const
{
    return isCompressedBlockAckReqNeeded(outstandingFrames, agreement);
}

bool OriginatorQosAckPolicy::isCompressedBlockAckReqNeeded(const std::vector<Packet *>& outstandingFrames, OriginatorBlockAckAgreement *agreement)
{
    // IEEE Std 802.11-2024, Table 11-8 and 10.25.6.1: use the compressed
    // variant only for an established immediate HT Block Ack agreement.
    // The agreement snapshots peer capability state when it is established.
    if (agreement == nullptr || !agreement->getIsCompressedBlockAckSupported() || !agreement->getIsAddbaResponseReceived() || agreement->getIsDelayedBlockAckPolicySupported())
        return false;
    bool hasMatchingOutstandingFrame = false;
    SequenceNumberCyclic startingSequenceNumber;
    for (auto frame : outstandingFrames) {
        auto header = dynamicPtrCast<const Ieee80211DataHeader>(frame->peekAtFront<Ieee80211MacHeader>());
        if (header == nullptr || header->getReceiverAddress() != agreement->getReceiverAddr() || header->getTid() != agreement->getTid())
            continue;
        if (!hasMatchingOutstandingFrame || header->getSequenceNumber() < startingSequenceNumber)
            startingSequenceNumber = header->getSequenceNumber();
        hasMatchingOutstandingFrame = true;
        if (header->getFragmentNumber() != 0 || header->getMoreFragments())
            return false;
    }
    for (auto frame : outstandingFrames) {
        auto header = dynamicPtrCast<const Ieee80211DataHeader>(frame->peekAtFront<Ieee80211MacHeader>());
        if (header != nullptr && header->getReceiverAddress() == agreement->getReceiverAddr() && header->getTid() == agreement->getTid()) {
            if (!BlockAckWindow::isWithin(startingSequenceNumber, 64, header->getSequenceNumber()))
                return false;
        }
    }
    return hasMatchingOutstandingFrame;
}

// FIXME
bool OriginatorQosAckPolicy::isBlockAckReqNeeded(InProgressFrames *inProgressFrames, TxopProcedure *txopProcedure, IOriginatorBlockAckAgreementHandler *blockAckAgreementHandler) const
{
    auto outstandingFramesPerAgreement = getOutstandingFramesPerAgreement(inProgressFrames, blockAckAgreementHandler);
    for (auto outstandingFrames : outstandingFramesPerAgreement) {
        if ((int)outstandingFrames.second.size() >= blockAckReqThreshold)
            return true;
    }
    return false;
}

// FIXME
std::tuple<MacAddress, SequenceNumberCyclic, Tid> OriginatorQosAckPolicy::computeBlockAckReqParameters(InProgressFrames *inProgressFrames, TxopProcedure *txopProcedure, IOriginatorBlockAckAgreementHandler *blockAckAgreementHandler) const
{
    auto outstandingFramesPerAgreement = getOutstandingFramesPerAgreement(inProgressFrames, blockAckAgreementHandler);
    auto largestOutstandingFrames = outstandingFramesPerAgreement.end();
    for (auto it = outstandingFramesPerAgreement.begin(); it != outstandingFramesPerAgreement.end(); it++) {
        if ((int)it->second.size() >= blockAckReqThreshold && (largestOutstandingFrames == outstandingFramesPerAgreement.end() || it->second.size() > largestOutstandingFrames->second.size()))
            largestOutstandingFrames = it;
    }
    if (largestOutstandingFrames != outstandingFramesPerAgreement.end()) {
        MacAddress receiverAddress = largestOutstandingFrames->first.first;
        Tid tid = largestOutstandingFrames->first.second;
        SequenceNumberCyclic startingSequenceNumber = computeStartingSequenceNumber(largestOutstandingFrames->second);
        return std::make_tuple(receiverAddress, startingSequenceNumber, tid);
    }
    return std::make_tuple(MacAddress::UNSPECIFIED_ADDRESS, SequenceNumberCyclic(), -1);
}

AckPolicy OriginatorQosAckPolicy::computeAckPolicy(Packet *packet, const Ptr<const Ieee80211DataHeader>& header, OriginatorBlockAckAgreement *agreement) const
{
    if (agreement == nullptr || agreement->isInactivityExpired())
        return AckPolicy::NORMAL_ACK;
    if (agreement->getIsAddbaResponseReceived() && isBlockAckPolicyEligibleFrame(packet, header)) {
        if (checkAgreementPolicy(header, agreement))
            return AckPolicy::BLOCK_ACK;
        else
            return AckPolicy::NORMAL_ACK;
    }
    else
        return AckPolicy::NORMAL_ACK;
}

bool OriginatorQosAckPolicy::isBlockAckPolicyEligibleFrame(Packet *packet, const Ptr<const Ieee80211DataHeader>& header) const
{
    return header->getType() == ST_DATA_WITH_QOS && packet->getByteLength() < maxBlockAckPolicyFrameLength;
}

bool OriginatorQosAckPolicy::checkAgreementPolicy(const Ptr<const Ieee80211DataHeader>& header, OriginatorBlockAckAgreement *agreement) const
{
    bool bufferFull = agreement->getBufferSize() == agreement->getNumSentBaPolicyFrames();
    bool aMsduOk = agreement->getIsAMsduSupported() || !header->getAMsduPresent();
    // TODO bool baPolicy = agreement->getIsDelayedBlockAckPolicySupported() || !frame->getAckPolicy();
    return !bufferFull && aMsduOk && (header->getSequenceNumber() >= agreement->getStartingSequenceNumber()); // TODO && baPolicy
}

//
// After transmitting an MPDU that requires an ACK frame as a response (see Annex G), the STA shall wait for an
// ACKTimeout interval, with a value of aSIFSTime + aSlotTime + aPHY-RX-START-Delay, starting at the
// PHY-TXEND.confirm primitive. If a PHY-RXSTART.indication primitive does not occur during the
// ACKTimeout interval, the STA concludes that the transmission of the MPDU has failed, and this STA shall
// invoke its backoff procedure upon expiration of the ACKTimeout interval.
//
simtime_t OriginatorQosAckPolicy::getAckTimeout(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) const
{
    return ackTimeout == -1 ? modeSet->getSifsTime() + modeSet->getSlotTime() + rateSelection->computeResponseAckFrameMode(packet, dataOrMgmtHeader)->getPhyRxStartDelay() : ackTimeout;
}

simtime_t OriginatorQosAckPolicy::getBlockAckTimeout(Packet *packet, const Ptr<const Ieee80211BlockAckReq>& blockAckReq) const
{
    return blockAckTimeout == -1 ? modeSet->getSifsTime() + modeSet->getSlotTime() + rateSelection->computeResponseBlockAckFrameMode(packet, blockAckReq)->getPhyRxStartDelay() : blockAckTimeout;
}

} /* namespace ieee80211 */
} /* namespace inet */
