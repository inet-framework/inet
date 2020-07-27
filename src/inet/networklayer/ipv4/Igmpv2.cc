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

#include <algorithm>

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/TcpIpChecksum.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/Igmpv2.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv4/Ipv4OptionsTag_m.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"

namespace inet {

Define_Module(Igmpv2);

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

Igmpv2::~Igmpv2()
{
    while (!hostData.empty())
        deleteHostInterfaceData(hostData.begin()->first);

    while (!routerData.empty())
        deleteRouterInterfaceData(routerData.begin()->first);
}

void Igmpv2::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        rt = getModuleFromPar<IIpv4RoutingTable>(par("routingTableModule"), this);

        externalRouter = false;
        enabled = par("enabled");
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
        const char *crcModeString = par("crcMode");
        crcMode = parseCrcMode(crcModeString, false);

        addWatches();
    }
    // TODO: INITSTAGE
    else if (stage == INITSTAGE_NETWORK_LAYER_PROTOCOLS) {
        cModule *host = getContainingNode(this);
        registerService(Protocol::igmp, nullptr, gate("ipIn"));
        registerProtocol(Protocol::igmp, gate("ipOut"), nullptr);
        for (int i = 0; i < ift->getNumInterfaces(); ++i) {
            InterfaceEntry *ie = ift->getInterface(i);
            if (ie->isMulticast()) {
                if (auto ipv4interfaceData = ie->findProtocolData<Ipv4InterfaceData>()) {
                    int n = ipv4interfaceData->getNumOfJoinedMulticastGroups();
                    for (int j = 0; j < n; j++) {
                        auto groupAddress = ipv4interfaceData->getJoinedMulticastGroup(j);
                        multicastGroupJoined(ie, groupAddress);
                    }
                }
            }
        }

        for (int i = 0; i < ift->getNumInterfaces(); ++i) {
            InterfaceEntry *ie = ift->getInterface(i);
            if (ie->isMulticast())
                configureInterface(ie);
        }

        host->subscribe(interfaceCreatedSignal, this);
        host->subscribe(interfaceDeletedSignal, this);
        host->subscribe(ipv4MulticastGroupJoinedSignal, this);
        host->subscribe(ipv4MulticastGroupLeftSignal, this);
    }
}

void Igmpv2::addWatches()
{
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

    WATCH_PTRMAP(hostData);
    WATCH_PTRMAP(routerData);
}

void Igmpv2::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();

    InterfaceEntry *ie;
    int interfaceId;
    const Ipv4MulticastGroupInfo *info;

    if (signalID == ipv4MulticastGroupJoinedSignal) {
        info = check_and_cast<const Ipv4MulticastGroupInfo *>(obj);
        multicastGroupJoined(info->ie, info->groupAddress);
    }
    else if (signalID == ipv4MulticastGroupLeftSignal) {
        info = check_and_cast<const Ipv4MulticastGroupInfo *>(obj);
        multicastGroupLeft(info->ie, info->groupAddress);
    }
    else if (signalID == interfaceCreatedSignal) {
        ie = check_and_cast<InterfaceEntry *>(obj);
        if (ie->isMulticast())
            configureInterface(ie);
    }
    else if (signalID == interfaceDeletedSignal) {
        ie = check_and_cast<InterfaceEntry *>(obj);
        if (ie->isMulticast()) {
            interfaceId = ie->getInterfaceId();
            deleteHostInterfaceData(interfaceId);
            deleteRouterInterfaceData(interfaceId);
        }
    }
}

void Igmpv2::multicastGroupJoined(InterfaceEntry *ie, const Ipv4Address& groupAddr)
{
    ASSERT(ie && ie->isMulticast());
    ASSERT(groupAddr.isMulticast());

    if (enabled && !groupAddr.isLinkLocalMulticast()) {
        HostGroupData *groupData = createHostGroupData(ie, groupAddr);
        numGroups++;
        numHostGroups++;

        sendReport(ie, groupData);
        groupData->flag = true;
        startHostTimer(ie, groupData, unsolicitedReportInterval);
        groupData->state = IGMP_HGS_DELAYING_MEMBER;
    }
}

