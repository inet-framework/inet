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

std::vector<Ieee80211DataOrMgmtFrame *> *Fragmentation::fragmentFrame(Ieee80211DataOrMgmtFrame *frame, const std::vector<int>& fragmentSizes)
{
    // Notes:
    // 1. only the MSDU is carried in the fragments (i.e. only frame's payload, without the 802.11 header)
    // 3. for convenience, this implementation sends the original frame encapsulated in the last fragment, all other fragments are dummies with no data
    //
    std::vector<Ieee80211DataOrMgmtFrame *> *fragments = new std::vector<Ieee80211DataOrMgmtFrame *>();

    cPacket *payload = frame->decapsulate();
    Ieee80211DataOrMgmtFrame *fragmentHeader = frame->dup();
    frame->encapsulate(payload); // restore original state

    for (int i = 0; i < (int)fragmentSizes.size(); i++) {
        bool lastFragment = i == (int)fragmentSizes.size() - 1;
        Ieee80211DataOrMgmtFrame *fragment = fragmentHeader->dup();
        fragment->setFragmentNumber(i);
        fragment->setMoreFragments(!lastFragment);
        if (lastFragment)
            fragment->encapsulate(frame); // TODO: khm, 802.11 in 802.11, convenient but wrong
        fragment->setByteLength(fragmentSizes.at(i));
        fragments->push_back(fragment);
    }
    delete fragmentHeader;
    return fragments;
}

} // namespace ieee80211
} // namespace inet

