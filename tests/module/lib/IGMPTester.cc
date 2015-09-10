#include <algorithm>
#include <fstream>

#include "inet/common/INETDefs.h"
#include "inet/common/scenario/IScriptable.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/common/IPSocket.h"
#include "inet/networklayer/ipv4/IGMPMessage.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/contract/NetworkProtocolCommand_m.h"

using namespace std;

namespace inet {

class INET_API IGMPTester : public cSimpleModule, public IScriptable
{
  private:
    IInterfaceTable *ift;
    map<IPv4Address, IPv4MulticastSourceList> socketState;

  protected:
    typedef IPv4InterfaceData::IPv4AddressVector IPv4AddressVector;
    virtual int numInitStages() const { return 2; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void processCommand(const cXMLElement &node);
  private:
    void processSendCommand(const cXMLElement &node);
    void processJoinCommand(IPv4Address group, const IPv4AddressVector &sources, InterfaceEntry* ie);
    void processLeaveCommand(IPv4Address group, const IPv4AddressVector &sources, InterfaceEntry* ie);
    void processBlockCommand(IPv4Address group, const IPv4AddressVector &sources, InterfaceEntry *ie);
    void processAllowCommand(IPv4Address group, const IPv4AddressVector &sources, InterfaceEntry *ie);
    void processSetFilterCommand(IPv4Address group, McastSourceFilterMode filterMode, const IPv4AddressVector &sources, InterfaceEntry *ie);
    void processDumpCommand(string what, InterfaceEntry *ie);
    void parseIPv4AddressVector(const char *str, IPv4AddressVector &result);
    void sendIGMP(IGMPMessage *msg, InterfaceEntry *ie, IPv4Address dest);
};

Define_Module(IGMPTester);

static ostream &operator<<(ostream &out, const IPv4AddressVector addresses)
{
    for (int i = 0; i < (int)addresses.size(); i++)
        out << (i>0?" ":"") << addresses[i];
    return out;
}

static ostream &operator<<(ostream &out, IGMPMessage* msg)
{
    out << msg->getClassName() << "<";

    switch (msg->getType())
    {
        case IGMP_MEMBERSHIP_QUERY:
        {
            IGMPQuery *query = check_and_cast<IGMPQuery*>(msg);
            out << "group=" << query->getGroupAddress();
            if (dynamic_cast<IGMPv3Query*>(msg))
            {
                IGMPv3Query *v3Query = dynamic_cast<IGMPv3Query*>(msg);
                out << ", sourceList=" << v3Query->getSourceList()
                    << ", maxRespCode=" << (int)v3Query->getMaxRespCode()
                    << ", suppressRouterProc=" << (int)v3Query->getSuppressRouterProc()
                    << ", robustnessVariable=" << (int)v3Query->getRobustnessVariable()
                    << ", queryIntervalCode=" << (int)v3Query->getQueryIntervalCode();
            }
            else if (dynamic_cast<IGMPv2Query*>(msg))
                out << ", maxRespTime=" << (int)dynamic_cast<IGMPv2Query*>(msg)->getMaxRespTime();
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
        case IGMPV3_MEMBERSHIP_REPORT:
        {
            IGMPv3Report *report = check_and_cast<IGMPv3Report*>(msg);
            for (unsigned int i = 0; i < report->getGroupRecordArraySize(); i++)
            {
                GroupRecord &record = report->getGroupRecord(i);
                out << (i>0?", ":"") << record.groupAddress << "=";
                switch (record.recordType)
                {
                    case MODE_IS_INCLUDE:        out << "IS_IN" ; break;
                    case MODE_IS_EXCLUDE:        out << "IS_EX" ; break;
                    case CHANGE_TO_INCLUDE_MODE: out << "TO_IN" ; break;
                    case CHANGE_TO_EXCLUDE_MODE: out << "TO_EX" ; break;
                    case ALLOW_NEW_SOURCES:      out << "ALLOW" ; break;
                    case BLOCK_OLD_SOURCE:       out << "BLOCK" ; break;
                }
                if (!record.sourceList.empty())
                    out << " " << record.sourceList;
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
    if (stage == 0)
    {
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);

        InterfaceEntry *interfaceEntry = new InterfaceEntry(this);
        interfaceEntry->setName("eth0");
        MACAddress address("AA:00:00:00:00:01");
        interfaceEntry->setMACAddress(address);
        interfaceEntry->setInterfaceToken(address.formInterfaceIdentifier());
        interfaceEntry->setMtu(par("mtu").longValue());
        interfaceEntry->setMulticast(true);
        interfaceEntry->setBroadcast(true);

        ift->addInterface(interfaceEntry);
    }
    else if (stage == 2)
    {
        InterfaceEntry *ie = ift->getInterface(0);
        ie->ipv4Data()->setIPAddress(IPv4Address("192.168.1.1"));
        ie->ipv4Data()->setNetmask(IPv4Address("255.255.0.0"));
    }
}

void IGMPTester::handleMessage(cMessage *msg)
{
    if (dynamic_cast<RegisterTransportProtocolCommand*>(msg))
    {
        delete msg;
        return;
    }

    IGMPMessage *igmpMsg = check_and_cast<IGMPMessage*>(msg);
    EV << "IGMPTester: Received: " << igmpMsg << ".\n";
    delete msg;
}

void IGMPTester::processCommand(const cXMLElement &node)
{
    Enter_Method_Silent();

    string tag = node.getTagName();
    const char *ifname = node.getAttribute("ifname");
    InterfaceEntry *ie = ifname ? ift->getInterfaceByName(ifname) : NULL;

    if (tag == "join")
    {
        const char *group = node.getAttribute("group");
        IPv4AddressVector sources;
        parseIPv4AddressVector(node.getAttribute("sources"), sources);
        processJoinCommand(IPv4Address(group), sources, ie);
    }
    else if (tag == "leave")
    {
        const char *group = node.getAttribute("group");
        IPv4AddressVector sources;
        parseIPv4AddressVector(node.getAttribute("sources"), sources);
        processLeaveCommand(IPv4Address(group), sources, ie);
    }
    else if (tag == "block")
    {
        const char *group = node.getAttribute("group");
        IPv4AddressVector sources;
        parseIPv4AddressVector(node.getAttribute("sources"), sources);
        processBlockCommand(IPv4Address(group), sources, ie);
    }
    else if (tag == "allow")
    {
        const char *group = node.getAttribute("group");
        IPv4AddressVector sources;
        parseIPv4AddressVector(node.getAttribute("sources"), sources);
        processAllowCommand(IPv4Address(group), sources, ie);
    }
    else if (tag == "set-filter")
    {
        const char *groupAttr = node.getAttribute("group");
        const char *sourcesAttr = node.getAttribute("sources");
        ASSERT((sourcesAttr[0] == 'I' || sourcesAttr[0] == 'E') && (sourcesAttr[1] == ' ' || sourcesAttr[1] == '\0'));
        McastSourceFilterMode filterMode = sourcesAttr[0] == 'I' ? MCAST_INCLUDE_SOURCES : MCAST_EXCLUDE_SOURCES;
        IPv4AddressVector sources;
        if (sourcesAttr[1])
            parseIPv4AddressVector(sourcesAttr+2, sources);
        processSetFilterCommand(IPv4Address(groupAttr), filterMode, sources, ie);
    }
    else if (tag == "dump")
    {
        const char *what = node.getAttribute("what");
        processDumpCommand(what, ie);
    }
    else if (tag == "send")
    {
        processSendCommand(node);
    }
}

void IGMPTester::processSendCommand(const cXMLElement &node)
{
    const char *ifname = node.getAttribute("ifname");
    InterfaceEntry *ie = ifname ? ift->getInterfaceByName(ifname) : ift->getInterface(0);
    string type = node.getAttribute("type");

    if (type == "IGMPv1Query")
    {
        // TODO
    }
    else if (type == "IGMPv2Query")
    {
        // TODO
    }
    else if (type == "IGMPv3Query")
    {
        const char *groupStr = node.getAttribute("group");
        const char *maxRespCodeStr = node.getAttribute("maxRespCode");
        const char *sourcesStr = node.getAttribute("source");

        IPv4Address group = groupStr ? IPv4Address(groupStr) : IPv4Address::UNSPECIFIED_ADDRESS;
        int maxRespCode = maxRespCodeStr ? atoi(maxRespCodeStr) : 100 /*10 sec*/;
        IPv4AddressVector sources;
        parseIPv4AddressVector(sourcesStr, sources);

        IGMPv3Query *msg = new IGMPv3Query("IGMPv3 query");
        msg->setType(IGMP_MEMBERSHIP_QUERY);
        msg->setGroupAddress(group);
        msg->setMaxRespCode(maxRespCode);
        msg->setSourceList(sources);
        msg->setByteLength(12 + (4 * sources.size()));
        sendIGMP(msg, ie, group.isUnspecified() ? IPv4Address::ALL_HOSTS_MCAST : group);

    }
    else if (type == "IGMPv2Report")
    {
        // TODO
    }
    else if (type == "IGMPv2Leave")
    {
        // TODO
    }
    else if (type == "IGMPv3Report")
    {
        cXMLElementList records = node.getElementsByTagName("record");
        IGMPv3Report *msg = new IGMPv3Report("IGMPv3 report");

        msg->setGroupRecordArraySize(records.size());
        for (int i = 0; i < (int)records.size(); ++i)
        {
            cXMLElement *recordNode = records[i];
            const char *groupStr = recordNode->getAttribute("group");
            string recordTypeStr = recordNode->getAttribute("type");
            const char *sourcesStr = recordNode->getAttribute("sources");
            ASSERT(groupStr);

            GroupRecord &record = msg->getGroupRecord(i);
            record.groupAddress = IPv4Address(groupStr);
            parseIPv4AddressVector(sourcesStr, record.sourceList);
            record.recordType = recordTypeStr == "IS_IN" ? MODE_IS_INCLUDE :
                                recordTypeStr == "IS_EX" ? MODE_IS_EXCLUDE :
                                recordTypeStr == "TO_IN" ? CHANGE_TO_INCLUDE_MODE :
                                recordTypeStr == "TO_EX" ? CHANGE_TO_EXCLUDE_MODE :
                                recordTypeStr == "ALLOW" ? ALLOW_NEW_SOURCES :
                                recordTypeStr == "BLOCK" ? BLOCK_OLD_SOURCE : 0;
            ASSERT(record.groupAddress.isMulticast());
            ASSERT(record.recordType);
        }

        sendIGMP(msg, ie, IPv4Address::ALL_IGMPV3_ROUTERS_MCAST);
    }
}

void IGMPTester::processJoinCommand(IPv4Address group, const IPv4AddressVector &sources, InterfaceEntry *ie)
{
    if (sources.empty())
    {
        ie->ipv4Data()->joinMulticastGroup(group);
        socketState[group] = IPv4MulticastSourceList::ALL_SOURCES;
    }
    else
    {
        IPv4MulticastSourceList &sourceList = socketState[group];
        ASSERT(sourceList.filterMode == MCAST_INCLUDE_SOURCES);
        IPv4AddressVector oldSources(sourceList.sources);
        for (IPv4AddressVector::const_iterator source = sources.begin(); source != sources.end(); ++source)
            sourceList.add(*source);
        if (oldSources != sourceList.sources)
            ie->ipv4Data()->changeMulticastGroupMembership(group, MCAST_INCLUDE_SOURCES, oldSources, MCAST_INCLUDE_SOURCES, sourceList.sources);
    }
}

void IGMPTester::processLeaveCommand(IPv4Address group, const IPv4AddressVector &sources, InterfaceEntry *ie)
{
    if (sources.empty())
    {
        ie->ipv4Data()->leaveMulticastGroup(group);
        socketState.erase(group);
    }
    else
    {
        IPv4MulticastSourceList &sourceList = socketState[group];
        ASSERT(sourceList.filterMode == MCAST_INCLUDE_SOURCES);
        IPv4AddressVector oldSources(sourceList.sources);
        for (IPv4AddressVector::const_iterator source = sources.begin(); source != sources.end(); ++source)
            sourceList.remove(*source);
        if (oldSources != sourceList.sources)
            ie->ipv4Data()->changeMulticastGroupMembership(group, MCAST_INCLUDE_SOURCES, oldSources, MCAST_INCLUDE_SOURCES, sourceList.sources);
        if (sourceList.sources.empty())
            socketState.erase(group);
    }
}

void IGMPTester::processBlockCommand(IPv4Address group, const IPv4AddressVector &sources, InterfaceEntry *ie)
{
    map<IPv4Address, IPv4MulticastSourceList>::iterator it = socketState.find(group);
    ASSERT(it != socketState.end());
    ASSERT(it->second.filterMode == MCAST_EXCLUDE_SOURCES);
    IPv4AddressVector oldSources(it->second.sources);
    for (IPv4AddressVector::const_iterator source = sources.begin(); source != sources.end(); ++source)
        it->second.add(*source);
    if (oldSources != it->second.sources)
        ie->ipv4Data()->changeMulticastGroupMembership(group, MCAST_EXCLUDE_SOURCES, oldSources, MCAST_EXCLUDE_SOURCES, it->second.sources);
}

void IGMPTester::processAllowCommand(IPv4Address group, const IPv4AddressVector &sources, InterfaceEntry *ie)
{
    map<IPv4Address, IPv4MulticastSourceList>::iterator it = socketState.find(group);
    ASSERT(it != socketState.end());
    ASSERT(it->second.filterMode == MCAST_EXCLUDE_SOURCES);
    IPv4AddressVector oldSources(it->second.sources);
    for (IPv4AddressVector::const_iterator source = sources.begin(); source != sources.end(); ++source)
        it->second.remove(*source);
    if (oldSources != it->second.sources)
        ie->ipv4Data()->changeMulticastGroupMembership(group, MCAST_EXCLUDE_SOURCES, oldSources, MCAST_EXCLUDE_SOURCES, it->second.sources);
}

void IGMPTester::processSetFilterCommand(IPv4Address group, McastSourceFilterMode filterMode, const IPv4AddressVector &sources, InterfaceEntry *ie)
{
    IPv4MulticastSourceList &sourceList = socketState[group];
    McastSourceFilterMode oldFilterMode = sourceList.filterMode;
    IPv4AddressVector oldSources(sourceList.sources);

    sourceList.filterMode = filterMode;
    sourceList.sources = sources;

    if (filterMode != oldFilterMode || oldSources != sourceList.sources)
        ie->ipv4Data()->changeMulticastGroupMembership(group, oldFilterMode, oldSources, sourceList.filterMode, sourceList.sources);
    if (sourceList.filterMode == MCAST_INCLUDE_SOURCES && sourceList.sources.empty())
        socketState.erase(group);
}

void IGMPTester::processDumpCommand(string what, InterfaceEntry *ie)
{
    EV << "IGMPTester: " << ie->getName() << ": " << what << " = ";

    if (what == "groups")
    {
        for (int i = 0; i < ie->ipv4Data()->getNumOfJoinedMulticastGroups(); i++)
        {
            IPv4Address group = ie->ipv4Data()->getJoinedMulticastGroup(i);
            const IPv4MulticastSourceList &sourceList = ie->ipv4Data()->getJoinedMulticastSources(i);
            EV << (i==0?"":", ") << group << " " << sourceList.info();
        }
    }
    else if (what == "listeners")
    {
        for (int i = 0; i < ie->ipv4Data()->getNumOfReportedMulticastGroups(); i++)
        {
            IPv4Address group = ie->ipv4Data()->getReportedMulticastGroup(i);
            const IPv4MulticastSourceList &sourceList = ie->ipv4Data()->getReportedMulticastSources(i);
            EV << (i==0?"":", ") << group << " " << sourceList.info();
        }
    }

    EV << ".\n";
}

void IGMPTester::sendIGMP(IGMPMessage *msg, InterfaceEntry *ie, IPv4Address dest)
{
    ASSERT(ie->isMulticast());

    IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
    controlInfo->setProtocol(IP_PROT_IGMP);
    controlInfo->setInterfaceId(ie->getInterfaceId());
    controlInfo->setTimeToLive(1);
    controlInfo->setDestAddr(dest);
    msg->setControlInfo(controlInfo);

    EV << "IGMPTester: Sending: " << msg << ".\n";
    send(msg, "igmpOut");
}

void IGMPTester::parseIPv4AddressVector(const char *str, IPv4AddressVector &result)
{
    if (str)
    {
        cStringTokenizer tokens(str);
        while (tokens.hasMoreTokens())
            result.push_back(IPv4Address(tokens.nextToken()));
    }
    sort(result.begin(), result.end());
}

} // namespace inet

