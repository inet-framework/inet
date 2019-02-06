//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/linklayer/ieee80211/mac/aggregation/BasicMpduAggregationPolicy.h"

namespace inet {
namespace ieee80211 {

Define_Module(BasicMpduAggregationPolicy);

void BasicMpduAggregationPolicy::initialize()
{
    subframeNumThreshold = par("subframeNumThreshold");
    aggregationLengthThreshold = par("aggregationLengthThreshold");
    maxAMpduSize = par("maxAMpduSize");
    qOsCheck = par("qOsCheck");
}

bool BasicMpduAggregationPolicy::isAggregationPossible(int numOfFramesToAggragate, int aMpduLength)
{
    return ((subframeNumThreshold == -1 || subframeNumThreshold <= numOfFramesToAggragate) &&
            (aggregationLengthThreshold == -1 || aggregationLengthThreshold <= aMpduLength));
}

bool BasicMpduAggregationPolicy::isEligible(const Ptr<const Ieee80211DataHeader>& header, Packet *testPacket, const Ptr<const Ieee80211DataHeader>& testHeader, int aMpduLength)
{
    const auto& testTrailer = testPacket->peekAtBack<Ieee80211MacTrailer>();
//   Only QoS data frames have a TID.
    if (qOsCheck && header->getType() != ST_DATA_WITH_QOS)
        return false;

//    The maximum MPDU length that can be transported using A-MPDU aggregation is 4095 octets. An
//    A-MSDU cannot be fragmented. Therefore, an A-MSDU of a length that exceeds 4065 octets (
//    4095 minus the QoS data MPDU overhead) cannot be transported in an A-MPDU.
    if (aMpduLength + B(testPacket->getTotalLength() - testHeader->getChunkLength() - testTrailer->getChunkLength() + b(LENGTH_A_MPDU_SUBFRAME_HEADER)).get() > maxAMpduSize) // default value of maxAMpduSize is 4065
        return false;

//    The value of TID present in the QoS Control field of the MPDU carrying the A-MSDU indicates the TID for
//    all MSDUs in the A-MSDU. Because this value of TID is common to all MSDUs in the A-MSDU, only MSDUs
//    delivered to the MAC by an MA-UNITDATA.request primitive with an integer priority parameter that maps
//    to the same TID can be aggregated together using A-MSDU.
    if (testHeader->getTid() != header->getTid())
        return false;

    return true;
}

std::vector<Packet *> *BasicMpduAggregationPolicy::computeAggregateFrames(std::vector<Packet *> *availableFrames)
{
    Enter_Method_Silent("computeAggregateFrames");
    b aMpduLength = b(0);
    auto firstPacket = availableFrames->at(0);
    Ptr<const Ieee80211DataOrMgmtHeader> firstHeader = nullptr;
    auto frames = new std::vector<Packet *>();
    for (auto dataPacket : *availableFrames)
    {
        const auto& dataHeader = dataPacket->peekAtFront<Ieee80211DataOrMgmtHeader>();
        const auto& dataTrailer = dataPacket->peekAtBack<Ieee80211MacTrailer>();
        if (!dynamicPtrCast<const Ieee80211DataHeader>(dataHeader))
            break;
        if (!firstHeader)
            firstHeader = dataHeader;
        if (!isEligible(staticPtrCast<const Ieee80211DataHeader>(dataHeader), firstPacket, staticPtrCast<const Ieee80211DataHeader>(firstHeader), B(aMpduLength).get())) {
            EV_TRACE << "Queued " << *dataPacket << " is not eligible for A-MPDU aggregation.\n";
            break;
        }
        EV_TRACE << "Queued " << *dataPacket << " is eligible for A-MPDU aggregation.\n";
        frames->push_back(dataPacket);
        aMpduLength += dataPacket->getTotalLength() - dataHeader->getChunkLength() - dataTrailer->getChunkLength() + b(LENGTH_A_MPDU_SUBFRAME_HEADER); // sum of MPDU lengths + subframe header
    }
    if (frames->size() <= 1 || !isAggregationPossible(frames->size(), B(aMpduLength).get())) {
        EV_DEBUG << "A-MPDU aggregation is not possible, collected " << frames->size() << " packets.\n";
        delete frames;
        return nullptr;
    }
    else {
        EV_DEBUG << "A-MPDU aggregation is possible, collected " << frames->size() << " packets.\n";
        return frames;
    }
}

} /* namespace ieee80211 */
} /* namespace inet */

