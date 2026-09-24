//
// Helpers for the MLD standards tests: read an MLD message, its Multicast Address Records, the
// IPv6 header and the Hop-by-Hop Options header around it out of a frame, and turn a rule into
// an assertion that reports what the message held (doc/project/evidence/protocol/mld/checks.md).
// The twin of tests/protocol/igmp/IgmpChecks.h.
//
// MldRequests plays the sockets of a host: it follows a timed script of requests, one filter
// for each named request, and calls the membership function of the interface with the old and
// the new filter of the request, as a socket does. The listening state of the interface is the
// union of the requests, as RFC 9777 section 4 defines it.
//
// How an assertion reports. A failing assertion throws with a sentence that names the rule and
// the value it found; the tester prints it as "the predicate raised: ...".
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_MLDCHECKS_H
#define __INET_PROTOCOLTEST_MLDCHECKS_H

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "PacketEvent.h"
#include "inet/common/InitStages.h"
#include "inet/common/MemoryOutputStream.h"
#include "inet/common/checksum/Checksum.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/icmpv6/MldMessage_m.h"
#include "inet/networklayer/icmpv6/Mldv2Message_m.h"
#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders_m.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6MulticastRoute.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {
namespace protocoltest {

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

inline Ptr<const Ipv6Header> ipv6Of(const Packet *p) { return findChunk<Ipv6Header>(p); }
inline Ptr<const EthernetMacHeader> ethOf(const Packet *p) { return findChunk<EthernetMacHeader>(p); }
inline Ptr<const Ipv6HopByHopOptionsHeader> hopByHopOf(const Packet *p) { return findChunk<Ipv6HopByHopOptionsHeader>(p); }

// The MLD message of a frame: an ICMPv6 message of type 130, 131, 132 or 143.
inline Ptr<const Icmpv6Header> mldOf(const Packet *p)
{
    auto m = findChunk<Icmpv6Header>(p);
    if (m == nullptr)
        return nullptr;
    int t = m->getType();
    return (t == ICMPv6_MLD_QUERY || t == ICMPv6_MLD_REPORT || t == ICMPv6_MLD_DONE || t == ICMPv6_MLDv2_REPORT) ? m : nullptr;
}

inline int mldType(const Packet *p) { auto m = mldOf(p); return m != nullptr ? (int)m->getType() : -1; }
inline int mldLength(const Packet *p) { auto m = mldOf(p); return m != nullptr ? (int)B(m->getChunkLength()).get() : -1; }

inline bool isQuery(const Packet *p) { return mldType(p) == ICMPv6_MLD_QUERY; }
inline Ptr<const MldMessage> mldMessageOf(const Packet *p) { return dynamicPtrCast<const MldMessage>(mldOf(p)); }
inline Ptr<const Mldv2Query> v2QueryOf(const Packet *p) { return dynamicPtrCast<const Mldv2Query>(mldOf(p)); }
inline Ptr<const Mldv2Report> v2ReportOf(const Packet *p) { return dynamicPtrCast<const Mldv2Report>(mldOf(p)); }
inline Ptr<const MldReport> v1ReportOf(const Packet *p) { return dynamicPtrCast<const MldReport>(mldOf(p)); }
inline Ptr<const MldDone> v1DoneOf(const Packet *p) { return dynamicPtrCast<const MldDone>(mldOf(p)); }
inline bool isV2Report(const Packet *p) { return v2ReportOf(p) != nullptr; }
inline bool isV1Report(const Packet *p) { return v1ReportOf(p) != nullptr; }
inline bool isV1Done(const Packet *p) { return v1DoneOf(p) != nullptr; }

// A Query of 28 octets or more is an MLDv2 Query; one of 24 octets is an MLDv1 Query (RFC 9777
// section 8.1).
inline bool isV2Query(const Packet *p) { return isQuery(p) && mldLength(p) >= 28; }
inline bool isV1Query(const Packet *p) { return isQuery(p) && mldLength(p) == 24; }
inline Ipv6Address queryAddress(const Packet *p) { auto q = mldMessageOf(p); return q != nullptr && isQuery(p) ? q->getMulticastAddress() : Ipv6Address(); }
inline bool isGeneralQuery(const Packet *p) { return isQuery(p) && queryAddress(p).isUnspecified(); }
inline int querySources(const Packet *p) { auto q = v2QueryOf(p); return q != nullptr ? (int)q->getSourceList().size() : 0; }
inline int maxRespCode(const Packet *p) { auto q = mldMessageOf(p); return q != nullptr ? (int)q->getMaxRespDelay() : -1; }
inline Ipv6Address messageAddress(const Packet *p) { auto m = mldMessageOf(p); return m != nullptr ? m->getMulticastAddress() : Ipv6Address(); }

// The delay that a Maximum Response Code stands for, in milliseconds: the plain value below
// 32768, else the floating-point code 1eee mmmm mmmm mmmm of RFC 9777 section 5.1.3.
inline double decodeMaxRespCode(int code)
{
    if (code < 32768)
        return code;
    int exp = (code >> 12) & 0x7, mant = code & 0xfff;
    return (double)((mant | 0x1000) << (exp + 3));
}

// The time that a QQIC stands for, in seconds: the plain value below 128, else the
// floating-point code of RFC 9777 section 5.1.9.
inline double decodeQqic(int code)
{
    if (code < 128)
        return code;
    int exp = (code >> 4) & 0x7, mant = code & 0xf;
    return (double)((mant | 0x10) << (exp + 3));
}

inline Ipv6Address srcOf(const Packet *p) { auto h = ipv6Of(p); return h ? h->getSrcAddress() : Ipv6Address(); }
inline Ipv6Address destOf(const Packet *p) { auto h = ipv6Of(p); return h ? h->getDestAddress() : Ipv6Address(); }
inline int hopLimitOf(const Packet *p) { auto h = ipv6Of(p); return h ? (int)h->getHopLimit() : -1; }

// The protocol of the upper layer: the Next Header of the Hop-by-Hop Options header when there
// is one, else the Next Header of the IPv6 header.
inline int upperProtocolOf(const Packet *p)
{
    auto hbh = hopByHopOf(p);
    if (hbh != nullptr)
        return (int)hbh->getNextHeaderProtocol();
    auto h = ipv6Of(p);
    return h ? (int)h->getProtocolId() : -1;
}

// The Router Alert option of RFC 2711, type 5, in the Hop-by-Hop Options header.
inline bool hasRouterAlert(const Packet *p)
{
    auto hbh = hopByHopOf(p);
    if (hbh == nullptr)
        return false;
    const TlvOptions& options = hbh->getTlvOptions();
    for (size_t i = 0; i < options.getTlvOptionArraySize(); i++)
        if (options.getTlvOption(i)->getType() == 5)
            return true;
    return false;
}

// True when the checksum of the MLD message, as the serializer writes it, is the ICMPv6
// checksum of RFC 4443 section 2.3: the one's complement sum of the pseudo-header of RFC 8200
// section 8.1 and the whole message, the checksum included, is all ones.
inline bool mldChecksumOk(const Packet *p)
{
    auto m = mldOf(p);
    auto h = ipv6Of(p);
    if (m == nullptr || h == nullptr)
        return false;
    try {
        MemoryOutputStream message;
        Chunk::serialize(message, m);
        const auto& body = message.getData();
        std::vector<uint8_t> data;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                data.push_back((h->getSrcAddress().words()[i] >> (24 - 8 * j)) & 0xff);
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                data.push_back((h->getDestAddress().words()[i] >> (24 - 8 * j)) & 0xff);
        uint32_t length = body.size();
        for (int j = 0; j < 4; j++)
            data.push_back((length >> (24 - 8 * j)) & 0xff);
        data.push_back(0); data.push_back(0); data.push_back(0); data.push_back(58);
        data.insert(data.end(), body.begin(), body.end());
        return internetChecksum(data) == 0;
    }
    catch (const std::exception&) {
        return false;
    }
}

// The Multicast Address Records of a Version 2 Report for an address, in order, as copies.
inline std::vector<Mldv2MulticastAddressRecord> recordsFor(const Packet *p, const Ipv6Address& address)
{
    std::vector<Mldv2MulticastAddressRecord> out;
    auto r = v2ReportOf(p);
    if (r != nullptr)
        for (size_t i = 0; i < r->getMulticastAddressRecordArraySize(); i++)
            if (r->getMulticastAddressRecord(i).getGroupAddress() == address)
                out.push_back(r->getMulticastAddressRecord(i));
    return out;
}

inline bool hasRecord(const Packet *p, const Ipv6Address& address) { return !recordsFor(p, address).empty(); }

inline bool hasRecordOf(const Packet *p, const Ipv6Address& address, int type)
{
    for (auto& rec : recordsFor(p, address))
        if (rec.getRecordType() == type)
            return true;
    return false;
}

inline std::set<Ipv6Address> sourcesOf(const Mldv2MulticastAddressRecord& rec)
{
    std::set<Ipv6Address> out;
    for (size_t i = 0; i < rec.getSourceList().size(); i++)
        out.insert(rec.getSourceList()[i]);
    return out;
}

// The sources of the record of this type for the address; empty when there is no such record.
inline std::set<Ipv6Address> sourcesOfRecord(const Packet *p, const Ipv6Address& address, int type)
{
    for (auto& rec : recordsFor(p, address))
        if (rec.getRecordType() == type)
            return sourcesOf(rec);
    return {};
}

inline std::string str(const std::set<Ipv6Address>& s)
{
    std::string out = "{";
    for (auto& a : s)
        out += (out.size() > 1 ? ", " : "") + a.str();
    return out + "}";
}

inline const char *recordTypeName(int t)
{
    switch (t) {
        case MLD_MODE_IS_INCLUDE: return "MODE_IS_INCLUDE";
        case MLD_MODE_IS_EXCLUDE: return "MODE_IS_EXCLUDE";
        case MLD_CHANGE_TO_INCLUDE_MODE: return "CHANGE_TO_INCLUDE_MODE";
        case MLD_CHANGE_TO_EXCLUDE_MODE: return "CHANGE_TO_EXCLUDE_MODE";
        case MLD_ALLOW_NEW_SOURCES: return "ALLOW_NEW_SOURCES";
        case MLD_BLOCK_OLD_SOURCES: return "BLOCK_OLD_SOURCES";
        default: return "unknown";
    }
}

// The records of a Report as text, for a message that reports them.
inline std::string recordsText(const Packet *p)
{
    auto r = v2ReportOf(p);
    if (r == nullptr)
        return "no Version 2 Report";
    std::string out;
    for (size_t i = 0; i < r->getMulticastAddressRecordArraySize(); i++) {
        const Mldv2MulticastAddressRecord& rec = r->getMulticastAddressRecord(i);
        out += (out.empty() ? "" : "; ") + std::string(recordTypeName(rec.getRecordType())) + " " + rec.getGroupAddress().str() + " " + str(sourcesOf(rec));
    }
    return out.empty() ? "no records" : out;
}

// A UDP datagram from a source to a multicast address.
inline bool isDatagram(const Packet *p, const Ipv6Address& source, const Ipv6Address& address)
{
    return upperProtocolOf(p) == IP_PROT_UDP && srcOf(p) == source && destOf(p) == address;
}

// The node's interface at run time.
inline NetworkInterface *interfaceOf(const char *nodeName, const char *interfaceName)
{
    cModule *node = getSimulation()->getSystemModule()->getSubmodule(nodeName);
    auto ift = check_and_cast<IInterfaceTable *>(node->getSubmodule("interfaceTable"));
    NetworkInterface *ie = ift->findInterfaceByName(interfaceName);
    if (ie == nullptr)
        throw cRuntimeError("no interface %s on %s", interfaceName, nodeName);
    return ie;
}

// The solicited-node address of an address: ff02::1:ff00:0 with the last 24 bits of it.
inline Ipv6Address solicitedNodeOf(const Ipv6Address& a)
{
    return Ipv6Address(0xff020000, 0, 1, 0xff000000 | (a.words()[3] & 0x00ffffff));
}

// The addresses of an interface at run time.
inline std::vector<Ipv6Address> addressesOf(const char *nodeName, const char *interfaceName)
{
    std::vector<Ipv6Address> out;
    auto data = interfaceOf(nodeName, interfaceName)->findProtocolData<Ipv6InterfaceData>();
    if (data != nullptr)
        for (int i = 0; i < data->getNumAddresses(); i++)
            out.push_back(data->getAddress(i));
    return out;
}

// For an assertion: true, or a throw that names the rule and what the message held.
inline bool require(bool holds, const std::string& what)
{
    if (!holds)
        throw std::runtime_error(what);
    return true;
}

// The requests of a host, as its sockets would make them. The script is a list of requests
// separated by ';': "<time> <request> <address> INCLUDE|EXCLUDE [<source> ...]". Each request
// keeps its own filter; "INCLUDE" with no sources ends it.
class MldRequests : public cSimpleModule
{
  protected:
    struct Request { simtime_t time; std::string name; Ipv6Address address; McastSourceFilterMode mode; std::vector<L3Address> sources; };
    struct Filter { McastSourceFilterMode mode = MCAST_INCLUDE_SOURCES; std::vector<L3Address> sources; };
    std::vector<Request> requests;
    std::map<std::pair<std::string, Ipv6Address>, Filter> filters;

