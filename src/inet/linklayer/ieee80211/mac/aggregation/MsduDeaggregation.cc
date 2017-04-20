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

#include "inet/linklayer/ieee80211/mac/aggregation/MsduDeaggregation.h"

namespace inet {
namespace ieee80211 {

Register_Class(MsduDeaggregation);

void MsduDeaggregation::setExplodedFrameAddress(const Ptr<Ieee80211DataFrame>& frame, const Ptr<Ieee80211MsduSubframe>& subframe, const Ptr<Ieee80211DataFrame>& aMsduFrame)
{
    bool toDS = aMsduFrame->getToDS();
    bool fromDS = aMsduFrame->getFromDS();
    if (fromDS == 0 && toDS == 0) // STA to STA
    {
        frame->setTransmitterAddress(aMsduFrame->getTransmitterAddress());
        frame->setReceiverAddress(aMsduFrame->getReceiverAddress());
    }
    else if (fromDS == 1 && toDS == 0) // AP to STA
    {
        frame->setTransmitterAddress(frame->getTransmitterAddress());
        frame->setReceiverAddress(subframe->getDa());
        frame->setAddress3(subframe->getSa());
    }
    else if (fromDS == 0 && toDS == 1) // STA to AP
    {
        frame->setTransmitterAddress(subframe->getSa());
        frame->setReceiverAddress(aMsduFrame->getReceiverAddress());
        frame->setAddress3(subframe->getDa());
    }
    else if (fromDS == 1 && toDS == 1) // AP to AP
    {
        frame->setReceiverAddress(aMsduFrame->getReceiverAddress());
        frame->setTransmitterAddress(aMsduFrame->getTransmitterAddress());
        frame->setAddress3(subframe->getDa());
        frame->setAddress4(subframe->getSa());
    }
}

std::vector<Packet *> *MsduDeaggregation::deaggregateFrame(Packet *aggregatedFrame)
{
    std::vector<Packet *> *frames = new std::vector<Packet *>();
    const auto& amsduHeader = aggregatedFrame->popHeader<Ieee80211DataFrame>();
    int tid = amsduHeader->getTid();
    while (aggregatedFrame->hasHeader<Ieee80211MsduSubframe>())
    {
        const auto& msduSubframeHeader = aggregatedFrame->popHeader<Ieee80211MsduSubframe>();
        const auto& msdu = aggregatedFrame->peekDataAt(bit(0), byte(msduSubframeHeader->getLength()));
        aggregatedFrame->setHeaderPopOffset(aggregatedFrame->getHeaderPopOffset() + msdu->getChunkLength());
        // TODO: review, restore snap header, see Ieee80211MsduSubframe
        auto frame = new Packet();
        frame->append(msdu);
        auto header = (msduSubframeHeader->getEtherType() != -1) ? std::make_shared<Ieee80211DataFrameWithSNAP>() : std::make_shared<Ieee80211DataFrame>();
        // dataFrame->setType(ST_DATA_WITH_QOS); FIXME:
        header->setType(ST_DATA);
        header->setTransmitterAddress(msduSubframeHeader->getSa());
        header->setToDS(amsduHeader->getToDS());
        header->setFromDS(amsduHeader->getFromDS());
        header->setTid(tid);
        // TODO: review, restore snap header, see Ieee80211MsduSubframe
        if (auto dataFrameWithSnap = std::dynamic_pointer_cast<Ieee80211DataFrameWithSNAP>(header))
            dataFrameWithSnap->setEtherType(msduSubframeHeader->getEtherType());
        setExplodedFrameAddress(header, msduSubframeHeader, amsduHeader);
        header->markImmutable();
        frame->pushHeader(header);
        frames->push_back(frame);
    }
    delete aggregatedFrame;
    return frames;
}

} /* namespace ieee80211 */
} /* namespace inet */