void Igmpv2::multicastGroupLeft(InterfaceEntry *ie, const Ipv4Address& groupAddr)
{
    ASSERT(ie && ie->isMulticast());
    ASSERT(groupAddr.isMulticast());

    if (enabled && !groupAddr.isLinkLocalMulticast()) {
        HostGroupData *groupData = getHostGroupData(ie, groupAddr);
        if (groupData) {
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

void Igmpv2::configureInterface(InterfaceEntry *ie)
{
    if (enabled && rt->isMulticastForwardingEnabled() && !externalRouter) {
        // start querier on this interface
        cMessage *timer = new cMessage("Igmpv2 query timer", IGMP_QUERY_TIMER);
        timer->setContextPointer(ie);
        RouterInterfaceData *routerData = getRouterInterfaceData(ie);
        routerData->igmpQueryTimer = timer;
        routerData->igmpRouterState = IGMP_RS_QUERIER;
        sendQuery(ie, Ipv4Address(), queryResponseInterval);    // general query
        startTimer(timer, startupQueryInterval);
    }
}

void Igmpv2::deleteHostInterfaceData(int interfaceId)
{
    auto interfaceIt = hostData.find(interfaceId);
    if (interfaceIt != hostData.end()) {
        HostInterfaceData *interface = interfaceIt->second;
        hostData.erase(interfaceIt);
        delete interface;
    }
}

void Igmpv2::deleteRouterInterfaceData(int interfaceId)
{
    auto interfaceIt = routerData.find(interfaceId);
    if (interfaceIt != routerData.end()) {
        RouterInterfaceData *interface = interfaceIt->second;
        routerData.erase(interfaceIt);
        delete interface;
    }
}

void Igmpv2::handleMessage(cMessage *msg)
{
    if (!enabled) {
        if (!msg->isSelfMessage()) {
            EV_ERROR << "Igmpv2 disabled, dropping packet.\n";
            delete msg;
        }
        return;
    }

    if (msg->isSelfMessage()) {
        switch (msg->getKind()) {
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
    else {
        auto packet = check_and_cast<Packet *>(msg);
        processIgmpMessage(packet);
    }
}

void Igmpv2::handleRegisterService(const Protocol& protocol, cGate *out, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
}

void Igmpv2::handleRegisterProtocol(const Protocol& protocol, cGate *in, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    if (protocol.getId() == Protocol::igmp.getId() && servicePrimitive == SP_INDICATION)
        externalRouter = true;
}

// --- Methods for handling self messages ---

void Igmpv2::processQueryTimer(cMessage *msg)
{
    InterfaceEntry *ie = (InterfaceEntry *)msg->getContextPointer();
    ASSERT(ie);
    EV_DEBUG << "Igmpv2: General Query timer expired, iface=" << ie->getInterfaceName() << "\n";
    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);
    RouterState state = interfaceData->igmpRouterState;
    if (state == IGMP_RS_QUERIER || state == IGMP_RS_NON_QUERIER) {
        interfaceData->igmpRouterState = IGMP_RS_QUERIER;
        sendQuery(ie, Ipv4Address(), queryResponseInterval);    // general query
        startTimer(msg, queryInterval);
    }
}

void Igmpv2::processHostGroupTimer(cMessage *msg)
{
    IgmpHostTimerContext *ctx = (IgmpHostTimerContext *)msg->getContextPointer();
    EV_DEBUG << "Igmpv2: Host Timer expired for group=" << ctx->hostGroup->groupAddr << " iface=" << ctx->ie->getInterfaceName() << "\n";
    sendReport(ctx->ie, ctx->hostGroup);
    ctx->hostGroup->flag = true;
    ctx->hostGroup->state = IGMP_HGS_IDLE_MEMBER;
}

void Igmpv2::processLeaveTimer(cMessage *msg)
{
    IgmpRouterTimerContext *ctx = (IgmpRouterTimerContext *)msg->getContextPointer();
    EV_DEBUG << "Igmpv2: Leave Timer expired, deleting " << ctx->routerGroup->groupAddr << " from listener list of '" << ctx->ie->getInterfaceName() << "'\n";

    // notify Ipv4InterfaceData to update its listener list
    ctx->ie->getProtocolData<Ipv4InterfaceData>()->removeMulticastListener(ctx->routerGroup->groupAddr);
    numRouterGroups--;

    if (ctx->routerGroup->state == IGMP_RGS_CHECKING_MEMBERSHIP)
        cancelEvent(ctx->routerGroup->rexmtTimer);

    ctx->routerGroup->state = IGMP_RGS_NO_MEMBERS_PRESENT;
    deleteRouterGroupData(ctx->ie, ctx->routerGroup->groupAddr);
    numGroups--;
}

void Igmpv2::processRexmtTimer(cMessage *msg)
{
    IgmpRouterTimerContext *ctx = (IgmpRouterTimerContext *)msg->getContextPointer();
    EV_DEBUG << "Igmpv2: Rexmt Timer expired for group=" << ctx->routerGroup->groupAddr << " iface=" << ctx->ie->getInterfaceName() << "\n";
    sendQuery(ctx->ie, ctx->routerGroup->groupAddr, lastMemberQueryInterval);
    startTimer(ctx->routerGroup->rexmtTimer, lastMemberQueryInterval);
    ctx->routerGroup->state = IGMP_RGS_CHECKING_MEMBERSHIP;
}

void Igmpv2::startTimer(cMessage *timer, double interval)
{
    ASSERT(timer);
    cancelEvent(timer);
    scheduleAfter(interval, timer);
}

void Igmpv2::startHostTimer(InterfaceEntry *ie, HostGroupData *group, double maxRespTime)
{
    if (!group->timer) {
        group->timer = new cMessage("Igmpv2 group timer", IGMP_HOSTGROUP_TIMER);
        group->timer->setContextPointer(new IgmpHostTimerContext(ie, group));
    }

    double delay = uniform(0.0, maxRespTime);
    EV_DEBUG << "setting host timer for " << ie->getInterfaceName() << " and group " << group->groupAddr.str() << " to " << delay << "\n";
    startTimer(group->timer, delay);
}

// --- Methods for processing IGMP messages ---

void Igmpv2::processIgmpMessage(Packet *packet)
{
    if (!verifyCrc(packet)) {
        EV_WARN << "incoming IGMP packet has wrong CRC, dropped\n";
        // drop packet
        PacketDropDetails details;
        details.setReason(INCORRECTLY_RECEIVED);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }

    const auto& igmp = packet->peekAtFront<IgmpMessage>();
    InterfaceEntry *ie = ift->getInterfaceById(packet->getTag<InterfaceInd>()->getInterfaceId());
    switch (igmp->getType()) {
        case IGMP_MEMBERSHIP_QUERY:
            processQuery(ie, packet);
            break;

        //case IGMPV1_MEMBERSHIP_REPORT:
        //    processV1Report(ie, msg);
        //    delete msg;
        //    break;
        case IGMPV2_MEMBERSHIP_REPORT:
            processV2Report(ie, packet);
            break;

        case IGMPV2_LEAVE_GROUP:
            processLeave(ie, packet);
            break;

        default:
            if (externalRouter)
                send(packet, "routerOut");
            else {
                //delete msg;
                throw cRuntimeError("Igmpv2: Unhandled message type (%d) in packet %s", igmp->getType(), packet->getName());
            }
            break;
    }
}

void Igmpv2::processQuery(InterfaceEntry *ie, Packet *packet)
{
    ASSERT(ie->isMulticast());

    Ipv4Address sender = packet->getTag<L3AddressInd>()->getSrcAddress().toIpv4();
    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    const auto& igmpQry = packet->peekAtFront<IgmpQuery>(b(packet->getBitLength()));   //peek entire igmp packet

    numQueriesRecv++;

    Ipv4Address groupAddr = igmpQry->getGroupAddress();
    const Ptr<const Igmpv2Query>& v2Query = dynamicPtrCast<const Igmpv2Query>(igmpQry);
    simtime_t maxRespTime = SimTime(v2Query ? v2Query->getMaxRespTimeCode() : 100, (SimTimeUnit)-1);

    if (groupAddr.isUnspecified()) {
        // general query
        EV_INFO << "Igmpv2: received General Membership Query on iface=" << ie->getInterfaceName() << "\n";
        numGeneralQueriesRecv++;
        for (auto & elem : interfaceData->groups)
            processGroupQuery(ie, elem.second, maxRespTime);
    }
    else {
        // group-specific query
        EV_INFO << "Igmpv2: received Membership Query for group=" << groupAddr << " iface=" << ie->getInterfaceName() << "\n";
        numGroupSpecificQueriesRecv++;
        auto it = interfaceData->groups.find(groupAddr);
        if (it != interfaceData->groups.end())
            processGroupQuery(ie, it->second, maxRespTime);
    }

    if (rt->isMulticastForwardingEnabled()) {
        if (externalRouter) {
            send(packet, "routerOut");
            return;
        }

        RouterInterfaceData *routerInterfaceData = getRouterInterfaceData(ie);
        if (sender < ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress()) {
            startTimer(routerInterfaceData->igmpQueryTimer, otherQuerierPresentInterval);
            routerInterfaceData->igmpRouterState = IGMP_RS_NON_QUERIER;
        }

        if (!groupAddr.isUnspecified() && routerInterfaceData->igmpRouterState == IGMP_RS_NON_QUERIER) {    // group specific query
            RouterGroupData *groupData = getRouterGroupData(ie, groupAddr);
            if (groupData->state == IGMP_RGS_MEMBERS_PRESENT) {
                startTimer(groupData->timer, maxRespTime.dbl() * lastMemberQueryCount);
                groupData->state = IGMP_RGS_CHECKING_MEMBERSHIP;
            }
        }
    }
    else
        EV_INFO << "Igmpv2: multicast forwarding is not enabled on this node. \n";

    delete packet;
}

void Igmpv2::processGroupQuery(InterfaceEntry *ie, HostGroupData *group, simtime_t maxRespTime)
{
    double maxRespTimeSecs = maxRespTime.dbl();         //FIXME use simtime_t !!!

    if (group->state == IGMP_HGS_DELAYING_MEMBER) {
        cMessage *timer = group->timer;
        simtime_t maxAbsoluteRespTime = simTime() + maxRespTimeSecs;
        if (timer->isScheduled() && maxAbsoluteRespTime < timer->getArrivalTime())
            startHostTimer(ie, group, maxRespTimeSecs);
    }
    else if (group->state == IGMP_HGS_IDLE_MEMBER) {
        startHostTimer(ie, group, maxRespTimeSecs);
        group->state = IGMP_HGS_DELAYING_MEMBER;
    }
    else {
        // ignored
    }
}

void Igmpv2::processV2Report(InterfaceEntry *ie, Packet *packet)
{
    ASSERT(ie->isMulticast());

    const auto& msg = packet->peekAtFront<Igmpv2Report>();

    Ipv4Address groupAddr = msg->getGroupAddress();

    EV_INFO << "Igmpv2: received V2 Membership Report for group=" << groupAddr << " iface=" << ie->getInterfaceName() << "\n";

    numReportsRecv++;

    HostGroupData *hostGroupData = getHostGroupData(ie, groupAddr);
    if (hostGroupData) {
        if (hostGroupData && hostGroupData->state == IGMP_HGS_DELAYING_MEMBER) {
            cancelEvent(hostGroupData->timer);
            hostGroupData->flag = false;
            hostGroupData->state = IGMP_HGS_IDLE_MEMBER;
        }
    }

    if (rt->isMulticastForwardingEnabled()) {
        if (externalRouter) {
            send(packet, "routerOut");
            return;
        }

        RouterGroupData *routerGroupData = getRouterGroupData(ie, groupAddr);
        if (!routerGroupData) {
            routerGroupData = createRouterGroupData(ie, groupAddr);
            numGroups++;
        }

        if (!routerGroupData->timer) {
            routerGroupData->timer = new cMessage("Igmpv2 leave timer", IGMP_LEAVE_TIMER);
            routerGroupData->timer->setContextPointer(new IgmpRouterTimerContext(ie, routerGroupData));
        }
        if (!routerGroupData->rexmtTimer) {
            routerGroupData->rexmtTimer = new cMessage("Igmpv2 rexmt timer", IGMP_REXMT_TIMER);
            routerGroupData->rexmtTimer->setContextPointer(new IgmpRouterTimerContext(ie, routerGroupData));
        }

        if (routerGroupData->state == IGMP_RGS_NO_MEMBERS_PRESENT) {
            // notify Ipv4InterfaceData to update its listener list
            ie->getProtocolData<Ipv4InterfaceData>()->addMulticastListener(groupAddr);
            numRouterGroups++;
        }
        else if (routerGroupData->state == IGMP_RGS_CHECKING_MEMBERSHIP)
            cancelEvent(routerGroupData->rexmtTimer);

        startTimer(routerGroupData->timer, groupMembershipInterval);
        routerGroupData->state = IGMP_RGS_MEMBERS_PRESENT;
    }
    else
        EV_INFO << "Igmpv2: multicast forwarding is not enabled on this node. \n";

    delete packet;
}

void Igmpv2::processLeave(InterfaceEntry *ie, Packet *packet)
{
    ASSERT(ie->isMulticast());

    const auto& msg = packet->peekAtFront<Igmpv2Leave>();

    EV_INFO << "Igmpv2: received Leave Group for group=" << msg->getGroupAddress() << " iface=" << ie->getInterfaceName() << "\n";

    numLeavesRecv++;

    if (rt->isMulticastForwardingEnabled()) {
        if (externalRouter) {
            send(packet, "routerOut");
            return;
        }

        Ipv4Address groupAddr = msg->getGroupAddress();
        RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);
        RouterGroupData *groupData = getRouterGroupData(ie, groupAddr);
        if (groupData) {
            if (groupData->state == IGMP_RGS_MEMBERS_PRESENT && interfaceData->igmpRouterState == IGMP_RS_QUERIER) {
                startTimer(groupData->timer, lastMemberQueryInterval * lastMemberQueryCount);
                startTimer(groupData->rexmtTimer, lastMemberQueryInterval);
                sendQuery(ie, groupAddr, lastMemberQueryInterval);
                groupData->state = IGMP_RGS_CHECKING_MEMBERSHIP;
            }
        }
    }

    delete packet;
}

// --- Methods for sending IGMP messages ---

void Igmpv2::sendQuery(InterfaceEntry *ie, const Ipv4Address& groupAddr, double maxRespTime)
{
    ASSERT(groupAddr.isUnspecified() || (groupAddr.isMulticast() && !groupAddr.isLinkLocalMulticast()));

    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);

    if (interfaceData->igmpRouterState == IGMP_RS_QUERIER) {
        if (groupAddr.isUnspecified())
            EV_INFO << "Igmpv2: sending General Membership Query on iface=" << ie->getInterfaceName() << "\n";
        else
            EV_INFO << "Igmpv2: sending Membership Query for group=" << groupAddr << " on iface=" << ie->getInterfaceName() << "\n";

        Packet *packet = new Packet("Igmpv2 query");
        const auto& msg = makeShared<Igmpv2Query>();
        msg->setType(IGMP_MEMBERSHIP_QUERY);
        msg->setGroupAddress(groupAddr);
        msg->setMaxRespTimeCode(SimTime(maxRespTime).inUnit((SimTimeUnit)-1));
        msg->setChunkLength(B(8));
        insertCrc(msg, packet);
        packet->insertAtFront(msg);
        sendToIP(packet, ie, groupAddr.isUnspecified() ? Ipv4Address::ALL_HOSTS_MCAST : groupAddr);

        numQueriesSent++;
        if (groupAddr.isUnspecified())
            numGeneralQueriesSent++;
        else
            numGroupSpecificQueriesSent++;
    }
}

void Igmpv2::sendReport(InterfaceEntry *ie, HostGroupData *group)
{
    ASSERT(group->groupAddr.isMulticast() && !group->groupAddr.isLinkLocalMulticast());

    EV_INFO << "Igmpv2: sending Membership Report for group=" << group->groupAddr << " on iface=" << ie->getInterfaceName() << "\n";
    Packet *packet = new Packet("Igmpv2 report");
    const auto& msg = makeShared<Igmpv2Report>();
    msg->setGroupAddress(group->groupAddr);
    msg->setChunkLength(B(8));
    insertCrc(msg, packet);
    packet->insertAtFront(msg);
    // TODO fill Router Alert option
    auto raOption = new Ipv4OptionRouterAlert();
    packet->addTag<Ipv4OptionsReq>()->insertOption(raOption);
    sendToIP(packet, ie, group->groupAddr);
    numReportsSent++;
}

void Igmpv2::sendLeave(InterfaceEntry *ie, HostGroupData *group)
{
    ASSERT(group->groupAddr.isMulticast() && !group->groupAddr.isLinkLocalMulticast());

    EV_INFO << "Igmpv2: sending Leave Group for group=" << group->groupAddr << " on iface=" << ie->getInterfaceName() << "\n";
    Packet *packet = new Packet("Igmpv2 leave");
    const auto& msg = makeShared<Igmpv2Leave>();
    msg->setGroupAddress(group->groupAddr);
    msg->setChunkLength(B(8));
    insertCrc(msg, packet);
    packet->insertAtFront(msg);
    // TODO fill Router Alert option
    auto raOption = new Ipv4OptionRouterAlert();
    packet->addTag<Ipv4OptionsReq>()->insertOption(raOption);
    sendToIP(packet, ie, Ipv4Address::ALL_ROUTERS_MCAST);
    numLeavesSent++;
}

// TODO add Router Alert option
void Igmpv2::sendToIP(Packet *msg, InterfaceEntry *ie, const Ipv4Address& dest)
{
    ASSERT(ie->isMulticast());

    msg->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::igmp);
    msg->addTagIfAbsent<DispatchProtocolInd>()->setProtocol(&Protocol::igmp);
    msg->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
    msg->addTagIfAbsent<InterfaceReq>()->setInterfaceId(ie->getInterfaceId());
    msg->addTagIfAbsent<L3AddressReq>()->setDestAddress(dest);
    msg->addTagIfAbsent<HopLimitReq>()->setHopLimit(1);

    send(msg, "ipOut");
}

// --- Utility Methods for Group Data ---

Igmpv2::RouterGroupData *Igmpv2::createRouterGroupData(InterfaceEntry *ie, const Ipv4Address& group)
{
    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);
    ASSERT(interfaceData->groups.find(group) == interfaceData->groups.end());
    RouterGroupData *data = new RouterGroupData(this, group);
    interfaceData->groups[group] = data;
    return data;
}

