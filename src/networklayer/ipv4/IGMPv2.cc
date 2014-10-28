//
// Copyright (C) 2011 CoCo Communications
// Copyright (C) 2012 Opensim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "IGMPv2.h"
#include "IPSocket.h"
#include "RoutingTableAccess.h"
#include "InterfaceTableAccess.h"
#include "IPv4ControlInfo.h"
#include "IPv4InterfaceData.h"
#include "NotificationBoard.h"

#include <algorithm>

Define_Module(IGMPv2);

// RFC 2236, Section 6: Host State Diagram
//                           ________________
//                          |                |
//                          |                |
//                          |                |
//                          |                |
//                --------->|   Non-Member   |<---------
//               |          |                |          |
//               |          |                |          |
//               |          |                |          |
//               |          |________________|          |
//               |                   |                  |
//               | leave group       | join group       | leave group
//               | (stop timer,      |(send report,     | (send leave
//               |  send leave if    | set flag,        |  if flag set)
//               |  flag set)        | start timer)     |
//       ________|________           |          ________|________
//      |                 |<---------          |                 |
//      |                 |                    |                 |
//      |                 |<-------------------|                 |
//      |                 |   query received   |                 |
//      | Delaying Member |    (start timer)   |   Idle Member   |
// ---->|                 |------------------->|                 |
//|     |                 |   report received  |                 |
//|     |                 |    (stop timer,    |                 |
//|     |                 |     clear flag)    |                 |
//|     |_________________|------------------->|_________________|
//| query received    |        timer expired
//| (reset timer if   |        (send report,
//|  Max Resp Time    |         set flag)
//|  < current timer) |
// -------------------
//
// RFC 2236, Section 7: Router State Diagram
//                                     --------------------------------
//                             _______|________  gen. query timer      |
// ---------                  |                |        expired        |
//| Initial |---------------->|                | (send general query,  |
// ---------  (send gen. q.,  |                |  set gen. q. timer)   |
//       set initial gen. q.  |                |<----------------------
//             timer)         |    Querier     |
//                            |                |
//                       -----|                |<---
//                      |     |                |    |
//                      |     |________________|    |
//query received from a |                           | other querier
//router with a lower   |                           | present timer
//IP address            |                           | expired
//(set other querier    |      ________________     | (send general
// present timer)       |     |                |    |  query,set gen.
//                      |     |                |    |  q. timer)
//                      |     |                |    |
//                       ---->|      Non       |----
//                            |    Querier     |
//                            |                |
//                            |                |
//                       ---->|                |----
//                      |     |________________|    |
//                      | query received from a     |
//                      | router with a lower IP    |
//                      | address                   |
//                      | (set other querier        |
//                      |  present timer)           |
//                       ---------------------------
//
//                              ________________
// ----------------------------|                |<-----------------------
//|                            |                |timer expired           |
//|               timer expired|                |(notify routing -,      |
//|          (notify routing -)|   No Members   |clear rxmt tmr)         |
//|                    ------->|    Present     |<-------                |
//|                   |        |                |       |                |
//|v1 report rec'd    |        |                |       |  ------------  |
//|(notify routing +, |        |________________|       | | rexmt timer| |
//| start timer,      |                    |            | |  expired   | |
//| start v1 host     |  v2 report received|            | | (send g-s  | |
//|  timer)           |  (notify routing +,|            | |  query,    | |
//|                   |        start timer)|            | |  st rxmt   | |
//|         __________|______              |       _____|_|______  tmr)| |
//|        |                 |<------------       |              |     | |
//|        |                 |                    |              |<----- |
//|        |                 | v2 report received |              |       |
//|        |                 | (start timer)      |              |       |
//|        | Members Present |<-------------------|    Checking  |       |
//|  ----->|                 | leave received     |   Membership |       |
//| |      |                 | (start timer*,     |              |       |
//| |      |                 |  start rexmt timer,|              |       |
//| |      |                 |  send g-s query)   |              |       |
//| |  --->|                 |------------------->|              |       |
//| | |    |_________________|                    |______________|       |
//| | |v2 report rec'd |  |                          |                   |
//| | |(start timer)   |  |v1 report rec'd           |v1 report rec'd    |
//| |  ----------------   |(start timer,             |(start timer,      |
//| |v1 host              | start v1 host timer)     | start v1 host     |
//| |tmr    ______________V__                        | timer)            |
//| |exp'd |                 |<----------------------                    |
//|  ------|                 |                                           |
//|        |    Version 1    |timer expired                              |
//|        | Members Present |(notify routing -)                         |
// ------->|                 |-------------------------------------------
//         |                 |<--------------------
// ------->|_________________| v1 report rec'd     |
//| v2 report rec'd |   |   (start timer,          |
//| (start timer)   |   |    start v1 host timer)  |
// -----------------     --------------------------

