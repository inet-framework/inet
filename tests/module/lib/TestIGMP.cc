#include <algorithm>
#include <fstream>

#include "inet/common/INETDefs.h"
#include "inet/common/scenario/IScriptable.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Igmpv2.h"

namespace inet {

enum StateKind
{
    HOST_GROUP_STATE = 0x01,
    ROUTER_GROUP_STATE = 0x02,
    ROUTER_IF_STATE = 0x04,
};

class INET_API TestIGMP : public Igmpv2, public IScriptable
{
  private:
     std::ofstream out;
     cModule *node;
  protected:
    typedef Ipv4InterfaceData::Ipv4AddressVector Ipv4AddressVector;
    virtual void initialize(int stage) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual void configureInterface(InterfaceEntry *ie) override;
    virtual void processIgmpMessage(Packet *packet) override;
    virtual void processHostGroupTimer(cMessage *msg) override;
    virtual void processQueryTimer(cMessage *msg) override;
    virtual void processLeaveTimer(cMessage *msg) override;
    virtual void processRexmtTimer(cMessage *msg) override;
    virtual void processCommand(const cXMLElement &node) override;
    virtual void sendToIP(Packet *msg, InterfaceEntry *ie, const Ipv4Address& dest) override;
  private:
    void dumpMulticastGroups(const char* name, const char *ifname, Ipv4AddressVector groups);
    void startEvent(const char *event, int stateMask, InterfaceEntry *ie, const Ipv4Address *group = NULL);
    void endEvent(int stateMask, InterfaceEntry *ie, const Ipv4Address *group = NULL);
    void printStates(int stateMask, InterfaceEntry *ie, const Ipv4Address *group);
};

Define_Module(TestIGMP);

void TestIGMP::initialize(int stage)
{
    if (stage == 0) {
        node = (cModule*)getOwner()->getOwner();
        const char *filename = par("outputFile");
        if (filename && (*filename)) {
            out.open(filename);
            if (out.fail())
                throw cRuntimeError("Failed to open output file: %s", filename);
        }
    }

    Igmpv2::initialize(stage);
}

void TestIGMP::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    const Ipv4MulticastGroupInfo *info;
    if (signalID == ipv4MulticastGroupJoinedSignal) {
        info = check_and_cast<const Ipv4MulticastGroupInfo*>(obj);
        startEvent("join group", HOST_GROUP_STATE, info->ie, &info->groupAddress);
        Igmpv2::receiveSignal(source, signalID, obj, details);
        endEvent(HOST_GROUP_STATE, info->ie, &info->groupAddress);
    }
    else if (signalID == ipv4MulticastGroupLeftSignal) {
        info = check_and_cast<const Ipv4MulticastGroupInfo*>(obj);
        startEvent("leave group", HOST_GROUP_STATE, info->ie, &info->groupAddress);
        Igmpv2::receiveSignal(source, signalID, obj, details);
        endEvent(HOST_GROUP_STATE, info->ie, &info->groupAddress);
    }
    else {
        Igmpv2::receiveSignal(source, signalID, obj, details);
    }
}

void TestIGMP::configureInterface(InterfaceEntry *ie)
{
    startEvent("configure interface", ROUTER_IF_STATE, ie);
    Igmpv2::configureInterface(ie);
    endEvent(ROUTER_IF_STATE, ie);
}


void TestIGMP::processIgmpMessage(Packet *packet)
{
    InterfaceEntry *ie = ift->getInterfaceById(packet->getTag<InterfaceInd>()->getInterfaceId());
    const auto& igmp = packet->peekAtFront<IgmpMessage>();
    Ipv4Address group = Ipv4Address::UNSPECIFIED_ADDRESS;
    switch (igmp->getType()) {
        case IGMP_MEMBERSHIP_QUERY:
            group = packet->peekAtFront<IgmpQuery>()->getGroupAddress();
            break;
        case IGMPV1_MEMBERSHIP_REPORT:
            group = packet->peekAtFront<Igmpv1Report>()->getGroupAddress();
            break;
        case IGMPV2_MEMBERSHIP_REPORT:
            group = packet->peekAtFront<Igmpv2Report>()->getGroupAddress();
            break;
        case IGMPV2_LEAVE_GROUP:
            group = packet->peekAtFront<Igmpv2Leave>()->getGroupAddress();
            break;
        case IGMPV3_MEMBERSHIP_REPORT:
            break;
    }
    int stateMask = 0;
    if (rt->isMulticastForwardingEnabled())
        stateMask |= ROUTER_IF_STATE;
    if (!group.isUnspecified())
        stateMask |= HOST_GROUP_STATE;
    if (!group.isUnspecified() && rt->isMulticastForwardingEnabled())
        stateMask |= ROUTER_GROUP_STATE;
    switch (igmp->getType()) {
        case IGMP_MEMBERSHIP_QUERY:
            startEvent("query received", stateMask, ie, &group);
            Igmpv2::processIgmpMessage(packet);
            endEvent(stateMask, ie, &group);
            break;
        case IGMPV2_MEMBERSHIP_REPORT:
            startEvent("report received", stateMask, ie, &group);
            Igmpv2::processIgmpMessage(packet);
            endEvent(stateMask, ie, &group);
            break;
        case IGMPV2_LEAVE_GROUP:
            startEvent("leave received", stateMask, ie, &group);
            Igmpv2::processIgmpMessage(packet);
            endEvent(stateMask, ie, &group);
            break;
        default:
            Igmpv2::processIgmpMessage(packet);
            break;
    }
}