Igmpv2::HostGroupData *Igmpv2::createHostGroupData(InterfaceEntry *ie, const Ipv4Address& group)
{
    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    ASSERT(interfaceData->groups.find(group) == interfaceData->groups.end());
    HostGroupData *data = new HostGroupData(this, group);
    interfaceData->groups[group] = data;
    return data;
}

Igmpv2::RouterGroupData *Igmpv2::getRouterGroupData(InterfaceEntry *ie, const Ipv4Address& group)
{
    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);
    auto it = interfaceData->groups.find(group);
    return it != interfaceData->groups.end() ? it->second : nullptr;
}

Igmpv2::HostGroupData *Igmpv2::getHostGroupData(InterfaceEntry *ie, const Ipv4Address& group)
{
    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    auto it = interfaceData->groups.find(group);
    return it != interfaceData->groups.end() ? it->second : nullptr;
}

void Igmpv2::deleteRouterGroupData(InterfaceEntry *ie, const Ipv4Address& group)
{
    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);
    auto it = interfaceData->groups.find(group);
    if (it != interfaceData->groups.end()) {
        RouterGroupData *data = it->second;
        interfaceData->groups.erase(it);
        delete data;
    }
}

void Igmpv2::deleteHostGroupData(InterfaceEntry *ie, const Ipv4Address& group)
{
    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    auto it = interfaceData->groups.find(group);
    if (it != interfaceData->groups.end()) {
        HostGroupData *data = it->second;
        interfaceData->groups.erase(it);
        delete data;
    }
}

