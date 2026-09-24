//
// Helpers for the IGMP standards tests: read an IGMP message, its Group Records and the IPv4
// header around it out of a frame, and turn a rule into an assertion that reports what the
// message held (doc/project/evidence/protocol/igmp/checks.md).
//
// IgmpRequests plays the sockets of a host: it follows a timed script of requests, one filter
// for each named request, and calls the membership function of the interface with the old and
// the new filter of the request, as a socket does. The interface state is the union of the
// requests, as RFC 9776 section 3 defines it.
//
// How an assertion reports. A failing assertion throws with a sentence that names the rule and
// the value it found; the tester prints it as "the predicate raised: ...".
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_IGMPCHECKS_H
#define __INET_PROTOCOLTEST_IGMPCHECKS_H

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
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/IgmpMessage_m.h"
#include "inet/networklayer/ipv4/Ipv4Route.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
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

inline Ptr<const Ipv4Header> ipv4Of(const Packet *p) { return findChunk<Ipv4Header>(p); }
inline Ptr<const EthernetMacHeader> ethOf(const Packet *p) { return findChunk<EthernetMacHeader>(p); }
inline Ptr<const IgmpMessage> igmpOf(const Packet *p) { return findChunk<IgmpMessage>(p); }

inline int igmpType(const Packet *p) { auto m = igmpOf(p); return m != nullptr ? (int)m->getType() : -1; }
inline int igmpLength(const Packet *p) { auto m = igmpOf(p); return m != nullptr ? (int)B(m->getChunkLength()).get() : -1; }

inline bool isQuery(const Packet *p) { return igmpType(p) == IGMP_MEMBERSHIP_QUERY; }
inline Ptr<const Igmpv3Query> v3QueryOf(const Packet *p) { return dynamicPtrCast<const Igmpv3Query>(igmpOf(p)); }
inline Ptr<const IgmpQuery> queryOf(const Packet *p) { return dynamicPtrCast<const IgmpQuery>(igmpOf(p)); }
inline Ptr<const Igmpv3Report> v3ReportOf(const Packet *p) { return dynamicPtrCast<const Igmpv3Report>(igmpOf(p)); }
inline Ptr<const Igmpv2Report> v2ReportOf(const Packet *p) { return dynamicPtrCast<const Igmpv2Report>(igmpOf(p)); }
inline Ptr<const Igmpv2Leave> v2LeaveOf(const Packet *p) { return dynamicPtrCast<const Igmpv2Leave>(igmpOf(p)); }
inline bool isV3Report(const Packet *p) { return v3ReportOf(p) != nullptr; }
inline bool isV2Report(const Packet *p) { return v2ReportOf(p) != nullptr; }
inline bool isV2Leave(const Packet *p) { return v2LeaveOf(p) != nullptr; }

// A Query of 12 octets or more is an IGMPv3 Query; one of 8 octets is IGMPv2 or IGMPv1 by its
// Max Resp Code (RFC 9776 section 7.1).
inline bool isV3Query(const Packet *p) { return isQuery(p) && igmpLength(p) >= 12; }
inline bool isV2Query(const Packet *p)
{
    auto q = dynamicPtrCast<const Igmpv2Query>(igmpOf(p));
    return isQuery(p) && igmpLength(p) == 8 && q != nullptr && q->getMaxRespTimeCode() != 0;
}
inline Ipv4Address queryGroup(const Packet *p) { auto q = queryOf(p); return q != nullptr ? q->getGroupAddress() : Ipv4Address(); }
inline bool isGeneralQuery(const Packet *p) { return isQuery(p) && queryGroup(p).isUnspecified(); }
inline int querySources(const Packet *p) { auto q = v3QueryOf(p); return q != nullptr ? (int)q->getSourceList().size() : 0; }
inline int maxRespCode(const Packet *p)
{
    auto q = dynamicPtrCast<const Igmpv2Query>(igmpOf(p));
    return q != nullptr ? (int)q->getMaxRespTimeCode() : -1;
}

// The time that a Max Resp Code or a QQIC stands for: the plain value below 128, else the
// floating-point code of RFC 9776 section 4.1.1 (in units of the code).
inline double decodeCode(int code)
{
    if (code < 128)
        return code;
    int exp = (code >> 4) & 0x7, mant = code & 0xf;
    return (double)((mant | 0x10) << (exp + 3));
}

