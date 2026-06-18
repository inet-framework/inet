//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Scriptable MLDv2 SSM tester, the IPv6/MLD counterpart of IGMPTester.
//
// It owns the tester NIC of a TestMLDNetwork node and maintains a per-(interface,group)
// Ipv6MulticastSourceList "socket state". ScenarioManager <join>/<leave>/<block>/<allow>/
// <set-filter> commands drive that state into the interface via
// Ipv6InterfaceData::changeMulticastGroupMembership(...), which fires ipv6MulticastChange
// and exercises the MLDv2 host state machine. <dump what="groups"/> prints the interface's
// merged INCLUDE/EXCLUDE filter, and <send type="Mldv2Query"/> injects a Query so a host's
// Current-State Report can be observed. Reports the MLD module sends back are decoded and
// logged by handleMessage(), exactly like IGMPTester.

#include <algorithm>

#include "inet/common/INETDefs.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/scenario/IScriptable.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"
#include "inet/networklayer/icmpv6/Icmpv6.h"
#include "inet/networklayer/icmpv6/MldMessage_m.h" // MLDv1 MldQuery/MldReport/MldDone for interop tests
#include "inet/networklayer/icmpv6/Mldv2.h"
#include "inet/networklayer/icmpv6/Mldv2Message_m.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"

using namespace std;

namespace inet {

class INET_API TestMld : public cSimpleModule, public IScriptable
{
  private:
    typedef Ipv6InterfaceData::Ipv6AddressVector Ipv6AddressVector;

    IInterfaceTable *ift;
    NetworkInterface *networkInterface;
    map<Ipv6Address, Ipv6MulticastSourceList> socketState;

    ChecksumMode checksumMode = CHECKSUM_MODE_UNDEFINED;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void processCommand(const cXMLElement &node) override;

