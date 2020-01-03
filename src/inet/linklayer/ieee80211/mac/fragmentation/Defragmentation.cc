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
#include "inet/linklayer/ieee80211/mac/fragmentation/Defragmentation.h"

namespace inet {
namespace ieee80211 {

Register_Class(Defragmentation);

Packet *Defragmentation::defragmentFrames(std::vector<Packet *> *fragmentFrames)
{
    EV_DEBUG << "Defragmenting " << fragmentFrames->size() << " fragments.\n";
    auto defragmentedFrame = new Packet();
    const auto& defragmentedHeader = staticPtrCast<Ieee80211DataOrMgmtHeader>(fragmentFrames->at(0)->peekAtFront<Ieee80211DataOrMgmtHeader>()->dupShared());
    for (auto fragmentFrame : *fragmentFrames) {
        fragmentFrame->popAtFront<Ieee80211DataOrMgmtHeader>();
        fragmentFrame->popAtBack<Ieee80211MacTrailer>(B(4));
        defragmentedFrame->insertAtBack(fragmentFrame->peekData());
        std::string defragmentedName(fragmentFrame->getName());
        auto index = defragmentedName.find("-frag");
        if (index != std::string::npos)
            defragmentedFrame->setName(defragmentedName.substr(0, index).c_str());
    }
    defragmentedHeader->setFragmentNumber(0);
    defragmentedHeader->setMoreFragments(false);
    defragmentedFrame->insertAtFront(defragmentedHeader);
    defragmentedFrame->insertAtBack(makeShared<Ieee80211MacTrailer>());
    EV_TRACE << "Created " << *defragmentedFrame << ".\n";
    return defragmentedFrame;
}

} // namespace ieee80211
} // namespace inet

