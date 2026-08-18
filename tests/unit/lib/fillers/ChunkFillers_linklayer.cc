//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// Chunk fillers for the small link-layer MAC headers: AckingMac, B-MAC, X-MAC,
// CSMA/CA, the (non-standard) IEEE 802.15.4 header and the shortcut MAC header.
//

#include "../ChunkFillers.h"
#include "inet/common/Protocol.h"
#include "inet/linklayer/acking/AckingMacHeader_m.h"
#include "inet/linklayer/bmac/BMacHeader_m.h"
#include "inet/linklayer/csmaca/CsmaCaMacHeader_m.h"
#include "inet/linklayer/ieee802154/Ieee802154MacHeader_m.h"
#include "inet/linklayer/shortcut/ShortcutMacHeader_m.h"
#include "inet/linklayer/xmac/XMacHeader_m.h"

namespace inet {

void addFillers_linklayer(std::vector<ChunkFiller>& fillers)
{
    // AckingMacHeader: self-padding -- it writes its own length as the first byte,
    // then pads to it (see AckingMacHeaderSerializer), so the .msg leaves chunkLength
    // unset and it must be given explicitly; any value from the 23-byte fixed part
    // (1 length + 6 src + 6 dest + 2 networkProtocol + 8 srcModuleId) up to 255 (it is
    // a single wire byte) works.
    fillers.push_back({"inet::AckingMacHeader", "", [](Chunk *c) {
        auto p = check_and_cast<AckingMacHeader *>(c);
        FillValues v;
        p->setSrc(v.mac());
        p->setDest(v.mac());
        p->setNetworkProtocol(v.u16()); // only the low 16 bits are ever on the wire (writeUint16Be)
        p->setSrcModuleId((int)v.u32());
        setChunkLength(c, B(48));
    }});

    // B-MAC / X-MAC: the base is dispatch-only (skipped: BMacHeaderBase/XMacHeaderBase
    // are on chunkSkipTypes), and `type` has no .msg default, so it must be set through
    // the typed setter -- PREAMBLE and ACK are indistinguishable on the wire (the
    // serializer's switch takes the same, empty branch for both), so ControlFrame pins
    // one (ACK) and notes the other. Both frames are self-padding (pad to chunkLength,
    // which is unset by default -- see BMacHeaderSerializer/XMacHeaderSerializer), so
    // chunkLength is set explicitly, generously above the fixed part.
    fillers.push_back({"inet::BMacControlFrame", "", [](Chunk *c) {
        auto p = check_and_cast<BMacControlFrame *>(c);
        FillValues v;
        p->setSrcAddr(v.mac());
        p->setDestAddr(v.mac());
        p->setType(BMAC_ACK); // type discriminator: PREAMBLE and ACK both produce this class identically; pin one
        setChunkLength(c, B(32)); // >= 15-byte fixed part (type+length+srcAddr+destAddr)
    }});
    fillers.push_back({"inet::BMacDataFrameHeader", "", [](Chunk *c) {
        auto p = check_and_cast<BMacDataFrameHeader *>(c);
        FillValues v;
        p->setSrcAddr(v.mac());
        p->setDestAddr(v.mac());
        p->setType(BMAC_DATA); // type discriminator: pinned by subclass (only case producing BMacDataFrameHeader)
        p->setSequenceId(v.u64());
        p->setNetworkProtocol(v.u16()); // only the low 16 bits are ever on the wire
        setChunkLength(c, B(40)); // >= 25-byte fixed part (+ sequenceId(8) + networkProtocol(2))
    }});
    fillers.push_back({"inet::XMacControlFrame", "", [](Chunk *c) {
        auto p = check_and_cast<XMacControlFrame *>(c);
        FillValues v;
        p->setSrcAddr(v.mac());
        p->setDestAddr(v.mac());
        p->setType(XMAC_ACK); // type discriminator: PREAMBLE and ACK both produce this class identically; pin one
        setChunkLength(c, B(32));
    }});
    fillers.push_back({"inet::XMacDataFrameHeader", "", [](Chunk *c) {
        auto p = check_and_cast<XMacDataFrameHeader *>(c);
        FillValues v;
        p->setSrcAddr(v.mac());
        p->setDestAddr(v.mac());
        p->setType(XMAC_DATA); // type discriminator: pinned by subclass (only case producing XMacDataFrameHeader)
        p->setSequenceId(v.u64());
        p->setNetworkProtocol(v.u16());
        setChunkLength(c, B(40));
    }});

    // CsmaCaMacDataHeader/AckHeader: chunkLength is a fixed .msg default (17 / 14
    // bytes, the header's true wire size), so it is left alone. headerLengthField is
    // a genuine wire field but the deserializer trusts *it* -- not chunkLength -- to
    // know how many trailing '?' padding bytes follow the fixed part (see
    // CsmaCaMacHeaderSerializer::deserialize); pin it to the true header length so a
    // headers with no padding still decodes the same number of bytes it encoded.
    fillers.push_back({"inet::CsmaCaMacDataHeader", "", [](Chunk *c) {
        auto p = check_and_cast<CsmaCaMacDataHeader *>(c);
        FillValues v;
        p->setType(CSMA_DATA); // discriminator: pinned by subclass
        p->setHeaderLengthField(17); // true header length (1 type + 1 length + 6 + 6 + 2 + 1); must agree, see above
        p->setTransmitterAddress(v.mac());
        p->setReceiverAddress(v.mac());
        p->setNetworkProtocol((int)v.u16()); // only the low 16 bits are ever on the wire
        p->setPriority((int)v.u8());
    }});
    fillers.push_back({"inet::CsmaCaMacAckHeader", "", [](Chunk *c) {
        auto p = check_and_cast<CsmaCaMacAckHeader *>(c);
        FillValues v;
        p->setType(CSMA_ACK); // discriminator: pinned by subclass
        p->setHeaderLengthField(14); // true header length (1 type + 1 length + 6 + 6); must agree, see CsmaCaMacDataHeader above
        p->setTransmitterAddress(v.mac());
        p->setReceiverAddress(v.mac());
    }});
    // CsmaCaMacTrailer: commonSetup already set fcsMode to FCS_COMPUTED.
    fillers.push_back({"inet::CsmaCaMacTrailer", "", [](Chunk *c) {
        auto p = check_and_cast<CsmaCaMacTrailer *>(c);
        FillValues v;
        p->setFcs(v.u32());
    }});

    // Ieee802154MacHeader: see the class's .msg doc -- this is a simplified,
    // INET-internal representation, NOT the real 802.15.4 wire format. The Frame
    // Control field, the destination PAN ID and the two address-padding halves are
    // hard-coded by the serializer (not modeled as fields), so there is nothing to
    // fill for them. The four modeled fields all round-trip: srcAddr/destAddr in
    // full, sequenceId only in its low 8 bits (readUint8 vs. the 64-bit model field --
    // `& 0xFF` on write makes this consistently reproducible, not a truncation bug per
    // round trip), networkProtocol misusing the Source PAN ID field (documented
    // KLUDGE). The header has no length field of its own and never pads, so its 23
    // bytes are a true fixed size, just not one the .msg declares -- measure it
    // instead of hand-computing, so a future field addition cannot silently drift.
    fillers.push_back({"inet::Ieee802154MacHeader", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee802154MacHeader *>(c);
        FillValues v;
        p->setSrcAddr(v.mac());
        p->setDestAddr(v.mac());
        p->setNetworkProtocol(v.u16());
        p->setSequenceId(v.u32());
        measureAndSetChunkLength(c);
    }});

    // ShortcutMacHeader: self-sizing (writes its own bit-length, then pads to it --
    // see ShortcutMacHeaderSerializer), so it needs an explicit chunkLength, not
    // measureAndSetChunkLength (a self-padding chunk accepts the oversized probe
    // length and so reports none, see that helper's doc comment). payloadProtocol is
    // looked up by number in the ethertype protocol group; point it at a protocol
    // registered there.
    fillers.push_back({"inet::ShortcutMacHeader", "", [](Chunk *c) {
        auto p = check_and_cast<ShortcutMacHeader *>(c);
        p->setPayloadProtocol(&Protocol::ipv4);
        setChunkLength(c, B(4)); // 2-byte length field + 2-byte ethertype: the fixed part, no padding
    }});
}

} // namespace inet
