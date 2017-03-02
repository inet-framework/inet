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

#include "inet/linklayer/ieee80211/mac/aggregation/MsduAggregation.h"

namespace inet {
namespace ieee80211 {

Register_Class(MsduAggregation);

void MsduAggregation::setSubframeAddress(Ieee80211MsduSubframe *subframe, Ieee80211DataFrame* frame)
{
    // Note: Addr1 (RA), Addr2 (TA)
    // Table 8-19—Address field contents
    MACAddress da, sa;
    bool toDS = frame->getToDS();
    bool fromDS = frame->getFromDS();
    if (toDS == 0 && fromDS == 0) // STA to STA
    {
        da = frame->getReceiverAddress();
        sa = frame->getTransmitterAddress();
    }
    else if (toDS == 0 && fromDS == 1) // AP to STA
    {
        da = frame->getReceiverAddress();
        sa = frame->getAddress3();
    }
    else if (toDS == 1 && fromDS == 0) // STA to AP
    {
        da = frame->getAddress3();
        sa = frame->getTransmitterAddress();
    }
    else if (toDS == 1 && fromDS == 1) // AP to AP
    {
        da = frame->getAddress3();
        sa = frame->getAddress4();
    }
    subframe->setDa(da);
    subframe->setSa(sa);
}

Ieee80211DataFrame *MsduAggregation::aggregateFrames(std::vector<Ieee80211DataFrame *> *frames)
{
    auto firstFrame = frames->at(0);
    auto tid = firstFrame->getTid();
    auto toDS = firstFrame->getToDS();
    auto fromDS = firstFrame->getFromDS();
    auto ra = firstFrame->getReceiverAddress();
    auto ta = firstFrame->getTransmitterAddress();
    auto aMsdu = new Ieee80211AMsdu();
    auto aMsduLength = 0;
    aMsdu->setSubframesArraySize(frames->size());
    for (int i = 0; i < (int)frames->size(); i++)
    {
        auto dataFrame = check_and_cast<Ieee80211DataFrame*>(frames->at(i));
        auto msdu = dataFrame->decapsulate();
        aMsduLength += msdu->getByteLength() + LENGTH_A_MSDU_SUBFRAME_HEADER / 8; // sum of MSDU lengths + subframe header
        Ieee80211MsduSubframe msduSubframe;
        setSubframeAddress(&msduSubframe, dataFrame);
        msduSubframe.encapsulate(msdu);
        aMsdu->setSubframes(i, msduSubframe);
        // FIXME: delete dataFrame; ownership problem
    }
    aMsdu->setByteLength(aMsduLength);
//    The MPDU containing the A-MSDU is carried in any of the following data frame subtypes: QoS Data,
//    QoS Data + CF-Ack, QoS Data + CF-Poll, QoS Data + CF-Ack + CF-Poll. The A-MSDU structure is
//    contained in the frame body of a single MPDU.
    auto aggregatedDataFrame = new Ieee80211DataFrame("A-MSDU");
    aggregatedDataFrame->setType(ST_DATA_WITH_QOS);
    aggregatedDataFrame->setToDS(toDS);
    aggregatedDataFrame->setFromDS(fromDS);
    aggregatedDataFrame->setAMsduPresent(true);
    aggregatedDataFrame->setTransmitterAddress(ta);
    aggregatedDataFrame->setReceiverAddress(ra);
    aggregatedDataFrame->setTid(tid);
    aggregatedDataFrame->encapsulate(aMsdu);
    // TODO: set addr3 and addr4 according to fromDS and toDS.
    return aggregatedDataFrame;
}

} /* namespace ieee80211 */
} /* namespace inet */