void TestIGMP::processHostGroupTimer(cMessage *msg)
{
    IgmpHostTimerContext *ctx = (IgmpHostTimerContext*)msg->getContextPointer();
    startEvent("timer expired", HOST_GROUP_STATE, ctx->ie, &ctx->hostGroup->groupAddr);
    Igmpv2::processHostGroupTimer(msg);
    endEvent(HOST_GROUP_STATE, ctx->ie, &ctx->hostGroup->groupAddr);
}

void TestIGMP::processQueryTimer(cMessage *msg)
{
    InterfaceEntry *ie = (InterfaceEntry*)msg->getContextPointer();
    RouterInterfaceData *routerData = getRouterInterfaceData(ie);
    const char *event = routerData && routerData->igmpRouterState == IGMP_RS_QUERIER ? "gen. query timer expired" :
                                                                                       "other querier present timer expired";
    startEvent(event, ROUTER_IF_STATE, ie);
    Igmpv2::processQueryTimer(msg);
    endEvent(ROUTER_IF_STATE, ie);
}

void TestIGMP::processLeaveTimer(cMessage *msg)
{
    IgmpRouterTimerContext *ctx = (IgmpRouterTimerContext*)msg->getContextPointer();
    InterfaceEntry *ie = ctx->ie;
    Ipv4Address group = ctx->routerGroup->groupAddr;
    startEvent("timer expired", ROUTER_GROUP_STATE, ie, &group);
    Igmpv2::processLeaveTimer(msg);
    endEvent(ROUTER_GROUP_STATE, ie, &group);
}

void TestIGMP::processRexmtTimer(cMessage *msg)
{
    IgmpRouterTimerContext *ctx = (IgmpRouterTimerContext*)msg->getContextPointer();
    startEvent("rexmt timer expired", ROUTER_GROUP_STATE, ctx->ie, &ctx->routerGroup->groupAddr);
    Igmpv2::processRexmtTimer(msg);
    endEvent(ROUTER_GROUP_STATE, ctx->ie, &ctx->routerGroup->groupAddr);
}

void TestIGMP::sendToIP(Packet *msg, InterfaceEntry *ie, const Ipv4Address& dest)
{
    if (out.is_open()) {
        const auto& igmp = CHK(msg->peekAtFront<IgmpMessage>());
        switch (igmp->getType()) {
            case IGMP_MEMBERSHIP_QUERY:
                out << "send query"; break;
            case IGMPV1_MEMBERSHIP_REPORT:
            case IGMPV2_MEMBERSHIP_REPORT:
            case IGMPV3_MEMBERSHIP_REPORT:
                out << "send report"; break;
            case IGMPV2_LEAVE_GROUP:
                out << "send leave"; break;
        }
    }
    Igmpv2::sendToIP(msg, ie, dest);
}