IGMPv2::HostGroupData::HostGroupData(IGMPv2 *owner, const IPv4Address &group)
    : owner(owner), groupAddr(group)
{
    ASSERT(owner);
    ASSERT(groupAddr.isMulticast());

    state = IGMP_HGS_NON_MEMBER;
    flag = false;
    timer = NULL;
}

IGMPv2::HostGroupData::~HostGroupData()
{
    if (timer)
    {
        delete (IGMPHostTimerContext*)timer->getContextPointer();
        owner->cancelAndDelete(timer);
    }
}

IGMPv2::RouterGroupData::RouterGroupData(IGMPv2 *owner, const IPv4Address &group)
    : owner(owner), groupAddr(group)
{
    ASSERT(owner);
    ASSERT(groupAddr.isMulticast());

    state = IGMP_RGS_NO_MEMBERS_PRESENT;
    timer = NULL;
    rexmtTimer = NULL;
    // v1HostTimer = NULL;
}

IGMPv2::RouterGroupData::~RouterGroupData()
{
    if (timer)
    {
        delete (IGMPRouterTimerContext*)timer->getContextPointer();
        owner->cancelAndDelete(timer);
    }
    if (rexmtTimer)
    {
        delete (IGMPRouterTimerContext*)rexmtTimer->getContextPointer();
        owner->cancelAndDelete(rexmtTimer);
    }
//    if (v1HostTimer)
//    {
//        delete (IGMPRouterTimerContext*)v1HostTimer->getContextPointer();
//        owner->cancelAndDelete(v1HostTimer);
//    }
}

IGMPv2::HostInterfaceData::HostInterfaceData(IGMPv2 *owner)
    : owner(owner)
{
    ASSERT(owner);
}

IGMPv2::HostInterfaceData::~HostInterfaceData()
{
    for (GroupToHostDataMap::iterator it = groups.begin(); it != groups.end(); ++it)
        delete it->second;
}

IGMPv2::RouterInterfaceData::RouterInterfaceData(IGMPv2 *owner)
    : owner(owner)
{
    ASSERT(owner);

    igmpRouterState = IGMP_RS_INITIAL;
    igmpQueryTimer = NULL;
}

IGMPv2::RouterInterfaceData::~RouterInterfaceData()
{
    owner->cancelAndDelete(igmpQueryTimer);

    for (GroupToRouterDataMap::iterator it = groups.begin(); it != groups.end(); ++it)
        delete it->second;
}

IGMPv2::HostInterfaceData *IGMPv2::createHostInterfaceData()
{
    return new HostInterfaceData(this);
}

IGMPv2::RouterInterfaceData *IGMPv2::createRouterInterfaceData()
{
    return new RouterInterfaceData(this);
}

IGMPv2::HostGroupData *IGMPv2::createHostGroupData(InterfaceEntry *ie, const IPv4Address &group)
{
    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    ASSERT(interfaceData->groups.find(group) == interfaceData->groups.end());
    HostGroupData *data = new HostGroupData(this, group);
    interfaceData->groups[group] = data;
    return data;
}

IGMPv2::RouterGroupData *IGMPv2::createRouterGroupData(InterfaceEntry *ie, const IPv4Address &group)
{
    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);
    ASSERT(interfaceData->groups.find(group) == interfaceData->groups.end());
    RouterGroupData *data = new RouterGroupData(this, group);
    interfaceData->groups[group] = data;
    return data;
}