// --- Utility Methods for Interface Data ---

Igmpv2::RouterInterfaceData *Igmpv2::getRouterInterfaceData(InterfaceEntry *ie)
{
    int interfaceId = ie->getInterfaceId();
    auto it = routerData.find(interfaceId);
    if (it != routerData.end())
        return it->second;

    // create one
    RouterInterfaceData *data = createRouterInterfaceData();
    routerData[interfaceId] = data;
    return data;
}

Igmpv2::RouterInterfaceData *Igmpv2::createRouterInterfaceData()
{
    return new RouterInterfaceData(this);
}

Igmpv2::HostInterfaceData *Igmpv2::getHostInterfaceData(InterfaceEntry *ie)
{
    int interfaceId = ie->getInterfaceId();
    auto it = hostData.find(interfaceId);
    if (it != hostData.end())
        return it->second;

    // create one
    HostInterfaceData *data = createHostInterfaceData();
    hostData[interfaceId] = data;
    return data;
}

Igmpv2::HostInterfaceData *Igmpv2::createHostInterfaceData()
{
    return new HostInterfaceData(this);
}

// --- HostGroupData structure implementation ---

Igmpv2::HostGroupData::HostGroupData(Igmpv2 *owner, const Ipv4Address& group)
    : owner(owner), groupAddr(group)
{
    ASSERT(owner);
    ASSERT(groupAddr.isMulticast());

    state = IGMP_HGS_NON_MEMBER;
    flag = false;
    timer = nullptr;
}

