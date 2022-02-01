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
    if (aMsduLength + packet->getTotalLength() - header->getChunkLength() - trailer->getChunkLength() + b(LENGTH_A_MSDU_SUBFRAME_HEADER) > maxAMsduSize) // default value of maxAMsduSize is 4065
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

std::vector<Packet *> *BasicMsduAggregationPolicy::computeAggregateFrames(queueing::IPacketQueue *queue)
{
    Enter_Method("computeAggregateFrames");
    ASSERT(!queue->isEmpty());
    b aMsduLength = b(0);
    Ptr<const Ieee80211DataHeader> firstHeader = nullptr;
    auto frames = new std::vector<Packet *>();
    for (int i = 0; i < queue->getNumPackets(); i++) {
        auto dataPacket = queue->getPacket(i);
        const auto& dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(dataPacket->peekAtFront<Ieee80211DataOrMgmtHeader>());
        if (dataHeader == nullptr)
            break;
        if (firstHeader == nullptr)
            firstHeader = dataHeader;
        const auto& dataTrailer = dataPacket->peekAtBack<Ieee80211MacTrailer>(B(4));
        if (!isEligible(dataPacket, staticPtrCast<const Ieee80211DataHeader>(dataHeader), dataTrailer, firstHeader, aMsduLength)) {
            EV_TRACE << "Queued " << *dataPacket << " is not eligible for A-MSDU aggregation.\n";
            break;
        }
        EV_TRACE << "Queued " << *dataPacket << " is eligible for A-MSDU aggregation.\n";
        frames->push_back(dataPacket);
        aMsduLength += dataPacket->getTotalLength() - dataHeader->getChunkLength() - dataTrailer->getChunkLength() + b(LENGTH_A_MSDU_SUBFRAME_HEADER); // sum of MSDU lengths + subframe header
    }
    if (frames->size() <= 1 || !isAggregationPossible(frames->size(), B(aMsduLength).get())) {
        EV_DEBUG << "A-MSDU aggregation is not possible, collected " << frames->size() << " packets.\n";
        delete frames;
        return nullptr;
    }
    else {
        EV_DEBUG << "A-MSDU aggregation is possible, collected " << frames->size() << " packets.\n";
        return frames;
    }
}

} /* namespace ieee80211 */
} /* namespace inet */

