// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_IEEE80211MESHAMSDU_H
#define INET_IEEE80211MESHAMSDU_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/aggregation/Ieee80211Psdu.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211VhtCapabilities.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"

namespace inet::ieee80211 {

struct Ieee80211MeshAmsduPart
{
    Ptr<const Ieee80211MeshMsduSubframeHeader> header;
    Ptr<const Ieee80211MeshControl> control;
    b controlOffset;
    b payloadOffset;
    B payloadLength;
};

// Whole-body validation precedes duplicate admission, TTL edits or delivery.
// 2024 9.3.2.2.3/Figure 9-124 and 10.11: native AE0, common endpoints/identity.
inline bool parseMeshAmsdu(const Packet *packet, const Ptr<const Ieee80211MeshDataHeader>& outer,
        std::vector<Ieee80211MeshAmsduPart>& parts)
{
    parts.clear();
    if (!outer || !outer->isCorrect() || !outer->isComplete() || outer->getType() != ST_DATA_WITH_QOS || !outer->getAMsduPresent() || !outer->getMeshControlPresent()
            || !outer->getToDS() || !outer->getFromDS() || outer->getChunkLength() != B(32)
            || outer->getReceiverAddress().isMulticast() || outer->getAddress3().isUnspecified() || outer->getAddress3().isMulticast()
            || outer->getAddress4().isUnspecified() || outer->getAddress4().isMulticast()
            || outer->getMoreFragments() || outer->getFragmentNumber() != 0)
        return false;
    b offset = outer->getChunkLength(), end = packet->getDataLength() - B(4);
    const int flags = Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE;
    while (offset < end) {
        if (end - offset < B(20)) return false;
        auto header = packet->peekDataAt<Ieee80211MeshMsduSubframeHeader>(offset, B(14), flags);
        auto control = packet->peekDataAt<Ieee80211MeshControl>(offset + B(14), B(6), flags);
        if (!header->isCorrect() || !header->isComplete() || !control->isCorrect() || !control->isComplete()
                || header->getDa() != outer->getAddress3() || header->getSa() != outer->getAddress4()
                || header->getLength() < 0 || header->getLength() > 2304 || control->getAddressExtensionMode() != 0
                || control->getTtl() == 0 || (!parts.empty() && control->getSequenceNumber() != parts.front().control->getSequenceNumber()))
            return false;
        B length(header->getLength());
        if (end - offset < B(20) + length) return false;
        parts.push_back({header, control, offset + B(14), offset + B(20), length});
        offset += B(20) + length;
        if (offset != end) {
            B padding((4 - (20 + length.get<B>()) % 4) % 4);
            if (end - offset <= padding) return false; // No trailing subframe padding.
            offset += padding;
        }
    }
    return offset == end && !parts.empty();
}

// Receive capability is directional. The limit includes every mesh subframe
// header, while the selected PHY member cap also includes outer header and FCS.
inline B meshAmsduBodyLimit(const Ieee80211PeerPhyCapabilities& receiver,
        const physicallayer::IIeee80211Mode *mode, B overhead = B(36))
{
    if (!receiver.htPresent || !mode) return B(0);
    if (mode && mode->getVhtMcsIndex() >= 0)
        return receiver.vhtPresent ? B(std::min(receiver.vht.maxMpduLength, mode->getMpduMaxLength())) - overhead : B(0);
    // Standalone HT uses the advertised body tier. The4095 HT member bound
    // belongs to an enclosing A-MPDU, not this standalone transport (10.11/10.12.2).
    // Non-HT OFDM still limits its complete PSDU to4095 (Table17-24).
    return std::min(B(receiver.ht.maxAmsduLength), B(mode->getMpduMaxLength()) - overhead);
}

// Selected5GHz HT-mixed/VHT complete PPDU limit: Table9-34 and Table21-28.
// ACK/SIFS and TXOP admission remain separate MAC exchange constraints.
constexpr int MESH_AMSDU_MAX_PPDU_MICROSECONDS = 5484;

inline bool meshAmsduFitsTransport(const Packet *packet, const Ptr<const Ieee80211MeshDataHeader>& header,
        const Ieee80211PeerPhyCapabilities& receiver, const physicallayer::IIeee80211Mode *mode)
{
    return mode && packet->getDataLength() - header->getChunkLength() - B(4)
            <= meshAmsduBodyLimit(receiver, mode, B(header->getChunkLength() + B(4)))
        && computeMpduPpduDuration(mode, packet) <= SimTime(MESH_AMSDU_MAX_PPDU_MICROSECONDS, SIMTIME_US);
}

} // namespace inet::ieee80211
#endif
