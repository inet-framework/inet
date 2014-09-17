#include <algorithm>
#include <fstream>

#include "inet/common/INETDefs.h"
#include "inet/common/scenario/IScriptable.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/ipv4/IGMPv2.h"

namespace inet {

enum StateKind
{
    HOST_GROUP_STATE = 0x01,
    ROUTER_GROUP_STATE = 0x02,
    ROUTER_IF_STATE = 0x04,
};

class INET_API TestIGMP : public IGMPv2, public IScriptable
{
  private:
     std::ofstream out;
     cModule *node;
//     std::string currentEvent;
//     std::vector<std::string> currentActions;
//     IGMPHostGroupState st;
//     IGMPRouterGroupState st2;
//     IGMPRouterState currentRouterGroupState;
  protected:
    typedef IPv4InterfaceData::IPv4AddressVector IPv4AddressVector;
    virtual void initialize(int stage);
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
    virtual void configureInterface(InterfaceEntry *ie);
    virtual void processIgmpMessage(IGMPMessage *msg);
    virtual void processHostGroupTimer(cMessage *msg);
    virtual void processQueryTimer(cMessage *msg);
    virtual void processLeaveTimer(cMessage *msg);
    virtual void processRexmtTimer(cMessage *msg);
    virtual void processCommand(const cXMLElement &node);
    virtual void sendToIP(IGMPMessage *msg, InterfaceEntry *ie, const IPv4Address& dest);
  private:
    void dumpMulticastGroups(const char* name, const char *ifname, IPv4AddressVector groups);
    void startEvent(const char *event, int stateMask, InterfaceEntry *ie, const IPv4Address *group = NULL);
    void endEvent(int stateMask, InterfaceEntry *ie, const IPv4Address *group = NULL);
    void printStates(int stateMask, InterfaceEntry *ie, const IPv4Address *group);
};

Define_Module(TestIGMP);

void TestIGMP::initialize(int stage)
{
    if (stage == 0)
    {
        node = (cModule*)getOwner()->getOwner();
        const char *filename = par("outputFile");
        if (filename && (*filename))
        {
            out.open(filename);
            if (out.fail())
                throw cRuntimeError("Failed to open output file: %s", filename);
        }
    }

    IGMPv2::initialize(stage);
}

void TestIGMP::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    const IPv4MulticastGroupInfo *info;
    if (signalID == NF_IPv4_MCAST_JOIN)
    {
        info = check_and_cast<const IPv4MulticastGroupInfo*>(obj);
        startEvent("join group", HOST_GROUP_STATE, info->ie, &info->groupAddress);
        IGMPv2::receiveSignal(source, signalID, obj);
        endEvent(HOST_GROUP_STATE, info->ie, &info->groupAddress);
    }
    else if (signalID == NF_IPv4_MCAST_LEAVE)
    {
        info = check_and_cast<const IPv4MulticastGroupInfo*>(obj);
        startEvent("leave group", HOST_GROUP_STATE, info->ie, &info->groupAddress);
        IGMPv2::receiveSignal(source, signalID, obj);
        endEvent(HOST_GROUP_STATE, info->ie, &info->groupAddress);
    }
    else
    {
        IGMPv2::receiveSignal(source, signalID, obj);
    }
}

void TestIGMP::configureInterface(InterfaceEntry *ie)
{
    startEvent("configure interface", ROUTER_IF_STATE, ie);
    IGMPv2::configureInterface(ie);
    endEvent(ROUTER_IF_STATE, ie);
}


void TestIGMP::processIgmpMessage(IGMPMessage *msg)
{
    IPv4ControlInfo *controlInfo = (IPv4ControlInfo *)msg->getControlInfo();
    InterfaceEntry *ie = ift->getInterfaceById(controlInfo->getInterfaceId());
    IPv4Address group = msg->getGroupAddress();
    int stateMask = 0;
    if (rt->isMulticastForwardingEnabled())
        stateMask |= ROUTER_IF_STATE;
    if (!group.isUnspecified())
        stateMask |= HOST_GROUP_STATE;
    if (!group.isUnspecified() && rt->isMulticastForwardingEnabled())
        stateMask |= ROUTER_GROUP_STATE;
    switch (msg->getType())
    {
        case IGMP_MEMBERSHIP_QUERY:
            startEvent("query received", stateMask, ie, &group);
            IGMPv2::processIgmpMessage(msg);
            endEvent(stateMask, ie, &group);
            break;
        case IGMPV2_MEMBERSHIP_REPORT:
            startEvent("report received", stateMask, ie, &group);
            IGMPv2::processIgmpMessage(msg);
            endEvent(stateMask, ie, &group);
            break;
        case IGMPV2_LEAVE_GROUP:
            startEvent("leave received", stateMask, ie, &group);
            IGMPv2::processIgmpMessage(msg);
            endEvent(stateMask, ie, &group);
            break;
        default:
            IGMPv2::processIgmpMessage(msg);
            break;
    }
}

