//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Contributor: Miguel González-López (QoS acknowledgement policy for Block Ack exchanges)
//


#include "inet/linklayer/ieee80211/mac/originator/OriginatorQosAckPolicy.h"

#include <algorithm>
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

bool OriginatorQosAckPolicy::isBlockAckReqNeeded(InProgressFrames *frames, TxopProcedure *txop, IOriginatorBlockAckAgreementHandler *agreements) const
{
    return std::get<2>(computeBlockAckReqParameters(frames, txop, agreements)) != -1;
}

std::tuple<MacAddress, SequenceNumberCyclic, Tid> OriginatorQosAckPolicy::computeBlockAckReqParameters(InProgressFrames *frames, TxopProcedure *txop, IOriginatorBlockAckAgreementHandler *agreements) const
{
    return selectBlockAckRequest(frames, agreements, nullptr, simTime());
}

std::tuple<MacAddress, SequenceNumberCyclic, Tid> OriginatorQosAckPolicy::computeBlockAckReqParametersAfterData(InProgressFrames *frames,
        const TxopProcedure *txop, IOriginatorBlockAckAgreementHandler *agreements, Packet *data, simtime_t dataEnd) const
{
    return selectBlockAckRequest(frames, agreements, data, dataEnd);
}

simtime_t OriginatorQosAckPolicy::getNextBlockAckReqDeadline(InProgressFrames *frames, IOriginatorBlockAckAgreementHandler *agreements) const
{
    auto deadline = SIMTIME_MAX;
    selectBlockAckRequest(frames, agreements, nullptr, simTime(), nullptr, &deadline);
    return deadline;
}

std::tuple<MacAddress, SequenceNumberCyclic, Tid> OriginatorQosAckPolicy::selectBlockAckRequest(InProgressFrames *frames,
        IOriginatorBlockAckAgreementHandler *agreements, Packet *transmittingData, simtime_t selectionTime, Packet *precedingData,
        simtime_t *nextDeadline) const
{
    using Id = std::pair<MacAddress, Tid>;
    std::map<Id, std::vector<Packet *>> groups;
    auto outstanding = frames->getOutstandingFrames(selectionTime);
    bool preview = transmittingData && !frames->containsFrame(transmittingData);
    auto sequenceOf = [=](Packet *frame) {
        return preview && frame == transmittingData ? frames->getCandidateSequenceNumber(frame) :
            frame->peekAtFront<Ieee80211DataHeader>()->getSequenceNumber();
    };
    if (precedingData && !frames->isExpired(precedingData, selectionTime) &&
        std::find(outstanding.begin(), outstanding.end(), precedingData) == outstanding.end())
        outstanding.push_back(precedingData);
    if (transmittingData && !frames->isExpired(transmittingData, selectionTime))
        outstanding.push_back(transmittingData);
    for (auto frame : outstanding) {
        auto header = frame->peekAtFront<Ieee80211DataHeader>();
        auto agreement = agreements ? agreements->getAgreement(header->getReceiverAddress(), header->getTid()) : nullptr;
        if (agreement && agreement->getIsAddbaResponseReceived() && agreement->containsSequence(sequenceOf(frame)))
            groups[{header->getReceiverAddress(), header->getTid()}].push_back(frame);
    }
    if (groups.empty())
        return std::make_tuple(MacAddress::UNSPECIFIED_ADDRESS, SequenceNumberCyclic(0), Tid(-1));
    auto candidate = transmittingData ? frames->peekFrameToTransmitExcluding(transmittingData) : frames->peekFrameToTransmit();
    auto data = candidate ? dynamicPtrCast<const Ieee80211DataHeader>(candidate->peekAtFront<Ieee80211MacHeader>()) : nullptr;
    auto result = std::make_tuple(MacAddress::UNSPECIFIED_ADDRESS, SequenceNumberCyclic(0), Tid(-1));
    simtime_t oldestArrival = SIMTIME_MAX;
    for (const auto& entry : groups) {
        const auto& id = entry.first;
        auto agreement = agreements->getAgreement(id.first, id.second);
        if ((int)entry.second.size() < blockAckReqThreshold) {
            if (preview)
                continue;
            bool sameFlow = data && data->getReceiverAddress() == id.first && data->getTid() == id.second;
            bool windowAllowsData = sameFlow && agreement->containsSequence(frames->getCandidateSequenceNumber(candidate));
            auto maxDelay = frames->getBlockAckReqMaxDelay();
            if (maxDelay == SIMTIME_ZERO) {
                if (windowAllowsData)
                    continue;
            }
            else if (!sameFlow || windowAllowsData) {
                auto deadline = SIMTIME_MAX;
                auto expiration = agreement->getExpirationTime();
                for (auto frame : entry.second) {
                    auto since = frame == transmittingData ? selectionTime : frames->getBlockAckOutstandingSince(frame);
                    deadline = std::min(deadline, since == SIMTIME_MAX ? selectionTime : since + maxDelay);
                    expiration = std::min(expiration, frames->getLifetimeDeadline(frame));
                }
                // Once the batch timer fires, even an uncontested grant needs AIFS and a BAR/bitmap exchange.
                bool deadlineFits = deadline < expiration &&
                    (expiration == SIMTIME_MAX || frames->getBasicFeedbackAccessDuration(id.first, id.second) < expiration - deadline);
                if (deadline > selectionTime && deadlineFits) {
                    if (nextDeadline)
                        *nextDeadline = std::min(*nextDeadline, deadline);
                    continue;
                }
            }
        }
        auto start = agreement->getStartingSequenceNumber();
        int oldestOffset = -1;
        simtime_t arrival = SIMTIME_MAX;
        for (auto frame : entry.second) {
            auto sequence = sequenceOf(frame);
            int offset = OriginatorBlockAckAgreement::computeSequenceDistance(start, sequence);
            if (oldestOffset == -1 || offset < oldestOffset) {
                oldestOffset = offset;
                arrival = frame->getArrivalTime();
            }
            else if (offset == oldestOffset)
                arrival = std::min(arrival, frame->getArrivalTime());
        }
        // Map order supplies the peer/TID tie break.
        if (std::get<2>(result) == -1 || arrival < oldestArrival) {
            oldestArrival = arrival;
            result = std::make_tuple(id.first, start + oldestOffset, id.second);
        }
    }
    return result;
}

