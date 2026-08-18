//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// Chunk fillers for OSPF: OSPFv2 and OSPFv3 packets (Hello, Database Description, Link
// State Request, Link State Update, Link State Acknowledgement) and the LSA types their
// Link State Update packet carries.
//

#include "../ChunkFillers.h"
#include "inet/common/checksum/ChecksumMode_m.h"
#include "inet/routing/ospf_common/OspfPacketBase_m.h"
#include "inet/routing/ospfv2/Ospfv2Packet_m.h"
#include "inet/routing/ospfv3/Ospfv3Packet_m.h"

namespace inet {

using namespace ospf;
using namespace ospfv2;
using namespace ospfv3;

namespace {

// Both OSPFv2 and OSPFv3 packets carry a packetLengthField that must agree with the
// actual serialized size (deserialize computes array element counts from it). Zero it
// first so measure's oversized probe serialize does not fold it into the natural body
// length, measure the natural length, then write that byte count back.
void finalizeOspfLength(ospf::OspfPacketBase *pkt, Chunk *c)
{
    pkt->setPacketLengthField(0);
    measureAndSetChunkLength(c);
    pkt->setPacketLengthField(c->getChunkLength().get<B>());
}

// ================= OSPFv2 =================

// unused_1/unused_2/unused_3 are RFC 2328's unused options bits; the other five are the
// real per-packet capability flags (E/MC/NP/EA/DC), alternated so adjacent bits differ.
void fillOspfv2Options(Ospfv2Options& o, FillValues& v)
{
    o.unused_1 = false; // RFC 2328 A.2: unused bit, sent as zero
    o.E_ExternalRoutingCapability = v.flag();
    o.MC_MulticastForwarding = v.flag();
    o.NP_Type7LSA = v.flag();
    o.EA_ForwardExternalLSAs = v.flag();
    o.DC_DemandCircuits = v.flag();
    o.unused_2 = false; // RFC 2328 A.2: unused bit, sent as zero
    o.unused_3 = false; // RFC 2328 A.2: unused bit, sent as zero
}

// The OSPFv2 header fields shared by every packet type (Ospfv2Packet and its
// OspfPacketBase parent). packetLengthField is left to finalizeOspfLength() since it
// depends on the type-specific body filled afterwards.
void fillOspfv2Header(Ospfv2Packet *p, OspfPacketType type, FillValues& v)
{
    p->setType(type); // pinned by the .msg for Hello/DD; LSR/LSU/LSAck need it set explicitly
    p->setRouterID(v.ipv4());
    p->setAreaID(v.ipv4());
    p->setChecksum(v.u16());
    // checksumMode is set to CHECKSUM_COMPUTED by commonSetup (applies to all OspfPacketBase)
    p->setAuthenticationType(v.u16());
    for (int i = 0; i < 8; i++)
        p->setAuthentication(i, (char)v.u8());
}

Ospfv2LsaHeader makeOspfv2LsaHeader(FillValues& v, Ospfv2LsaType type, uint16_t lsaLength)
{
    Ospfv2LsaHeader h;
    h.setLsAge(v.u16());
    fillOspfv2Options(h.getLsOptionsForUpdate(), v);
    h.setLsType(type);
    h.setLinkStateID(v.ipv4());
    h.setAdvertisingRouter(v.ipv4());
    h.setLsSequenceNumber((int32_t)v.u32());
    h.setLsChecksum(v.u16());
    h.setLsChecksumMode(CHECKSUM_COMPUTED); // Ospfv2PacketSerializer throws unless COMPUTED
    h.setLsaLength(lsaLength);
    return h;
}

// ================= OSPFv3 =================

// reserved/reservedOne/reservedTwo and xBit (deprecated MOSPF bit) are RFC 5340 A.2
// reserved bits; dcBit/rBit/nBit/eBit/v6Bit are the real capability flags.
void fillOspfv3Options(Ospfv3Options& o, FillValues& v)
{
    o.reserved = 0; // RFC 5340 A.2: reserved, must be sent as zero
    o.reservedOne = false; // RFC 5340 A.2: reserved (OSPFv2 compatibility), must be zero
    o.reservedTwo = false; // RFC 5340 A.2: reserved (OSPFv2 compatibility), must be zero
    o.dcBit = v.flag();
    o.rBit = v.flag();
    o.nBit = v.flag();
    o.xBit = false; // RFC 5340 A.2: deprecated (MOSPF), must always be zero
    o.eBit = v.flag();
    o.v6Bit = v.flag();
}

void fillOspfv3Header(Ospfv3Packet *p, OspfPacketType type, FillValues& v)
{
    p->setType(type); // pinned by the .msg for Hello/DD; LSR/LSU/LSAck need it set explicitly
    p->setRouterID(v.ipv4());
    p->setAreaID(v.ipv4());
    p->setChecksum(v.u16());
    // checksumMode is set to CHECKSUM_COMPUTED by commonSetup (applies to all OspfPacketBase)
    p->setInstanceID((int8_t)v.u8());
    p->setReserved(0); // RFC 5340 A.3.1: reserved, must be sent as zero
}

// The 16-bit LS Type on the wire is derived from the function code alone
// (Ospfv3PacketSerializer::encodeLsType); "options" (the U/S2/S1 scope bits) is
// overwritten by that derivation on deserialize, so its filled value here does not
// survive a round trip -- filled anyway so the coverage audit sees it touched.
Ospfv3LsaHeader makeOspfv3LsaHeader(FillValues& v, unsigned short functionCode, uint16_t lsaLength)
{
    Ospfv3LsaHeader h;
    h.setLsaAge(v.u16());
    h.setOptions(v.u8());
    h.setLsaType(functionCode);
    h.setLinkStateID(v.ipv4());
    h.setAdvertisingRouter(v.ipv4());
    h.setLsaSequenceNumber(v.u32());
    h.setLsaChecksum(v.u16());
    h.setLsChecksumMode(CHECKSUM_COMPUTED);
    h.setLsaLength(lsaLength);
    return h;
}

// reserved1-3 and the trailing 16-bit "reserved" are RFC 5340 A.4.1's reserved prefix
// bits; dnBit/pBit/xBit/laBit/nuBit are the real per-prefix flags.
void fillPrefix0(Ospfv3LsaPrefix0& p, FillValues& v, uint8_t prefixLen)
{
    p.reserved1 = false; // RFC 5340 A.4.1: reserved, must be sent as zero
    p.reserved2 = false; // RFC 5340 A.4.1: reserved, must be sent as zero
    p.reserved3 = false; // RFC 5340 A.4.1: reserved, must be sent as zero
    p.dnBit = v.flag();
    p.pBit = v.flag();
    p.xBit = v.flag();
    p.laBit = v.flag();
    p.nuBit = v.flag();
    p.prefixLen = prefixLen;
    p.reserved = 0; // RFC 5340 A.4.1: reserved, must be sent as zero
    p.addressPrefix = v.l3Ipv6();
}

void fillPrefixMetric(Ospfv3LsaPrefixMetric& p, FillValues& v, uint8_t prefixLen)
{
    p.reserved1 = false; // RFC 5340 A.4.1: reserved, must be sent as zero
    p.reserved2 = false; // RFC 5340 A.4.1: reserved, must be sent as zero
    p.reserved3 = false; // RFC 5340 A.4.1: reserved, must be sent as zero
    p.dnBit = v.flag();
    p.pBit = v.flag();
    p.xBit = v.flag();
    p.laBit = v.flag();
    p.nuBit = v.flag();
    p.prefixLen = prefixLen;
    p.metric = v.u16();
    p.addressPrefix = v.l3Ipv6();
}

} // namespace

void addFillers_ospf(std::vector<ChunkFiller>& fillers)
{
    // ---------------- OSPFv2 ----------------

    fillers.push_back({"inet::ospfv2::Ospfv2HelloPacket", "", [](Chunk *c) {
        auto h = check_and_cast<Ospfv2HelloPacket *>(c);
        FillValues v;
        fillOspfv2Header(h, HELLO_PACKET, v);
        h->setNetworkMask(v.ipv4());
        h->setHelloInterval(v.u16());
        fillOspfv2Options(h->getOptionsForUpdate(), v);
        h->setRouterPriority((char)v.u8());
        h->setRouterDeadInterval(v.u32());
        h->setDesignatedRouter(v.ipv4());
        h->setBackupDesignatedRouter(v.ipv4());
        h->setNeighborArraySize(2);
        h->setNeighbor(0, v.ipv4());
        h->setNeighbor(1, v.ipv4());
        finalizeOspfLength(h, c);
    }});

    fillers.push_back({"inet::ospfv2::Ospfv2DatabaseDescriptionPacket", "", [](Chunk *c) {
        auto dd = check_and_cast<Ospfv2DatabaseDescriptionPacket *>(c);
        FillValues v;
        fillOspfv2Header(dd, DATABASE_DESCRIPTION_PACKET, v);
        dd->setInterfaceMTU(v.u16());
        fillOspfv2Options(dd->getOptionsForUpdate(), v);
        auto& ddo = dd->getDdOptionsForUpdate();
        ddo.unused = 0; // RFC 2328 A.3.3: reserved, must be sent as zero
        ddo.I_Init = v.flag();
        ddo.M_More = v.flag();
        ddo.MS_MasterSlave = v.flag();
        dd->setDdSequenceNumber(v.u32());
        dd->setLsaHeadersArraySize(2);
        dd->setLsaHeaders(0, makeOspfv2LsaHeader(v, ROUTERLSA_TYPE, v.u16()));
        dd->setLsaHeaders(1, makeOspfv2LsaHeader(v, NETWORKLSA_TYPE, v.u16()));
        finalizeOspfLength(dd, c);
    }});

    fillers.push_back({"inet::ospfv2::Ospfv2LinkStateRequestPacket", "", [](Chunk *c) {
        auto lsr = check_and_cast<Ospfv2LinkStateRequestPacket *>(c);
        FillValues v;
        fillOspfv2Header(lsr, LINKSTATE_REQUEST_PACKET, v);
        lsr->setRequestsArraySize(2);
        Ospfv2LsaRequest r0;
        r0.lsType = ROUTERLSA_TYPE;
        r0.linkStateID = v.ipv4();
        r0.advertisingRouter = v.ipv4();
        lsr->setRequests(0, r0);
        Ospfv2LsaRequest r1;
        r1.lsType = AS_EXTERNAL_LSA_TYPE;
        r1.linkStateID = v.ipv4();
        r1.advertisingRouter = v.ipv4();
        lsr->setRequests(1, r1);
        finalizeOspfLength(lsr, c);
    }});

    fillers.push_back({"inet::ospfv2::Ospfv2LinkStateAcknowledgementPacket", "", [](Chunk *c) {
        auto ack = check_and_cast<Ospfv2LinkStateAcknowledgementPacket *>(c);
        FillValues v;
        fillOspfv2Header(ack, LINKSTATE_ACKNOWLEDGEMENT_PACKET, v);
        ack->setLsaHeadersArraySize(2);
        ack->setLsaHeaders(0, makeOspfv2LsaHeader(v, NETWORKLSA_TYPE, v.u16()));
        ack->setLsaHeaders(1, makeOspfv2LsaHeader(v, AS_EXTERNAL_LSA_TYPE, v.u16()));
        finalizeOspfLength(ack, c);
    }});

    // Link State Update: one LSA of each of the four types the serializer supports
    // (Router/Network/Summary/AsExternal), each with a populated inner list, so every
    // content-dependent branch of Ospfv2PacketSerializer::serializeLsa runs. Per-LSA
    // lsaLength must match the actual encoding byte-for-byte: NetworkLsa/SummaryLsa/
    // AsExternalLsa deserialize by computing their element counts from it.
    fillers.push_back({"inet::ospfv2::Ospfv2LinkStateUpdatePacket", "", [](Chunk *c) {
        auto lsu = check_and_cast<Ospfv2LinkStateUpdatePacket *>(c);
        FillValues v;
        fillOspfv2Header(lsu, LINKSTATE_UPDATE_PACKET, v);

        // Router LSA: fixed part (flags byte + reserved byte + link count) 4 B, one link
        // (12 B fixed) carrying one TOS entry (4 B) -- both the link loop and the
        // per-link TOS loop run. header 20 + fixed 4 + link (12 + 4) = 40.
        auto *routerLsa = new Ospfv2RouterLsa();
        routerLsa->setHeader(makeOspfv2LsaHeader(v, ROUTERLSA_TYPE, 40));
        routerLsa->setReserved1(0); // RFC 1583 A.4.2: reserved, sent as zero
        routerLsa->setV_VirtualLinkEndpoint(v.flag());
        routerLsa->setE_ASBoundaryRouter(v.flag());
        routerLsa->setB_AreaBorderRouter(v.flag());
        routerLsa->setReserved2(0); // RFC 1583 A.4.2: reserved, sent as zero
        routerLsa->setNumberOfLinks(1);
        routerLsa->setLinksArraySize(1);
        Ospfv2Link link;
        link.setLinkID(v.ipv4());
        link.setLinkData(v.u32());
        link.setType(TRANSIT_LINK); // != the field's own default (POINTTOPOINT_LINK)
        link.setNumberOfTOS(1);
        link.setLinkCost(v.u16());
        link.setTosDataArraySize(1);
        Ospfv2TosData tos;
        tos.tos = v.u8();
        tos.tosMetric = v.u32();
        link.setTosData(0, tos);
        routerLsa->setLinks(0, link);

        // Network LSA: mask (4 B) + two attached routers (4 B each). header 20 + 4 + 8 = 32.
        auto *networkLsa = new Ospfv2NetworkLsa();
        networkLsa->setHeader(makeOspfv2LsaHeader(v, NETWORKLSA_TYPE, 32));
        networkLsa->setNetworkMask(v.ipv4());
        networkLsa->setAttachedRoutersArraySize(2);
        networkLsa->setAttachedRouters(0, v.ipv4());
        networkLsa->setAttachedRouters(1, v.ipv4());

        // Summary LSA: mask + pad + 24-bit cost (8 B) + one TOS-metric entry (4 B).
        // header 20 + 8 + 4 = 32.
        auto *summaryLsa = new Ospfv2SummaryLsa();
        summaryLsa->setHeader(makeOspfv2LsaHeader(v, SUMMARYLSA_NETWORKS_TYPE, 32));
        summaryLsa->setNetworkMask(v.ipv4());
        summaryLsa->setRouteCost(v.uint(24));
        summaryLsa->setTosDataArraySize(1);
        Ospfv2TosData stos;
        stos.tos = v.u8();
        stos.tosMetric = v.uint(24);
        summaryLsa->setTosData(0, stos);

        // AS-External LSA: mask (4 B) + one external-TOS entry (12 B). header 20 + 4 + 12 = 36.
        auto *asExternalLsa = new Ospfv2AsExternalLsa();
        asExternalLsa->setHeader(makeOspfv2LsaHeader(v, AS_EXTERNAL_LSA_TYPE, 36));
        auto& contents = asExternalLsa->getContentsForUpdate();
        contents.setNetworkMask(v.ipv4());
        contents.setExternalTOSInfoArraySize(1);
        Ospfv2ExternalTosInfo extTos;
        extTos.tos = v.uint(7);
        extTos.E_ExternalMetricType = v.flag();
        extTos.routeCost = v.uint(24);
        extTos.forwardingAddress = v.ipv4();
        extTos.externalRouteTag = v.u32();
        contents.setExternalTOSInfo(0, extTos);

        lsu->setOspfLSAsArraySize(4);
        lsu->setOspfLSAs(0, routerLsa);
        lsu->setOspfLSAs(1, networkLsa);
        lsu->setOspfLSAs(2, summaryLsa);
        lsu->setOspfLSAs(3, asExternalLsa);
        finalizeOspfLength(lsu, c);
    }});

    // ---------------- OSPFv3 ----------------

    fillers.push_back({"inet::ospfv3::Ospfv3HelloPacket", "", [](Chunk *c) {
        auto h = check_and_cast<Ospfv3HelloPacket *>(c);
        FillValues v;
        fillOspfv3Header(h, HELLO_PACKET, v);
        h->setInterfaceID(v.u32());
        h->setRouterPriority((char)v.u8());
        fillOspfv3Options(h->getOptionsForUpdate(), v);
        h->setHelloInterval(v.u16());
        h->setDeadInterval(v.u16());
        h->setDesignatedRouterID(v.ipv4());
        h->setBackupDesignatedRouterID(v.ipv4());
        h->setNeighborIDArraySize(2);
        h->setNeighborID(0, v.ipv4());
        h->setNeighborID(1, v.ipv4());
        finalizeOspfLength(h, c);
    }});

    fillers.push_back({"inet::ospfv3::Ospfv3DatabaseDescriptionPacket", "", [](Chunk *c) {
        auto dd = check_and_cast<Ospfv3DatabaseDescriptionPacket *>(c);
        FillValues v;
        fillOspfv3Header(dd, DATABASE_DESCRIPTION_PACKET, v);
        dd->setReserved1(0); // RFC 5340 A.3.3: reserved, must be sent as zero
        fillOspfv3Options(dd->getOptionsForUpdate(), v);
        dd->setInterfaceMTU(v.u16());
        auto& ddo = dd->getDdOptionsForUpdate();
        ddo.reserved = 0; // RFC 5340 A.3.3: reserved, must be sent as zero
        ddo.iBit = v.flag();
        ddo.mBit = v.flag();
        ddo.msBit = v.flag();
        dd->setSequenceNumber(v.u32());
        // the LS type of a header encodes the flooding scope, which differs per function
        // code: link-local for a Link LSA, AS-wide for an AS-External one, area otherwise
        dd->setLsaHeadersArraySize(4);
        dd->setLsaHeaders(0, makeOspfv3LsaHeader(v, ROUTER_LSA, v.u16()));
        dd->setLsaHeaders(1, makeOspfv3LsaHeader(v, NETWORK_LSA, v.u16()));
        dd->setLsaHeaders(2, makeOspfv3LsaHeader(v, LINK_LSA, v.u16()));
        dd->setLsaHeaders(3, makeOspfv3LsaHeader(v, AS_EXTERNAL_LSA, v.u16()));
        finalizeOspfLength(dd, c);
    }});

    fillers.push_back({"inet::ospfv3::Ospfv3LinkStateRequestPacket", "", [](Chunk *c) {
        auto lsr = check_and_cast<Ospfv3LinkStateRequestPacket *>(c);
        FillValues v;
        fillOspfv3Header(lsr, LINKSTATE_REQUEST_PACKET, v);
        lsr->setRequestsArraySize(2);
        Ospfv3LsRequest r0;
        r0.lsaType = ROUTER_LSA;
        r0.lsaID = v.ipv4();
        r0.advertisingRouter = v.ipv4();
        lsr->setRequests(0, r0);
        Ospfv3LsRequest r1;
        r1.lsaType = LINK_LSA;
        r1.lsaID = v.ipv4();
        r1.advertisingRouter = v.ipv4();
        lsr->setRequests(1, r1);
        finalizeOspfLength(lsr, c);
    }});

    fillers.push_back({"inet::ospfv3::Ospfv3LinkStateAcknowledgementPacket", "", [](Chunk *c) {
        auto ack = check_and_cast<Ospfv3LinkStateAcknowledgementPacket *>(c);
        FillValues v;
        fillOspfv3Header(ack, LINKSTATE_ACKNOWLEDGEMENT_PACKET, v);
        ack->setLsaHeadersArraySize(2);
        ack->setLsaHeaders(0, makeOspfv3LsaHeader(v, NETWORK_LSA, v.u16()));
        ack->setLsaHeaders(1, makeOspfv3LsaHeader(v, LINK_LSA, v.u16()));
        finalizeOspfLength(ack, c);
    }});

    // Link State Update: the single ordered Ospfv3Lsa *lsas[] list carries one LSA of
    // each of the five types Ospfv3PacketSerializer supports (Router, Network,
    // Inter-Area-Prefix, Link, Intra-Area-Prefix), each with a populated inner list, in
    // their on-the-wire order. Per-LSA lsaLength must match the actual RFC 5340 A.4.x
    // encoding byte-for-byte (all deserializers compute element counts from it).
    fillers.push_back({"inet::ospfv3::Ospfv3LinkStateUpdatePacket", "", [](Chunk *c) {
        auto upd = check_and_cast<Ospfv3LinkStateUpdatePacket *>(c);
        FillValues v;
        fillOspfv3Header(upd, LINKSTATE_UPDATE_PACKET, v);

        // Router LSA (A.4.3): flags byte + Options (4 B fixed) + one router-link entry
        // (16 B). header 20 + 4 + 16 = 40.
        auto *routerLsa = new Ospfv3RouterLsa();
        routerLsa->setHeader(makeOspfv3LsaHeader(v, ROUTER_LSA, 40));
        routerLsa->setNtBit(v.flag());
        routerLsa->setXBit(v.flag());
        routerLsa->setVBit(v.flag());
        routerLsa->setEBit(v.flag());
        routerLsa->setBBit(v.flag());
        fillOspfv3Options(routerLsa->getOspfOptionsForUpdate(), v);
        routerLsa->setRoutersArraySize(1);
        Ospfv3RouterLsaBody rb;
        rb.type = TRANSIT_NETWORK;
        rb.metric = v.u16();
        rb.interfaceID = v.u32();
        rb.neighborInterfaceID = v.u32();
        rb.neighborRouterID = v.ipv4();
        routerLsa->setRouters(0, rb);

        // Network LSA (A.4.4): pad byte + Options (4 B fixed) + two attached routers
        // (4 B each). header 20 + 4 + 8 = 32.
        auto *networkLsa = new Ospfv3NetworkLsa();
        networkLsa->setHeader(makeOspfv3LsaHeader(v, NETWORK_LSA, 32));
        fillOspfv3Options(networkLsa->getOspfOptionsForUpdate(), v);
        networkLsa->setAttachedRouterArraySize(2);
        networkLsa->setAttachedRouter(0, v.ipv4());
        networkLsa->setAttachedRouter(1, v.ipv4());

        // Inter-Area-Prefix LSA (A.4.5): reserved byte + 24-bit metric (4 B) + a /64
        // prefix (4 B fixed + 2*4 B address words). header 20 + 4 + 12 = 36.
        auto *iapLsa = new Ospfv3InterAreaPrefixLsa();
        iapLsa->setHeader(makeOspfv3LsaHeader(v, INTER_AREA_PREFIX_LSA, 36));
        iapLsa->setReserved1(0); // RFC 5340 A.4.5: reserved, must be sent as zero
        iapLsa->setMetric(v.uint(24));
        fillPrefix0(iapLsa->getPrefixForUpdate(), v, 64);

        // Link LSA (A.4.8): priority + Options + 16 B link-local address + numPrefixes
        // (24 B fixed) + one /64 prefix (12 B). header 20 + 24 + 12 = 56.
        auto *linkLsa = new Ospfv3LinkLsa();
        linkLsa->setHeader(makeOspfv3LsaHeader(v, LINK_LSA, 56));
        linkLsa->setRouterPriority(v.u8());
        fillOspfv3Options(linkLsa->getOspfOptionsForUpdate(), v);
        linkLsa->setLinkLocalInterfaceAdd(v.l3Ipv6());
        linkLsa->setNumPrefixes(1);
        linkLsa->setPrefixesArraySize(1);
        Ospfv3LsaPrefix0 p0;
        fillPrefix0(p0, v, 64);
        linkLsa->setPrefixes(0, p0);

        // Intra-Area-Prefix LSA (A.4.9): numPrefixes+referencedLSType+referencedLSID+
        // referencedAdvRtr (12 B fixed) + one /64 prefix-with-metric (12 B).
        // header 20 + 12 + 12 = 44.
        auto *intraLsa = new Ospfv3IntraAreaPrefixLsa();
        intraLsa->setHeader(makeOspfv3LsaHeader(v, INTRA_AREA_PREFIX_LSA, 44));
        intraLsa->setNumPrefixes(1);
        intraLsa->setReferencedLSType(ROUTER_LSA);
        intraLsa->setReferencedLSID(v.ipv4());
        intraLsa->setReferencedAdvRtr(v.ipv4());
        intraLsa->setPrefixesArraySize(1);
        Ospfv3LsaPrefixMetric pm;
        fillPrefixMetric(pm, v, 64);
        intraLsa->setPrefixes(0, pm);

        upd->setLsaCount(5);
        upd->setLsasArraySize(5);
        upd->setLsas(0, routerLsa);
        upd->setLsas(1, networkLsa);
        upd->setLsas(2, iapLsa);
        upd->setLsas(3, linkLsa);
        upd->setLsas(4, intraLsa);
        finalizeOspfLength(upd, c);
    }});
}

} // namespace inet
