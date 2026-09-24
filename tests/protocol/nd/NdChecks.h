//
// Helpers for the ND standards tests: read a Neighbor Discovery message, its options and the
// IPv6 header around it out of a frame, and turn a rule into an assertion that reports what the
// message held (doc/project/evidence/protocol/nd/checks.md).
//
// Why a helper header and not filter expressions. The icmpv6 dissector reaches every ND
// message, so an expression can select one by `icmpv6.type`. But an address field is an object
// and not a string, the options are a list of objects, and several checks compare a field with
// an address that exists only at run time (a link-local address, a MAC address). The helpers
// read the chunks of the frame directly.
//
// How an assertion reports. A failing assertion throws with a sentence that names the rule and
// the value it found; the tester prints it as "the predicate raised: ...".
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_NDCHECKS_H
#define __INET_PROTOCOLTEST_NDCHECKS_H

#include <algorithm>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "PacketEvent.h"
#include "inet/common/InitStages.h"
#include "inet/common/MemoryOutputStream.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#include "inet/networklayer/icmpv6/Ipv6NdMessage_m.h"
#include "inet/networklayer/icmpv6/MldMessage_m.h"
#include "inet/networklayer/icmpv6/Mldv2Message_m.h"
#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders_m.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"

namespace inet {
namespace protocoltest {

// The ICMPv6 types of RFC 4861 section 4.
const int ND_RS = 133;
const int ND_RA = 134;
const int ND_NS = 135;
const int ND_NA = 136;
const int ND_REDIRECT = 137;

// The first chunk of type T in a frame, or nullptr. A frame seen at a MAC starts with the
// link-layer header; the loop stops at the first chunk it cannot split.
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

// Every chunk of type T in a frame, in order: an ICMPv6 error carries the header of the packet
// that caused it after its own.
template<typename T>
inline std::vector<Ptr<const T>> findChunks(const Packet *packet)
{
    std::vector<Ptr<const T>> found;
    if (packet == nullptr)
        return found;
    std::unique_ptr<Packet> copy(packet->dup());
    try {
        while (copy->getDataLength() > b(0)) {
            const auto& front = copy->peekAtFront<Chunk>(b(-1), Chunk::PF_ALLOW_NULLPTR | Chunk::PF_ALLOW_INCORRECT);
            if (front == nullptr)
                break;
            if (auto wanted = dynamicPtrCast<const T>(front))
                found.push_back(wanted);
            copy->removeAtFront<Chunk>(front->getChunkLength(), Chunk::PF_ALLOW_INCORRECT);
        }
    }
    catch (const std::exception&) {
    }
    return found;
}

inline Ptr<const Ipv6Header> ipv6Of(const Packet *p) { return findChunk<Ipv6Header>(p); }
inline Ptr<const EthernetMacHeader> ethOf(const Packet *p) { return findChunk<EthernetMacHeader>(p); }
inline Ptr<const Ipv6NdMessage> ndOf(const Packet *p) { return findChunk<Ipv6NdMessage>(p); }
inline Ptr<const Ipv6RouterSolicitation> rsOf(const Packet *p) { return findChunk<Ipv6RouterSolicitation>(p); }
inline Ptr<const Ipv6RouterAdvertisement> raOf(const Packet *p) { return findChunk<Ipv6RouterAdvertisement>(p); }
inline Ptr<const Ipv6NeighbourSolicitation> nsOf(const Packet *p) { return findChunk<Ipv6NeighbourSolicitation>(p); }
inline Ptr<const Ipv6NeighbourAdvertisement> naOf(const Packet *p) { return findChunk<Ipv6NeighbourAdvertisement>(p); }
inline Ptr<const Ipv6Redirect> redirectOf(const Packet *p) { return findChunk<Ipv6Redirect>(p); }

// The ICMPv6 type of the ND message a frame carries, or -1.
inline int ndType(const Packet *p)
{
    auto nd = ndOf(p);
    return nd != nullptr ? (int)nd->getType() : -1;
}

inline bool isRs(const Packet *p) { return ndType(p) == ND_RS; }
inline bool isRa(const Packet *p) { return ndType(p) == ND_RA; }
inline bool isNs(const Packet *p) { return ndType(p) == ND_NS; }
inline bool isNa(const Packet *p) { return ndType(p) == ND_NA; }
inline bool isRedirect(const Packet *p) { return ndType(p) == ND_REDIRECT; }

inline Ipv6Address srcOf(const Packet *p) { auto h = ipv6Of(p); return h ? h->getSrcAddress() : Ipv6Address(); }
inline Ipv6Address destOf(const Packet *p) { auto h = ipv6Of(p); return h ? h->getDestAddress() : Ipv6Address(); }
inline int hopLimitOf(const Packet *p) { auto h = ipv6Of(p); return h ? (int)h->getHopLimit() : -1; }

// The length of the IPv6 packet in a frame: the header and its payload, in octets; or -1.
inline int ipv6LengthOf(const Packet *p)
{
    auto h = ipv6Of(p);
    return h != nullptr && h->getPayloadLength() >= B(0) ? 40 + (int)B(h->getPayloadLength()).get() : -1;
}

inline bool inPrefix(const Ipv6Address& a, const Ipv6Address& prefix, int length) { return a.matches(prefix, length); }

// The first ICMPv6 header of a frame, the one that the IPv6 header carries; or nullptr.
inline Ptr<const Icmpv6Header> icmpv6Of(const Packet *p) { return findChunk<Icmpv6Header>(p); }
inline int icmpv6TypeOf(const Packet *p) { auto h = icmpv6Of(p); return h != nullptr ? (int)h->getType() : -1; }
inline bool isEchoRequest(const Packet *p) { return icmpv6TypeOf(p) == ICMPv6_ECHO_REQUEST; }

// An MLD Report, version 1 or version 2, that names this group.
inline bool isMldReportFor(const Packet *p, const Ipv6Address& group)
{
    auto h = icmpv6Of(p);
    if (h == nullptr)
        return false;
    if (auto r = dynamicPtrCast<const MldReport>(h))
        return r->getMulticastAddress() == group;
    if (auto r = dynamicPtrCast<const Mldv2Report>(h)) {
        for (size_t i = 0; i < r->getMulticastAddressRecordArraySize(); i++)
            if (r->getMulticastAddressRecord(i).getGroupAddress() == group)
                return true;
    }
    return false;
}

// True when the datagram carries a Fragment extension header, which is a chunk of its own.
inline bool isFragmented(const Packet *p) { return findChunk<Ipv6FragmentHeader>(p) != nullptr; }

// The options of the ND message of a frame, or nullptr.
inline const Ipv6NdOptions *optionsOf(const Packet *p)
{
    if (auto m = rsOf(p)) return &m->getOptions();
    if (auto m = raOf(p)) return &m->getOptions();
    if (auto m = nsOf(p)) return &m->getOptions();
    if (auto m = naOf(p)) return &m->getOptions();
    if (auto m = redirectOf(p)) return &m->getOptions();
    return nullptr;
}

inline const Ipv6NdOption *optionOf(const Packet *p, Ipv6NdOptionTypes type)
{
    const Ipv6NdOptions *o = optionsOf(p);
    return o != nullptr ? o->findOption(type) : nullptr;
}

// The link-layer address that a Source or Target Link-layer Address option carries, or an
// unspecified MAC address when the option is absent.
inline MacAddress linkLayerOption(const Packet *p, Ipv6NdOptionTypes type)
{
    auto o = dynamic_cast<const Ipv6NdSourceTargetLinkLayerAddress *>(optionOf(p, type));
    return o != nullptr ? o->getLinkLayerAddress() : MacAddress::UNSPECIFIED_ADDRESS;
}

inline bool hasOption(const Packet *p, Ipv6NdOptionTypes type) { return optionOf(p, type) != nullptr; }

// The Prefix Information option of a Router Advertisement for this prefix, or nullptr.
inline const Ipv6NdPrefixInformation *prefixInfoOf(const Packet *p, const Ipv6Address& prefix, int length)
{
    const Ipv6NdOptions *o = optionsOf(p);
    if (o == nullptr)
        return nullptr;
    for (size_t i = 0; i < o->getOptionArraySize(); i++) {
        auto pi = dynamic_cast<const Ipv6NdPrefixInformation *>(o->getOption(i));
        if (pi != nullptr && pi->getPrefix() == prefix && pi->getPrefixLength() == length)
            return pi;
    }
    return nullptr;
}

// The octets of the ND message of a frame as the serializer writes them, or an empty vector.
// The Reserved fields and the Redirected Header option are read here, in their wire form.
inline std::vector<uint8_t> ndBytesOf(const Packet *p)
{
    auto nd = ndOf(p);
    if (nd == nullptr)
        return {};
    try {
        MemoryOutputStream stream;
        Chunk::serialize(stream, nd);
        return stream.getData();
    }
    catch (const std::exception&) {
        return {};
    }
}

// The length of the fixed part of each ND message, RFC 4861 section 4.
inline size_t ndFixedLength(int type)
{
    switch (type) {
        case ND_RS: return 8;
        case ND_RA: return 16;
        case ND_NS: case ND_NA: return 24;
        case ND_REDIRECT: return 40;
        default: return 0;
    }
}

// True when the Reserved field of the fixed part is zero: 32 bits in a Router Solicitation, a
// Neighbor Solicitation and a Redirect; the 6 bits after M and O in a Router Advertisement; the
// 29 bits after R, S and O in a Neighbor Advertisement.
inline bool ndReservedZero(const Packet *p)
{
    auto b = ndBytesOf(p);
    int t = ndType(p);
    if (b.size() < ndFixedLength(t) || b.size() < 8)
        return false;
    switch (t) {
        case ND_RS: case ND_NS: case ND_REDIRECT: return b[4] == 0 && b[5] == 0 && b[6] == 0 && b[7] == 0;
        case ND_RA: return (b[5] & 0x3f) == 0;
        case ND_NA: return (b[4] & 0x1f) == 0 && b[5] == 0 && b[6] == 0 && b[7] == 0;
        default: return false;
    }
}

// The octets of the first option of this type in the serialized message, or an empty vector.
inline std::vector<uint8_t> ndOptionBytes(const Packet *p, int optionType)
{
    auto b = ndBytesOf(p);
    size_t i = ndFixedLength(ndType(p));
    while (i + 2 <= b.size()) {
        size_t length = b[i + 1] * 8;
        if (length == 0 || i + length > b.size())
            break;
        if (b[i] == optionType)
            return std::vector<uint8_t>(b.begin() + i, b.begin() + i + length);
        i += length;
    }
    return {};
}

// The node's interface, and its link-local address and MAC address, at run time.
inline NetworkInterface *interfaceOf(const char *nodeName, const char *interfaceName)
{
    cModule *node = getSimulation()->getSystemModule()->getSubmodule(nodeName);
    auto ift = check_and_cast<IInterfaceTable *>(node->getSubmodule("interfaceTable"));
    NetworkInterface *ie = ift->findInterfaceByName(interfaceName);
    if (ie == nullptr)
        throw cRuntimeError("no interface %s on %s", interfaceName, nodeName);
    return ie;
}

inline Ipv6Address linkLocalOf(const char *nodeName, const char *interfaceName)
{
    return interfaceOf(nodeName, interfaceName)->getProtocolData<Ipv6InterfaceData>()->getLinkLocalAddress();
}

inline MacAddress macOf(const char *nodeName, const char *interfaceName)
{
    return interfaceOf(nodeName, interfaceName)->getMacAddress();
}

// The solicited-node multicast address of RFC 4291 section 2.7.1 for an address.
inline Ipv6Address solicitedNodeOf(const Ipv6Address& a) { return a.formSolicitedNodeMulticastAddress(); }

// For an assertion: true, or a throw that names the rule and what the message held.
inline bool require(bool holds, const std::string& what)
{
    if (!holds)
        throw std::runtime_error(what);
    return true;
}

// The system management of a router for the two variables that the model has no parameter for,
// AdvReachableTime and AdvRetransTimer (RFC 4861 section 6.2.1): it writes them into the
// interface data after the network layer has configured the interface, before the first
// advertisement. A negative value leaves the variable as it is.
class NdRouterVariables : public cSimpleModule
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override
    {
        if (stage != INITSTAGE_LAST)
            return;
        auto data = interfaceOf(par("node").stringValue(), par("interfaceName").stringValue())->getProtocolDataForUpdate<Ipv6InterfaceData>();
        if ((int)par("advReachableTime") >= 0)
            data->setAdvReachableTime(par("advReachableTime"));
        if ((int)par("advRetransTimer") >= 0)
            data->setAdvRetransTimer(par("advRetransTimer"));
    }
};

Define_Module(NdRouterVariables);

} // namespace protocoltest
} // namespace inet

#endif
