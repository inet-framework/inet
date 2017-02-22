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

#include "inet/linklayer/ieee80211/mac/aggregation/BasicMsduAggregationPolicy.h"

namespace inet {
namespace ieee80211 {

Define_Module(BasicMsduAggregationPolicy);

void BasicMsduAggregationPolicy::initialize()
{
    subframeNumThreshold = par("subframeNumThreshold");
    aggregationLengthThreshold = par("aggregationLengthThreshold");
    maxAMsduSize = par("maxAMsduSize");
    qOsCheck = par("qOsCheck");
}

bool BasicMsduAggregationPolicy::isAggregationPossible(int numOfFramesToAggragate, int aMsduLength)
{
    return ((subframeNumThreshold == -1 || subframeNumThreshold <= numOfFramesToAggragate) &&
            (aggregationLengthThreshold == -1 || aggregationLengthThreshold <= aMsduLength));
}

bool BasicMsduAggregationPolicy::isEligible(Ieee80211DataFrame* frame, Ieee80211DataFrame* testFrame, int aMsduLength)
{
//   Only QoS data frames have a TID.
    if (qOsCheck && frame->getType() != ST_DATA_WITH_QOS)
        return false;

//    The maximum MPDU length that can be transported using A-MPDU aggregation is 4095 octets. An
//    A-MSDU cannot be fragmented. Therefore, an A-MSDU of a length that exceeds 4065 octets (
//    4095 minus the QoS data MPDU overhead) cannot be transported in an A-MPDU.
    if (aMsduLength + testFrame->getEncapsulatedPacket()->getByteLength() + LENGTH_A_MSDU_SUBFRAME_HEADER / 8 > maxAMsduSize) // default value of maxAMsduSize is 4065
        return false;

//    The value of TID present in the QoS Control field of the MPDU carrying the A-MSDU indicates the TID for
//    all MSDUs in the A-MSDU. Because this value of TID is common to all MSDUs in the A-MSDU, only MSDUs
//    delivered to the MAC by an MA-UNITDATA.request primitive with an integer priority parameter that maps
//    to the same TID can be aggregated together using A-MSDU.
    if (testFrame->getTid() != frame->getTid())
        return false;

//    An A-MSDU contains only MSDUs whose DA and SA parameter values map to the same receiver address
//    (RA) and transmitter address (TA) values, i.e., all the MSDUs are intended to be received by a single
//    receiver, and necessarily they are all transmitted by the same transmitter. The rules for determining RA and
//    TA are independent of whether the frame body carries an A-MSDU.
    if (testFrame->getReceiverAddress() != frame->getReceiverAddress() ||
        testFrame->getTransmitterAddress() != frame->getTransmitterAddress())
        return false;

    return true;
}

std::vector<Ieee80211DataFrame *> *BasicMsduAggregationPolicy::computeAggregateFrames(cQueue *queue)
{
    ASSERT(!queue->isEmpty());
    int aMsduLength = 0;
    auto firstFrame = dynamic_cast<Ieee80211DataFrame *>(queue->front());
    auto frames = new std::vector<Ieee80211DataFrame *>();
    for (cQueue::Iterator it(*queue); !it.end(); it++)
    {
        auto dataFrame = dynamic_cast<Ieee80211DataFrame *>(*it);
        if (!dataFrame || !isEligible(dataFrame, firstFrame, aMsduLength))
            break;
        frames->push_back(dataFrame);
        aMsduLength += dataFrame->getEncapsulatedPacket()->getByteLength() + LENGTH_A_MSDU_SUBFRAME_HEADER / 8; // sum of MSDU lengths + subframe header
    }
    if (frames->size() <= 1 || !isAggregationPossible(frames->size(), aMsduLength)) {
        delete frames;
        return nullptr;
    }
    else
        return frames;
}

} /* namespace ieee80211 */
} /* namespace inet */