Igmpv2::HostGroupData::~HostGroupData()
{
    if (timer) {
        delete (IgmpHostTimerContext *)timer->getContextPointer();
        owner->cancelAndDelete(timer);
    }
}

// --- RouterGroupData structure implementation ---

Igmpv2::RouterGroupData::RouterGroupData(Igmpv2 *owner, const Ipv4Address& group)
    : owner(owner), groupAddr(group)
{
    ASSERT(owner);
    ASSERT(groupAddr.isMulticast());

    state = IGMP_RGS_NO_MEMBERS_PRESENT;
    timer = nullptr;
    rexmtTimer = nullptr;
    // v1HostTimer = nullptr;
}

Igmpv2::RouterGroupData::~RouterGroupData()
{
    if (timer) {
        delete (IgmpRouterTimerContext *)timer->getContextPointer();
        owner->cancelAndDelete(timer);
    }
    if (rexmtTimer) {
        delete (IgmpRouterTimerContext *)rexmtTimer->getContextPointer();
        owner->cancelAndDelete(rexmtTimer);
    }
//    if (v1HostTimer)
//    {
//        delete (IgmpRouterTimerContext*)v1HostTimer->getContextPointer();
//        owner->cancelAndDelete(v1HostTimer);
//    }
}

// --- HostInterfaceData structure implementation ---