IGMPv2::HostInterfaceData *IGMPv2::getHostInterfaceData(InterfaceEntry *ie)
{
    int interfaceId = ie->getInterfaceId();
    InterfaceToHostDataMap::iterator it = hostData.find(interfaceId);
    if (it != hostData.end())
        return it->second;

    // create one
    HostInterfaceData *data = createHostInterfaceData();
    hostData[interfaceId] = data;
    return data;
}

IGMPv2::RouterInterfaceData *IGMPv2::getRouterInterfaceData(InterfaceEntry *ie)
{
    int interfaceId = ie->getInterfaceId();
    InterfaceToRouterDataMap::iterator it = routerData.find(interfaceId);
    if (it != routerData.end())
        return it->second;

    // create one
    RouterInterfaceData *data = createRouterInterfaceData();
    routerData[interfaceId] = data;
    return data;
}

IGMPv2::HostGroupData *IGMPv2::getHostGroupData(InterfaceEntry *ie, const IPv4Address &group)
{
    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    GroupToHostDataMap::iterator it = interfaceData->groups.find(group);
    return it != interfaceData->groups.end() ? it->second : NULL;
}

IGMPv2::RouterGroupData *IGMPv2::getRouterGroupData(InterfaceEntry *ie, const IPv4Address &group)
{
    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);
    GroupToRouterDataMap::iterator it = interfaceData->groups.find(group);
    return it != interfaceData->groups.end() ? it->second : NULL;
}

void IGMPv2::deleteHostInterfaceData(int interfaceId)
{
    InterfaceToHostDataMap::iterator interfaceIt = hostData.find(interfaceId);
    if (interfaceIt != hostData.end())
    {
        HostInterfaceData *interface = interfaceIt->second;
        hostData.erase(interfaceIt);
        delete interface;
    }
}

void IGMPv2::deleteRouterInterfaceData(int interfaceId)
{
    InterfaceToRouterDataMap::iterator interfaceIt = routerData.find(interfaceId);
    if (interfaceIt != routerData.end())
    {
        RouterInterfaceData *interface = interfaceIt->second;
        routerData.erase(interfaceIt);
        delete interface;
    }
}

void IGMPv2::deleteHostGroupData(InterfaceEntry *ie, const IPv4Address &group)
{
    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    GroupToHostDataMap::iterator it = interfaceData->groups.find(group);
    if (it != interfaceData->groups.end())
    {
        HostGroupData *data = it->second;
        interfaceData->groups.erase(it);
        delete data;
    }
}

void IGMPv2::deleteRouterGroupData(InterfaceEntry *ie, const IPv4Address &group)
{
    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);
    GroupToRouterDataMap::iterator it = interfaceData->groups.find(group);
    if (it != interfaceData->groups.end())
    {
        RouterGroupData *data = it->second;
        interfaceData->groups.erase(it);
        delete data;
    }
}

