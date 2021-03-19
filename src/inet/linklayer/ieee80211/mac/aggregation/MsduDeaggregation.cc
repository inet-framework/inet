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

void MsduDeaggregation::setExplodedFrameAddress(const Ptr<Ieee80211DataHeader>& header, const Ptr<const Ieee80211MsduSubframeHeader>& subframeHeader, const Ptr<const Ieee80211DataHeader>& aMsduHeader)
{
    bool toDS = aMsduHeader->getToDS();
    bool fromDS = aMsduHeader->getFromDS();
    if (fromDS == 0 && toDS == 0) // STA to STA
    {
        header->setTransmitterAddress(aMsduHeader->getTransmitterAddress());
        header->setReceiverAddress(aMsduHeader->getReceiverAddress());
    }
    else if (fromDS == 1 && toDS == 0) // AP to STA
    {
        header->setTransmitterAddress(aMsduHeader->getTransmitterAddress());
        header->setReceiverAddress(subframeHeader->getDa());
        header->setAddress3(subframeHeader->getSa());
    }
    else if (fromDS == 0 && toDS == 1) // STA to AP
    {
        header->setTransmitterAddress(subframeHeader->getSa());
        header->setReceiverAddress(aMsduHeader->getReceiverAddress());
        header->setAddress3(subframeHeader->getDa());
    }
    else if (fromDS == 1 && toDS == 1) // AP to AP
    {
        header->setReceiverAddress(aMsduHeader->getReceiverAddress());
        header->setTransmitterAddress(aMsduHeader->getTransmitterAddress());
        header->setAddress3(subframeHeader->getDa());
        header->setAddress4(subframeHeader->getSa());
    }
    ASSERT(!header->getReceiverAddress().isUnspecified());
    ASSERT(!header->getTransmitterAddress().isUnspecified());
}

std::vector<Packet *> *MsduDeaggregation::deaggregateFrame(Packet *aggregatedFrame)
{
    EV_DEBUG << "Deaggregating A-MSDU " << *aggregatedFrame << " into multiple packets.\n";
    std::vector<Packet *> *frames = new std::vector<Packet *>();
    const auto& amsduHeader = aggregatedFrame->popAtFront<Ieee80211DataHeader>();
    aggregatedFrame->popAtBack<Ieee80211MacTrailer>(B(4));
    int tid = amsduHeader->getTid();
    int paddingLength = 0;
    cStringTokenizer tokenizer(aggregatedFrame->getName(), "+");
    while (aggregatedFrame->getDataLength() > b(0))
    {
        aggregatedFrame->setFrontOffset(aggregatedFrame->getFrontOffset() + B(paddingLength == 4 ? 0 : paddingLength));
        const auto& msduSubframeHeader = aggregatedFrame->popAtFront<Ieee80211MsduSubframeHeader>();
        const auto& msdu = aggregatedFrame->peekDataAt(b(0), B(msduSubframeHeader->getLength()));
        paddingLength = 4 - B(msduSubframeHeader->getChunkLength() + msdu->getChunkLength()).get() % 4;
        aggregatedFrame->setFrontOffset(aggregatedFrame->getFrontOffset() + msdu->getChunkLength());
        auto frame = new Packet();
        frame->setName(tokenizer.nextToken());
        frame->insertAtBack(msdu);
        auto header = makeShared<Ieee80211DataHeader>();
        header->setType(ST_DATA_WITH_QOS);
        header->addChunkLength(QOSCONTROL_PART_LENGTH);
        header->setToDS(amsduHeader->getToDS());
        header->setFromDS(amsduHeader->getFromDS());
        header->setTid(tid);
        header->setSequenceNumber(SequenceNumberCyclic(0));
        setExplodedFrameAddress(header, msduSubframeHeader, amsduHeader);
        frame->insertAtFront(header);
        frame->insertAtBack(makeShared<Ieee80211MacTrailer>());
        EV_TRACE << "Created " << *frame << " from A-MSDU.\n";
        frames->push_back(frame);
    }
    delete aggregatedFrame;
    EV_TRACE << "Created " << frames->size() << " packets from A-MSDU.\n";
    return frames;
}

} /* namespace ieee80211 */
} /* namespace inet */

