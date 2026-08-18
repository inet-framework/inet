//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// Chunk fillers for Ethernet and the link-layer headers commonly carried in an
// Ethernet frame: the MAC header and its FCS/control-frame relatives, STP/RSTP
// BPDUs, CFM continuity check, IEEE 802.2 LLC (+ SNAP), the bare 802 EPD header,
// the 802.1Q/802.1ae/802.1r tag variants, MPLS, and PPP.
//

#include "../ChunkFillers.h"
#include "inet/linklayer/ethernet/common/EthernetControlFrame_m.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/linklayer/ieee8021ae/Ieee8021aeTagHeader_m.h"
#include "inet/linklayer/ieee8021d/common/Ieee8021dBpdu_m.h"
#include "inet/linklayer/ieee8021q/Ieee8021qTagHeader_m.h"
#include "inet/linklayer/ieee8021r/Ieee8021rTagHeader_m.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"
#include "inet/linklayer/ieee8022/Ieee8022SnapHeader_m.h"
#include "inet/linklayer/ieee802/Ieee802EpdHeader_m.h"
#include "inet/linklayer/mrp/CfmContinuityCheckMessage_m.h"
#include "inet/linklayer/ppp/PppFrame_m.h"
#include "inet/networklayer/mpls/MplsPacket_m.h"