void IGMPv2::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == 0)
    {
        ift = InterfaceTableAccess().get();
        rt = RoutingTableAccess().get();
        nb = NotificationBoardAccess().get();

        nb->subscribe(this, NF_INTERFACE_DELETED);
        nb->subscribe(this, NF_IPv4_MCAST_JOIN);
        nb->subscribe(this, NF_IPv4_MCAST_LEAVE);

        enabled = par("enabled");
        externalRouter = gate("routerIn")->isPathOK() && gate("routerOut")->isPathOK();
        robustness = par("robustnessVariable");
        queryInterval = par("queryInterval");
        queryResponseInterval = par("queryResponseInterval");
        groupMembershipInterval = par("groupMembershipInterval");
        otherQuerierPresentInterval = par("otherQuerierPresentInterval");
        startupQueryInterval = par("startupQueryInterval");
        startupQueryCount = par("startupQueryCount");
        lastMemberQueryInterval = par("lastMemberQueryInterval");
        lastMemberQueryCount = par("lastMemberQueryCount");
        unsolicitedReportInterval = par("unsolicitedReportInterval");
        //version1RouterPresentInterval = par("version1RouterPresentInterval");

        numGroups = 0;
        numHostGroups = 0;
        numRouterGroups = 0;

        numQueriesSent = 0;
        numQueriesRecv = 0;
        numGeneralQueriesSent = 0;
        numGeneralQueriesRecv = 0;
        numGroupSpecificQueriesSent = 0;
        numGroupSpecificQueriesRecv = 0;
        numReportsSent = 0;
        numReportsRecv = 0;
        numLeavesSent = 0;
        numLeavesRecv = 0;

        WATCH(numGroups);
        WATCH(numHostGroups);
        WATCH(numRouterGroups);

        WATCH(numQueriesSent);
        WATCH(numQueriesRecv);
        WATCH(numGeneralQueriesSent);
        WATCH(numGeneralQueriesRecv);
        WATCH(numGroupSpecificQueriesSent);
        WATCH(numGroupSpecificQueriesRecv);
        WATCH(numReportsSent);
        WATCH(numReportsRecv);
        WATCH(numLeavesSent);
        WATCH(numLeavesRecv);
    }
    else if (stage == 1)
    {
        for (int i = 0; i < (int)ift->getNumInterfaces(); ++i)
        {
            InterfaceEntry *ie = ift->getInterface(i);
            if (ie->isMulticast())
                configureInterface(ie);
        }
        nb->subscribe(this, NF_INTERFACE_CREATED);

        IPSocket ipSocket(gate("ipOut"));
        ipSocket.registerProtocol(IP_PROT_IGMP);
    }
}

IGMPv2::~IGMPv2()
{
    while (!hostData.empty())
        deleteHostInterfaceData(hostData.begin()->first);
    while (!routerData.empty())
        deleteRouterInterfaceData(routerData.begin()->first);
}

void IGMPv2::receiveChangeNotification(int category, const cPolymorphic *details)
{
    Enter_Method_Silent();

    InterfaceEntry *ie;
    int interfaceId;
    const IPv4MulticastGroupInfo *info;

    if (category == NF_INTERFACE_CREATED)
    {
        ie = const_cast<InterfaceEntry *>(check_and_cast<const InterfaceEntry*>(details));
        if (ie->isMulticast())
            configureInterface(ie);
    }
    else if (category == NF_INTERFACE_DELETED)
    {
        ie = const_cast<InterfaceEntry *>(check_and_cast<const InterfaceEntry*>(details));
        if (ie->isMulticast())
        {
            interfaceId = ie->getInterfaceId();
            deleteHostInterfaceData(interfaceId);
            deleteRouterInterfaceData(interfaceId);
        }
    }
    else if (category == NF_IPv4_MCAST_JOIN)
    {
        info = check_and_cast<const IPv4MulticastGroupInfo*>(details);
        multicastGroupJoined(info->ie, info->groupAddress);
    }
    else if (category == NF_IPv4_MCAST_LEAVE)
    {
        info = check_and_cast<const IPv4MulticastGroupInfo*>(details);
        multicastGroupLeft(info->ie, info->groupAddress);
    }
}

void IGMPv2::configureInterface(InterfaceEntry *ie)
{
    if (enabled && rt->isMulticastForwardingEnabled() && !externalRouter)
    {
        // start querier on this interface
        cMessage *timer = new cMessage("IGMPv2 query timer", IGMP_QUERY_TIMER);
        timer->setContextPointer(ie);
        RouterInterfaceData *routerData = getRouterInterfaceData(ie);
        routerData->igmpQueryTimer = timer;
        routerData->igmpRouterState = IGMP_RS_QUERIER;
        sendQuery(ie, IPv4Address(), queryResponseInterval); // general query
        startTimer(timer, startupQueryInterval);
    }
}

void IGMPv2::handleMessage(cMessage *msg)
{
    if (!enabled)
    {
        if (!msg->isSelfMessage())
        {
            EV << "IGMPv2 disabled, dropping packet.\n";
            delete msg;
        }
        return;
    }

    if (msg->isSelfMessage())
    {
        switch (msg->getKind())
        {
            case IGMP_QUERY_TIMER:
                processQueryTimer(msg);
                break;
            case IGMP_HOSTGROUP_TIMER:
                processHostGroupTimer(msg);
                break;
            case IGMP_LEAVE_TIMER:
                processLeaveTimer(msg);
                break;
            case IGMP_REXMT_TIMER:
                processRexmtTimer(msg);
                break;
            default:
                ASSERT(false);
                break;
        }
    }
    else if (!strcmp(msg->getArrivalGate()->getName(), "routerIn"))
        send(msg, "ipOut");
    else if (dynamic_cast<IGMPMessage *>(msg))
        processIgmpMessage((IGMPMessage *)msg);
    else
        ASSERT(false);
}

