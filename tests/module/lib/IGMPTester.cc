#include <algorithm>
#include <fstream>

#include "inet/common/INETDefs.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/scenario/IScriptable.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/IgmpMessage.h"
#include "inet/networklayer/ipv4/Igmpv3.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

using namespace std;

namespace inet {

class INET_API IGMPTester : public cSimpleModule, public IScriptable
{
  private:
    IInterfaceTable *ift;
    InterfaceEntry *interfaceEntry;
    map<Ipv4Address, Ipv4MulticastSourceList> socketState;

    //crcMode
    CrcMode crcMode = CRC_MODE_UNDEFINED;

  protected:
    typedef Ipv4InterfaceData::Ipv4AddressVector Ipv4AddressVector;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void processCommand(const cXMLElement &node) override;
  private:
    void processSendCommand(const cXMLElement &node);
    void processJoinCommand(Ipv4Address group, const Ipv4AddressVector &sources, InterfaceEntry* ie);
    void processLeaveCommand(Ipv4Address group, const Ipv4AddressVector &sources, InterfaceEntry* ie);
    void processBlockCommand(Ipv4Address group, const Ipv4AddressVector &sources, InterfaceEntry *ie);
    void processAllowCommand(Ipv4Address group, const Ipv4AddressVector &sources, InterfaceEntry *ie);
    void processSetFilterCommand(Ipv4Address group, McastSourceFilterMode filterMode, const Ipv4AddressVector &sources, InterfaceEntry *ie);
    void processDumpCommand(string what, InterfaceEntry *ie);
    void parseIPv4AddressVector(const char *str, Ipv4AddressVector &result);
    void sendIGMP(Packet *msg, InterfaceEntry *ie, Ipv4Address dest);
};

Define_Module(IGMPTester);

static ostream &operator<<(ostream &out, const IgmpMessage* msg)
{
    out << msg->getClassName() << "<";

    switch (msg->getType()) {
        case IGMP_MEMBERSHIP_QUERY: {
            auto *query = check_and_cast<const IgmpQuery*>(msg);
            out << "group=" << query->getGroupAddress();
            if (auto v3Query = dynamic_cast<const Igmpv3Query*>(msg)) {
                out << ", sourceList=" << v3Query->getSourceList()
                    << ", maxRespTime=" << SimTime(Igmpv3::decodeTime(v3Query->getMaxRespTimeCode()), (SimTimeUnit)-1)
                    << ", suppressRouterProc=" << (int)v3Query->getSuppressRouterProc()
                    << ", robustnessVariable=" << (int)v3Query->getRobustnessVariable()
                    << ", queryIntervalCode=" << SimTime(Igmpv3::decodeTime(v3Query->getQueryIntervalCode()), SIMTIME_S);
            }
            else if (auto v2Query = dynamic_cast<const Igmpv2Query*>(msg))
                out << ", maxRespTime=" << SimTime(v2Query->getMaxRespTimeCode(), (SimTimeUnit)-1);
            break;
        }
        case IGMPV1_MEMBERSHIP_REPORT:
            // TODO
            break;
        case IGMPV2_MEMBERSHIP_REPORT:
            // TODO
            break;
        case IGMPV2_LEAVE_GROUP:
            // TODO
            break;
        case IGMPV3_MEMBERSHIP_REPORT: {
            auto report = check_and_cast<const Igmpv3Report*>(msg);
            for (unsigned int i = 0; i < report->getGroupRecordArraySize(); i++) {
                const GroupRecord &record = report->getGroupRecord(i);
                out << (i>0?", ":"") << record.getGroupAddress() << "=";
                switch (record.getRecordType()) {
                    case MODE_IS_INCLUDE:        out << "IS_IN" ; break;
                    case MODE_IS_EXCLUDE:        out << "IS_EX" ; break;
                    case CHANGE_TO_INCLUDE_MODE: out << "TO_IN" ; break;
                    case CHANGE_TO_EXCLUDE_MODE: out << "TO_EX" ; break;
                    case ALLOW_NEW_SOURCES:      out << "ALLOW" ; break;
                    case BLOCK_OLD_SOURCE:       out << "BLOCK" ; break;
                }
                if (!record.getSourceList().empty())
                    out << " " << record.getSourceList();
            }
            break;
        }
        default:
            throw cRuntimeError("Unexpected message.");
            break;
    }
    out << ">";
    return out;
}

void IGMPTester::initialize(int stage)
{
    if (stage == INITSTAGE_LINK_LAYER) {
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        interfaceEntry = getContainingNicModule(this);
        interfaceEntry->setInterfaceName("eth0");
        MacAddress address("AA:00:00:00:00:01");
        interfaceEntry->setMacAddress(address);
        interfaceEntry->setInterfaceToken(address.formInterfaceIdentifier());
        interfaceEntry->setMtu(par("mtu"));
        interfaceEntry->setMulticast(true);
        interfaceEntry->setBroadcast(true);
        const char *crcModeString = par("crcMode");
        crcMode = parseCrcMode(crcModeString, false);
    }
    else if (stage == INITSTAGE_NETWORK_ADDRESS_ASSIGNMENT) {
        interfaceEntry->getProtocolData<Ipv4InterfaceData>()->setIPAddress(Ipv4Address("192.168.1.1"));
        interfaceEntry->getProtocolData<Ipv4InterfaceData>()->setNetmask(Ipv4Address("255.255.0.0"));
    }
}

void IGMPTester::handleMessage(cMessage *msg)
{
    Packet *pk = check_and_cast<Packet*>(msg);
    const auto& igmpMsg = pk->peekAtFront<IgmpMessage>();
    EV << "IGMPTester: Received: " << igmpMsg.get() << ".\n";
    delete msg;
}

void IGMPTester::processCommand(const cXMLElement &node)
{
    Enter_Method_Silent();

    string tag = node.getTagName();
    const char *ifname = node.getAttribute("ifname");
    InterfaceEntry *ie = CHK(ift->findInterfaceByName(CHK(ifname)));

    if (tag == "join") {
        const char *group = node.getAttribute("group");
        Ipv4AddressVector sources;
        parseIPv4AddressVector(node.getAttribute("sources"), sources);
        processJoinCommand(Ipv4Address(group), sources, ie);
    }
    else if (tag == "leave") {
        const char *group = node.getAttribute("group");
        Ipv4AddressVector sources;
        parseIPv4AddressVector(node.getAttribute("sources"), sources);
        processLeaveCommand(Ipv4Address(group), sources, ie);
    }
    else if (tag == "block") {
        const char *group = node.getAttribute("group");
        Ipv4AddressVector sources;
        parseIPv4AddressVector(node.getAttribute("sources"), sources);
        processBlockCommand(Ipv4Address(group), sources, ie);
    }
    else if (tag == "allow") {
        const char *group = node.getAttribute("group");
        Ipv4AddressVector sources;
        parseIPv4AddressVector(node.getAttribute("sources"), sources);
        processAllowCommand(Ipv4Address(group), sources, ie);
    }
    else if (tag == "set-filter") {
        const char *groupAttr = node.getAttribute("group");
        const char *sourcesAttr = node.getAttribute("sources");
        ASSERT((sourcesAttr[0] == 'I' || sourcesAttr[0] == 'E') && (sourcesAttr[1] == ' ' || sourcesAttr[1] == '\0'));
        McastSourceFilterMode filterMode = sourcesAttr[0] == 'I' ? MCAST_INCLUDE_SOURCES : MCAST_EXCLUDE_SOURCES;
        Ipv4AddressVector sources;
        if (sourcesAttr[1])
            parseIPv4AddressVector(sourcesAttr+2, sources);
        processSetFilterCommand(Ipv4Address(groupAttr), filterMode, sources, ie);
    }
    else if (tag == "dump") {
        const char *what = node.getAttribute("what");
        processDumpCommand(what, ie);
    }
    else if (tag == "send") {
        processSendCommand(node);
    }
}

void IGMPTester::processSendCommand(const cXMLElement &node)
{
    const char *ifname = node.getAttribute("ifname");
    InterfaceEntry *ie = CHK(ift->findInterfaceByName(CHK(ifname)));
    string type = node.getAttribute("type");

    if (type == "Igmpv1Query") {
        // TODO
    }
    else if (type == "Igmpv2Query") {
        // TODO
    }
    else if (type == "Igmpv3Query") {
        const char *groupStr = node.getAttribute("group");
        const char *maxRespCodeStr = node.getAttribute("maxRespCode");
        const char *sourcesStr = node.getAttribute("source");

        Ipv4Address group = groupStr ? Ipv4Address(groupStr) : Ipv4Address::UNSPECIFIED_ADDRESS;
        int maxRespCode = maxRespCodeStr ? atoi(maxRespCodeStr) : 100 /*10 sec*/;
        Ipv4AddressVector sources;
        parseIPv4AddressVector(sourcesStr, sources);

        Packet *packet = new Packet("Igmpv3 query");
        const auto& msg = makeShared<Igmpv3Query>();
        msg->setType(IGMP_MEMBERSHIP_QUERY);
        msg->setGroupAddress(group);
        msg->setMaxRespTimeCode(Igmpv3::codeTime(maxRespCode));
        msg->setSourceList(sources);
        msg->setChunkLength(B(12 + (4 * sources.size())));
        Igmpv3::insertCrc(crcMode, msg, packet);
        packet->insertAtFront(msg);
        sendIGMP(packet, ie, group.isUnspecified() ? Ipv4Address::ALL_HOSTS_MCAST : group);
    }
    else if (type == "Igmpv2Report") {
        // TODO
    }
    else if (type == "Igmpv2Leave") {
        // TODO
    }
    else if (type == "Igmpv3Report") {
        cXMLElementList records = node.getElementsByTagName("record");
        Packet *packet = new Packet("Igmpv3 report");
        const auto& msg = makeShared<Igmpv3Report>();
        unsigned int byteLength = 8;   // Igmpv3Report header size

        msg->setGroupRecordArraySize(records.size());
        for (int i = 0; i < (int)records.size(); ++i) {
            cXMLElement *recordNode = records[i];
            const char *groupStr = recordNode->getAttribute("group");
            string recordTypeStr = recordNode->getAttribute("type");
            const char *sourcesStr = recordNode->getAttribute("sources");
            ASSERT(groupStr);

            GroupRecord &record = msg->getGroupRecordForUpdate(i);
            record.setGroupAddress(Ipv4Address(groupStr));
            parseIPv4AddressVector(sourcesStr, record.getSourceListForUpdate());
            record.setRecordType(recordTypeStr == "IS_IN" ? MODE_IS_INCLUDE :
                                recordTypeStr == "IS_EX" ? MODE_IS_EXCLUDE :
                                recordTypeStr == "TO_IN" ? CHANGE_TO_INCLUDE_MODE :
                                recordTypeStr == "TO_EX" ? CHANGE_TO_EXCLUDE_MODE :
                                recordTypeStr == "ALLOW" ? ALLOW_NEW_SOURCES :
                                recordTypeStr == "BLOCK" ? BLOCK_OLD_SOURCE : 0);
            ASSERT(record.getGroupAddress().isMulticast());
            ASSERT(record.getRecordType());
            byteLength += 8 + record.getSourceList().size() * 4;    // 8 byte header + n * 4 byte (Ipv4Address)
        }
        msg->setChunkLength(B(byteLength));
        Igmpv3::insertCrc(crcMode, msg, packet);
        packet->insertAtFront(msg);

        sendIGMP(packet, ie, Ipv4Address::ALL_IGMPV3_ROUTERS_MCAST);
    }
}

void IGMPTester::processJoinCommand(Ipv4Address group, const Ipv4AddressVector &sources, InterfaceEntry *ie)
{
    if (sources.empty()) {
        ie->getProtocolData<Ipv4InterfaceData>()->joinMulticastGroup(group);
        socketState[group] = Ipv4MulticastSourceList::ALL_SOURCES;
    }
    else {
        Ipv4MulticastSourceList &sourceList = socketState[group];
        ASSERT(sourceList.filterMode == MCAST_INCLUDE_SOURCES);
        Ipv4AddressVector oldSources(sourceList.sources);
        for (Ipv4AddressVector::const_iterator source = sources.begin(); source != sources.end(); ++source)
            sourceList.add(*source);
        if (oldSources != sourceList.sources)
            ie->getProtocolData<Ipv4InterfaceData>()->changeMulticastGroupMembership(group, MCAST_INCLUDE_SOURCES, oldSources, MCAST_INCLUDE_SOURCES, sourceList.sources);
    }
}

void IGMPTester::processLeaveCommand(Ipv4Address group, const Ipv4AddressVector &sources, InterfaceEntry *ie)
{
    if (sources.empty()) {
        ie->getProtocolData<Ipv4InterfaceData>()->leaveMulticastGroup(group);
        socketState.erase(group);
    }
    else {
        Ipv4MulticastSourceList &sourceList = socketState[group];
        ASSERT(sourceList.filterMode == MCAST_INCLUDE_SOURCES);
        Ipv4AddressVector oldSources(sourceList.sources);
        for (Ipv4AddressVector::const_iterator source = sources.begin(); source != sources.end(); ++source)
            sourceList.remove(*source);
        if (oldSources != sourceList.sources)
            ie->getProtocolData<Ipv4InterfaceData>()->changeMulticastGroupMembership(group, MCAST_INCLUDE_SOURCES, oldSources, MCAST_INCLUDE_SOURCES, sourceList.sources);
        if (sourceList.sources.empty())
            socketState.erase(group);
    }
}

void IGMPTester::processBlockCommand(Ipv4Address group, const Ipv4AddressVector &sources, InterfaceEntry *ie)
{
    map<Ipv4Address, Ipv4MulticastSourceList>::iterator it = socketState.find(group);
    ASSERT(it != socketState.end());
    ASSERT(it->second.filterMode == MCAST_EXCLUDE_SOURCES);
    Ipv4AddressVector oldSources(it->second.sources);
    for (Ipv4AddressVector::const_iterator source = sources.begin(); source != sources.end(); ++source)
        it->second.add(*source);
    if (oldSources != it->second.sources)
        ie->getProtocolData<Ipv4InterfaceData>()->changeMulticastGroupMembership(group, MCAST_EXCLUDE_SOURCES, oldSources, MCAST_EXCLUDE_SOURCES, it->second.sources);
}

void IGMPTester::processAllowCommand(Ipv4Address group, const Ipv4AddressVector &sources, InterfaceEntry *ie)
{
    map<Ipv4Address, Ipv4MulticastSourceList>::iterator it = socketState.find(group);
    ASSERT(it != socketState.end());
    ASSERT(it->second.filterMode == MCAST_EXCLUDE_SOURCES);
    Ipv4AddressVector oldSources(it->second.sources);
    for (Ipv4AddressVector::const_iterator source = sources.begin(); source != sources.end(); ++source)
        it->second.remove(*source);
    if (oldSources != it->second.sources)
        ie->getProtocolData<Ipv4InterfaceData>()->changeMulticastGroupMembership(group, MCAST_EXCLUDE_SOURCES, oldSources, MCAST_EXCLUDE_SOURCES, it->second.sources);
}

void IGMPTester::processSetFilterCommand(Ipv4Address group, McastSourceFilterMode filterMode, const Ipv4AddressVector &sources, InterfaceEntry *ie)
{
    Ipv4MulticastSourceList &sourceList = socketState[group];
    McastSourceFilterMode oldFilterMode = sourceList.filterMode;
    Ipv4AddressVector oldSources(sourceList.sources);

    sourceList.filterMode = filterMode;
    sourceList.sources = sources;

    if (filterMode != oldFilterMode || oldSources != sourceList.sources)
        ie->getProtocolData<Ipv4InterfaceData>()->changeMulticastGroupMembership(group, oldFilterMode, oldSources, sourceList.filterMode, sourceList.sources);
    if (sourceList.filterMode == MCAST_INCLUDE_SOURCES && sourceList.sources.empty())
        socketState.erase(group);
}

void IGMPTester::processDumpCommand(string what, InterfaceEntry *ie)
{
    EV << "IGMPTester: " << ie->getInterfaceName() << ": " << what << " = ";

    if (what == "groups") {
        for (int i = 0; i < ie->getProtocolData<Ipv4InterfaceData>()->getNumOfJoinedMulticastGroups(); i++) {
            Ipv4Address group = ie->getProtocolData<Ipv4InterfaceData>()->getJoinedMulticastGroup(i);
            const Ipv4MulticastSourceList &sourceList = ie->getProtocolData<Ipv4InterfaceData>()->getJoinedMulticastSources(i);
            EV << (i==0 ? "" : ", ") << group << " " << sourceList.str();
        }
    }
    else if (what == "listeners")
    {
        for (int i = 0; i < ie->getProtocolData<Ipv4InterfaceData>()->getNumOfReportedMulticastGroups(); i++) {
            Ipv4Address group = ie->getProtocolData<Ipv4InterfaceData>()->getReportedMulticastGroup(i);
            const Ipv4MulticastSourceList &sourceList = ie->getProtocolData<Ipv4InterfaceData>()->getReportedMulticastSources(i);
            EV << (i==0 ? "" : ", ") << group << " " << sourceList.str();
        }
    }

    EV << ".\n";
}

void IGMPTester::sendIGMP(Packet *msg, InterfaceEntry *ie, Ipv4Address dest)
{
    ASSERT(ie->isMulticast());

    msg->addTagIfAbsent<InterfaceInd>()->setInterfaceId(ie->getInterfaceId());
    msg->addTagIfAbsent<L3AddressInd>()->setDestAddress(dest);
    msg->addTagIfAbsent<HopLimitInd>()->setHopLimit(1);
    msg->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::igmp);
    msg->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::igmp);
    msg->addTagIfAbsent<DispatchProtocolInd>()->setProtocol(&Protocol::ipv4);

    EV << "IGMPTester: Sending: " << msg << ".\n";
    send(msg, "upperLayerOut");
}

void IGMPTester::parseIPv4AddressVector(const char *str, Ipv4AddressVector &result)
{
    if (str) {
        cStringTokenizer tokens(str);
        while (tokens.hasMoreTokens())
            result.push_back(Ipv4Address(tokens.nextToken()));
    }
    sort(result.begin(), result.end());
}

} // namespace inet