inline Ipv4Address srcOf(const Packet *p) { auto h = ipv4Of(p); return h ? h->getSrcAddress() : Ipv4Address(); }
inline Ipv4Address destOf(const Packet *p) { auto h = ipv4Of(p); return h ? h->getDestAddress() : Ipv4Address(); }
inline int ttlOf(const Packet *p) { auto h = ipv4Of(p); return h ? (int)h->getTimeToLive() : -1; }
inline int protocolOf(const Packet *p) { auto h = ipv4Of(p); return h ? (int)h->getProtocolId() : -1; }
inline int precedenceOf(const Packet *p) { auto h = ipv4Of(p); return h ? (int)(h->getTypeOfService() >> 5) : -1; }
inline bool hasRouterAlert(const Packet *p) { auto h = ipv4Of(p); return h != nullptr && h->findOptionByType(IPOPTION_ROUTER_ALERT) != nullptr; }

// True when the checksum of the IGMP message, as the serializer writes it, is correct: the one's
// complement sum of the whole message, the checksum included, is all ones.
inline bool igmpChecksumOk(const Packet *p)
{
    auto m = igmpOf(p);
    if (m == nullptr)
        return false;
    try {
        MemoryOutputStream stream;
        Chunk::serialize(stream, m);
        const auto& data = stream.getData();
        return internetChecksum(data) == 0;
    }
    catch (const std::exception&) {
        return false;
    }
}

// The Group Records of a Version 3 Report for a group, in order, as copies.
inline std::vector<GroupRecord> recordsFor(const Packet *p, const Ipv4Address& group)
{
    std::vector<GroupRecord> out;
    auto r = v3ReportOf(p);
    if (r != nullptr)
        for (size_t i = 0; i < r->getGroupRecordArraySize(); i++)
            if (r->getGroupRecord(i).getGroupAddress() == group)
                out.push_back(r->getGroupRecord(i));
    return out;
}

inline bool hasRecord(const Packet *p, const Ipv4Address& group) { return !recordsFor(p, group).empty(); }

inline bool hasRecordOf(const Packet *p, const Ipv4Address& group, int type)
{
    for (auto& rec : recordsFor(p, group))
        if (rec.getRecordType() == type)
            return true;
    return false;
}

inline std::set<Ipv4Address> sourcesOf(const GroupRecord& rec)
{
    std::set<Ipv4Address> out;
    for (size_t i = 0; i < rec.getSourceList().size(); i++)
        out.insert(rec.getSourceList()[i]);
    return out;
}

// The sources of the record of this type for the group; empty when there is no such record.
inline std::set<Ipv4Address> sourcesOfRecord(const Packet *p, const Ipv4Address& group, int type)
{
    for (auto& rec : recordsFor(p, group))
        if (rec.getRecordType() == type)
            return sourcesOf(rec);
    return {};
}

inline std::string str(const std::set<Ipv4Address>& s)
{
    std::string out = "{";
    for (auto& a : s)
        out += (out.size() > 1 ? ", " : "") + a.str();
    return out + "}";
}

inline const char *recordTypeName(int t)
{
    switch (t) {
        case MODE_IS_INCLUDE: return "MODE_IS_INCLUDE";
        case MODE_IS_EXCLUDE: return "MODE_IS_EXCLUDE";
        case CHANGE_TO_INCLUDE_MODE: return "CHANGE_TO_INCLUDE_MODE";
        case CHANGE_TO_EXCLUDE_MODE: return "CHANGE_TO_EXCLUDE_MODE";
        case ALLOW_NEW_SOURCES: return "ALLOW_NEW_SOURCES";
        case BLOCK_OLD_SOURCE: return "BLOCK_OLD_SOURCES";
        default: return "unknown";
    }
}

