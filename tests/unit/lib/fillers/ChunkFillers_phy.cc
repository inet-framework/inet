//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// Chunk fillers for the physical-layer headers: the generic/shortcut/APSK PHY
// headers, the Ethernet PHY preamble headers, and the IEEE 802.11 PHY headers
// (FHSS, IR, DSSS/HR-DSSS, OFDM/ERP-OFDM, HT, VHT).
//

#include "../ChunkFillers.h"
#include "inet/common/Protocol.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyHeader_m.h"
#include "inet/physicallayer/wireless/apsk/packetlevel/ApskPhyHeader_m.h"
#include "inet/physicallayer/wireless/generic/GenericPhyHeader_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"
#include "inet/physicallayer/wireless/shortcut/ShortcutPhyHeader_m.h"

namespace inet {

void addFillers_phy(std::vector<ChunkFiller>& fillers)
{
    // --- ApskPhyHeader: a self-sizing header (like Generic/ShortcutPhyHeader below) --
    // headerLengthField is not free data: on deserialize it becomes the chunk's own
    // chunkLength (ApskPhyHeaderSerializer.cc), so it must be pinned to the header's
    // actual bit length for the round trip to reproduce the same bytes. The fixed part
    // (headerLengthField + payloadLengthField + fcs + payloadProtocol) is 8 B, matching
    // the .msg default chunkLength, so the "" variant needs no padding and no explicit
    // setChunkLength. The "padded" variant exercises the '?'-byte padding branch.
    fillers.push_back({"inet::ApskPhyHeader", "", [](Chunk *c) {
        auto p = check_and_cast<ApskPhyHeader *>(c);
        FillValues v;
        p->setHeaderLengthField(b(64)); // pinned: header's own length (8 B), no padding
        p->setPayloadLengthField(b(v.u16()));
        p->setFcs(v.u16());
        // fcsMode: left as set by commonSetup (FCS_COMPUTED)
        p->setPayloadProtocol(&Protocol::ethernetMac);
    }});
    fillers.push_back({"inet::ApskPhyHeader", "padded", [](Chunk *c) {
        auto p = check_and_cast<ApskPhyHeader *>(c);
        FillValues v;
        setChunkLength(c, B(12)); // 8 B fixed content + 4 B of '?' padding
        p->setHeaderLengthField(b(96)); // pinned: must match the chunk's actual length
        p->setPayloadLengthField(b(v.u16()));
        p->setFcs(v.u16());
        p->setPayloadProtocol(&Protocol::ieee80211Mac);
    }});

    // --- GenericPhyHeader: self-sizing (GenericPhyHeaderSerializer writes chunkLength
    // itself as the length field, then zero-pads to it), so chunkLength is content-
    // dependent and always needs an explicit value; the .msg declares no default (it
    // is 0, which would fail FieldsChunkSerializer's length check for any real content).
    fillers.push_back({"inet::GenericPhyHeader", "", [](Chunk *c) {
        auto p = check_and_cast<GenericPhyHeader *>(c);
        setChunkLength(c, B(4)); // minimal: length field (2 B) + payloadProtocol (2 B)
        p->setPayloadProtocol(&Protocol::bmac);
    }});
    fillers.push_back({"inet::GenericPhyHeader", "padded", [](Chunk *c) {
        auto p = check_and_cast<GenericPhyHeader *>(c);
        setChunkLength(c, B(6)); // 4 B fixed content + 2 B of zero-bit padding
        p->setPayloadProtocol(&Protocol::lmac);
    }});

    // --- ShortcutPhyHeader: same self-sizing shape as GenericPhyHeader.
    fillers.push_back({"inet::ShortcutPhyHeader", "", [](Chunk *c) {
        auto p = check_and_cast<ShortcutPhyHeader *>(c);
        setChunkLength(c, B(4));
        p->setPayloadProtocol(&Protocol::shortcutMac);
    }});
    fillers.push_back({"inet::ShortcutPhyHeader", "padded", [](Chunk *c) {
        auto p = check_and_cast<ShortcutPhyHeader *>(c);
        setChunkLength(c, B(6));
        p->setPayloadProtocol(&Protocol::xmac);
    }});

    // --- Ethernet PHY preamble: the concrete serializer ignores the chunk's own state
    // and always emits the fixed 7x0x55 preamble + SFD (0xD5); there is no wire field
    // to fill. EthernetPhyHeaderBase / EthernetPhyHeaderBaseSerializer (the abstract
    // dispatch pair) are handled by the .test's skipTypes/skipSerializerClasses.
    fillers.push_back({"inet::physicallayer::EthernetPhyHeader", "", [](Chunk *c) {
        check_and_cast<physicallayer::EthernetPhyHeader *>(c);
    }});

    // --- Ethernet fragment PHY preamble: preambleType selects one of five distinct
    // encodings in EthernetFragmentPhyHeaderSerializer's switch -- one variant per
    // branch. smdNumber/fragmentNumber only matter (and are only serialized) for the
    // SMD_Sx / SMD_Cx branches, so they are left at their default 0 elsewhere; the
    // SMD_Sx and SMD_Cx variants below fill them and so cover the field overall.
    //
    // SUSPECTED SERIALIZER BUG (EthernetPhyHeaderSerializer.cc, deserialize()): the SMD_Verify
    // (0x07) and SMD_Respond (0x19) branches are not decoded at all. deserialize() only ever
    // distinguishes SFD (0xD5) from "not 0xD5", and misclassifies anything else -- including
    // SMD_Verify/SMD_Respond -- as SMD_Sx, looking the byte up in smdSxValues[]. Since 0x07/0x19
    // are not in that array, std::distance(...) returns 4 (one past the end), and smdNumber is
    // set to that out-of-range index; re-serializing then reads smdSxValues[4], an out-of-bounds
    // access. Expect these two variants to fail the round trip (byte mismatch or CRASHED).
    fillers.push_back({"inet::physicallayer::EthernetFragmentPhyHeader", "sfd", [](Chunk *c) {
        auto p = check_and_cast<physicallayer::EthernetFragmentPhyHeader *>(c);
        p->setPreambleType(physicallayer::SFD);
    }});
    fillers.push_back({"inet::physicallayer::EthernetFragmentPhyHeader", "smdverify", [](Chunk *c) {
        auto p = check_and_cast<physicallayer::EthernetFragmentPhyHeader *>(c);
        p->setPreambleType(physicallayer::SMD_Verify);
    }});
    fillers.push_back({"inet::physicallayer::EthernetFragmentPhyHeader", "smdrespond", [](Chunk *c) {
        auto p = check_and_cast<physicallayer::EthernetFragmentPhyHeader *>(c);
        p->setPreambleType(physicallayer::SMD_Respond);
    }});
    fillers.push_back({"inet::physicallayer::EthernetFragmentPhyHeader", "smdsx", [](Chunk *c) {
        auto p = check_and_cast<physicallayer::EthernetFragmentPhyHeader *>(c);
        FillValues v;
        p->setPreambleType(physicallayer::SMD_Sx);
        p->setSmdNumber(v.uint(2)); // index into smdSxValues[4], 0..3
    }});
    fillers.push_back({"inet::physicallayer::EthernetFragmentPhyHeader", "smdcx", [](Chunk *c) {
        auto p = check_and_cast<physicallayer::EthernetFragmentPhyHeader *>(c);
        FillValues v;
        p->setPreambleType(physicallayer::SMD_Cx);
        p->setSmdNumber(v.uint(2)); // index into smdCxValues[4], 0..3
        p->setFragmentNumber(v.uint(2)); // index into fragmentNumberValues[4], 0..3
    }});

    // --- IEEE 802.11 FHSS PHY header: chunkLength is a fixed b(32) (12+4+16 bits, no
    // padding), matching the .msg default. lengthField is inherited from the
    // Ieee80211PhyHeader base but Ieee80211FhssPhyHeaderSerializer never reads or
    // writes it -- filled anyway (harmless: it never reaches the wire either way) so
    // the coverage gate sees every inherited field touched.
    fillers.push_back({"inet::physicallayer::Ieee80211FhssPhyHeader", "", [](Chunk *c) {
        auto p = check_and_cast<physicallayer::Ieee80211FhssPhyHeader *>(c);
        FillValues v;
        p->setLengthField(B(v.u16())); // not on the wire for this subtype; see above
        p->setPlw(v.uint(12));
        p->setPsf(v.uint(4));
        p->setFcs(v.u16());
        // fcsMode: left as set by commonSetup (FCS_COMPUTED)
    }});

    // --- IEEE 802.11 IR PHY header: only fcs (2 B) reaches the wire. Unlike its
    // lengthField is inherited but, like FHSS, unused by this subtype's serializer;
    // filled anyway for the same reason as above.
    fillers.push_back({"inet::physicallayer::Ieee80211IrPhyHeader", "", [](Chunk *c) {
        auto p = check_and_cast<physicallayer::Ieee80211IrPhyHeader *>(c);
        FillValues v;
        p->setLengthField(B(v.u16())); // not on the wire for this subtype
        p->setFcs(v.u16());
        // fcsMode: left as set by commonSetup (FCS_COMPUTED)
    }});

    // --- IEEE 802.11 DSSS / HR-DSSS PHY headers: fixed B(6) header (signal, service,
    // lengthField, fcs), matching the .msg default -- no padding, no content-dependent
    // length. HR-DSSS reuses the DSSS wire layout verbatim (same fields, no own
    // serializer branch), so one filler each is enough.
    fillers.push_back({"inet::physicallayer::Ieee80211DsssPhyHeader", "", [](Chunk *c) {
        auto p = check_and_cast<physicallayer::Ieee80211DsssPhyHeader *>(c);
        FillValues v;
        p->setSignal(v.u8());
        p->setService(v.u8());
        p->setLengthField(B(v.u16()));
        p->setFcs(v.u16());
        // fcsMode: left as set by commonSetup (FCS_COMPUTED)
    }});
    fillers.push_back({"inet::physicallayer::Ieee80211HrDsssPhyHeader", "", [](Chunk *c) {
        auto p = check_and_cast<physicallayer::Ieee80211HrDsssPhyHeader *>(c);
        FillValues v;
        p->setSignal(v.u8());
        p->setService(v.u8());
        p->setLengthField(B(v.u16()));
        p->setFcs(v.u16());
        // fcsMode: left as set by commonSetup (FCS_COMPUTED, via the DsssPhyHeader cast)
    }});

    // --- IEEE 802.11 OFDM / ERP-OFDM PHY headers: fixed B(5) header (3 B packed
    // SIGNAL field + 2 B SERVICE), matching the .msg default. rate/tail are narrow
    // bitfields packed into the SIGNAL field alongside reserved/parity/lengthField
    // (Ieee80211OfdmSignalField.h); reserved/parity default false, so both are pinned
    // true here (the only way to move a bool off its default). ERP-OFDM reuses the
    // OFDM wire layout verbatim.
    fillers.push_back({"inet::physicallayer::Ieee80211OfdmPhyHeader", "", [](Chunk *c) {
        auto p = check_and_cast<physicallayer::Ieee80211OfdmPhyHeader *>(c);
        FillValues v;
        p->setRate(v.uint(4));
        p->setReserved(true); // default is false; pinned to the only non-default value
        p->setLengthField(B(v.uint(12)));
        p->setParity(true); // default is false; pinned to the only non-default value
        p->setTail(v.uint(6));
        p->setService(v.u16());
    }});
    fillers.push_back({"inet::physicallayer::Ieee80211ErpOfdmPhyHeader", "", [](Chunk *c) {
        auto p = check_and_cast<physicallayer::Ieee80211ErpOfdmPhyHeader *>(c);
        FillValues v;
        p->setRate(v.uint(4));
        p->setReserved(true);
        p->setLengthField(B(v.uint(12)));
        p->setParity(true);
        p->setTail(v.uint(6));
        p->setService(v.u16());
    }});

}

} // namespace inet
