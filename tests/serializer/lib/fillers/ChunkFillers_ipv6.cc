//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// Chunk fillers for the IPv6 network layer: the IPv6 header, its extension headers
// (Hop-by-Hop/Destination Options TLVs, Routing, Fragment, Authentication, ESP),
// ICMPv6, IPv6 Neighbour Discovery (and its TLV option list), and MLDv1/v2.
//
// The MIPv6 Mobility Header types (BindingUpdate, HomeTest, ...) are a different
// group's responsibility -- not filled here.
//

#include "../ChunkFillers.h"
#include "inet/networklayer/mipv6/MobilityHeader_m.h"
#include "inet/common/Protocol.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#include "inet/networklayer/icmpv6/Ipv6NdMessage_m.h"
#include "inet/networklayer/icmpv6/MldMessage_m.h"
#include "inet/networklayer/icmpv6/Mldv2Message_m.h"
#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders_m.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"

namespace inet {

namespace {

// ---- IPv6 extension headers with a TLV option list (Hop-by-Hop / Destination
// Options): append one option and set chunkLength to the header's rounded size (a
// 2-byte next-header+len prefix + the option, padded up to a multiple of 8 -- the
// serializer fills any leftover bytes with zero, which deserialize back as implicit
// Pad1 options and round-trip). Both header serializers pad the stream out to the
// *declared* chunkLength themselves, so measureAndSetChunkLength() cannot probe
// their natural size (the "over-large declared length" trick just gets echoed back
// -- see ChunkRoundTripTest.h); the length must be computed and set explicitly.
// tlvOptions is declared on each ext-header subclass, so the concrete type is a
// template parameter.
template<typename T>
void fillIpv6ExtOption(Chunk *c, TlvOptionBase *opt, B optBytes)
{
    auto h = check_and_cast<T *>(c);
    h->setNextHeaderProtocol(IP_PROT_TCP);
    h->getTlvOptionsForUpdate().appendTlvOption(opt);
    B len = B(2) + optBytes;
    if (len.get<B>() % 8 != 0)
        len += B(8 - len.get<B>() % 8);
    setChunkLength(c, len);
}

// The three branches serializeIpv6TlvOptions()/deserializeIpv6TlvOptions() take: the
// single-byte Pad1 (no length field), the explicit PadN (a length + zero payload),
// and any other type via the raw fallback.
TlvOptionBase *tlvPad1() { auto o = new TlvOptionBase(); o->setType(IPv6TLVOPTION_NOP1); o->setLength(0); return o; }
TlvOptionBase *tlvPadN(int n) { auto o = new TlvOptionBase(); o->setType(IPv6TLVOPTION_NOPN); o->setLength(n); return o; }
TlvOptionRaw *tlvRaw(FillValues& v, int type, int n)
{
    auto o = new TlvOptionRaw();
    o->setType(type);
    o->setBytesArraySize(n);
    for (int i = 0; i < n; i++)
        o->setBytes(i, v.u8());
    o->setLength(n);
    return o;
}

// ---- IPv6 ND options (serializeIpv6NdOptions()/deserializeIpv6NdOptions() in
// Icmpv6HeaderSerializer.cc): one factory per option type, each consuming FillValues
// for its own fields so the round trip has byte-asymmetric content to catch a
// mis-ordered write/read. `options` is declared per ND-message subclass (not on the
// shared base), so insertUniqueOption() is called by the caller on the concrete type.
Ipv6NdSourceLinkLayerAddress *ndSrcLL(FillValues& v) { auto o = new Ipv6NdSourceLinkLayerAddress(); o->setLinkLayerAddress(v.mac()); return o; }
Ipv6NdTargetLinkLayerAddress *ndTgtLL(FillValues& v) { auto o = new Ipv6NdTargetLinkLayerAddress(); o->setLinkLayerAddress(v.mac()); return o; }
Ipv6NdMtu *ndMtu(FillValues& v) { auto o = new Ipv6NdMtu(); o->setReserved(v.u16()); o->setMtu(v.u32()); return o; }
Ipv6NdPrefixInformation *ndPrefix(FillValues& v)
{
    auto o = new Ipv6NdPrefixInformation();
    o->setPrefixLength(v.u8()); // 1 byte on the wire; RFC range is 0..128 but any byte round-trips
    o->setOnlinkFlag(v.flag());
    o->setAutoAddressConfFlag(v.flag());
    o->setRouterAddressFlag(v.flag());
    o->setReserved1(v.uint(5));
    o->setValidLifetime(v.u32());
    o->setPreferredLifetime(v.u32());
    o->setReserved2(v.u32());
    o->setPrefix(v.ipv6());
    return o;
}
Mipv6NdAdvertisementInterval *ndAdvInt(FillValues& v) { auto o = new Mipv6NdAdvertisementInterval(); o->setReserved(v.u16()); o->setAdvertisementInterval(v.u32()); return o; }
Mipv6HaInformation *ndHaInfo(FillValues& v)
{
    auto o = new Mipv6HaInformation();
    o->setReserved(v.u16());
    o->setHomeAgentPreference(v.u16());
    o->setHomeAgentLifetime(v.u16());
    return o;
}

// Router Advertisement's fixed fields, shared by its "no options" case and every
// option-type variant below.
void fillRouterAdBase(Ipv6RouterAdvertisement *p, FillValues& v)
{
    p->setChksum(v.u16());
    p->setCode(v.u8());
    p->setCurHopLimit(v.u8());
    p->setManagedAddrConfFlag(v.flag());
    p->setOtherStatefulConfFlag(v.flag());
    p->setHomeAgentFlag(v.flag());
    p->setReserved(v.uint(5)); // masked into the same byte as the three flags above
    p->setRouterLifetime(v.u16());
    p->setReachableTime(v.u32());
    p->setRetransTimer(v.u32());
}

// MLDv1 messages (MldQuery/MldReport/MldDone) share this fixed layout in full.
void fillMldBase(MldMessage *p, FillValues& v)
{
    p->setChksum(v.u16());
    p->setCode(v.u8());
    p->setMaxRespDelay(v.u16());
    p->setReserved(v.u16());
    p->setMulticastAddress(v.ipv6());
}

} // namespace

void addFillers_ipv6(std::vector<ChunkFiller>& fillers)
{
    // ---- IPv6 header: fixed 40-byte header, no options. The protocol pointer
    // serializes as the 1-byte protocolId (next header), mapped through
    // getIpProtocolGroup() -- same mechanism as Ipv4Header.
    fillers.push_back({"inet::Ipv6Header", "", [](Chunk *c) {
        auto h = check_and_cast<Ipv6Header *>(c);
        FillValues v;
        h->setVersion(6); // IPv6, fixed
        h->setTrafficClass(v.u8());
        h->setFlowLabel(v.uint(20));
        h->setHopLimit(v.u8());
        h->setPayloadLength(B(v.u16()));
        h->setSrcAddress(v.ipv6());
        h->setDestAddress(v.ipv6());
        h->setProtocol(&Protocol::udp);
    }});

    // ---- Hop-by-Hop / Destination Options headers: one variant per TLV option
    // branch (see fillIpv6ExtOption's comment). Both headers share the same TLV
    // serialization, so exercising all three branches on each is cheap and symmetric.
    fillers.push_back({"inet::Ipv6HopByHopOptionsHeader", "pad1", [](Chunk *c) {
        fillIpv6ExtOption<Ipv6HopByHopOptionsHeader>(c, tlvPad1(), B(1));
    }});
    fillers.push_back({"inet::Ipv6HopByHopOptionsHeader", "padn", [](Chunk *c) {
        fillIpv6ExtOption<Ipv6HopByHopOptionsHeader>(c, tlvPadN(4), B(6));
    }});
    fillers.push_back({"inet::Ipv6HopByHopOptionsHeader", "raw", [](Chunk *c) {
        FillValues v;
        fillIpv6ExtOption<Ipv6HopByHopOptionsHeader>(c, tlvRaw(v, IPv6TLVOPTION_TLV_GPSR, 6), B(8));
    }});
    fillers.push_back({"inet::Ipv6DestinationOptionsHeader", "pad1", [](Chunk *c) {
        fillIpv6ExtOption<Ipv6DestinationOptionsHeader>(c, tlvPad1(), B(1));
    }});
    fillers.push_back({"inet::Ipv6DestinationOptionsHeader", "padn", [](Chunk *c) {
        fillIpv6ExtOption<Ipv6DestinationOptionsHeader>(c, tlvPadN(4), B(6));
    }});
    // the MIPv6 Home Address option (RFC 6275 6.3) rides in a Destination Options
    // header and has its own serializer branch
    fillers.push_back({"inet::Ipv6DestinationOptionsHeader", "homeaddress", [](Chunk *c) {
        FillValues v;
        auto o = new HomeAddressOption();
        o->setHomeAddress(v.ipv6());
        fillIpv6ExtOption<Ipv6DestinationOptionsHeader>(c, o, B(2 + 16));
    }});
    fillers.push_back({"inet::Ipv6DestinationOptionsHeader", "raw", [](Chunk *c) {
        FillValues v;
        fillIpv6ExtOption<Ipv6DestinationOptionsHeader>(c, tlvRaw(v, IPv6TLVOPTION_TLV_GPSR, 6), B(8));
    }});

    // ---- Routing header (type 0 layout; the serializer does not branch on
    // routingType, it always writes lastEntry/flags/tag then the address list -- see
    // Ipv6ExtensionHeaderSerializer.cc). Unlike Hop-by-Hop/DestOpts, this serializer's
    // byte count is driven purely by the address array, not by the declared
    // chunkLength, so measureAndSetChunkLength() works here.
    fillers.push_back({"inet::Ipv6RoutingHeader", "", [](Chunk *c) {
        auto h = check_and_cast<Ipv6RoutingHeader *>(c);
        FillValues v;
        h->setNextHeaderProtocol(IP_PROT_TCP);
        h->setRoutingType(v.u8());
        h->setSegmentsLeft(v.u8());
        h->setLastEntry(v.u8());
        h->setFlags(v.u8());
        h->setTag(v.u16());
        h->setAddressArraySize(2);
        h->setAddress(0, v.ipv6());
        h->setAddress(1, v.ipv6());
        measureAndSetChunkLength(c);
    }});

    // ---- Fragment header: fixed 8 bytes. fragmentOffset is a 13-bit field in units
    // of 8 octets (ASSERTed to be a multiple of 8 by the serializer).
    fillers.push_back({"inet::Ipv6FragmentHeader", "", [](Chunk *c) {
        auto h = check_and_cast<Ipv6FragmentHeader *>(c);
        FillValues v;
        h->setNextHeaderProtocol(IP_PROT_TCP);
        h->setFragmentOffset(8 * v.uint(13));
        h->setReserved(v.uint(2));
        h->setMoreFragments(v.flag());
        h->setIdentification(v.u32());
    }});

    // ---- Authentication / ESP headers: not modelled beyond the 2-byte prefix (the
    // rest is emitted/consumed as opaque zero bytes with no corresponding field --
    // see Ipv6ExtensionHeaderSerializer.cc), so nextHeaderProtocol is the only wire
    // field there is to fill; chunkLength keeps its fixed 8-byte .msg default.
    fillers.push_back({"inet::Ipv6AuthenticationHeader", "", [](Chunk *c) {
        check_and_cast<Ipv6AuthenticationHeader *>(c)->setNextHeaderProtocol(IP_PROT_TCP);
    }});
    fillers.push_back({"inet::Ipv6EncapsulatingSecurityPayloadHeader", "", [](Chunk *c) {
        check_and_cast<Ipv6EncapsulatingSecurityPayloadHeader *>(c)->setNextHeaderProtocol(IP_PROT_TCP);
    }});

    // ---- ICMPv6: the type is pinned by the subclass (a meaningful, non-sentinel
    // enum value, so it needs no explicit fill -- see ChunkRoundTripTest.cc's
    // enumValueIsMeaningful()). code/chksum/etc are ordinary wire fields with no
    // masking or assertion tying them to a particular value, so -- like
    // Ipv4Header::reservedBit in the IPv4 group -- they get real, non-default
    // FillValues rather than the RFC's conventional zero: that is what lets a
    // corrupted/mis-ordered serialization show up as a byte difference.
    fillers.push_back({"inet::Icmpv6EchoRequestMsg", "", [](Chunk *c) {
        auto p = check_and_cast<Icmpv6EchoRequestMsg *>(c);
        FillValues v;
        p->setChksum(v.u16());
        p->setCode(v.u8());
        p->setIdentifier(v.u16());
        p->setSeqNumber(v.u16());
    }});
    fillers.push_back({"inet::Icmpv6EchoReplyMsg", "", [](Chunk *c) {
        auto p = check_and_cast<Icmpv6EchoReplyMsg *>(c);
        FillValues v;
        p->setChksum(v.u16());
        p->setCode(v.u8());
        p->setIdentifier(v.u16());
        p->setSeqNumber(v.u16());
    }});
    fillers.push_back({"inet::Icmpv6PacketTooBigMsg", "", [](Chunk *c) {
        auto p = check_and_cast<Icmpv6PacketTooBigMsg *>(c);
        FillValues v;
        p->setChksum(v.u16());
        p->setCode(v.u8());
        p->setMTU(v.u32());
    }});
    // DestUnreachable/ParamProblem/TimeExceeded: `code` is an enum of RFC-specific
    // sub-reasons; pick a real, non-zero member of each (any member is a valid wire
    // byte, but a distinct one is more useful for catching a mis-decoded byte).
    fillers.push_back({"inet::Icmpv6DestUnreachableMsg", "", [](Chunk *c) {
        auto p = check_and_cast<Icmpv6DestUnreachableMsg *>(c);
        FillValues v;
        p->setChksum(v.u16());
        p->setCode(ADDRESS_UNREACHABLE);
    }});
    fillers.push_back({"inet::Icmpv6ParamProblemMsg", "", [](Chunk *c) {
        auto p = check_and_cast<Icmpv6ParamProblemMsg *>(c);
        FillValues v;
        p->setChksum(v.u16());
        p->setCode(UNRECOGNIZED_IPV6_OPTION);
        p->setPointer(v.u32());
    }});
    fillers.push_back({"inet::Icmpv6TimeExceededMsg", "", [](Chunk *c) {
        auto p = check_and_cast<Icmpv6TimeExceededMsg *>(c);
        FillValues v;
        p->setChksum(v.u16());
        p->setCode(ND_FRAGMENT_REASSEMBLY_TIME);
    }});

    // ---- IPv6 Neighbour Discovery: one message carries the "no options" branch
    // (Router Advertisement, which also carries every option-type branch, being in
    // practice the message with the widest option repertoire), the others carry the
    // option type RFC 4861 assigns them. Together this exercises every branch of
    // serializeIpv6NdOptions()/deserializeIpv6NdOptions(), including the MIPv6 ND
    // options (Advertisement Interval, Home Agent Information), which -- unlike the
    // MIPv6 Mobility Header messages -- are declared unconditionally in
    // Ipv6NdMessage.msg and so belong to this group, not the MIPv6 one.
    fillers.push_back({"inet::Ipv6RouterSolicitation", "srcll", [](Chunk *c) {
        auto p = check_and_cast<Ipv6RouterSolicitation *>(c);
        FillValues v;
        p->setChksum(v.u16());
        p->setCode(v.u8());
        p->setReserved(v.u32());
        auto o = ndSrcLL(v);
        // exercise the option's trailing padding-bytes loop too: declare it one
        // 8-byte unit longer than its natural size and fill the padding explicitly.
        o->setOptionLength(2);
        o->setPaddingBytesArraySize(8);
        for (int i = 0; i < 8; i++)
            o->setPaddingBytes(i, (char)v.u8());
        p->getOptionsForUpdate().insertUniqueOption(o);
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::Ipv6RouterAdvertisement", "", [](Chunk *c) {
        auto p = check_and_cast<Ipv6RouterAdvertisement *>(c);
        FillValues v;
        fillRouterAdBase(p, v);
        // no options: the empty-options-array branch.
    }});
    fillers.push_back({"inet::Ipv6RouterAdvertisement", "srcll", [](Chunk *c) {
        auto p = check_and_cast<Ipv6RouterAdvertisement *>(c);
        FillValues v;
        fillRouterAdBase(p, v);
        p->getOptionsForUpdate().insertUniqueOption(ndSrcLL(v));
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::Ipv6RouterAdvertisement", "mtu", [](Chunk *c) {
        auto p = check_and_cast<Ipv6RouterAdvertisement *>(c);
        FillValues v;
        fillRouterAdBase(p, v);
        p->getOptionsForUpdate().insertUniqueOption(ndMtu(v));
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::Ipv6RouterAdvertisement", "prefix", [](Chunk *c) {
        auto p = check_and_cast<Ipv6RouterAdvertisement *>(c);
        FillValues v;
        fillRouterAdBase(p, v);
        p->getOptionsForUpdate().insertUniqueOption(ndPrefix(v));
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::Ipv6RouterAdvertisement", "advint", [](Chunk *c) {
        auto p = check_and_cast<Ipv6RouterAdvertisement *>(c);
        FillValues v;
        fillRouterAdBase(p, v);
        p->getOptionsForUpdate().insertUniqueOption(ndAdvInt(v));
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::Ipv6RouterAdvertisement", "hainfo", [](Chunk *c) {
        auto p = check_and_cast<Ipv6RouterAdvertisement *>(c);
        FillValues v;
        fillRouterAdBase(p, v);
        p->getOptionsForUpdate().insertUniqueOption(ndHaInfo(v));
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::Ipv6NeighbourSolicitation", "srcll", [](Chunk *c) {
        auto p = check_and_cast<Ipv6NeighbourSolicitation *>(c);
        FillValues v;
        p->setChksum(v.u16());
        p->setCode(v.u8());
        p->setReserved(v.u32());
        p->setTargetAddress(v.ipv6());
        p->getOptionsForUpdate().insertUniqueOption(ndSrcLL(v));
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::Ipv6NeighbourAdvertisement", "tgtll", [](Chunk *c) {
        auto p = check_and_cast<Ipv6NeighbourAdvertisement *>(c);
        FillValues v;
        p->setChksum(v.u16());
        p->setCode(v.u8());
        p->setRouterFlag(v.flag());
        p->setSolicitedFlag(v.flag());
        p->setOverrideFlag(v.flag());
        p->setReserved(v.uint(29)); // masked into the same word as the three flags above
        p->setTargetAddress(v.ipv6());
        p->getOptionsForUpdate().insertUniqueOption(ndTgtLL(v));
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::Ipv6Redirect", "tgtll", [](Chunk *c) {
        auto p = check_and_cast<Ipv6Redirect *>(c);
        FillValues v;
        p->setChksum(v.u16());
        p->setCode(v.u8());
        p->setTargetAddress(v.ipv6());
        p->setDestinationAddress(v.ipv6());
        p->getOptionsForUpdate().insertUniqueOption(ndTgtLL(v));
        measureAndSetChunkLength(c);
    }});

    // ---- MLDv1: MldQuery / MldReport / MldDone share the MldMessage layout in full.
    fillers.push_back({"inet::MldQuery", "", [](Chunk *c) { FillValues v; fillMldBase(check_and_cast<MldQuery *>(c), v); }});
    fillers.push_back({"inet::MldReport", "", [](Chunk *c) { FillValues v; fillMldBase(check_and_cast<MldReport *>(c), v); }});
    fillers.push_back({"inet::MldDone", "", [](Chunk *c) { FillValues v; fillMldBase(check_and_cast<MldDone *>(c), v); }});

    // ---- MLDv2 Query: shares the 24-byte MLDv1 Query prefix, then Resv/S/QRV/QQIC/
    // sourceList. A general query (no sources) and a source-specific one exercise the
    // empty vs. populated sourceList branch the deserializer distinguishes by length
    // (see MldHeaderSerializer.cc: `type == ICMPv6_MLD_QUERY && start > B(24)`).
    // sourceList is an Ipv6AddressVector -- a bare std::vector with no class
    // descriptor, like Igmpv3Query::sourceList in the IPv4 group -- so the coverage
    // audit cannot see its contents and it stays in knownUnfilled regardless.
    for (int sources : {0, 2}) {
        fillers.push_back({"inet::Mldv2Query", sources == 0 ? "general" : "sources", [sources](Chunk *c) {
            auto p = check_and_cast<Mldv2Query *>(c);
            FillValues v;
            fillMldBase(p, v);
            p->setResv(v.uint(4));
            p->setSuppressRouterProc(v.flag());
            p->setRobustnessVariable(v.uint(3)); // ASSERTed <= 7 by the serializer
            p->setQueryIntervalCode(v.u8());
            for (int i = 0; i < sources; i++)
                p->getSourceListForUpdate().push_back(v.ipv6());
            measureAndSetChunkLength(c);
        }});
    }

    // ---- MLDv2 Report: one record per record-type layout the serializer walks --
    // an empty one, one with sources, and one with auxiliary data (mirrors
    // Igmpv3Report in the IPv4 group, its closest analogue).
    fillers.push_back({"inet::Mldv2Report", "", [](Chunk *c) {
        auto p = check_and_cast<Mldv2Report *>(c);
        FillValues v;
        p->setChksum(v.u16());
        p->setCode(v.u8());
        p->setReserved(v.u16());
        p->setMulticastAddressRecordArraySize(3);
        Mldv2MulticastAddressRecord r0;
        r0.setRecordType(MLD_MODE_IS_INCLUDE);
        r0.setGroupAddress(v.ipv6());
        p->setMulticastAddressRecord(0, r0);
        Mldv2MulticastAddressRecord r1;
        r1.setRecordType(MLD_CHANGE_TO_EXCLUDE_MODE);
        r1.setGroupAddress(v.ipv6());
        r1.getSourceListForUpdate().push_back(v.ipv6());
        r1.getSourceListForUpdate().push_back(v.ipv6());
        p->setMulticastAddressRecord(1, r1);
        Mldv2MulticastAddressRecord r2;
        r2.setRecordType(MLD_BLOCK_OLD_SOURCES);
        r2.setGroupAddress(v.ipv6());
        r2.setAuxDataArraySize(2);
        r2.setAuxData(0, v.u32());
        r2.setAuxData(1, v.u32());
        p->setMulticastAddressRecord(2, r2);
        measureAndSetChunkLength(c);
    }});
}

} // namespace inet
