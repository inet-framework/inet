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

#include "inet/linklayer/ieee80211/mac/fragmentation/Fragmentation.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

Register_Class(Fragmentation);

std::vector<Packet*> *Fragmentation::fragmentFrame(Packet *frame, const std::vector<int>& fragmentSizes)
{
    // Notes:
    // 1. only the MSDU is carried in the fragments (i.e. only frame's payload, without the 802.11 header)
    // 3. for convenience, this implementation sends the original frame encapsulated in the last fragment, all other fragments are dummies with no data
    //
    byte offset = byte(0);
    std::vector<Packet *> *fragments = new std::vector<Packet *>();
    const auto& frameHeader = frame->popHeader<Ieee80211DataOrMgmtFrame>();
    frame->popTrailer<Ieee80211MacTrailer>();
    for (int i = 0; i < (int)fragmentSizes.size(); i++) {
        bool lastFragment = i == (int)fragmentSizes.size() - 1;
        std::string name = std::string(frame->getName()) + "-frag" + std::to_string(i);
        auto fragment = new Packet(name.c_str());
        byte length = byte(fragmentSizes.at(i));
        fragment->append(frame->peekDataAt(offset, length));
        offset += length;
        const auto& fragmentHeader = std::static_pointer_cast<Ieee80211DataOrMgmtFrame>(frameHeader->dupShared());
        fragmentHeader->setSequenceNumber(frameHeader->getSequenceNumber());
        fragmentHeader->setFragmentNumber(i);
        fragmentHeader->setMoreFragments(!lastFragment);
        fragment->insertHeader(fragmentHeader);
        fragment->insertTrailer(std::make_shared<Ieee80211MacTrailer>());
        fragments->push_back(fragment);
    }
    delete frame;
    return fragments;
}

} // namespace ieee80211
} // namespace inet