// The records of a Report as text, for a message that reports them.
inline std::string recordsText(const Packet *p)
{
    auto r = v3ReportOf(p);
    if (r == nullptr)
        return "no Version 3 Report";
    std::string out;
    for (size_t i = 0; i < r->getGroupRecordArraySize(); i++) {
        const GroupRecord& rec = r->getGroupRecord(i);
        out += (out.empty() ? "" : "; ") + std::string(recordTypeName(rec.getRecordType())) + " " + rec.getGroupAddress().str() + " " + str(sourcesOf(rec));
    }
    return out.empty() ? "no records" : out;
}

// A UDP datagram from a source to a group.
inline bool isDatagram(const Packet *p, const Ipv4Address& source, const Ipv4Address& group)
{
    return protocolOf(p) == IP_PROT_UDP && srcOf(p) == source && destOf(p) == group;
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

// For an assertion: true, or a throw that names the rule and what the message held.
inline bool require(bool holds, const std::string& what)
{
    if (!holds)
        throw std::runtime_error(what);
    return true;
}

// The requests of a host, as its sockets would make them. The script is a list of requests
// separated by ';': "<time> <request> <group> INCLUDE|EXCLUDE [<source> ...]". Each request keeps
// its own filter; "INCLUDE" with no sources ends it.
class IgmpRequests : public cSimpleModule
{
  protected:
    struct Request { simtime_t time; std::string name; Ipv4Address group; McastSourceFilterMode mode; std::vector<L3Address> sources; };
    struct Filter { McastSourceFilterMode mode = MCAST_INCLUDE_SOURCES; std::vector<L3Address> sources; };
    std::vector<Request> requests;
    std::map<std::pair<std::string, Ipv4Address>, Filter> filters;

    virtual void initialize() override
    {
        std::string script = par("script").stdstringValue();
        std::stringstream all(script);
        std::string item;
        while (std::getline(all, item, ';')) {
            std::stringstream words(item);
            double t;
            std::string name, group, mode, source;
            if (!(words >> t >> name >> group >> mode))
                continue;
            Request r;
            r.time = t;
            r.name = name;
            r.group = Ipv4Address(group.c_str());
            r.mode = mode == "EXCLUDE" ? MCAST_EXCLUDE_SOURCES : MCAST_INCLUDE_SOURCES;
            while (words >> source)
                r.sources.push_back(L3Address(Ipv4Address(source.c_str())));
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
        Filter& f = filters[{r.name, r.group}];
        std::vector<L3Address> newSources = r.sources;
        std::sort(newSources.begin(), newSources.end());
        EV_INFO << "request " << r.name << " for " << r.group << ": " << (r.mode == MCAST_EXCLUDE_SOURCES ? "EXCLUDE" : "INCLUDE") << " with " << newSources.size() << " sources\n";
        ie->changeMulticastGroupMembership(L3Address(r.group), f.mode, f.sources, r.mode, newSources);
        f.mode = r.mode;
        f.sources = newSources;
    }
};

Define_Module(IgmpRequests);

// The multicast routing of R, reduced to what the checks need: one route from the sources of
// L2 to L1 for the groups, with L1 marked as a leaf, so that the IPv4 forwarding asks the IGMP
// router state whether L1 has listeners. It stands in for a multicast routing protocol, which
// is outside the in-scope set, and is added after the network layer has configured the router.
class MulticastLeafRoute : public cSimpleModule
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override
    {
        if (stage != INITSTAGE_LAST)
            return;
        const char *node = par("node").stringValue();
        cModule *nodeModule = getSimulation()->getSystemModule()->getSubmodule(node);
        auto rt = check_and_cast<IIpv4RoutingTable *>(nodeModule->getModuleByPath(".ipv4.routingTable"));
        auto route = new Ipv4MulticastRoute();
        route->setSourceType(IMulticastRoute::MANUAL);
        route->setOrigin(Ipv4Address(par("origin").stringValue()));
        route->setOriginNetmask(Ipv4Address(par("netmask").stringValue()));
        route->setMulticastGroup(Ipv4Address(par("group").stringValue()));
        route->setInInterface(new IMulticastRoute::InInterface(interfaceOf(node, par("inInterface").stringValue())));
        route->addOutInterface(new IMulticastRoute::OutInterface(interfaceOf(node, par("outInterface").stringValue()), true));
        rt->addMulticastRoute(route);
    }
};

Define_Module(MulticastLeafRoute);

} // namespace protocoltest
} // namespace inet

#endif