  private:
    void processSendCommand(const cXMLElement &node);
    void processJoinCommand(Ipv6Address group, const Ipv6AddressVector &sources, NetworkInterface *ie);
    void processLeaveCommand(Ipv6Address group, const Ipv6AddressVector &sources, NetworkInterface *ie);
    void processBlockCommand(Ipv6Address group, const Ipv6AddressVector &sources, NetworkInterface *ie);
    void processAllowCommand(Ipv6Address group, const Ipv6AddressVector &sources, NetworkInterface *ie);
    void processSetFilterCommand(Ipv6Address group, McastSourceFilterMode filterMode, const Ipv6AddressVector &sources, NetworkInterface *ie);
    void processDumpCommand(string what, NetworkInterface *ie);
    void parseIpv6AddressVector(const char *str, Ipv6AddressVector &result);
    void sendMld(Packet *msg, NetworkInterface *ie, Ipv6Address dest);
};

Define_Module(TestMld);

static ostream &operator<<(ostream &out, const Mldv2Report *report)
{
    out << report->getClassName() << "<";
    for (unsigned int i = 0; i < report->getMulticastAddressRecordArraySize(); i++) {
        const Mldv2MulticastAddressRecord &record = report->getMulticastAddressRecord(i);
        out << (i > 0 ? ", " : "") << record.getGroupAddress() << "=";
        switch (record.getRecordType()) {
            case MLD_MODE_IS_INCLUDE:        out << "IS_IN"; break;
            case MLD_MODE_IS_EXCLUDE:        out << "IS_EX"; break;
            case MLD_CHANGE_TO_INCLUDE_MODE: out << "TO_IN"; break;
            case MLD_CHANGE_TO_EXCLUDE_MODE: out << "TO_EX"; break;
            case MLD_ALLOW_NEW_SOURCES:      out << "ALLOW"; break;
            case MLD_BLOCK_OLD_SOURCES:      out << "BLOCK"; break;
        }
        if (!record.getSourceList().empty())
            out << " " << record.getSourceList();
    }
    out << ">";
    return out;
}

void TestMld::initialize(int stage)
{
    if (stage == INITSTAGE_LINK_LAYER) {
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        networkInterface = getContainingNicModule(this);
        networkInterface->setInterfaceName("eth0");
        MacAddress address("AA:00:00:00:00:01");
        networkInterface->setMacAddress(address);
        networkInterface->setInterfaceToken(address.formInterfaceIdentifier());
        networkInterface->setMtu(par("mtu"));
        networkInterface->setMulticast(true);
        networkInterface->setBroadcast(true);
        const char *checksumModeString = par("checksumMode");
        checksumMode = parseChecksumMode(checksumModeString, false);
    }
    else if (stage == INITSTAGE_NETWORK_ADDRESS_ASSIGNMENT) {
        // Give the interface a link-local address so it counts as a configured IPv6 interface.
        auto ipv6Data = networkInterface->getProtocolDataForUpdate<Ipv6InterfaceData>();
        ipv6Data->assignAddress(Ipv6Address("fe80::aa00:ff:fe00:1"), false, SIMTIME_ZERO, SIMTIME_ZERO);
    }
}

void TestMld::handleMessage(cMessage *msg)
{
    Packet *pk = check_and_cast<Packet *>(msg);
    const auto &icmp = pk->peekAtFront<Icmpv6Header>();
    switch (icmp->getType()) {
        case ICMPv6_MLDv2_REPORT: {
            const auto &report = pk->peekAtFront<Mldv2Report>();
            EV << "TestMld: Received: " << report.get() << ".\n";
            break;
        }
        case ICMPv6_MLD_REPORT: {
            const auto &report = pk->peekAtFront<MldReport>();
            EV << "TestMld: Received: " << report->getClassName() << "<group=" << report->getMulticastAddress() << ">.\n";
            break;
        }
        case ICMPv6_MLD_DONE: {
            const auto &done = pk->peekAtFront<MldDone>();
            EV << "TestMld: Received: " << done->getClassName() << "<group=" << done->getMulticastAddress() << ">.\n";
            break;
        }
        default:
            EV << "TestMld: Received: unexpected MLD type " << (int)icmp->getType() << ".\n";
            break;
    }
    delete msg;
}

void TestMld::processCommand(const cXMLElement &node)
{
    Enter_Method("processCommand");

    string tag = node.getTagName();
    const char *ifname = node.getAttribute("ifname");
    NetworkInterface *ie = CHK(ift->findInterfaceByName(CHK(ifname)));

    if (tag == "join") {
        const char *group = node.getAttribute("group");
        Ipv6AddressVector sources;
        parseIpv6AddressVector(node.getAttribute("sources"), sources);
        processJoinCommand(Ipv6Address(group), sources, ie);
    }
    else if (tag == "leave") {
        const char *group = node.getAttribute("group");
        Ipv6AddressVector sources;
        parseIpv6AddressVector(node.getAttribute("sources"), sources);
        processLeaveCommand(Ipv6Address(group), sources, ie);
    }
    else if (tag == "block") {
        const char *group = node.getAttribute("group");
        Ipv6AddressVector sources;
        parseIpv6AddressVector(node.getAttribute("sources"), sources);
        processBlockCommand(Ipv6Address(group), sources, ie);
    }
    else if (tag == "allow") {
        const char *group = node.getAttribute("group");
        Ipv6AddressVector sources;
        parseIpv6AddressVector(node.getAttribute("sources"), sources);
        processAllowCommand(Ipv6Address(group), sources, ie);
    }
    else if (tag == "set-filter") {
        const char *groupAttr = node.getAttribute("group");
        const char *sourcesAttr = node.getAttribute("sources");
        ASSERT((sourcesAttr[0] == 'I' || sourcesAttr[0] == 'E') && (sourcesAttr[1] == ' ' || sourcesAttr[1] == '\0'));
        McastSourceFilterMode filterMode = sourcesAttr[0] == 'I' ? MCAST_INCLUDE_SOURCES : MCAST_EXCLUDE_SOURCES;
        Ipv6AddressVector sources;
        if (sourcesAttr[1])
            parseIpv6AddressVector(sourcesAttr + 2, sources);
        processSetFilterCommand(Ipv6Address(groupAttr), filterMode, sources, ie);
    }
    else if (tag == "dump") {
        const char *what = node.getAttribute("what");
        processDumpCommand(what, ie);
    }
    else if (tag == "send") {
        processSendCommand(node);
    }
}

void TestMld::processSendCommand(const cXMLElement &node)
{
    const char *ifname = node.getAttribute("ifname");
    NetworkInterface *ie = CHK(ift->findInterfaceByName(CHK(ifname)));
    string type = node.getAttribute("type");

    if (type == "Mldv2Query") {
        const char *groupStr = node.getAttribute("group");
        const char *maxRespCodeStr = node.getAttribute("maxRespCode");
        const char *sourcesStr = node.getAttribute("source");

        Ipv6Address group = groupStr ? Ipv6Address(groupStr) : Ipv6Address::UNSPECIFIED_ADDRESS;
        // maxRespCode is in milliseconds (RFC 3810 §5.1.3); default 10000 ms (10 s).
        int maxRespCode = maxRespCodeStr ? atoi(maxRespCodeStr) : 10000;
        Ipv6AddressVector sources;
        parseIpv6AddressVector(sourcesStr, sources);

        Packet *packet = new Packet("Mldv2 query");
        const auto &msg = makeShared<Mldv2Query>();
        msg->setType(ICMPv6_MLD_QUERY);
        msg->setMulticastAddress(group);
        msg->setMaxRespDelay(Mldv2::codeMaxRespCode((uint16_t)maxRespCode));
        msg->setSourceList(sources);
        msg->setChunkLength(B(28 + (16 * sources.size())));
        Icmpv6::insertChecksum(checksumMode, msg, packet);
        packet->insertAtFront(msg);
        sendMld(packet, ie, group.isUnspecified() ? Ipv6Address::ALL_NODES_2 : group);
    }
    else if (type == "MldQuery") {
        // MLDv1 Query (24-byte, ICMPv6 type 130). General if group unspecified.
        const char *groupStr = node.getAttribute("group");
        const char *maxRespCodeStr = node.getAttribute("maxRespCode");
        Ipv6Address group = groupStr ? Ipv6Address(groupStr) : Ipv6Address::UNSPECIFIED_ADDRESS;
        int maxRespCode = maxRespCodeStr ? atoi(maxRespCodeStr) : 10000; // ms

        Packet *packet = new Packet("Mldv1 query");
        const auto &msg = makeShared<MldQuery>();
        msg->setType(ICMPv6_MLD_QUERY);
        msg->setMulticastAddress(group);
        msg->setMaxRespDelay((uint16_t)maxRespCode);
        msg->setChunkLength(B(24));
        Icmpv6::insertChecksum(checksumMode, msg, packet);
        packet->insertAtFront(msg);
        sendMld(packet, ie, group.isUnspecified() ? Ipv6Address::ALL_NODES_2 : group);
    }
    else if (type == "Mldv2Report") {
        // MLDv2 Multicast Listener Report (ICMPv6 type 143) with one or more records.
        cXMLElementList records = node.getElementsByTagName("record");
        Packet *packet = new Packet("Mldv2 report");
        const auto &msg = makeShared<Mldv2Report>();
        unsigned int byteLength = 8; // Mldv2Report header size

        msg->setMulticastAddressRecordArraySize(records.size());
        for (size_t i = 0; i < records.size(); ++i) {
            cXMLElement *recordNode = records[i];
            const char *groupStr = recordNode->getAttribute("group");
            string recordTypeStr = recordNode->getAttribute("type");
            const char *sourcesStr = recordNode->getAttribute("sources");
            ASSERT(groupStr);

            Mldv2MulticastAddressRecord &record = msg->getMulticastAddressRecordForUpdate(i);
            record.setGroupAddress(Ipv6Address(groupStr));
            parseIpv6AddressVector(sourcesStr, record.getSourceListForUpdate());
            record.setRecordType(recordTypeStr == "IS_IN" ? MLD_MODE_IS_INCLUDE :
                                recordTypeStr == "IS_EX" ? MLD_MODE_IS_EXCLUDE :
                                recordTypeStr == "TO_IN" ? MLD_CHANGE_TO_INCLUDE_MODE :
                                recordTypeStr == "TO_EX" ? MLD_CHANGE_TO_EXCLUDE_MODE :
                                recordTypeStr == "ALLOW" ? MLD_ALLOW_NEW_SOURCES :
                                recordTypeStr == "BLOCK" ? MLD_BLOCK_OLD_SOURCES : 0);
            ASSERT(record.getGroupAddress().isMulticast());
            ASSERT(record.getRecordType());
            byteLength += 20 + record.getSourceList().size() * 16; // 20 byte record header + n * 16 byte (Ipv6Address)
        }
        msg->setChunkLength(B(byteLength));
        Icmpv6::insertChecksum(checksumMode, msg, packet);
        packet->insertAtFront(msg);
        sendMld(packet, ie, Ipv6Address::ALL_MLDV2_ROUTERS);
    }
    else if (type == "MldReport" || type == "MldDone") {
        // MLDv1 Report (type 131, dest=group) / Done (type 132, dest=ff02::2).
        const char *groupStr = node.getAttribute("group");
        ASSERT(groupStr);
        Ipv6Address group = Ipv6Address(groupStr);

        Packet *packet = new Packet("Mldv1 older-version");
        Ptr<MldMessage> msg;
        Ipv6Address dest;
        if (type == "MldReport") {
            const auto &m = makeShared<MldReport>();
            m->setMulticastAddress(group);
            m->setChunkLength(B(24));
            msg = m;
            dest = group;
        }
        else { // MldDone
            const auto &m = makeShared<MldDone>();
            m->setMulticastAddress(group);
            m->setChunkLength(B(24));
            msg = m;
            dest = Ipv6Address::ALL_ROUTERS_2;
        }
        Icmpv6::insertChecksum(checksumMode, msg, packet);
        packet->insertAtFront(msg);
        sendMld(packet, ie, dest);
    }
}

void TestMld::processJoinCommand(Ipv6Address group, const Ipv6AddressVector &sources, NetworkInterface *ie)
{
    if (sources.empty()) {
        ie->getProtocolDataForUpdate<Ipv6InterfaceData>()->joinMulticastGroup(group);
        socketState[group] = Ipv6MulticastSourceList::ALL_SOURCES;
    }
    else {
        Ipv6MulticastSourceList &sourceList = socketState[group];
        ASSERT(sourceList.filterMode == MCAST_INCLUDE_SOURCES);
        Ipv6AddressVector oldSources(sourceList.sources);
        for (auto source = sources.begin(); source != sources.end(); ++source)
            sourceList.add(*source);
        if (oldSources != sourceList.sources)
            ie->getProtocolDataForUpdate<Ipv6InterfaceData>()->changeMulticastGroupMembership(group, MCAST_INCLUDE_SOURCES, oldSources, MCAST_INCLUDE_SOURCES, sourceList.sources);
    }
}

void TestMld::processLeaveCommand(Ipv6Address group, const Ipv6AddressVector &sources, NetworkInterface *ie)
{
    if (sources.empty()) {
        ie->getProtocolDataForUpdate<Ipv6InterfaceData>()->leaveMulticastGroup(group);
        socketState.erase(group);
    }
    else {
        Ipv6MulticastSourceList &sourceList = socketState[group];
        ASSERT(sourceList.filterMode == MCAST_INCLUDE_SOURCES);
        Ipv6AddressVector oldSources(sourceList.sources);
        for (auto source = sources.begin(); source != sources.end(); ++source)
            sourceList.remove(*source);
        if (oldSources != sourceList.sources)
            ie->getProtocolDataForUpdate<Ipv6InterfaceData>()->changeMulticastGroupMembership(group, MCAST_INCLUDE_SOURCES, oldSources, MCAST_INCLUDE_SOURCES, sourceList.sources);
        if (sourceList.sources.empty())
            socketState.erase(group);
    }
}

void TestMld::processBlockCommand(Ipv6Address group, const Ipv6AddressVector &sources, NetworkInterface *ie)
{
    auto it = socketState.find(group);
    ASSERT(it != socketState.end());
    ASSERT(it->second.filterMode == MCAST_EXCLUDE_SOURCES);
    Ipv6AddressVector oldSources(it->second.sources);
    for (auto source = sources.begin(); source != sources.end(); ++source)
        it->second.add(*source);
    if (oldSources != it->second.sources)
        ie->getProtocolDataForUpdate<Ipv6InterfaceData>()->changeMulticastGroupMembership(group, MCAST_EXCLUDE_SOURCES, oldSources, MCAST_EXCLUDE_SOURCES, it->second.sources);
}

void TestMld::processAllowCommand(Ipv6Address group, const Ipv6AddressVector &sources, NetworkInterface *ie)
{
    auto it = socketState.find(group);
    ASSERT(it != socketState.end());
    ASSERT(it->second.filterMode == MCAST_EXCLUDE_SOURCES);
    Ipv6AddressVector oldSources(it->second.sources);
    for (auto source = sources.begin(); source != sources.end(); ++source)
        it->second.remove(*source);
    if (oldSources != it->second.sources)
        ie->getProtocolDataForUpdate<Ipv6InterfaceData>()->changeMulticastGroupMembership(group, MCAST_EXCLUDE_SOURCES, oldSources, MCAST_EXCLUDE_SOURCES, it->second.sources);
}

void TestMld::processSetFilterCommand(Ipv6Address group, McastSourceFilterMode filterMode, const Ipv6AddressVector &sources, NetworkInterface *ie)
{
    Ipv6MulticastSourceList &sourceList = socketState[group];
    McastSourceFilterMode oldFilterMode = sourceList.filterMode;
    Ipv6AddressVector oldSources(sourceList.sources);

    sourceList.filterMode = filterMode;
    sourceList.sources = sources;

    if (filterMode != oldFilterMode || oldSources != sourceList.sources)
        ie->getProtocolDataForUpdate<Ipv6InterfaceData>()->changeMulticastGroupMembership(group, oldFilterMode, oldSources, sourceList.filterMode, sourceList.sources);
    if (sourceList.filterMode == MCAST_INCLUDE_SOURCES && sourceList.sources.empty())
        socketState.erase(group);
}

void TestMld::processDumpCommand(string what, NetworkInterface *ie)
{
    EV << "TestMld: " << ie->getInterfaceName() << ": " << what << " = ";

    if (what == "groups") {
        auto ipv6Data = ie->getProtocolDataForUpdate<Ipv6InterfaceData>();
        for (int i = 0; i < ipv6Data->getNumOfJoinedMulticastGroups(); i++) {
            Ipv6Address group = ipv6Data->getJoinedMulticastGroup(i);
            const Ipv6MulticastSourceList &sourceList = ipv6Data->getJoinedMulticastSources(i);
            EV << (i == 0 ? "" : ", ") << group << " " << sourceList.str();
        }
    }
    else if (what == "listeners") {
        auto ipv6Data = ie->getProtocolData<Ipv6InterfaceData>();
        for (int i = 0; i < ipv6Data->getNumOfReportedMulticastGroups(); i++) {
            Ipv6Address group = ipv6Data->getReportedMulticastGroup(i);
            const Ipv6MulticastSourceList &sourceList = ipv6Data->getReportedMulticastSources(i);
            EV << (i == 0 ? "" : ", ") << group << " " << sourceList.str();
        }
    }

    EV << ".\n";
}

void TestMld::sendMld(Packet *msg, NetworkInterface *ie, Ipv6Address dest)
{
    ASSERT(ie->isMulticast());

    msg->addTagIfAbsent<InterfaceInd>()->setInterfaceId(ie->getInterfaceId());
    msg->addTagIfAbsent<L3AddressInd>()->setDestAddress(dest);
    msg->addTagIfAbsent<HopLimitInd>()->setHopLimit(1);
    msg->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::mld);
    msg->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::mld);
    msg->addTagIfAbsent<DispatchProtocolInd>()->setProtocol(&Protocol::ipv6);

    EV << "TestMld: Sending: " << msg << ".\n";
    send(msg, "upperLayerOut");
}

void TestMld::parseIpv6AddressVector(const char *str, Ipv6AddressVector &result)
{
    if (str) {
        cStringTokenizer tokens(str);
        while (tokens.hasMoreTokens())
            result.push_back(Ipv6Address(tokens.nextToken()));
    }
    sort(result.begin(), result.end());
}

} // namespace inet
