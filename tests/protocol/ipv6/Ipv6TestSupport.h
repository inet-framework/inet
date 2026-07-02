//
// IPv6 conformance suite -- shared test support.
//
// C++ predicates for the parts of an ND message the PacketFilter string engine cannot
// address: the IPv6 source/destination of the carrying datagram (to recognise a DAD
// probe or a solicited-node multicast target) and the TLV options nested inside a
// Router Advertisement (the Prefix Information option that drives SLAAC). Used from a
// test via EventPattern::match(lambda):
//
//   .match([](const MatchContext& c){ return isDadProbe(c.event); })
//   .match([](const MatchContext& c){ return raHasAutonomousPrefix(c.event); })
//
// Plain scalar/flag/address-equality assertions do NOT need this header -- write them as
// PacketFilter strings, e.g. .packet("ipv6.srcAddress == \"::\"") or
// .packet("Ipv6NeighbourAdvertisement.solicitedFlag == true").
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
#ifndef __INET_PROTOCOLTEST_IPV6_IPV6TESTSUPPORT_H
#define __INET_PROTOCOLTEST_IPV6_IPV6TESTSUPPORT_H

#include "ProtocolTest.h"

#include "inet/common/packet/dissector/PacketDissector.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#include "inet/networklayer/icmpv6/Ipv6NdMessage_m.h"
#include "inet/networklayer/icmpv6/MldMessage_m.h"
#include "inet/networklayer/icmpv6/Mldv2Message_m.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"

