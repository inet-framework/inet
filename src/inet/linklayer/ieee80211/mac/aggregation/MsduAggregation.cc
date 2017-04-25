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

#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/linklayer/ieee80211/mac/aggregation/MsduAggregation.h"

namespace inet {
namespace ieee80211 {

Register_Class(MsduAggregation);

void MsduAggregation::setSubframeAddress(const Ptr<Ieee80211MsduSubframe>& subframe, const Ptr<Ieee80211DataFrame>& frame)
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

Packet *MsduAggregation::aggregateFrames(std::vector<Packet*> *frames)
{
    auto firstFrame = frames->at(0);
    auto firstHeader = firstFrame->peekHeader<Ieee80211DataFrame>();
    auto tid = firstHeader->getTid();
    auto toDS = firstHeader->getToDS();
    auto fromDS = firstHeader->getFromDS();
    auto ra = firstHeader->getReceiverAddress();
    auto ta = firstHeader->getTransmitterAddress();
    auto aggregatedFrame = new Packet("A-MSDU");
    for (int i = 0; i < (int)frames->size(); i++)
    {
        auto msduSubframeHeader = std::make_shared<Ieee80211MsduSubframe>();
        auto frame = frames->at(i);
        const auto& header = frame->peekHeader<Ieee80211DataFrame>();
        auto msdu = frame->peekAt(header->getChunkLength(), frame->getTotalLength() - header->getChunkLength());
        if (auto dataFrameWithSnap = std::dynamic_pointer_cast<Ieee80211DataFrameWithSNAP>(header)) {
            msduSubframeHeader->setChunkLength(msduSubframeHeader->getChunkLength() + byte(SNAP_HEADER_BYTES)); // TODO: review, see Ieee80211MsduSubframe
            msduSubframeHeader->setEtherType(dataFrameWithSnap->getEtherType()); // TODO: review, see Ieee80211MsduSubframe
            msduSubframeHeader->setLength(byte(msdu->getChunkLength() + byte(SNAP_HEADER_BYTES)).get());
        }
        else {
            msduSubframeHeader->setEtherType(-1); // TODO: review, see Ieee80211MsduSubframe
            msduSubframeHeader->setLength(byte(msdu->getChunkLength()).get());
        }
        std::cout << "MSDU: " << msdu << std::endl;
        setSubframeAddress(msduSubframeHeader, header);
        msduSubframeHeader->markImmutable();
        aggregatedFrame->append(msduSubframeHeader);
        aggregatedFrame->append(msdu);
        int paddingLength = 4 - byte(msduSubframeHeader->getChunkLength() + msdu->getChunkLength()).get() % 4;
        if (i != (int)frames->size() - 1 && paddingLength != 4) {
            auto padding = std::make_shared<ByteCountChunk>(byte(paddingLength));
            padding->markImmutable();
            aggregatedFrame->append(padding);
        }
        // FIXME: delete dataFrame; ownership problem
    }
    // The MPDU containing the A-MSDU is carried in any of the following data frame subtypes: QoS Data,
    // QoS Data + CF-Ack, QoS Data + CF-Poll, QoS Data + CF-Ack + CF-Poll. The A-MSDU structure is
    // contained in the frame body of a single MPDU.
    auto amsduHeader = std::make_shared<Ieee80211DataFrame>();
    amsduHeader->setType(ST_DATA_WITH_QOS);
    amsduHeader->setToDS(toDS);
    amsduHeader->setFromDS(fromDS);
    amsduHeader->setAMsduPresent(true);
    amsduHeader->setTransmitterAddress(ta);
    amsduHeader->setReceiverAddress(ra);
    amsduHeader->setTid(tid);
    amsduHeader->setChunkLength(amsduHeader->getChunkLength() + byte(2));
    // TODO: set addr3 and addr4 according to fromDS and toDS.
    amsduHeader->markImmutable();
    aggregatedFrame->pushHeader(amsduHeader);
    return aggregatedFrame;
}

} /* namespace ieee80211 */
} /* namespace inet */