void TestIGMP::processCommand(const cXMLElement &node)
{
    Enter_Method_Silent();

    const char *tag = node.getTagName();
    const char *ifname = node.getAttribute("ifname");
    InterfaceEntry *ie = ifname ? CHK(ift->findInterfaceByName(ifname)) : nullptr;

    if (!strcmp(tag, "join")) {
        if (!ie)
            throw cRuntimeError("'ifname' attribute is missing at XML node ", node.str().c_str());
        const char *group = node.getAttribute("group");
        ie->getProtocolData<Ipv4InterfaceData>()->joinMulticastGroup(Ipv4Address(group));
    }
    else if (!strcmp(tag, "leave")) {
        if (!ie)
            throw cRuntimeError("'ifname' attribute is missing at XML node ", node.str().c_str());
        const char *group = node.getAttribute("group");
        ie->getProtocolData<Ipv4InterfaceData>()->leaveMulticastGroup(Ipv4Address(group));
    }
    else if (!strcmp(tag, "dump")) {
        const char *what = node.getAttribute("what");
        if (!strcmp(what, "groups")) {
            if (!ie)
                throw cRuntimeError("'ifname' attribute is missing at XML node ", node.str().c_str());
            Ipv4AddressVector joinedGroups;
            const int count = ie->getProtocolData<Ipv4InterfaceData>()->getNumOfJoinedMulticastGroups();
            for (int i = 0; i < count; ++i)
                joinedGroups.push_back(ie->getProtocolData<Ipv4InterfaceData>()->getJoinedMulticastGroup(i));
            dumpMulticastGroups(what, ifname, joinedGroups);
        }
        else if (!strcmp(what, "listeners")) {
            if (!ie)
                throw cRuntimeError("'ifname' attribute is missing at XML node ", node.str().c_str());
            Ipv4AddressVector reportedGroups;
            const int count = ie->getProtocolData<Ipv4InterfaceData>()->getNumOfReportedMulticastGroups();
            for (int i = 0; i < count; ++i)
                reportedGroups.push_back(ie->getProtocolData<Ipv4InterfaceData>()->getReportedMulticastGroup(i));
            dumpMulticastGroups("listeners", ifname, reportedGroups);
        }
    }
    else if (!strcmp(tag, "disable")) {
        enabled = false;
    }
    else if (!strcmp(tag, "enable")) {
        enabled = true;
    }
    else if (!strcmp(tag, "send")) {
        const char *what = node.getAttribute("what");
        if (!strcmp(what,"query")) {
            const char *groupAttr = node.getAttribute("group");
            Ipv4Address group = groupAttr ? Ipv4Address(groupAttr) : Ipv4Address::UNSPECIFIED_ADDRESS;
            const char *maxRespTimeAttr = node.getAttribute("maxRespTime");
            double maxRespTime = maxRespTimeAttr ? atof(maxRespTimeAttr) : queryResponseInterval;
            sendQuery(ie, group, maxRespTime);
        }
    }
}

void TestIGMP::startEvent(const char * event, int stateMask, InterfaceEntry *ie, const Ipv4Address *group)
{
    if (out.is_open()) {
        out << "t=" << simTime() << " " << node->getFullName() << "/" << ie->getInterfaceName();
        if (group)
            out << "/" << *group;
        out << ":";
        printStates(stateMask, ie, group);
        out << " --> " << event << " <";
    }
}

void TestIGMP::endEvent(int stateMask, InterfaceEntry *ie, const Ipv4Address *group)
{
    if (out.is_open()) {
        out << "> -->";
        printStates(stateMask, ie, group);
        out << "\n";
        out.flush();
    }
}

void TestIGMP::printStates(int stateMask, InterfaceEntry *ie, const Ipv4Address *group)
{
    if (stateMask & ROUTER_IF_STATE) {
        RouterInterfaceData *routerIfData = getRouterInterfaceData(ie);
        if (routerIfData) {
            switch (routerIfData->igmpRouterState) {
                case IGMP_RS_INITIAL: out << " INITIAL"; break;
                case IGMP_RS_QUERIER: out << " QUERIER"; break;
                case IGMP_RS_NON_QUERIER: out << " NON_QUERIER"; break;
                default: out << "???"; break;
            }
        }
        else
            out << " <NONE>";
    }
    if ((stateMask & ROUTER_GROUP_STATE) && group) {
        RouterGroupData *routerGroupData = getRouterGroupData(ie, *group);
        if (routerGroupData) {
            switch (routerGroupData->state) {
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
    if ((stateMask & HOST_GROUP_STATE) && group) {
        HostGroupData *hostGroupData = getHostGroupData(ie, *group);
        if (hostGroupData) {
            switch (hostGroupData->state) {
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

void TestIGMP::dumpMulticastGroups(const char* name, const char *ifname, Ipv4AddressVector groups)
{
    if (!out.is_open())
        return;

    out << "t=" << simTime() << " " << node->getFullName() << "/" << ifname << ": " << name << " = <";

    sort(groups.begin(), groups.end());
    for (Ipv4AddressVector::iterator it = groups.begin(); it != groups.end(); ++it)
        out << (it == groups.begin()?"":",") << *it;
    out << ">\n";
}

} // namespace inet

