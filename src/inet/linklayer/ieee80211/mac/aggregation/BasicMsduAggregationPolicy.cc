//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/aggregation/BasicMsduAggregationPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/Ieee80211MeshAggregationTag_m.h"

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

    if (auto mesh = dynamicPtrCast<const Ieee80211MeshDataHeader>(header)) {
        auto pending = packet->findTag<Ieee80211MeshAggregationTag>();
        auto first = dynamicPtrCast<const Ieee80211MeshDataHeader>(testHeader);
        if (!pending || !pending->getPendingSourceIdentity() || !first || header->getAMsduPresent()
                || !header->getToDS() || !header->getFromDS() || !mesh->getMeshControlPresent()
                || header->getReceiverAddress().isMulticast() || header->getMoreFragments() || header->getFragmentNumber() != 0
                || first->getAddress3() != header->getAddress3() || first->getAddress4() != header->getAddress4()
                || first->getTid() != header->getTid() || first->getReceiverAddress() != header->getReceiverAddress()
                || first->getTransmitterAddress() != header->getTransmitterAddress())
            return false;
        b aligned = B((aMsduLength.get<B>() + 3) / 4 * 4);
        b length = packet->getDataLength() - header->getChunkLength() - trailer->getChunkLength() + B(14);
        return maxAMsduSize == B(-1) || aligned + length <= maxAMsduSize;
    }
    if (dynamicPtrCast<const Ieee80211MeshDataHeader>(testHeader))
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

std::vector<Packet *> *BasicMsduAggregationPolicy::computeAggregateFrames(queueing::IPacketQueue *queue)
{
    Enter_Method("computeAggregateFrames");
    ASSERT(!queue->isEmpty());
    return collectAggregateFrames(queue->getNumPackets(), [queue](size_t index) { return queue->getPacket(index); });
}

std::vector<Packet *> *BasicMsduAggregationPolicy::computeAggregateFrames(const std::vector<Packet *>& candidates)
{
    Enter_Method("computeAggregateFrames");
    return collectAggregateFrames(candidates.size(), [&candidates](size_t index) { return candidates[index]; });
}

std::vector<Packet *> *BasicMsduAggregationPolicy::collectAggregateFrames(size_t count, const std::function<Packet *(size_t)>& getCandidate)
{
    b aMsduLength = b(0);
    Ptr<const Ieee80211DataHeader> firstHeader = nullptr;
    auto frames = new std::vector<Packet *>();
    // Indexed queue access may walk from the front. Do not inspect a tail that
    // cannot participate once this ordered prefix reaches its first rejection.
    for (size_t i = 0; i < count; i++) {
        auto dataPacket = getCandidate(i);
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
        if (dynamicPtrCast<const Ieee80211MeshDataHeader>(dataHeader) && aMsduLength != b(0))
            aMsduLength = B((aMsduLength.get<B>() + 3) / 4 * 4);
        aMsduLength += dataPacket->getDataLength() - dataHeader->getChunkLength() - dataTrailer->getChunkLength() + b(LENGTH_A_MSDU_SUBFRAME_HEADER); // sum of MSDU lengths + subframe header
    }
    if (frames->size() <= 1 || !isAggregationPossible(frames->size(), aMsduLength.get<B>())) {
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

