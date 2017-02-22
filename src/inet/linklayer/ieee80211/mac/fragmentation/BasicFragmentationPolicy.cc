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

#include "inet/linklayer/ieee80211/mac/fragmentation/BasicFragmentationPolicy.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

Define_Module(BasicFragmentationPolicy);

void BasicFragmentationPolicy::initialize()
{
    fragmentationThreshold = par("fragmentationThreshold");
}

std::vector<int> BasicFragmentationPolicy::computeFragmentSizes(Ieee80211DataOrMgmtFrame *frame)
{
    if (fragmentationThreshold < frame->getByteLength()) {
        std::vector<int> sizes;
        int payloadLength = 0;
        int headerLength = 0;
        // Mgmt frames don't have payload
        if (dynamic_cast<Ieee80211DataFrame*>(frame)) {
            cPacket *payload = frame->decapsulate();
            payloadLength = payload->getByteLength();
            headerLength = frame->getByteLength();
            frame->encapsulate(payload); // restore original state
        }
        else
            headerLength = frame->getByteLength();
        int maxFragmentPayload = fragmentationThreshold - headerLength;
        if (payloadLength >= maxFragmentPayload * MAX_NUM_FRAGMENTS)
            throw cRuntimeError("Fragmentation: frame \"%s\" too large, won't fit into %d fragments", frame->getName(), MAX_NUM_FRAGMENTS);
        for(int i = 0; headerLength + payloadLength > fragmentationThreshold; i++) {
            sizes.push_back(fragmentationThreshold);
            payloadLength -= maxFragmentPayload;
        }
        sizes.push_back(headerLength + payloadLength);
        return sizes;
    }
    return std::vector<int>();
}

} // namespace ieee80211
} // namespace inet
