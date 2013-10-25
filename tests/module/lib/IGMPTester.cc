#include <algorithm>
#include <fstream>

#include "INETDefs.h"
#include "IScriptable.h"
#include "InterfaceTableAccess.h"
#include "IPv4Address.h"
#include "IPv4InterfaceData.h"
#include "IPv4ControlInfo.h"
#include "IInterfaceTable.h"
#include "IIPv4RoutingTable.h"
#include "IPSocket.h"
#include "IGMPMessage.h"

class INET_API IGMPTester : public cSimpleModule, public IScriptable
{
  private:
    IInterfaceTable *ift;
  protected:
    typedef IPv4InterfaceData::IPv4AddressVector IPv4AddressVector;
    virtual int numInitStages() const { return 2; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void processCommand(const cXMLElement &node);
  private:
    void processSendCommand(const cXMLElement &node);
    void parseIPv4AddressVector(const char *str, IPv4AddressVector &result);
    void sendIGMP(IGMPMessage *msg, InterfaceEntry *ie, IPv4Address dest);
};

Define_Module(IGMPTester);

using namespace std;

static ostream &operator<<(ostream &out, const IPv4AddressVector addresses)
{
    out << "<"; // regexp friendly bracket
    for (int i = 0; i < (int)addresses.size(); i++)
        out << (i>0?",":"") << addresses[i];
    out << ">";
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
            break;
        case IGMPV2_MEMBERSHIP_REPORT:
            break;
        case IGMPV2_LEAVE_GROUP:
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
                out << record.sourceList;
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
        ift = InterfaceTableAccess().get();

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
    if (msg->getKind() == IP_C_REGISTER_PROTOCOL)
    {
        delete msg;
        return;
    }

    IGMPMessage *igmpMsg = check_and_cast<IGMPMessage*>(msg);
    EV_DEBUG << "Received: " << igmpMsg << ".\n";
    delete msg;
}

void IGMPTester::processCommand(const cXMLElement &node)
{
    Enter_Method_Silent();

    const char *tag = node.getTagName();
    const char *ifname = node.getAttribute("ifname");
    InterfaceEntry *ie = ifname ? ift->getInterfaceByName(ifname) : NULL;

    if (!strcmp(tag, "join"))
    {
        const char *group = node.getAttribute("group");
        ie->ipv4Data()->joinMulticastGroup(IPv4Address(group));
    }
    else if (!strcmp(tag, "leave"))
    {
        const char *group = node.getAttribute("group");
        ie->ipv4Data()->leaveMulticastGroup(IPv4Address(group));
    }
    else if (!strcmp(tag, "dump"))
    {
        const char *what = node.getAttribute("what");
        if (!strcmp(what, "groups"))
        {
            EV_DEBUG << ifname << ": groups = ";
            for (int i = 0; i < ie->ipv4Data()->getNumOfJoinedMulticastGroups(); i++)
            {
                IPv4Address group = ie->ipv4Data()->getJoinedMulticastGroup(i);
                McastSourceFilterMode filterMode = ie->ipv4Data()->getJoinedGroupFilterMode(i);
                const IPv4AddressVector &sourceList = ie->ipv4Data()->getJoinedGroupSourceList(i);
                EV_DEBUG << (i==0?"":", ") << group << " " << (filterMode == MCAST_INCLUDE_SOURCES?"I":"E")
                         << sourceList;
            }
            EV_DEBUG << "\n";
        }
        else if (!strcmp(what, "listeners"))
        {
            IPv4AddressVector groups(ie->ipv4Data()->getReportedMulticastGroups());
            sort(groups.begin(), groups.end());
            EV_DEBUG << ifname << ": listeners = " << groups << "\n";
        }
    }
    else if (!strcmp(tag, "send"))
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

    }
    else if (type == "IGMPv2Query")
    {

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

    }
    else if (type == "IGMPv2Leave")
    {

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

void IGMPTester::sendIGMP(IGMPMessage *msg, InterfaceEntry *ie, IPv4Address dest)
{
    ASSERT(ie->isMulticast());

    IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
    controlInfo->setProtocol(IP_PROT_IGMP);
    controlInfo->setInterfaceId(ie->getInterfaceId());
    controlInfo->setTimeToLive(1);
    controlInfo->setDestAddr(dest);
    msg->setControlInfo(controlInfo);

    EV_DEBUG << "Sending: " << msg << ".\n";
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
}

