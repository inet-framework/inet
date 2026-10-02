//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/originator/OriginatorQosAckPolicy.h"

#include <tuple>

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

std::map<MacAddress, std::vector<Packet *>> OriginatorQosAckPolicy::getOutstandingFramesPerReceiver(InProgressFrames *inProgressFrames) const
{
    auto outstandingFrames = inProgressFrames->getOutstandingFrames();
    std::map<MacAddress, std::vector<Packet *>> outstandingFramesPerReceiver;
    for (auto frame : outstandingFrames)
        outstandingFramesPerReceiver[frame->peekAtFront<Ieee80211MacHeader>()->getReceiverAddress()].push_back(frame);
    return outstandingFramesPerReceiver;
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

bool OriginatorQosAckPolicy::isCompressedBlockAckReq(const std::vector<Packet *>& outstandingFrames, int startingSequenceNumber) const
{
    // The Compressed Bitmap subfield of the BA Control field or BAR Control field shall be set to 1 in all
    // BlockAck and BlockAckReq frames sent from one HT STA to another HT STA and shall be set to 0 otherwise.
    return false; // non-HT STA
//    for (auto frame : outstandingFrames)
//        if (frame->getSequenceNumber() >= startingSequenceNumber && frame->getFragmentNumber() > 0)
//            return false;
//    return true;
}

bool OriginatorQosAckPolicy::isBlockAckReqNeeded(InProgressFrames *frames, TxopProcedure *txop) const
{
    return !std::get<0>(projectBlockAckReq(frames, nullptr, frames->findPreparedCandidate() == nullptr || txop->getLimit() == SIMTIME_ZERO)).isUnspecified();
}

std::tuple<MacAddress, SequenceNumberCyclic, Tid> OriginatorQosAckPolicy::computeBlockAckReqParameters(InProgressFrames *frames, TxopProcedure *txop) const
{
    return projectBlockAckReq(frames, nullptr, frames->findPreparedCandidate() == nullptr || txop->getLimit() == SIMTIME_ZERO);
}

std::tuple<MacAddress, SequenceNumberCyclic, Tid> OriginatorQosAckPolicy::projectBlockAckReq(
        InProgressFrames *frames, Packet *projectedBlockAckFrame, bool finalExchange) const
{
    std::map<std::pair<MacAddress, Tid>, std::vector<Packet *>> groups;
    auto outstanding = frames->getOutstandingFrames();
    if (projectedBlockAckFrame != nullptr && std::find(outstanding.begin(), outstanding.end(), projectedBlockAckFrame) == outstanding.end())
        outstanding.push_back(projectedBlockAckFrame);
    for (auto packet : outstanding) {
        auto header = packet->peekAtFront<Ieee80211DataHeader>();
        groups[{header->getReceiverAddress(), header->getTid()}].push_back(packet);
    }
    auto largest = groups.end();
    for (auto it = groups.begin(); it != groups.end(); ++it)
        if ((finalExchange || (int)it->second.size() >= blockAckReqThreshold) &&
                (largest == groups.end() || it->second.size() > largest->second.size()))
            largest = it;
    if (largest == groups.end())
        return {MacAddress::UNSPECIFIED_ADDRESS, SequenceNumberCyclic(0), -1};
    return {largest->first.first, computeStartingSequenceNumber(largest->second), largest->first.second};
}

AckPolicy OriginatorQosAckPolicy::computeAckPolicy(Packet *packet, const Ptr<const Ieee80211DataHeader>& header, OriginatorBlockAckAgreement *agreement) const
{
    if (agreement == nullptr)
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
    return getAckTimeout(rateSelection->computeResponseAckFrameMode(packet, dataOrMgmtHeader));
}

simtime_t OriginatorQosAckPolicy::getBlockAckTimeout(Packet *packet, const Ptr<const Ieee80211BlockAckReq>& blockAckReq) const
{
    return getBlockAckTimeout(rateSelection->computeResponseBlockAckFrameMode(packet, blockAckReq));
}

simtime_t OriginatorQosAckPolicy::getAckTimeout(const physicallayer::IIeee80211Mode *responseMode) const
{
    return ackTimeout == -1 ? modeSet->getSifsTime() + modeSet->getSlotTime() + responseMode->getPhyRxStartDelay() : ackTimeout;
}

simtime_t OriginatorQosAckPolicy::getBlockAckTimeout(const physicallayer::IIeee80211Mode *responseMode) const
{
    return blockAckTimeout == -1 ? modeSet->getSifsTime() + modeSet->getSlotTime() + responseMode->getPhyRxStartDelay() : blockAckTimeout;
}

} /* namespace ieee80211 */
} /* namespace inet */