Igmpv2::HostInterfaceData::HostInterfaceData(Igmpv2 *owner)
    : owner(owner)
{
    ASSERT(owner);
}

Igmpv2::HostInterfaceData::~HostInterfaceData()
{
    for (auto & elem : groups)
        delete elem.second;
}

// --- RouterInterfaceData structure implementation ---

Igmpv2::RouterInterfaceData::RouterInterfaceData(Igmpv2 *owner)
    : owner(owner)
{
    ASSERT(owner);

    igmpRouterState = IGMP_RS_INITIAL;
    igmpQueryTimer = nullptr;
}

Igmpv2::RouterInterfaceData::~RouterInterfaceData()
{
    owner->cancelAndDelete(igmpQueryTimer);

    for (auto & elem : groups)
        delete elem.second;
}

// Miscellaneous

const std::string Igmpv2::getRouterStateString(Igmpv2::RouterState rs)
{
    if(rs == IGMP_RS_INITIAL)
        return "INITIAL";
    else if(rs == IGMP_RS_QUERIER)
        return "QUERIER";
    else if(rs == IGMP_RS_NON_QUERIER)
        return "NON_QUERIER";

    return "UNKNOWN";
}

const std::string Igmpv2::getRouterGroupStateString(Igmpv2::RouterGroupState rgs)
{
    if(rgs == IGMP_RGS_NO_MEMBERS_PRESENT)
        return "NO_MEMBERS_PRESENT";
    else if(rgs == IGMP_RGS_MEMBERS_PRESENT)
        return "MEMBERS_PRESENT";
    else if(rgs == IGMP_RGS_V1_MEMBERS_PRESENT)
        return "V1_MEMBERS_PRESENT";
    else if(rgs == IGMP_RGS_CHECKING_MEMBERSHIP)
        return "CHECKING_MEMBERSHIP";

    return "UNKNOWN";
}

