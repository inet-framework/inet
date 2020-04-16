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

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/Fragmentation.h"

namespace inet {
namespace ieee80211 {

Register_Class(Fragmentation);

std::vector<Packet *> *Fragmentation::fragmentFrame(Packet *frame, const std::vector<int>& fragmentSizes)
{
    EV_DEBUG << "Fragmenting " << *frame << " into " << fragmentSizes.size() << " fragments.\n";
    B offset = B(0);
    std::vector<Packet *> *fragments = new std::vector<Packet *>();
    const auto& frameHeader = frame->popAtFront<Ieee80211DataOrMgmtHeader>();
    frame->popAtBack<Ieee80211MacTrailer>(B(4));
    for (size_t i = 0; i < fragmentSizes.size(); i++) {
        bool lastFragment = i == fragmentSizes.size() - 1;
        std::string name = std::string(frame->getName()) + "-frag" + std::to_string(i);
        auto fragment = new Packet(name.c_str());
        B length = B(fragmentSizes.at(i));
        fragment->insertAtBack(frame->peekDataAt(offset, length));
        offset += length;
        const auto& fragmentHeader = staticPtrCast<Ieee80211DataOrMgmtHeader>(frameHeader->dupShared());
        fragmentHeader->setSequenceNumber(frameHeader->getSequenceNumber());
        fragmentHeader->setFragmentNumber(i);
        fragmentHeader->setMoreFragments(!lastFragment);
        fragment->insertAtFront(fragmentHeader);
        fragment->insertAtBack(makeShared<Ieee80211MacTrailer>());
        EV_TRACE << "Created " << *fragment << " fragment.\n";
        fragments->push_back(fragment);
    }
    delete frame;
    EV_TRACE << "Created " << fragments->size() << " fragments.\n";
    return fragments;
}

} // namespace ieee80211
} // namespace inet