std::tuple<MacAddress, SequenceNumberCyclic, Tid> OriginatorQosAckPolicy::computeBlockAckReqParametersForPreview(InProgressFrames *frames,
        IOriginatorBlockAckAgreementHandler *agreements, Packet *candidate, Packet *precedingData, simtime_t completion) const
{
    return selectBlockAckRequest(frames, agreements, candidate, completion, precedingData);
}

AckPolicy OriginatorQosAckPolicy::computeAckPolicy(Packet *packet, const Ptr<const Ieee80211DataHeader>& header, OriginatorBlockAckAgreement *agreement) const
{
    if (agreement == nullptr)
        return AckPolicy::NORMAL_ACK;
    if (agreement->getIsAddbaResponseReceived() && header->getAMsduPresent() && !agreement->isAMsduPermitted())
        throw cRuntimeError("A-MSDU is not permitted by both ADDBA flags");
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
    return isBlockAckPolicyEligibleFrame(packet->getDataLength(), header);
}

bool OriginatorQosAckPolicy::isBlockAckPolicyEligibleFrame(b length, const Ptr<const Ieee80211DataHeader>& header) const
{
    return header->getType() == ST_DATA_WITH_QOS && length < B(maxBlockAckPolicyFrameLength);
}

bool OriginatorQosAckPolicy::checkAgreementPolicy(const Ptr<const Ieee80211DataHeader>& header, OriginatorBlockAckAgreement *agreement) const
{
    bool aMsduOk = agreement->isAMsduPermitted() || !header->getAMsduPresent();
    // TODO bool baPolicy = agreement->getIsDelayedBlockAckPolicySupported() || !frame->getAckPolicy();
    return aMsduOk && agreement->containsSequence(header->getSequenceNumber());
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
    return ackTimeout == -1 ? modeSet->getSifsTime() + modeSet->getSlotTime() + rateSelection->computeResponseAckFrameMode(packet, dataOrMgmtHeader, ResponseQueryRole::PREDICT)->getPhyRxStartDelay() : ackTimeout;
}

simtime_t OriginatorQosAckPolicy::getBlockAckTimeout(Packet *packet, const Ptr<const Ieee80211BlockAckReq>& blockAckReq) const
{
    return blockAckTimeout == -1 ? modeSet->getSifsTime() + modeSet->getSlotTime() + rateSelection->computeResponseBlockAckFrameMode(packet, blockAckReq, ResponseQueryRole::PREDICT)->getPhyRxStartDelay() : blockAckTimeout;
}

} /* namespace ieee80211 */
} /* namespace inet */
