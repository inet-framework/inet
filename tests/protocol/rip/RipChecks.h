//
// Helpers for the RIP standards tests: read the RIP message, the UDP header and the IP header
// out of a frame, find a route entry by its destination, and sort a message the way the check
// procedures do (doc/project/evidence/protocol/rip/checks.md, "Rules every check obeys").
//
// Why a helper header and not filter expressions. The UDP port table of the model has no entry
// for port 520 or 521 (src/inet/common/ProtocolGroup.cc, udpProtocols), so the dissector of a
// frame never reaches the RIP header, and an expression over `rip.*` cannot match. The helpers
// walk the chunks of the frame instead, which needs no dissector. The gap is recorded in
// doc/project/evidence/model/rip/results.md.
//
// How a subnet mask is read. The route entry of the model holds a prefix length where RFC 2453
// has a subnet mask field: a mask of 255.255.255.0 is the prefix length 24. The helpers take the
// prefix length, and each test names the mask it stands for.
//
// How an assertion reports. An assertion that fails throws with a sentence that names the rule
// and the value it found; the tester prints it as "the predicate raised: ...". A plain `false`
// would only say that "the predicate does not hold".
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_RIPCHECKS_H
#define __INET_PROTOCOLTEST_RIPCHECKS_H

#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include "PacketEvent.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"
#include "inet/routing/rip/RipPacket_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {
namespace protocoltest {

// RFC 2453 section 3.6 and RFC 2080 section 2.1.
const int RIP_PORT = 520;
const int RIPNG_PORT = 521;
const unsigned int RIP_INFINITY = 16;

// The first chunk of type T in a frame, or nullptr. A frame seen at a MAC starts with the
// link-layer header and carries the RIP message behind the IP and UDP headers. The loop stops at
// the first chunk it cannot split, which is why it returns nullptr instead of throwing.
template<typename T>
inline Ptr<const T> findChunk(const Packet *packet)
{
    if (packet == nullptr)
        return nullptr;
    std::unique_ptr<Packet> copy(packet->dup());
    try {
        while (copy->getDataLength() > b(0)) {
            const auto& front = copy->peekAtFront<Chunk>(b(-1), Chunk::PF_ALLOW_NULLPTR | Chunk::PF_ALLOW_INCORRECT);
            if (front == nullptr)
                return nullptr;
            auto wanted = dynamicPtrCast<const T>(front);
            if (wanted != nullptr)
                return wanted;
            copy->removeAtFront<Chunk>(front->getChunkLength(), Chunk::PF_ALLOW_INCORRECT);
        }
    }
    catch (const std::exception&) {
        return nullptr;
    }
    return nullptr;
}

inline Ptr<const RipPacket> ripOf(const Packet *packet) { return findChunk<RipPacket>(packet); }
inline Ptr<const UdpHeader> udpOf(const Packet *packet) { return findChunk<UdpHeader>(packet); }
inline Ptr<const Ipv4Header> ipv4Of(const Packet *packet) { return findChunk<Ipv4Header>(packet); }
inline Ptr<const Ipv6Header> ipv6Of(const Packet *packet) { return findChunk<Ipv6Header>(packet); }

// The addresses and the hop limit of the IP header of a frame, IPv4 or IPv6.
inline L3Address sourceAddressOf(const Packet *packet)
{
    if (auto h = ipv4Of(packet))
        return L3Address(h->getSrcAddress());
    if (auto h = ipv6Of(packet))
        return L3Address(h->getSrcAddress());
    return L3Address();
}

inline L3Address destinationAddressOf(const Packet *packet)
{
    if (auto h = ipv4Of(packet))
        return L3Address(h->getDestAddress());
    if (auto h = ipv6Of(packet))
        return L3Address(h->getDestAddress());
    return L3Address();
}

inline int hopLimitOf(const Packet *packet)
{
    if (auto h = ipv4Of(packet))
        return h->getTimeToLive();
    if (auto h = ipv6Of(packet))
        return h->getHopLimit();
    return -1;
}

// The multicast groups of RFC 2453 section 4.5 and RFC 2080 section 2.5.
inline bool isRipGroup(const L3Address& address)
{
    return address == L3Address("224.0.0.9") || address == L3Address("ff02::9");
}

inline bool isResponse(const Packet *packet)
{
    auto rip = ripOf(packet);
    return rip != nullptr && rip->getCommand() == RIP_RESPONSE;
}

inline bool isRequest(const Packet *packet)
{
    auto rip = ripOf(packet);
    return rip != nullptr && rip->getCommand() == RIP_REQUEST;
}

// An update: a response to the multicast group (checks.md, "Rules every check obeys").
inline bool isUpdate(const Packet *packet)
{
    return isResponse(packet) && isRipGroup(destinationAddressOf(packet));
}

// The route entry for this destination, or nullptr. RIPng next hop entries (metric 0xFF) are
// not route entries and are never returned.
inline const RipEntry *findEntry(const Ptr<const RipPacket>& rip, const L3Address& address, int prefixLength)
{
    if (rip == nullptr)
        return nullptr;
    for (size_t i = 0; i < rip->getEntryArraySize(); i++) {
        const RipEntry& entry = rip->getEntry(i);
        if (entry.metric != 0xFF && entry.address == address && entry.prefixLength == prefixLength)
            return &entry;
    }
    return nullptr;
}

// The metric of the entry for this destination, or -1 when the message holds no such entry.
inline int metricOf(const Packet *packet, const char *address, int prefixLength)
{
    const RipEntry *entry = findEntry(ripOf(packet), L3Address(address), prefixLength);
    return entry != nullptr ? (int)entry->metric : -1;
}

// A periodic update: an update that holds the entry of the marker (checks.md).
inline bool isPeriodicUpdate(const Packet *packet, const char *markerAddress, int markerPrefixLength)
{
    return isUpdate(packet) && metricOf(packet, markerAddress, markerPrefixLength) >= 0;
}

// An update that holds this destination with this metric.
inline bool isUpdateWith(const Packet *packet, const char *address, int prefixLength, int metric)
{
    return isUpdate(packet) && metricOf(packet, address, prefixLength) == metric;
}

// An update that holds this destination with a finite metric, below 16.
inline bool isUpdateWithFinite(const Packet *packet, const char *address, int prefixLength)
{
    int metric = metricOf(packet, address, prefixLength);
    return isUpdate(packet) && metric >= 0 && metric < (int)RIP_INFINITY;
}

// The link-local address of an interface of a node, for example ("R1", "eth1"). The address
// exists only at run time, so a check that compares a source address looks it up then.
inline L3Address linkLocalOf(const char *nodeName, const char *interfaceName)
{
    cModule *node = getSimulation()->getSystemModule()->getSubmodule(nodeName);
    auto ift = check_and_cast<IInterfaceTable *>(node->getSubmodule("interfaceTable"));
    NetworkInterface *ie = ift->findInterfaceByName(interfaceName);
    if (ie == nullptr)
        throw cRuntimeError("no interface %s on %s", interfaceName, nodeName);
    return L3Address(ie->getProtocolData<Ipv6InterfaceData>()->getLinkLocalAddress());
}

// For an assertion: true, or a throw that names the rule and what the message held.
inline bool require(bool holds, const std::string& what)
{
    if (!holds)
        throw std::runtime_error(what);
    return true;
}

// The entries of a message, one per line, for the reason of a failed assertion.
inline std::string entriesOf(const Ptr<const RipPacket>& rip)
{
    std::ostringstream os;
    if (rip == nullptr)
        return "no RIP message";
    for (size_t i = 0; i < rip->getEntryArraySize(); i++) {
        const RipEntry& e = rip->getEntry(i);
        os << (i ? "; " : "") << "af " << (int)e.addressFamilyId << " " << e.address.str() << "/" << e.prefixLength
           << " metric " << e.metric << " next hop " << e.nextHop.str() << " tag " << e.routeTag;
    }
    return os.str();
}

} // namespace protocoltest
} // namespace inet

#endif
