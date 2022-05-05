//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/fragmentation/Defragmentation.h"

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

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
        defragmentedFrame->getRegionTags().copyTags(fragmentFrame->getRegionTags(), fragmentFrame->getFrontOffset(), defragmentedFrame->getDataLength() - fragmentFrame->getDataLength(), fragmentFrame->getDataLength());
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

