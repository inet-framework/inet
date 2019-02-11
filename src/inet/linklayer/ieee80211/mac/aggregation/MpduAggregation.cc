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
#include "inet/linklayer/ieee80211/mac/aggregation/MpduAggregation.h"

namespace inet {
namespace ieee80211 {

Register_Class(MpduAggregation);

Packet *MpduAggregation::aggregateFrames(std::vector<Packet *> *frames)
{
    EV_DEBUG << "Aggregating " << frames->size() << " packets into A-MPDU.\n";
    auto aggregatedFrame = new Packet();
    std::string aggregatedName;
    for (int i = 0; i < (int)frames->size(); i++)
    {
        auto mpduSubframeHeader = makeShared<Ieee80211MpduSubframeHeader>();
        auto frame = frames->at(i);
        auto mpdu = frame->peekAll();
        mpduSubframeHeader->setLength(B(mpdu->getChunkLength()).get());
        aggregatedFrame->insertAtBack(mpduSubframeHeader);
        aggregatedFrame->insertAtBack(mpdu);
        int paddingLength = 4 - B(mpduSubframeHeader->getChunkLength() + mpdu->getChunkLength()).get() % 4;
        if (i != (int)frames->size() - 1 && paddingLength != 4) {
            auto padding = makeShared<ByteCountChunk>(B(paddingLength));
            aggregatedFrame->insertAtBack(padding);
        }
        if (i != 0)
            aggregatedName.append("+");
        aggregatedName.append(frame->getName());
        delete frame;
    }
    aggregatedFrame->setName(aggregatedName.c_str());
    EV_TRACE << "Created A-MPDU " << *aggregatedFrame << ".\n";
    return aggregatedFrame;
}

} /* namespace ieee80211 */
} /* namespace inet */

