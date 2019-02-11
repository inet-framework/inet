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

#include "inet/linklayer/ieee80211/mac/aggregation/MpduDeaggregation.h"

namespace inet {
namespace ieee80211 {

Register_Class(MpduDeaggregation);

std::vector<Packet *> *MpduDeaggregation::deaggregateFrame(Packet *aggregatedFrame)
{
    EV_DEBUG << "Deaggregating A-MPDU " << *aggregatedFrame << " into multiple packets.\n";
    std::vector<Packet *> *frames = new std::vector<Packet *>();
    int paddingLength = 0;
    cStringTokenizer tokenizer(aggregatedFrame->getName(), "+");
    while (aggregatedFrame->getDataLength() > b(0))
    {
        aggregatedFrame->setFrontOffset(aggregatedFrame->getFrontOffset() + B(paddingLength == 4 ? 0 : paddingLength));
        const auto& mpduSubframeHeader = aggregatedFrame->popAtFront<Ieee80211MpduSubframeHeader>();
        const auto& mpdu = aggregatedFrame->peekDataAt(b(0), B(mpduSubframeHeader->getLength()));
        paddingLength = 4 - B(mpduSubframeHeader->getChunkLength() + mpdu->getChunkLength()).get() % 4;
        aggregatedFrame->setFrontOffset(aggregatedFrame->getFrontOffset() + mpdu->getChunkLength());
        auto frame = new Packet();
        frame->setName(tokenizer.nextToken());
        frame->insertAtBack(mpdu);
        EV_TRACE << "Created " << *frame << " from A-MPDU.\n";
        // TODO: check CRC?
        frames->push_back(frame);
    }
    delete aggregatedFrame;
    EV_TRACE << "Created " << frames->size() << " packets from A-MPDU.\n";
    return frames;
}

} /* namespace ieee80211 */
} /* namespace inet */