namespace inet {

void addFillers_ethernet(std::vector<ChunkFiller>& fillers)
{
    // --- Ethernet MAC header, its address/type sub-chunks, and its FCS/control-frame
    // relatives. All are fixed-size; the .msg chunkLength default already matches what
    // the serializer writes, so none of these fillers touch chunkLength.
    fillers.push_back({"inet::EthernetMacAddressFields", "", [](Chunk *c) {
        auto p = check_and_cast<EthernetMacAddressFields *>(c);
        FillValues v;
        p->setDest(v.mac());
        p->setSrc(v.mac());
    }});
    fillers.push_back({"inet::EthernetTypeOrLengthField", "", [](Chunk *c) {
        auto p = check_and_cast<EthernetTypeOrLengthField *>(c);
        FillValues v;
        p->setTypeOrLength(v.u16());
    }});
    fillers.push_back({"inet::EthernetMacHeader", "", [](Chunk *c) {
        auto p = check_and_cast<EthernetMacHeader *>(c);
        FillValues v;
        p->setDest(v.mac());
        p->setSrc(v.mac());
        p->setTypeOrLength(v.u16());
    }});
    fillers.push_back({"inet::EthernetFcs", "", [](Chunk *c) {
        auto p = check_and_cast<EthernetFcs *>(c);
        FillValues v;
        p->setFcs(v.u32()); // fcsMode is set to FCS_COMPUTED by commonSetup
    }});
    fillers.push_back({"inet::EthernetFragmentFcs", "", [](Chunk *c) {
        auto p = check_and_cast<EthernetFragmentFcs *>(c);
        FillValues v;
        p->setFcs(v.u32()); // fcsMode is set to FCS_COMPUTED by commonSetup
        // mFcs is left at its default: the .msg documents it as "meta information, not
        // represented directly in the frame" -- EthernetFcsSerializer never reads or
        // writes it (see the round-trip report for the suspected companion bug: the
        // same serializer's deserialize() always instantiates a plain EthernetFcs, so a
        // fragment FCS can never come back as an EthernetFragmentFcs anyway).
    }});
    fillers.push_back({"inet::EthernetPauseFrame", "", [](Chunk *c) {
        auto p = check_and_cast<EthernetPauseFrame *>(c);
        FillValues v;
        // opCode keeps the .msg-pinned ETHERNET_CONTROL_PAUSE (the only opcode this
        // class's discriminator supports; the serializer throws for any other opCode).
        p->setPauseTime(v.u16());
    }});

    // --- STP/RSTP BPDUs (802.1D). protocolIdentifier/protocolVersionIdentifier/bpduType
    // all keep their .msg-pinned per-subclass values (the wire format's subtype
    // discriminator); BpduTcn carries no other wire field.
    fillers.push_back({"inet::BpduTcn", "", [](Chunk *c) {
        check_and_cast<BpduTcn *>(c); // all wire fields are class-pinned .msg defaults
    }});
    fillers.push_back({"inet::BpduCfg", "", [](Chunk *c) {
        auto p = check_and_cast<BpduCfg *>(c);
        FillValues v;
        p->setTcaFlag(v.flag());
        p->setAgreementFlag(v.flag());
        p->setProposalFlag(v.flag());
        p->setTcFlag(v.flag());
        p->setRootPriority(v.u16());
        p->setRootAddress(v.mac());
        p->setRootPathCost(v.u32());
        p->setBridgePriority(v.u16());
        p->setBridgeAddress(v.mac());
        p->setPortPriority(v.u8());
        p->setPortNum(v.u8());
        p->setMessageAge(v.time());
        p->setMaxAge(v.time());
        p->setHelloTime(v.time());
        p->setForwardDelay(v.time());
    }});

    // --- CFM Continuity Check Message (Y.1731 9.2.2). The counters after the MAID are
    // emitted as zeroes by the serializer (not modelled as fields), so only the modelled
    // fields need filling. Fixed size: whatever the names are, the MAID they sit in is
    // padded to 48 octets.
    fillers.push_back({"inet::CfmContinuityCheckMessage", "", [](Chunk *c) {
        auto p = check_and_cast<CfmContinuityCheckMessage *>(c);
        FillValues v;
        p->setMdLevel(v.uint(3));
        p->setVersion(v.uint(5));
        // opCode keeps the .msg-pinned CCM opcode (1) -- the only CFM message this
        // class models.
        p->setFlags(v.u8());
        p->setSequenceNumber(v.u32());
        p->setEndpointIdentifier(v.u16());
        // the MAID: a maintenance association name in the format the .msg pins, with no
        // maintenance domain name to go with it
        p->setMaName(v.text("ma").c_str());
    }});

    // The same message with both names present: an MD name too, in the character-string
    // format an 802.1ag frame uses, which is the layout the fixed one cannot represent
    // when its length octet is read as part of a name.
    fillers.push_back({"inet::CfmContinuityCheckMessage", "mdname", [](Chunk *c) {
        auto p = check_and_cast<CfmContinuityCheckMessage *>(c);
        FillValues v;
        p->setMdLevel(v.uint(3));
        p->setVersion(v.uint(5));
        p->setFlags(v.u8());
        p->setSequenceNumber(v.u32());
        p->setEndpointIdentifier(v.u16());
        p->setMdNameFormat(4); // character string
        p->setMdName(v.text("md").c_str());
        p->setMaNameFormat(2); // character string
        p->setMaName(v.text("ma").c_str());
    }});

    // --- The TLV that ends the TLV list of a CFM PDU. Its only field is the type, and
    // the only value that makes it that TLV is zero. The base it extends is a dispatch
    // base: deserializing one yields the TLV its type names, so it is never a chunk of
    // its own on the wire.
    fillers.push_back({"inet::CfmEndTlv", "", [](Chunk *c) {
        auto p = check_and_cast<CfmEndTlv *>(c);
        p->setType(0);
    }});

    // --- Any other TLV of a CFM PDU: what it carries is not modelled, so it holds the
    // octets themselves, and the length it declares has to match how many there are.
    fillers.push_back({"inet::CfmTlvRaw", "", [](Chunk *c) {
        auto p = check_and_cast<CfmTlvRaw *>(c);
        FillValues v;
        p->setType(v.u8() | 1); // any type but the one that ends the list
        p->setLength(5);
        p->setBytesArraySize(5);
        for (size_t i = 0; i < 5; i++)
            p->setBytes(i, v.u8());
        p->setChunkLength(B(3 + 5));
    }});

    // --- IEEE 802.2 LLC header. The control field is 1 or 2 bytes depending on its own
    // low 2 bits, so that is a content-dependent layout: one case per width. dsap/ssap
    // are kept away from 0xAA/0xAA (and the 1-byte control's byte away from 0x03) so
    // deserialize's SNAP auto-detection (see Ieee8022LlcHeaderSerializer.cc) does not
    // misfire on a plain (non-SNAP) header -- SNAP itself is exercised via the separate
    // Ieee8022LlcSnapHeader filler below.
    fillers.push_back({"inet::Ieee8022LlcHeader", "1bytecontrol", [](Chunk *c) {
        auto p = check_and_cast<Ieee8022LlcHeader *>(c);
        FillValues v;
        p->setDsap(v.u8());
        p->setSsap(v.u8());
        p->setControl(v.u8() | 0x3); // low 2 bits == 3 -> the 1-byte control format
    }});
    fillers.push_back({"inet::Ieee8022LlcHeader", "2bytecontrol", [](Chunk *c) {
        auto p = check_and_cast<Ieee8022LlcHeader *>(c);
        FillValues v;
        p->setDsap(v.u8());
        p->setSsap(v.u8());
        p->setControl((int)(v.u16() & ~0x3)); // low 2 bits != 3 -> the 2-byte control format
        setChunkLength(c, B(4)); // 1 (dsap) + 1 (ssap) + 2 (control)
    }});
    fillers.push_back({"inet::Ieee8022LlcSnapHeader", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee8022LlcSnapHeader *>(c);
        FillValues v;
        // dsap/ssap/control keep their .msg-pinned SNAP values (0xAA/0xAA/0x03), which
        // is also exactly what deserialize()'s auto-detection requires to come back as
        // a Ieee8022LlcSnapHeader rather than a plain Ieee8022LlcHeader.
        p->setOui((int)v.uint(24));
        p->setProtocolId(v.u16());
    }});