namespace inet {
namespace protocoltest {

// Dissects a packet and keeps every visited chunk, so a test can reach a nested chunk
// (e.g. the concrete ND message, or -- via its getOptions() -- an RA's Prefix option).
class Ipv6ChunkCollector : public PacketDissector::ICallback
{
  public:
    std::vector<Ptr<const Chunk>> chunks;
    bool shouldDissectProtocolDataUnit(const Protocol *) override { return true; }
    void startProtocolDataUnit(const Protocol *) override {}
    void endProtocolDataUnit(const Protocol *) override {}
    void markIncorrect() override {}
    void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *) override { chunks.push_back(chunk); }
};

// The first dissected chunk of type T in the event's packet, or nullptr.
template<typename T>
inline const T *chunkOfType(const PacketEvent& e)
{
    if (e.packet == nullptr)
        return nullptr;
    Ipv6ChunkCollector collector;
    PacketDissector dissector(ProtocolDissectorRegistry::getInstance(), collector);
    dissector.dissectPacket(const_cast<Packet *>(e.packet));
    for (const auto& chunk : collector.chunks)
        if (auto typed = dynamicPtrCast<const T>(chunk))
            return typed.get();
    return nullptr;
}

// True if the packet is IPv6-in-IPv6 tunneled (carries two or more IPv6 headers) -- e.g. a
// Home Agent tunneling a correspondent's packet to a mobile node's care-of address (MIPv6).
inline bool isIpv6Tunneled(const PacketEvent& e)
{
    if (e.packet == nullptr)
        return false;
    Ipv6ChunkCollector collector;
    PacketDissector dissector(ProtocolDissectorRegistry::getInstance(), collector);
    dissector.dissectPacket(const_cast<Packet *>(e.packet));
    int headers = 0;
    for (const auto& chunk : collector.chunks)
        if (dynamicPtrCast<const Ipv6Header>(chunk))
            headers++;
    return headers >= 2;
}

// IPv6 source / destination address of the observed datagram (UNSPECIFIED if not IPv6).
inline Ipv6Address ipv6Src(const PacketEvent& e)
{
    auto h = chunkOfType<Ipv6Header>(e);
    return h ? h->getSrcAddress() : Ipv6Address::UNSPECIFIED_ADDRESS;
}
inline Ipv6Address ipv6Dst(const PacketEvent& e)
{
    auto h = chunkOfType<Ipv6Header>(e);
    return h ? h->getDestAddress() : Ipv6Address::UNSPECIFIED_ADDRESS;
}

// RFC 4291: solicited-node multicast is ff02::1:ff00:0/104.
inline bool isSolicitedNodeMulticast(const Ipv6Address& a)
{
    return a.matches(Ipv6Address::SOLICITED_NODE_PREFIX, 104);
}

// A DAD probe (RFC 4862 5.4.2): a Neighbour Solicitation whose IPv6 source is the
// unspecified address and whose destination is the target's solicited-node multicast.
inline bool isDadProbe(const PacketEvent& e)
{
    auto ns = chunkOfType<Ipv6NeighbourSolicitation>(e);
    if (ns == nullptr)
        return false;
    return ipv6Src(e).isUnspecified() && isSolicitedNodeMulticast(ipv6Dst(e));
}

// The Neighbour Solicitation / Advertisement target address (UNSPECIFIED if none).
inline Ipv6Address nsTarget(const PacketEvent& e)
{
    auto ns = chunkOfType<Ipv6NeighbourSolicitation>(e);
    return ns ? ns->getTargetAddress() : Ipv6Address::UNSPECIFIED_ADDRESS;
}
inline Ipv6Address naTarget(const PacketEvent& e)
{
    auto na = chunkOfType<Ipv6NeighbourAdvertisement>(e);
    return na ? na->getTargetAddress() : Ipv6Address::UNSPECIFIED_ADDRESS;
}

// The Prefix Information option (with the autonomous/SLAAC flag) inside a Router
// Advertisement, or nullptr.
inline const Ipv6NdPrefixInformation *raPrefixOption(const PacketEvent& e)
{
    auto ra = chunkOfType<Ipv6RouterAdvertisement>(e);
    if (ra == nullptr)
        return nullptr;
    const Ipv6NdOptions& opts = ra->getOptions();
    for (size_t i = 0; i < opts.getOptionArraySize(); i++)
        if (auto pi = dynamic_cast<const Ipv6NdPrefixInformation *>(opts.getOption(i)))
            return pi;
    return nullptr;
}

// The RA carries a Prefix Information option with the autonomous (SLAAC) flag set.
inline bool raHasAutonomousPrefix(const PacketEvent& e)
{
    auto pi = raPrefixOption(e);
    return pi != nullptr && pi->getAutoAddressConfFlag();
}

// A Neighbour Solicitation carries a Source Link-Layer Address option (RFC 4861 4.6.1) --
// present on an address-resolution NS, absent on a DAD probe (unspecified source).
inline bool nsHasSllaOption(const PacketEvent& e)
{
    auto ns = chunkOfType<Ipv6NeighbourSolicitation>(e);
    if (ns == nullptr)
        return false;
    const Ipv6NdOptions& opts = ns->getOptions();
    for (size_t i = 0; i < opts.getOptionArraySize(); i++)
        if (opts.getOption(i)->getType() == IPv6ND_SOURCE_LINK_LAYER_ADDR_OPTION)
            return true;
    return false;
}

// A Neighbour Advertisement carries a Target Link-Layer Address option (RFC 4861 4.6.1).
inline bool naHasTllaOption(const PacketEvent& e)
{
    auto na = chunkOfType<Ipv6NeighbourAdvertisement>(e);
    if (na == nullptr)
        return false;
    const Ipv6NdOptions& opts = na->getOptions();
    for (size_t i = 0; i < opts.getOptionArraySize(); i++)
        if (opts.getOption(i)->getType() == IPv6ND_TARGET_LINK_LAYER_ADDR_OPTION)
            return true;
    return false;
}

// The RA carries an MTU option (RFC 4861 4.6.4).
inline bool raHasMtuOption(const PacketEvent& e)
{
    auto ra = chunkOfType<Ipv6RouterAdvertisement>(e);
    if (ra == nullptr)
        return false;
    const Ipv6NdOptions& opts = ra->getOptions();
    for (size_t i = 0; i < opts.getOptionArraySize(); i++)
        if (dynamic_cast<const Ipv6NdMtu *>(opts.getOption(i)))
            return true;
    return false;
}

// The multicast group address of an MLD message (Query/Report/Done share MldMessage's
// multicastAddress field), or UNSPECIFIED if the packet is not MLD.
inline Ipv6Address mldGroup(const PacketEvent& e)
{
    auto m = chunkOfType<MldMessage>(e);
    return m ? m->getMulticastAddress() : Ipv6Address::UNSPECIFIED_ADDRESS;
}

// An MLDv2 Report carrying a source-specific record (a group record with a non-empty source
// list) -- i.e. source-specific multicast (INCLUDE/EXCLUDE filtering), RFC 3810.
inline bool mldv2HasSourceRecord(const PacketEvent& e)
{
    auto r = chunkOfType<Mldv2Report>(e);
    if (r == nullptr)
        return false;
    for (size_t i = 0; i < r->getMulticastAddressRecordArraySize(); i++)
        if (r->getMulticastAddressRecord(i).getSourceList().size() > 0)
            return true;
    return false;
}

} // namespace protocoltest
} // namespace inet

#endif
