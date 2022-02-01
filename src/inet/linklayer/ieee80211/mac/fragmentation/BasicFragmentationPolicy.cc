//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

std::vector<int> BasicFragmentationPolicy::computeFragmentSizes(Packet *frame)
{
    Enter_Method("computeFragmentSizes");
    if (fragmentationThreshold < frame->getByteLength()) {
        EV_DEBUG << "Computing fragment sizes: fragmentationThreshold = " << fragmentationThreshold << ", packet = " << *frame << ".\n";
        std::vector<int> sizes;
        int payloadLength = 0;
        int headerLength = 0;
        // Mgmt frames don't have payload
        const auto& header = frame->peekAtFront<Ieee80211MacHeader>();
        const auto& trailer = frame->peekAtBack<Ieee80211MacTrailer>(B(4));
        int trailerLength = B(trailer->getChunkLength()).get();
        if (dynamicPtrCast<const Ieee80211DataHeader>(header)) {
            headerLength = B(header->getChunkLength()).get();
            payloadLength = frame->getByteLength() - headerLength - trailerLength;
        }
        else
            headerLength = frame->getByteLength();
        int maxFragmentPayload = fragmentationThreshold - headerLength - trailerLength;
        if (payloadLength > maxFragmentPayload * MAX_NUM_FRAGMENTS)
            throw cRuntimeError("Fragmentation: frame \"%s\" too large, won't fit into %d fragments", frame->getName(), MAX_NUM_FRAGMENTS);
        for (int i = 0; headerLength + trailerLength + payloadLength > fragmentationThreshold; i++) {
            auto size = fragmentationThreshold - headerLength - trailerLength;
            EV_TRACE << "Computed fragment: i = " << i << ", size = " << size << ".\n";
            sizes.push_back(size);
            payloadLength -= maxFragmentPayload;
        }
        if (payloadLength != 0) {
            EV_TRACE << "Computed last fragment: size = " << payloadLength << ".\n";
            sizes.push_back(payloadLength);
        }
        EV_TRACE << "Fragmentation is suggested into " << sizes.size() << " packets.\n";
        return sizes;
    }
    EV_DEBUG << "Packet is not large enough for fragmentation: fragmentationThreshold = " << fragmentationThreshold << ".\n";
    return std::vector<int>();
}

} // namespace ieee80211
} // namespace inet