void IGMPv2::multicastGroupJoined(InterfaceEntry *ie, const IPv4Address& groupAddr)
{
    ASSERT(ie && ie->isMulticast());
    ASSERT(groupAddr.isMulticast());

    if (enabled && !groupAddr.isLinkLocalMulticast())
    {
        HostGroupData *groupData = createHostGroupData(ie, groupAddr);
        numGroups++;
        numHostGroups++;

        sendReport(ie, groupData);
        groupData->flag = true;
        startHostTimer(ie, groupData, unsolicitedReportInterval);
        groupData->state = IGMP_HGS_DELAYING_MEMBER;
    }
}

void IGMPv2::multicastGroupLeft(InterfaceEntry *ie, const IPv4Address& groupAddr)
{
    ASSERT(ie && ie->isMulticast());
    ASSERT(groupAddr.isMulticast());

    if (enabled && !groupAddr.isLinkLocalMulticast())
    {
        HostGroupData *groupData = getHostGroupData(ie, groupAddr);
        if (groupData)
        {
            if (groupData->state == IGMP_HGS_DELAYING_MEMBER)
                cancelEvent(groupData->timer);

            if (groupData->flag)
                sendLeave(ie, groupData);
        }

        deleteHostGroupData(ie, groupAddr);
        numHostGroups--;
        numGroups--;
    }
}

void IGMPv2::startTimer(cMessage *timer, double interval)
{
    ASSERT(timer);
    cancelEvent(timer);
    scheduleAt(simTime() + interval, timer);
}

void IGMPv2::startHostTimer(InterfaceEntry *ie, HostGroupData* group, double maxRespTime)
{
    if (!group->timer)
    {
        group->timer = new cMessage("IGMPv2 group timer", IGMP_HOSTGROUP_TIMER);
        group->timer->setContextPointer(new IGMPHostTimerContext(ie, group));
    }

    double delay = uniform(0.0, maxRespTime);
    EV << "setting host timer for " << ie->getName() << " and group " << group->groupAddr.str() << " to " << delay << "\n";
    startTimer(group->timer, delay);
}

void IGMPv2::sendQuery(InterfaceEntry *ie, const IPv4Address& groupAddr, double maxRespTime)
{
    ASSERT(groupAddr.isUnspecified() || (groupAddr.isMulticast() && !groupAddr.isLinkLocalMulticast()));

    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);

    if (interfaceData->igmpRouterState == IGMP_RS_QUERIER)
    {
        if (groupAddr.isUnspecified())
            EV << "IGMPv2: sending General Membership Query on iface=" << ie->getName() << "\n";
        else
            EV << "IGMPv2: sending Membership Query for group=" << groupAddr << " on iface=" << ie->getName() << "\n";

        IGMPMessage *msg = new IGMPMessage("IGMPv2 query");
        msg->setType(IGMP_MEMBERSHIP_QUERY);
        msg->setGroupAddress(groupAddr);
        msg->setMaxRespTime((int)(maxRespTime * 10.0));
        msg->setByteLength(8);
        sendToIP(msg, ie, groupAddr.isUnspecified() ? IPv4Address::ALL_HOSTS_MCAST : groupAddr);

        numQueriesSent++;
        if (groupAddr.isUnspecified())
            numGeneralQueriesSent++;
        else
            numGroupSpecificQueriesSent++;
    }
}