const std::string Igmpv2::getHostGroupStateString(Igmpv2::HostGroupState hgs)
{
    if(hgs == IGMP_HGS_NON_MEMBER)
        return "NON_MEMBER";
    else if(hgs == IGMP_HGS_DELAYING_MEMBER)
        return "DELAYING_MEMBER";
    else if(hgs == IGMP_HGS_IDLE_MEMBER)
        return "IDLE_MEMBER";

    return "UNKNOWN";
}

void Igmpv2::insertCrc(CrcMode crcMode, const Ptr<IgmpMessage>& igmpMsg, Packet *packet)
{
    igmpMsg->setCrcMode(crcMode);
    switch (crcMode) {
        case CRC_DECLARED_CORRECT:
            // if the CRC mode is declared to be correct, then set the CRC to an easily recognizable value
            igmpMsg->setCrc(0xC00D);
            break;
        case CRC_DECLARED_INCORRECT:
            // if the CRC mode is declared to be incorrect, then set the CRC to an easily recognizable value
            igmpMsg->setCrc(0xBAAD);
            break;
        case CRC_COMPUTED: {
            // if the CRC mode is computed, then compute the CRC and set it
            igmpMsg->setCrc(0x0000); // make sure that the CRC is 0 in the header before computing the CRC
            MemoryOutputStream igmpStream;
            Chunk::serialize(igmpStream, igmpMsg);
            Chunk::serialize(igmpStream, packet->peekData(Chunk::PF_ALLOW_EMPTY));
            uint16_t crc = TcpIpChecksum::checksum(igmpStream.getData());
            igmpMsg->setCrc(crc);
            break;
        }
        default:
            throw cRuntimeError("Unknown CRC mode");
    }
}

bool Igmpv2::verifyCrc(const Packet *packet)
{
    const auto& igmpMsg = packet->peekAtFront<IgmpMessage>(b(-1), Chunk::PF_ALLOW_INCORRECT);
    switch (igmpMsg->getCrcMode()) {
        case CRC_DECLARED_CORRECT:
            // if the CRC mode is declared to be correct, then the check passes if and only if the chunks are correct
            return igmpMsg->isCorrect();
        case CRC_DECLARED_INCORRECT:
            // if the CRC mode is declared to be incorrect, then the check fails
            return false;
        case CRC_COMPUTED: {
            // otherwise compute the CRC, the check passes if the result is 0xFFFF (includes the received CRC)
            auto dataBytes = packet->peekDataAsBytes(Chunk::PF_ALLOW_INCORRECT);
            uint16_t crc = TcpIpChecksum::checksum(dataBytes->getBytes());
            // TODO: delete these isCorrect calls, rely on CRC only
            return crc == 0 && igmpMsg->isCorrect();
        }
        default:
            throw cRuntimeError("Unknown CRC mode");
    }
}

}    // namespace inet