    virtual void initialize() override
    {
        std::string script = par("script").stdstringValue();
        std::stringstream all(script);
        std::string item;
        while (std::getline(all, item, ';')) {
            std::stringstream words(item);
            double t;
            std::string name, address, mode, source;
            if (!(words >> t >> name >> address >> mode))
                continue;
            Request r;
            r.time = t;
            r.name = name;
            r.address = Ipv6Address(address.c_str());
            r.mode = mode == "EXCLUDE" ? MCAST_EXCLUDE_SOURCES : MCAST_INCLUDE_SOURCES;
            while (words >> source)
                r.sources.push_back(L3Address(Ipv6Address(source.c_str())));
            requests.push_back(r);
            cMessage *msg = new cMessage("request");
            msg->setContextPointer((void *)(intptr_t)(requests.size() - 1));
            scheduleAt(r.time, msg);
        }
    }

    virtual void handleMessage(cMessage *msg) override
    {
        const Request& r = requests[(intptr_t)msg->getContextPointer()];
        delete msg;
        NetworkInterface *ie = interfaceOf(par("node").stringValue(), par("interfaceName").stringValue());
        Filter& f = filters[{r.name, r.address}];
        std::vector<L3Address> newSources = r.sources;
        std::sort(newSources.begin(), newSources.end());
        EV_INFO << "request " << r.name << " for " << r.address << ": " << (r.mode == MCAST_EXCLUDE_SOURCES ? "EXCLUDE" : "INCLUDE") << " with " << newSources.size() << " sources\n";
        ie->changeMulticastGroupMembership(L3Address(r.address), f.mode, f.sources, r.mode, newSources);
        f.mode = r.mode;
        f.sources = newSources;
    }
};

Define_Module(MldRequests);

// The multicast routing of R, reduced to what the checks need: one route from the sources of
// L2 to L1 for all multicast addresses, with L1 marked as a leaf, so that the IPv6 forwarding
// asks the MLD router state whether L1 has listeners. It stands in for a multicast routing
// protocol, which is outside the in-scope set, and is added after the network layer has
// configured the router.
class MulticastLeafRoute6 : public cSimpleModule
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override
    {
        if (stage != INITSTAGE_LAST)
            return;
        const char *node = par("node").stringValue();
        cModule *nodeModule = getSimulation()->getSystemModule()->getSubmodule(node);
        auto rt = check_and_cast<Ipv6RoutingTable *>(nodeModule->getModuleByPath(".ipv6.routingTable"));
        auto route = new Ipv6MulticastRoute();
        route->setSourceType(IMulticastRoute::MANUAL);
        route->setOrigin(Ipv6Address(par("origin").stringValue()));
        route->setPrefixLength(par("prefixLength").intValue());
        route->setMulticastGroup(Ipv6Address(par("group").stringValue()));
        route->setInInterface(new IMulticastRoute::InInterface(interfaceOf(node, par("inInterface").stringValue())));
        route->addOutInterface(new IMulticastRoute::OutInterface(interfaceOf(node, par("outInterface").stringValue()), true));
        rt->addMulticastRoute(route);
    }
};

Define_Module(MulticastLeafRoute6);

} // namespace protocoltest
} // namespace inet

#endif