void TestIGMP::processHostGroupTimer(cMessage *msg)
{
    IGMPHostTimerContext *ctx = (IGMPHostTimerContext*)msg->getContextPointer();
    startEvent("timer expired", HOST_GROUP_STATE, ctx->ie, &ctx->hostGroup->groupAddr);
    IGMPv2::processHostGroupTimer(msg);
    endEvent(HOST_GROUP_STATE, ctx->ie, &ctx->hostGroup->groupAddr);
}

void TestIGMP::processQueryTimer(cMessage *msg)
{
    InterfaceEntry *ie = (InterfaceEntry*)msg->getContextPointer();
    RouterInterfaceData *routerData = getRouterInterfaceData(ie);
    const char *event = routerData && routerData->igmpRouterState == IGMP_RS_QUERIER ? "gen. query timer expired" :
                                                                                       "other querier present timer expired";
    startEvent(event, ROUTER_IF_STATE, ie);
    IGMPv2::processQueryTimer(msg);
    endEvent(ROUTER_IF_STATE, ie);
}

void TestIGMP::processLeaveTimer(cMessage *msg)
{
    IGMPRouterTimerContext *ctx = (IGMPRouterTimerContext*)msg->getContextPointer();
    InterfaceEntry *ie = ctx->ie;
    IPv4Address group = ctx->routerGroup->groupAddr;
    startEvent("timer expired", ROUTER_GROUP_STATE, ie, &group);
    IGMPv2::processLeaveTimer(msg);
    endEvent(ROUTER_GROUP_STATE, ie, &group);
}

void TestIGMP::processRexmtTimer(cMessage *msg)
{
    IGMPRouterTimerContext *ctx = (IGMPRouterTimerContext*)msg->getContextPointer();
    startEvent("rexmt timer expired", ROUTER_GROUP_STATE, ctx->ie, &ctx->routerGroup->groupAddr);
    IGMPv2::processRexmtTimer(msg);
    endEvent(ROUTER_GROUP_STATE, ctx->ie, &ctx->routerGroup->groupAddr);
}

void TestIGMP::sendToIP(IGMPMessage *msg, InterfaceEntry *ie, const IPv4Address& dest)
{
    if (out.is_open())
    {
        switch (msg->getType())
        {
            case IGMP_MEMBERSHIP_QUERY:
                out << "send query"; break;
            case IGMPV1_MEMBERSHIP_REPORT:
            case IGMPV2_MEMBERSHIP_REPORT:
                out << "send report"; break;
            case IGMPV2_LEAVE_GROUP:
                out << "send leave"; break;
        }
    }
    IGMPv2::sendToIP(msg, ie, dest);
}

