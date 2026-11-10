// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_IEEE80211PSDU_H
#define INET_IEEE80211PSDU_H

#include "inet/common/MemoryInputStream.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211TxVector.h"

namespace inet::ieee80211 {

/** One payload member transport (2024 9.7.1,10.12.6--10.12.8), without H3's aggregation engine. */
struct INET_API Ieee80211SingleMpduLayout
{
    B memberOffset = B(4);
    B memberLength = B(0);
    B suffixLength = B(0);
};

inline B getSingleMpduPsduLength(B length)
{
    return B(4) + length + B((4 - length.get<B>() % 4) % 4);
}

inline physicallayer::Ieee80211TxVector getMpduTxVector(const physicallayer::IIeee80211Mode *mode,
        const Ptr<const Ieee80211MacHeader>& header)
{
    physicallayer::Ieee80211TxVector vector(mode);
    vector.singleMpdu = mode->getVhtMcsIndex() >= 0 && dynamicPtrCast<const Ieee80211MeshDataHeader>(header)
        && !header->getReceiverAddress().isMulticast();
    return vector;
}

inline simtime_t computeMpduPpduDuration(const physicallayer::IIeee80211Mode *mode, const Packet *packet)
{
    auto vector = getMpduTxVector(mode, packet->peekAtFront<Ieee80211MacHeader>());
    B length(packet->getDataLength());
    return mode->getDuration(vector.singleMpdu ? getSingleMpduPsduLength(length) : length);
}

inline bool findSingleMpdu(const Packet *packet, b offset, B length, Ieee80211SingleMpduLayout& layout)
{
    if (offset < B(0) || length < B(4) || offset + length > packet->getDataLength())
        return false;
    constexpr int flags = Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE | Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED;
    auto delimiter = packet->peekDataAt<Ieee80211MpduSubframeHeader>(offset, B(4), flags | Chunk::PF_ALLOW_REINTERPRETATION);
    if (!delimiter->isCorrect() || !delimiter->isComplete() || !delimiter->getEof()
            || delimiter->getLength() < 4 || B(delimiter->getLength()) > length - B(4))
        return false;
    B member(delimiter->getLength());
    // Before any MAC observer can see this interval, establish its complete
    // type-dependent header, excluding the member FCS. A valid delimiter can
    // otherwise expose an empty or truncated QoS/four-address MAC header.
    if (member < B(6))
        return false;
    auto bytes = packet->peekDataAt<BytesChunk>(offset + B(4), member - B(4))->getBytes();
    MemoryInputStream input(bytes);
    auto header = dynamicPtrCast<Ieee80211MacHeader>(Chunk::deserialize(input, typeid(Ieee80211MacHeader)));
    if (!header || !header->isCorrect() || !header->isComplete())
        return false;
    B cursor = B(4) + member;
    // Final alignment, followed optionally by EOF padding delimiters and up to3 trailing octets.
    if (cursor < length) {
        B padding((4 - member.get<B>() % 4) % 4);
        if (padding > length - cursor)
            return false;
        cursor += padding;
        while (length - cursor >= B(4)) {
            auto eof = packet->peekDataAt<Ieee80211MpduSubframeHeader>(offset + cursor, B(4), flags | Chunk::PF_ALLOW_REINTERPRETATION);
            if (!eof->isCorrect() || !eof->isComplete() || !eof->getEof() || eof->getLength() != 0)
                return false;
            cursor += B(4);
        }
    }
    layout.memberLength = member;
    layout.suffixLength = length - B(4) - member;
    return true;
}

inline void wrapSingleMpdu(Packet *packet)
{
    B member(packet->getDataLength());
    if (member < B(4) || member > B(16383))
        throw cRuntimeError("VHT single MPDU exceeds delimiter bounds");
    auto delimiter = makeShared<Ieee80211MpduSubframeHeader>();
    delimiter->setVht(true);
    delimiter->setEof(true);
    delimiter->setLength(member.get<B>());
    packet->insertAtFront(delimiter);
    B padding = getSingleMpduPsduLength(member) - B(4) - member;
    if (padding != B(0))
        packet->insertAtBack(makeShared<BytesChunk>(std::vector<uint8_t>(padding.get<B>(), 0)));
}

} // namespace inet::ieee80211
#endif
