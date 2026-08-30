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

    WATCH(fragmentationThreshold);
}

std::vector<int> BasicFragmentationPolicy::computeFragmentSizes(Packet *frame)
{
    Enter_Method("computeFragmentSizes");
    if (fragmentationThreshold < frame->getByteLength()) {
        EV_DEBUG << "Computing fragment sizes: fragmentationThreshold = " << fragmentationThreshold << ", packet = " << *frame << ".\n";
        std::vector<int> sizes;
        int payloadLength = 0;
        int headerLength = 0;
        const auto& header = frame->peekAtFront<Ieee80211MacHeader>();
        // IEEE Std 802.11-2024, 10.4: only individually addressed MPDUs
        // carrying an MSDU or MMPDU are eligible for fragmentation.
        if (header->getReceiverAddress().isMulticast())
            return {};
        const auto& trailer = frame->peekAtBack<Ieee80211MacTrailer>(B(4));
        int trailerLength = trailer->getChunkLength().get<B>();
        if (dynamicPtrCast<const Ieee80211DataHeader>(header)) {
            headerLength = header->getChunkLength().get<B>();
            payloadLength = frame->getByteLength() - headerLength - trailerLength;
        }
        else if (dynamicPtrCast<const Ieee80211MgmtHeader>(header)) {
            // Management subclasses currently combine the common MAC header
            // and typed MMPDU body. Only the common header is repeated.
            headerLength = makeShared<Ieee80211MgmtHeader>()->getChunkLength().get<B>();
            payloadLength = frame->getByteLength() - headerLength - trailerLength;
        }
        else
            return {};
        int maxFragmentPayload = fragmentationThreshold - headerLength - trailerLength;
        if (maxFragmentPayload <= 0)
            throw cRuntimeError("Fragmentation threshold %d is not larger than the %d byte header and trailer", fragmentationThreshold, headerLength + trailerLength);
        // IEEE Std 802.11-2024, 10.4: all non-final fragments have the same
        // even number of body octets; only the final fragment may be odd.
        maxFragmentPayload &= ~1;
        if (maxFragmentPayload == 0)
            throw cRuntimeError("Fragmentation threshold %d leaves no even-length fragment body", fragmentationThreshold);
        if (payloadLength > maxFragmentPayload * MAX_NUM_FRAGMENTS)
            throw cRuntimeError("Fragmentation: frame \"%s\" too large, won't fit into %d fragments", frame->getName(), MAX_NUM_FRAGMENTS);
        for (int i = 0; payloadLength > maxFragmentPayload; i++) {
            EV_TRACE << "Computed fragment: i = " << i << ", size = " << maxFragmentPayload << ".\n";
            sizes.push_back(maxFragmentPayload);
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