void TestIGMP::processCommand(const cXMLElement &node)
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
        if (!ie)
            throw cRuntimeError("'ifname' attribute is missing at XML node ", node.detailedInfo().c_str());
        IPv4AddressVector joinedGroups;
        const int count = ie->ipv4Data()->getNumOfJoinedMulticastGroups();
        for (int i = 0; i < count; ++i)
            joinedGroups.push_back(ie->ipv4Data()->getJoinedMulticastGroup(i));
        dumpMulticastGroups(what, ifname, joinedGroups);
    }
    else if (!strcmp(what, "listeners"))
    {
        if (!ie)
            throw cRuntimeError("'ifname' attribute is missing at XML node ", node.detailedInfo().c_str());
        IPv4AddressVector reportedGroups;
        const int count = ie->ipv4Data()->getNumOfReportedMulticastGroups();
        for (int i = 0; i < count; ++i)
            reportedGroups.push_back(ie->ipv4Data()->getReportedMulticastGroup(i));
        dumpMulticastGroups("listeners", ifname, reportedGroups);
    }
  }
  else if (!strcmp(tag, "disable"))
  {
      enabled = false;
  }
  else if (!strcmp(tag, "enable"))
  {
      enabled = true;
  }
  else if (!strcmp(tag, "send"))
  {
      const char *what = node.getAttribute("what");
      if (!strcmp(what,"query"))
      {
          const char *groupAttr = node.getAttribute("group");
          IPv4Address group = groupAttr ? IPv4Address(groupAttr) : IPv4Address::UNSPECIFIED_ADDRESS;
          const char *maxRespTimeAttr = node.getAttribute("maxRespTime");
          double maxRespTime = maxRespTimeAttr ? atof(maxRespTimeAttr) : queryResponseInterval;
          sendQuery(ie, group, maxRespTime);
      }
  }
}

void TestIGMP::startEvent(const char * event, int stateMask, InterfaceEntry *ie, const IPv4Address *group)
{
    if (out.is_open())
    {
        out << "t=" << simTime() << " " << node->getFullName() << "/" << ie->getName();
        if (group)
            out << "/" << *group;
        out << ":";
        printStates(stateMask, ie, group);
        out << " --> " << event << " <";
    }
}

void TestIGMP::endEvent(int stateMask, InterfaceEntry *ie, const IPv4Address *group)
{
    if (out.is_open())
    {
        out << "> -->";
        printStates(stateMask, ie, group);
        out << "\n";
        out.flush();
    }
}

void TestIGMP::printStates(int stateMask, InterfaceEntry *ie, const IPv4Address *group)
{
    if (stateMask & ROUTER_IF_STATE)
    {
        RouterInterfaceData *routerIfData = getRouterInterfaceData(ie);
        if (routerIfData)
        {
            switch (routerIfData->igmpRouterState)
            {
                case IGMP_RS_INITIAL: out << " INITIAL"; break;
                case IGMP_RS_QUERIER: out << " QUERIER"; break;
                case IGMP_RS_NON_QUERIER: out << " NON_QUERIER"; break;
                default: out << "???"; break;
            }
        }
        else
            out << " <NONE>";
    }
    if ((stateMask & ROUTER_GROUP_STATE) && group)
    {
        RouterGroupData *routerGroupData = getRouterGroupData(ie, *group);
        if (routerGroupData)
        {
            switch (routerGroupData->state)
            {
                case IGMP_RGS_NO_MEMBERS_PRESENT: out << " NO_MEMBERS_PRESENT"; break;
                case IGMP_RGS_MEMBERS_PRESENT:    out << " MEMBERS_PRESENT"; break;
                case IGMP_RGS_V1_MEMBERS_PRESENT: out << " V1_MEMBERS_PRESENT"; break;
                case IGMP_RGS_CHECKING_MEMBERSHIP: out << " CHECKING_MEMBERSHIP"; break;
                default: out << "???"; break;
            }
        }
        else
            out << " NO_MEMBERS_PRESENT";
    }
    if ((stateMask & HOST_GROUP_STATE) && group)
    {
        HostGroupData *hostGroupData = getHostGroupData(ie, *group);
        if (hostGroupData)
        {
            switch (hostGroupData->state)
            {
                case IGMP_HGS_NON_MEMBER:      out << " NON_MEMBER"; break;
                case IGMP_HGS_DELAYING_MEMBER: out << " DELAYING_MEMBER"; break;
                case IGMP_HGS_IDLE_MEMBER:     out << " IDLE_MEMBER"; break;
                default: out << "???"; break;
            }
        }
        else
            out << " NON_MEMBER";
    }
}


void TestIGMP::dumpMulticastGroups(const char* name, const char *ifname, IPv4AddressVector groups)
{
  if (!out.is_open())
      return;

  out << "t=" << simTime() << " " << node->getFullName() << "/" << ifname << ": " << name << " = <";

  sort(groups.begin(), groups.end());
  for (IPv4AddressVector::iterator it = groups.begin(); it != groups.end(); ++it)
      out << (it == groups.begin()?"":",") << *it;
  out << ">\n";
}

} // namespace inet
