//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/aggregation/BasicMsduAggregationPolicy.h"

namespace inet {
namespace ieee80211 {

Define_Module(BasicMsduAggregationPolicy);

void BasicMsduAggregationPolicy::initialize()
{
    subframeNumThreshold = par("subframeNumThreshold");
    aggregationLengthThreshold = par("aggregationLengthThreshold");
    maxAMsduSize = B(par("maxAMsduSize"));
    qOsCheck = par("qOsCheck");
}

bool BasicMsduAggregationPolicy::isAggregationPossible(int numOfFramesToAggragate, int aMsduLength)
{
    return (subframeNumThreshold == -1 || subframeNumThreshold <= numOfFramesToAggragate) &&
           (aggregationLengthThreshold == -1 || aggregationLengthThreshold <= aMsduLength);
}

bool BasicMsduAggregationPolicy::isEligible(Packet *packet, const Ptr<const Ieee80211DataHeader>& header, const Ptr<const Ieee80211MacTrailer>& trailer, const Ptr<const Ieee80211DataHeader>& testHeader, b aMsduLength)
{
    // Only QoS data frames have a TID.
    if (qOsCheck && header->getType() != ST_DATA_WITH_QOS)
        return false;

    // The maximum MPDU length that can be transported using A-MPDU aggregation is 4095 octets. An
    // A-MSDU cannot be fragmented. Therefore, an A-MSDU of a length that exceeds 4065 octets (
    // 4095 minus the QoS data MPDU overhead) cannot be transported in an A-MPDU.
    if (aMsduLength + packet->getDataLength() - header->getChunkLength() - trailer->getChunkLength() + b(LENGTH_A_MSDU_SUBFRAME_HEADER) > maxAMsduSize) // default value of maxAMsduSize is 4065
        return false;

    // The value of TID present in the QoS Control field of the MPDU carrying the A-MSDU indicates the TID for
    // all MSDUs in the A-MSDU. Because this value of TID is common to all MSDUs in the A-MSDU, only MSDUs
    // delivered to the MAC by an MA-UNITDATA.request primitive with an integer priority parameter that maps
    // to the same TID can be aggregated together using A-MSDU.
    if (testHeader->getTid() != header->getTid())
        return false;

    // An A-MSDU contains only MSDUs whose DA and SA parameter values map to the same receiver address
    // (RA) and transmitter address (TA) values, i.e., all the MSDUs are intended to be received by a single
    // receiver, and necessarily they are all transmitted by the same transmitter. The rules for determining RA and
    // TA are independent of whether the frame body carries an A-MSDU.
    if (testHeader->getReceiverAddress() != header->getReceiverAddress() ||
        testHeader->getTransmitterAddress() != header->getTransmitterAddress())
        return false;

    return true;
}

std::vector<Packet *> *BasicMsduAggregationPolicy::computeAggregateFrames(queueing::IPacketQueue *queue, Packet *candidate, const std::function<bool(const Packet *)>& isFrameEligible)
{
    Enter_Method("computeAggregateFrames");
    ASSERT(candidate != nullptr);
    ASSERT(queue->findPacket([candidate](const Packet *packet) { return packet == candidate; }) == candidate);
    b aMsduLength = b(0);
    int candidateIndex = -1;
    for (int i = 0; i < queue->getNumPackets(); i++) {
        if (queue->getPacket(i) == candidate) { candidateIndex = i; break; }
    }
    if (candidateIndex == -1)
        return nullptr;
    const auto& firstHeader = dynamicPtrCast<const Ieee80211DataHeader>(candidate->peekAtFront<Ieee80211DataOrMgmtHeader>());
    if (firstHeader == nullptr || !isFrameEligible(candidate))
        return nullptr;
    auto frames = new std::vector<Packet *>();
    auto appendIfEligible = [&](Packet *dataPacket) {
        if (!isFrameEligible(dataPacket))
            return false;
        const auto& dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(dataPacket->peekAtFront<Ieee80211DataOrMgmtHeader>());
        if (dataHeader == nullptr)
            return false;
        const auto& dataTrailer = dataPacket->peekAtBack<Ieee80211MacTrailer>(B(4));
        if (!isEligible(dataPacket, dataHeader, dataTrailer, firstHeader, aMsduLength))
            return false;
        frames->push_back(dataPacket);
        aMsduLength += dataPacket->getDataLength() - dataHeader->getChunkLength() - dataTrailer->getChunkLength() + b(LENGTH_A_MSDU_SUBFRAME_HEADER);
        return true;
    };
    if (!appendIfEligible(candidate)) {
        delete frames;
        return nullptr;
    }
    int numPackets = queue->getNumPackets();
    for (int offset = 1; offset < numPackets; offset++) {
        auto dataPacket = queue->getPacket((candidateIndex + offset) % numPackets);
        appendIfEligible(dataPacket);
    }
    if (frames->size() <= 1 || !isAggregationPossible(frames->size(), aMsduLength.get<B>())) {
        EV_DEBUG << "A-MSDU aggregation is not possible, collected " << frames->size() << " packets.\n";
        delete frames;
        return nullptr;
    }
    EV_DEBUG << "A-MSDU aggregation is possible, collected " << frames->size() << " packets.\n";
    return frames;
}

} /* namespace ieee80211 */
} /* namespace inet */