void IGMPv2::sendReport(InterfaceEntry *ie, HostGroupData* group)
{
    ASSERT(group->groupAddr.isMulticast() && !group->groupAddr.isLinkLocalMulticast());

    EV << "IGMPv2: sending Membership Report for group=" << group->groupAddr << " on iface=" << ie->getName() << "\n";
    IGMPMessage *msg = new IGMPMessage("IGMPv2 report");
    msg->setType(IGMPV2_MEMBERSHIP_REPORT);
    msg->setGroupAddress(group->groupAddr);
    msg->setByteLength(8);
    sendToIP(msg, ie, group->groupAddr);
    numReportsSent++;
}

void IGMPv2::sendLeave(InterfaceEntry *ie, HostGroupData* group)
{
    ASSERT(group->groupAddr.isMulticast() && !group->groupAddr.isLinkLocalMulticast());

    EV << "IGMPv2: sending Leave Group for group=" << group->groupAddr << " on iface=" << ie->getName() << "\n";
    IGMPMessage *msg = new IGMPMessage("IGMPv2 leave");
    msg->setType(IGMPV2_LEAVE_GROUP);
    msg->setGroupAddress(group->groupAddr);
    msg->setByteLength(8);
    sendToIP(msg, ie, IPv4Address::ALL_ROUTERS_MCAST);
    numLeavesSent++;
}

void IGMPv2::sendToIP(IGMPMessage *msg, InterfaceEntry *ie, const IPv4Address& dest)
{
    ASSERT(ie->isMulticast());

    IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
    controlInfo->setProtocol(IP_PROT_IGMP);
    controlInfo->setInterfaceId(ie->getInterfaceId());
    controlInfo->setTimeToLive(1);
    controlInfo->setDestAddr(dest);
    msg->setControlInfo(controlInfo);

    send(msg, "ipOut");
}

void IGMPv2::processIgmpMessage(IGMPMessage *msg)
{
    IPv4ControlInfo *controlInfo = (IPv4ControlInfo *)msg->getControlInfo();
    InterfaceEntry *ie = ift->getInterfaceById(controlInfo->getInterfaceId());
    switch (msg->getType())
    {
        case IGMP_MEMBERSHIP_QUERY:
            processQuery(ie, controlInfo->getSrcAddr(), msg);
            break;
        //case IGMPV1_MEMBERSHIP_REPORT:
        //    processV1Report(ie, msg);
        //    delete msg;
        //    break;
        case IGMPV2_MEMBERSHIP_REPORT:
            processV2Report(ie, msg);
            break;
        case IGMPV2_LEAVE_GROUP:
            processLeave(ie, msg);
            break;
        default:
            if (externalRouter)
                send(msg, "routerOut");
            else
            {
                delete msg;
                throw cRuntimeError("IGMPv2: Unhandled message type (%dq)", msg->getType());
            }
            break;
    }
}

void IGMPv2::processQueryTimer(cMessage *msg)
{
    InterfaceEntry *ie = (InterfaceEntry*)msg->getContextPointer();
    ASSERT(ie);
    EV << "IGMPv2: General Query timer expired, iface=" << ie->getName() << "\n";
    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);
    RouterState state = interfaceData->igmpRouterState;
    if (state == IGMP_RS_QUERIER || state == IGMP_RS_NON_QUERIER)
    {
        interfaceData->igmpRouterState = IGMP_RS_QUERIER;
        sendQuery(ie, IPv4Address(), queryResponseInterval); // general query
        startTimer(msg, queryInterval);
    }
}

void IGMPv2::processHostGroupTimer(cMessage *msg)
{
    IGMPHostTimerContext *ctx = (IGMPHostTimerContext*)msg->getContextPointer();
    EV << "IGMPv2: Host Timer expired for group=" << ctx->hostGroup->groupAddr << " iface=" << ctx->ie->getName() << "\n";
    sendReport(ctx->ie, ctx->hostGroup);
    ctx->hostGroup->flag = true;
    ctx->hostGroup->state = IGMP_HGS_IDLE_MEMBER;
}

