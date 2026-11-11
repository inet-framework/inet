// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_IEEE80211AMPDU_H
#define INET_IEEE80211AMPDU_H

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/Checksum.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"

namespace inet::ieee80211 {

struct INET_API Ieee80211AmpduMember
{
    B delimiterOffset;
    B offset;
    B length;
};

struct INET_API Ieee80211AmpduLayout
{
    std::vector<Ieee80211AmpduMember> members;
    B contentEnd = B(0); // Last pre-EOF content endpoint, before final alignment only.
    B preEofLength = B(0); // Duration-equivalent received prefix, not the exact pre-padding APEP request.
    B psduLength = B(0);
};

inline B minimumAmpduStartDistance(int code, const physicallayer::IIeee80211Mode *mode)
{
    static constexpr double MINIMUM_SPACING_MICROSECONDS[] = {0, 0.25, 0.5, 1, 2, 4, 8, 16};
    if (code < 0 || code > 7 || !mode)
        throw cRuntimeError("Invalid minimum MPDU start spacing or selected mode");
    // IEEE Std 802.11-2024, 10.12.3, Equation (10-12): selected-GI NET rate.
    return B(int64_t(std::ceil(MINIMUM_SPACING_MICROSECONDS[code] * mode->getDataMode()->getNetBitrate().get()/8e6)));
}

inline bool isAmpduMemberFcsOk(const Packet *packet, b offset, B length)
{
    if (length < B(4) || offset < b(0) || offset+length > packet->getDataLength())
        return false;
    auto bytes = packet->peekDataAt<BytesChunk>(offset,length)->getBytes();
    size_t end = bytes.size()-4;
    // ethernetFcs returns INET's byte-reversed CRC value, as stored by the
    // ordinary MacTrailerSerializer's big-endian field representation.
    uint32_t received = (uint32_t(bytes[end])<<24) | (uint32_t(bytes[end+1])<<16)
        | (uint32_t(bytes[end+2])<<8) | uint32_t(bytes[end+3]);
    return ethernetFcs(bytes.data(),end) == received;
}

/** Atomic delimiter/layout validation, independently of individual member FCS.
 * IEEE Std 802.11-2024, 9.7.1 and 10.12.6--10.12.7. Payload members retain
 * EOF=0 even when selective recovery leaves only one of them.
 */
inline bool findAmpdu(const Packet *packet, b offset, B length, bool vht, Ieee80211AmpduLayout& result, bool single = false)
{
    result = {};
    if (offset < b(0) || length < B(4) || offset + length > packet->getDataLength() || (single && !vht))
        return false;
    Ieee80211AmpduLayout layout;
    layout.psduLength = length;
    B cursor(0);
    bool eofPadding = false;
    bool eofSeen = false;
    constexpr int PEEK_FLAGS = Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE |
        Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED | Chunk::PF_ALLOW_REINTERPRETATION;
    while (cursor < length) {
        if (length - cursor < B(4)) {
            if (!vht || layout.members.empty())
                return false;
            if (!eofPadding) {
                layout.preEofLength = cursor;
                eofPadding = true;
            }
            cursor = length; // VHT permits up to three final octets.
            break;
        }
        auto delimiter = packet->peekDataAt<Ieee80211MpduSubframeHeader>(offset + cursor,B(4),PEEK_FLAGS);
        // HT reserves high length bits B2--B3; the shared decoder retains them
        // for VHT, so interpret them only with the receiver-derived format.
        int memberLength = vht ? delimiter->getLength() : delimiter->getLength() & 0xFFF;
        if (!delimiter->isCorrect() || !delimiter->isComplete() || !delimiter->isProperlyRepresented()
                || memberLength < 0 || memberLength > (vht ? 16383 : 4095)
                || (!vht && delimiter->getEof()) || B(memberLength) > length - cursor - B(4))
            return false;
        if (memberLength == 0) {
            if (delimiter->getEof()) {
                if (!vht || layout.members.empty())
                    return false;
                if (!eofPadding)
                    layout.preEofLength = cursor;
                eofPadding = true;
                eofSeen = true;
            }
            else {
                if (eofSeen)
                    return false;
                layout.contentEnd = cursor+B(4);
            }
            cursor += B(4);
            continue;
        }
        if (eofPadding || delimiter->getEof() != single || memberLength < 4
                || (single && (cursor != B(0) || !layout.members.empty())))
            return false;
        layout.members.push_back({cursor,cursor+B(4),B(memberLength)});
        eofSeen |= delimiter->getEof();
        cursor += B(4 + memberLength);
        layout.contentEnd = cursor;
        if (cursor == length)
            break;
        B padding((4-memberLength%4)%4);
        if (padding > length-cursor) {
            if (!vht)
                return false;
            cursor = length; // Fitting final alignment may exhaust PSDU capacity.
            break;
        }
        cursor += padding;
        if (cursor == length && !vht)
            return false; // HT's final member has no alignment padding.
    }
    if (layout.members.empty())
        return false;
    if (!eofPadding)
        layout.preEofLength = length;
    result = std::move(layout);
    return true;
}

/** Borrows already-finalized MPDUs. The returned PSDU owns no source packets. */
inline Packet *buildAmpdu(const std::vector<Packet *>& members, bool vht, B minimumStartDistance = B(0))
{
    if (members.empty() || minimumStartDistance < B(0))
        throw cRuntimeError("Cannot build an empty A-MPDU or use a negative start distance");
    auto result = new Packet("A-MPDU");
    result->copyTags(*members.front());
    result->addTagIfAbsent<PacketProtocolTag>()->set(&Protocol::unknown);
    std::string name;
    for (size_t i=0;i<members.size();i++) {
        auto member = members[i];
        B length(member->getDataLength());
        if (length < B(4) || length > B(vht ? 16383 : 4095)) {
            delete result;
            throw cRuntimeError("A-MPDU member length is outside the selected delimiter format");
        }
        auto delimiter = makeShared<Ieee80211MpduSubframeHeader>();
        delimiter->setVht(vht);
        delimiter->setEof(false);
        delimiter->setLength(length.get<B>());
        result->insertAtBack(delimiter);
        b memberOffset = result->getDataLength();
        result->insertAtBack(member->peekData());
        result->copyRegionTags(*member,member->getFrontOffset(),memberOffset,length);
        bool last = i+1 == members.size();
        if (!last) {
            B padding((4-length.get<B>()%4)%4);
            if (padding != B(0))
                result->insertAtBack(makeShared<BytesChunk>(std::vector<uint8_t>(padding.get<B>(),0)));
        }
        if (!last) {
            B occupied = B(4)+length+B((4-length.get<B>()%4)%4);
            while (occupied < minimumStartDistance) {
                auto spacing = makeShared<Ieee80211MpduSubframeHeader>();
                spacing->setVht(vht); spacing->setEof(false); spacing->setLength(0);
                result->insertAtBack(spacing);
                occupied += B(4);
            }
        }
        if (i) name += "+";
        name += member->getName();
    }
    result->setName(name.c_str());
    return result;
}

/** SU/BCC, no STBC/LDPC: use existing raw-symbol duration to find capacity.
 * IEEE Std 802.11-2024, 10.12.6 and 21.4.3. Whole-mode SGI rounding cannot
 * establish capacity because adjacent symbol counts can have equal PPDU time.
 */
inline B getVhtPsduCapacity(B apepLength, const physicallayer::IIeee80211Mode *mode)
{
    if (!mode || mode->getVhtMcsIndex() < 0 || apepLength <= B(0))
        throw cRuntimeError("VHT PSDU capacity requires a VHT mode and a positive APEP length");
    auto duration = mode->getDataMode()->getDuration(apepLength);
    B capacity = apepLength;
    while (mode->getDataMode()->getDuration(capacity+B(1)) == duration)
        capacity += B(1);
    return capacity;
}

inline void padVhtPsdu(Packet *packet, const physicallayer::IIeee80211Mode *mode)
{
    B capacity = getVhtPsduCapacity(B(packet->getDataLength()),mode);
    while (packet->getDataLength() < capacity && packet->getDataLength().get<B>()%4 != 0)
        packet->insertAtBack(makeShared<BytesChunk>(std::vector<uint8_t>(1,0)));
    while (capacity-packet->getDataLength() >= B(4)) {
        auto padding = makeShared<Ieee80211MpduSubframeHeader>();
        padding->setVht(true); padding->setEof(true); padding->setLength(0);
        packet->insertAtBack(padding);
    }
    if (packet->getDataLength() < capacity)
        packet->insertAtBack(makeShared<BytesChunk>(std::vector<uint8_t>(B(capacity-packet->getDataLength()).get<B>(),0)));
}

inline bool isVhtPsduWithinCapacity(B psduLength, const Ieee80211AmpduLayout& layout,
        const physicallayer::IIeee80211Mode *mode)
{
    return mode && mode->getVhtMcsIndex() >= 0 && layout.preEofLength > B(0) && layout.preEofLength <= psduLength
        && mode->getDataMode()->getDuration(psduLength) == mode->getDataMode()->getDuration(layout.preEofLength);
}

inline bool isAmpduWithinLengthLimit(const Ieee80211AmpduLayout& layout, B limit, bool vht)
{
    if (limit < B(0) || layout.contentEnd > limit)
        return false;
    if (!vht)
        return layout.preEofLength <= limit;
    // Selected model interpretation of 2024 10.12.2/10.12.6: preserve the
    // exact content edge while permitting only its final alignment beyond it.
    // Extra EOF0 spacing is content and cannot claim this alignment exception.
    B alignedLimit((limit.get<B>()+3)/4*4);
    return layout.preEofLength >= layout.contentEnd && layout.preEofLength-layout.contentEnd <= B(3)
        && layout.preEofLength <= alignedLimit;
}

} // namespace inet::ieee80211
#endif
