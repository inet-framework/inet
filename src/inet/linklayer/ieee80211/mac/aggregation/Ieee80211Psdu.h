// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_IEEE80211PSDU_H
#define INET_IEEE80211PSDU_H

#include "inet/common/MemoryInputStream.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/aggregation/Ieee80211Ampdu.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211TxVector.h"

namespace inet::ieee80211 {

/** EOF1 single-member Normal-Ack transport (2024 9.7.1,10.12.6--10.12.8),
 * distinct from an EOF0 aggregate exchange that currently contains one member. */
struct INET_API Ieee80211SingleMpduLayout
{
    B memberOffset = B(4);
    B memberLength = B(0);
    B suffixLength = B(0);
    B preEofLength = B(0);
};

inline B getSingleMpduApepLength(B length)
{
    return B(4) + length;
}

inline B getSingleMpduPsduLength(B length, const physicallayer::IIeee80211Mode *mode)
{
    return getVhtPsduCapacity(getSingleMpduApepLength(length),mode);
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
    return mode->getDuration(vector.singleMpdu ? getSingleMpduApepLength(length) : length);
}

inline bool findSingleMpdu(const Packet *packet, b offset, B length, Ieee80211SingleMpduLayout& layout)
{
    Ieee80211AmpduLayout aggregate;
    if (!findAmpdu(packet,offset,length,true,aggregate,true) || aggregate.members.size() != 1)
        return false;
    B member = aggregate.members.front().length;
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
    layout.memberLength = member;
    layout.suffixLength = length - B(4) - member;
    layout.preEofLength = aggregate.preEofLength;
    return true;
}

inline void wrapSingleMpdu(Packet *packet, const physicallayer::IIeee80211Mode *mode)
{
    B member(packet->getDataLength());
    if (member < B(4) || member > B(16383))
        throw cRuntimeError("VHT single MPDU exceeds delimiter bounds");
    auto delimiter = makeShared<Ieee80211MpduSubframeHeader>();
    delimiter->setVht(true);
    delimiter->setEof(true);
    delimiter->setLength(member.get<B>());
    packet->insertAtFront(delimiter);
    padVhtPsdu(packet,mode);
}

} // namespace inet::ieee80211
#endif