void IGMPv2::processLeaveTimer(cMessage *msg)
{
    IGMPRouterTimerContext *ctx = (IGMPRouterTimerContext*)msg->getContextPointer();
    EV << "IGMPv2: Leave Timer expired, deleting " << ctx->routerGroup->groupAddr << " from listener list of '" << ctx->ie->getName() << "'\n";

    // notify IPv4InterfaceData to update its listener list
    ctx->ie->ipv4Data()->removeMulticastListener(ctx->routerGroup->groupAddr);

    IPv4MulticastGroupInfo info(ctx->ie, ctx->routerGroup->groupAddr);
    nb->fireChangeNotification(NF_IPv4_MCAST_UNREGISTERED, &info);
    numRouterGroups--;

    if (ctx->routerGroup->state ==  IGMP_RGS_CHECKING_MEMBERSHIP)
        cancelEvent(ctx->routerGroup->rexmtTimer);

    ctx->routerGroup->state = IGMP_RGS_NO_MEMBERS_PRESENT;
    deleteRouterGroupData(ctx->ie, ctx->routerGroup->groupAddr);
    numGroups--;
}

void IGMPv2::processRexmtTimer(cMessage *msg)
{
    IGMPRouterTimerContext *ctx = (IGMPRouterTimerContext*)msg->getContextPointer();
    EV << "IGMPv2: Rexmt Timer expired for group=" << ctx->routerGroup->groupAddr << " iface=" << ctx->ie->getName() << "\n";
    sendQuery(ctx->ie, ctx->routerGroup->groupAddr, lastMemberQueryInterval);
    startTimer(ctx->routerGroup->rexmtTimer, lastMemberQueryInterval);
    ctx->routerGroup->state = IGMP_RGS_CHECKING_MEMBERSHIP;
}

void IGMPv2::processQuery(InterfaceEntry *ie, const IPv4Address& sender, IGMPMessage *msg)
{
    ASSERT(ie->isMulticast());

    HostInterfaceData *interfaceData = getHostInterfaceData(ie);

    numQueriesRecv++;

    IPv4Address &groupAddr = msg->getGroupAddress();
    if (groupAddr.isUnspecified())
    {
        // general query
        EV << "IGMPv2: received General Membership Query on iface=" << ie->getName() << "\n";
        numGeneralQueriesRecv++;
        for (GroupToHostDataMap::iterator it = interfaceData->groups.begin(); it != interfaceData->groups.end(); ++it)
            processGroupQuery(ie, it->second, msg->getMaxRespTime());
    }
    else
    {
        // group-specific query
        EV << "IGMPv2: received Membership Query for group=" << groupAddr << " iface=" << ie->getName() << "\n";
        numGroupSpecificQueriesRecv++;
        GroupToHostDataMap::iterator it = interfaceData->groups.find(groupAddr);
        if (it != interfaceData->groups.end())
            processGroupQuery(ie, it->second, msg->getMaxRespTime());
    }

    if (rt->isMulticastForwardingEnabled())
    {
        if (externalRouter)
        {
            send(msg, "routerOut");
            return;
        }

        RouterInterfaceData *routerInterfaceData = getRouterInterfaceData(ie);
        if (sender < ie->ipv4Data()->getIPAddress())
        {
            startTimer(routerInterfaceData->igmpQueryTimer, otherQuerierPresentInterval);
            routerInterfaceData->igmpRouterState = IGMP_RS_NON_QUERIER;
        }

        if (!groupAddr.isUnspecified() && routerInterfaceData->igmpRouterState == IGMP_RS_NON_QUERIER) // group specific query
        {
            RouterGroupData *groupData = getRouterGroupData(ie, groupAddr);
            if (groupData->state == IGMP_RGS_MEMBERS_PRESENT)
            {
                double maxResponseTime = (double)msg->getMaxRespTime() / 10.0;
                startTimer(groupData->timer, maxResponseTime * lastMemberQueryCount);
                groupData->state = IGMP_RGS_CHECKING_MEMBERSHIP;
            }
        }
    }

    delete msg;
}