    // --- Bare IEEE 802 EtherType Protocol Discrimination header.
    fillers.push_back({"inet::Ieee802EpdHeader", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee802EpdHeader *>(c);
        FillValues v;
        p->setEtherType(v.u16());
    }});

    // --- IEEE 802.1Q VLAN tag: the TPID-first form and the EPD (TPID-shifted-out) form.
    fillers.push_back({"inet::Ieee8021qTagTpidHeader", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee8021qTagTpidHeader *>(c);
        FillValues v;
        p->setTpid(v.u16());
        p->setPcp(v.uint(3));
        p->setDei(v.flag());
        p->setVid(v.uint(12));
    }});
    fillers.push_back({"inet::Ieee8021qTagEpdHeader", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee8021qTagEpdHeader *>(c);
        FillValues v;
        p->setPcp(v.uint(3));
        p->setDei(v.flag());
        p->setVid(v.uint(12));
        p->setTypeOrLength(v.u16());
    }});

    // --- IEEE 802.1CB (802.1r) tag: TPID and reserved word are emitted directly by the
    // serializer as fixed constants (0xF1C1 / 0), not modelled as fields.
    fillers.push_back({"inet::Ieee8021rTagTpidHeader", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee8021rTagTpidHeader *>(c);
        FillValues v;
        p->setSequenceNumber(v.u16());
    }});
    fillers.push_back({"inet::Ieee8021rTagEpdHeader", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee8021rTagEpdHeader *>(c);
        FillValues v;
        p->setSequenceNumber(v.u16());
        p->setTypeOrLength(v.u16());
    }});

    // --- IEEE 802.1AE (MACsec) tag: TPID-first and EPD forms. tpid is set to the real
    // MACsec EtherType even though Ieee8021aeTagTpidHeaderSerializer currently ignores
    // the field and writes a hardcoded constant instead (see the round-trip report for
    // the suspected bug in that constant); sci is left at its default since the
    // serializer never reads or writes it (both commented out).
    fillers.push_back({"inet::Ieee8021aeTagTpidHeader", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee8021aeTagTpidHeader *>(c);
        FillValues v;
        p->setTpid(0x88E5); // IEEE 802.1AE MACsec EtherType
        p->setTciAn(v.u8());
        p->setSl(v.u8());
        p->setPn(v.u32());
    }});
    fillers.push_back({"inet::Ieee8021aeTagEpdHeader", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee8021aeTagEpdHeader *>(c);
        FillValues v;
        p->setTciAn(v.u8());
        p->setSl(v.u8());
        p->setPn(v.u32());
        p->setTypeOrLength(v.u16());
    }});

    // --- MPLS label stack entry (RFC 3032).
    fillers.push_back({"inet::MplsHeader", "", [](Chunk *c) {
        auto p = check_and_cast<MplsHeader *>(c);
        FillValues v;
        p->setLabel(v.uint(20));
        p->setTc(v.uint(3));
        p->setS(v.flag());
        p->setTtl(v.u8());
    }});

    // --- PPP (RFC 1662). flag/address/control are the protocol's fixed framing bytes,
    // pinned by the .msg default; only protocol is per-frame content.
    fillers.push_back({"inet::PppHeader", "", [](Chunk *c) {
        auto p = check_and_cast<PppHeader *>(c);
        FillValues v;
        p->setProtocol(v.u16());
    }});
    fillers.push_back({"inet::PppTrailer", "", [](Chunk *c) {
        auto p = check_and_cast<PppTrailer *>(c);
        FillValues v;
        p->setFcs(v.u16());
        // flag is left at its default: PppTrailerSerializer never reads or writes it
        // (commented out, with the .msg's own FIXME that the trailer is kept at 2 bytes
        // instead of the RFC's 3 -- see the round-trip report).
    }});
}

} // namespace inet