void IGMPv2::processGroupQuery(InterfaceEntry *ie, HostGroupData* group, int maxRespTime)
{
    double maxRespTimeSecs = (double)maxRespTime / 10.0;

    if (group->state == IGMP_HGS_DELAYING_MEMBER)
    {
        cMessage *timer = group->timer;
        simtime_t maxAbsoluteRespTime = simTime() + maxRespTimeSecs;
        if (timer->isScheduled() && maxAbsoluteRespTime < timer->getArrivalTime())
            startHostTimer(ie, group, maxRespTimeSecs);
    }
    else if (group->state == IGMP_HGS_IDLE_MEMBER)
    {
        startHostTimer(ie, group, maxRespTimeSecs);
        group->state = IGMP_HGS_DELAYING_MEMBER;
    }
    else
    {
        // ignored
    }
}

void IGMPv2::processV2Report(InterfaceEntry *ie, IGMPMessage *msg)
{
    ASSERT(ie->isMulticast());

    IPv4Address &groupAddr = msg->getGroupAddress();

    EV << "IGMPv2: received V2 Membership Report for group=" << groupAddr << " iface=" << ie->getName() << "\n";

    numReportsRecv++;

    HostGroupData *hostGroupData = getHostGroupData(ie, groupAddr);
    if (hostGroupData)
    {
        if (hostGroupData && hostGroupData->state == IGMP_HGS_DELAYING_MEMBER)
        {
            cancelEvent(hostGroupData->timer);
            hostGroupData->flag = false;
            hostGroupData->state = IGMP_HGS_IDLE_MEMBER;
        }
    }

    if (rt->isMulticastForwardingEnabled())
    {
        if (externalRouter)
        {
            send(msg, "routerOut");
            return;
        }

        RouterGroupData* routerGroupData = getRouterGroupData(ie, groupAddr);
        if (!routerGroupData)
        {
            routerGroupData = createRouterGroupData(ie, groupAddr);
            numGroups++;
        }

        if (!routerGroupData->timer)
        {
            routerGroupData->timer = new cMessage("IGMPv2 leave timer", IGMP_LEAVE_TIMER);
            routerGroupData->timer->setContextPointer(new IGMPRouterTimerContext(ie, routerGroupData));
        }
        if (!routerGroupData->rexmtTimer)
        {
            routerGroupData->rexmtTimer = new cMessage("IGMPv2 rexmt timer", IGMP_REXMT_TIMER);
            routerGroupData->rexmtTimer->setContextPointer(new IGMPRouterTimerContext(ie, routerGroupData));
        }

        if (routerGroupData->state == IGMP_RGS_NO_MEMBERS_PRESENT)
        {
            // notify IPv4InterfaceData to update its listener list
            ie->ipv4Data()->addMulticastListener(groupAddr);
            // notify routing
            IPv4MulticastGroupInfo info(ie, routerGroupData->groupAddr);
            nb->fireChangeNotification(NF_IPv4_MCAST_REGISTERED, &info);
            numRouterGroups++;
        }
        else if (routerGroupData->state == IGMP_RGS_CHECKING_MEMBERSHIP)
            cancelEvent(routerGroupData->rexmtTimer);

        startTimer(routerGroupData->timer, groupMembershipInterval);
        routerGroupData->state = IGMP_RGS_MEMBERS_PRESENT;
    }

    delete msg;
}

void IGMPv2::processLeave(InterfaceEntry *ie, IGMPMessage *msg)
{
    ASSERT(ie->isMulticast());

    EV << "IGMPv2: received Leave Group for group=" << msg->getGroupAddress() << " iface=" << ie->getName() << "\n";

    numLeavesRecv++;

    if (rt->isMulticastForwardingEnabled())
    {
        if (externalRouter)
        {
            send(msg, "routerOut");
            return;
        }

        IPv4Address &groupAddr = msg->getGroupAddress();
        RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);
        RouterGroupData *groupData = getRouterGroupData(ie, groupAddr);
        if (groupData)
        {
            if (groupData->state == IGMP_RGS_MEMBERS_PRESENT && interfaceData->igmpRouterState == IGMP_RS_QUERIER)
            {
                startTimer(groupData->timer, lastMemberQueryInterval * lastMemberQueryCount);
                startTimer(groupData->rexmtTimer, lastMemberQueryInterval);
                sendQuery(ie, groupAddr, lastMemberQueryInterval);
                groupData->state = IGMP_RGS_CHECKING_MEMBERSHIP;
            }
        }
    }

    delete msg;
}
