// Copyright (C) 2012 - 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

/**
 * @file IGMPv3.cc
 * @author Adam Malik(mailto:towdie13@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 12.5.2013
 * @brief
 * @detail
 */

#include "IGMPv3.h"
#include "IPv4RoutingTableAccess.h"
#include "InterfaceTableAccess.h"
#include "IPv4ControlInfo.h"
#include "IPv4InterfaceData.h"
#include "NotificationBoard.h"

#include <algorithm>
#include <bitset>

using namespace std;

Define_Module(IGMPv3);

IGMPv3::HostGroupData::HostGroupData(IGMPv3 *owner, const IPv4Address &group)
    : owner(owner), groupAddr(group)
{
    ASSERT(owner);
    ASSERT(groupAddr.isMulticast());

    state = IGMPV3_HGS_NON_MEMBER;
    filter = IGMPV3_FM_INITIAL;
    sourceAddressList.clear();
    timer = NULL;           //timer for group query
    sourceTimer = NULL;     //timer for group and source query
}

IGMPv3::HostGroupData::~HostGroupData()
{
    if(timer)
    {
        delete (IGMPV3HostTimerGroupContext*)timer->getContextPointer();
        owner->cancelAndDelete(timer);
    }
    if(sourceTimer)
    {
        delete (IGMPV3HostTimerSourceContext*)sourceTimer->getContextPointer();
        owner->cancelAndDelete(sourceTimer);
    }
}


IGMPv3::RouterGroupData::RouterGroupData(IGMPv3 *owner, const IPv4Address &group)
    : owner(owner), groupAddr(group)
{
    ASSERT(owner);
    ASSERT(groupAddr.isMulticast());

    state = IGMPV3_RGS_NO_MEMBERS_PRESENT;
    timer = NULL;
    filter = IGMPV3_FM_INITIAL;
    sources.clear();
}

IGMPv3::RouterGroupData::~RouterGroupData()
{
    if(timer)
    {
        delete (IGMPV3RouterTimerContext*)timer->getContextPointer();
        owner->cancelAndDelete(timer);
    }
}

IGMPv3::SourceRecord::SourceRecord(IGMPv3 *owner, const IPv4Address &source)
    : owner(owner), sourceAddr(source)
{
    ASSERT(owner);
    sourceTimer = NULL;
}

IGMPv3::SourceRecord::~SourceRecord()
{
    if(sourceTimer)
    {
        owner->cancelAndDelete(sourceTimer);
    }
}

IGMPv3::HostInterfaceData::HostInterfaceData(IGMPv3 *owner)
    : owner(owner)
{
    ASSERT(owner);

    generalQueryTimer = NULL;
}

IGMPv3::HostInterfaceData::~HostInterfaceData()
{
    for (GroupToHostDataMap::iterator it = groups.begin(); it != groups.end(); ++it)
        delete it->second;
}

IGMPv3::RouterInterfaceData::RouterInterfaceData(IGMPv3 *owner)
    : owner(owner)
{
    ASSERT(owner);

    state = IGMPV3_RS_INITIAL;
    generalQueryTimer = NULL;
}

IGMPv3::RouterInterfaceData::~RouterInterfaceData()
{
    if(generalQueryTimer)
    {
        owner->cancelAndDelete(generalQueryTimer);
    }

    for(GroupToRouterDataMap::iterator it = groups.begin(); it != groups.end(); ++it)
        delete it->second;
}

IGMPv3::HostInterfaceData *IGMPv3::createHostInterfaceData()
{
    return new HostInterfaceData(this);
}

IGMPv3::RouterInterfaceData *IGMPv3::createRouterInterfaceData()
{
    return new RouterInterfaceData(this);
}

IGMPv3::HostGroupData *IGMPv3::createHostGroupData(InterfaceEntry *ie, const IPv4Address &group)
{
    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    ASSERT(interfaceData->groups.find(group) == interfaceData->groups.end());
    HostGroupData *data = new HostGroupData(this, group);
    interfaceData->groups[group] = data;
    return data;
}

IGMPv3::SourceRecord *IGMPv3::createSourceRecord(InterfaceEntry *ie, const IPv4Address &group, const IPv4Address &source)
{
    RouterGroupData *groupData = getRouterGroupData(ie, group);
    ASSERT(groupData->sources.find(source) == groupData->sources.end());
    SourceRecord *record = new SourceRecord(this, source);
    groupData->sources[source] = record;
    return record;
}

IGMPv3::RouterGroupData *IGMPv3::createRouterGroupData(InterfaceEntry *ie, const IPv4Address &group)
{
    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);
    ASSERT(interfaceData->groups.find(group) == interfaceData->groups.end());
    RouterGroupData *data = new RouterGroupData(this, group);
    interfaceData->groups[group] = data;
    return data;
}

IGMPv3::HostInterfaceData *IGMPv3::getHostInterfaceData(InterfaceEntry *ie)
{
    int interfaceId = ie->getInterfaceId();
    InterfaceToHostDataMap::iterator it = hostData.find(interfaceId);
    if(it != hostData.end())
        return it->second;

    HostInterfaceData *data = createHostInterfaceData();
    hostData[interfaceId] = data;
    return data;
}

IGMPv3::RouterInterfaceData *IGMPv3::getRouterInterfaceData(InterfaceEntry *ie)
{
    int interfaceId = ie->getInterfaceId();
    InterfaceToRouterDataMap::iterator it = routerData.find(interfaceId);
    if(it != routerData.end())
        return it->second;

    RouterInterfaceData *data = createRouterInterfaceData();
    routerData[interfaceId] = data;
    return data;
}

IGMPv3::HostGroupData *IGMPv3::getHostGroupData(InterfaceEntry *ie, const IPv4Address &group)
{
    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    GroupToHostDataMap::iterator it = interfaceData->groups.find(group);
    return it != interfaceData->groups.end() ? it->second : NULL;
}

IGMPv3::RouterGroupData *IGMPv3::getRouterGroupData(InterfaceEntry *ie, const IPv4Address &group)
{
    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);
    GroupToRouterDataMap::iterator it = interfaceData->groups.find(group);
    return it != interfaceData->groups.end() ? it->second : NULL;
}

IGMPv3::SourceRecord *IGMPv3::getSourceRecord(InterfaceEntry *ie, const IPv4Address &group, const IPv4Address &source)
{
    RouterGroupData *groupData = getRouterGroupData(ie, group);
    SourceToGroupDataMap::iterator it = groupData->sources.find(source);
    return it != groupData->sources.end() ? it->second : NULL;
}

void IGMPv3::deleteHostInterfaceData(int interfaceId)
{
    InterfaceToHostDataMap::iterator interfaceIt = hostData.find(interfaceId);
    if(interfaceIt != hostData.end())
    {
        HostInterfaceData *interface = interfaceIt->second;
        hostData.erase(interfaceIt);
        delete interface;
    }
}

void IGMPv3::deleteRouterInterfaceData(int interfaceId)
{
    InterfaceToRouterDataMap::iterator interfaceIt = routerData.find(interfaceId);
    if(interfaceIt != routerData.end())
    {
        RouterInterfaceData *interface = interfaceIt->second;
        routerData.erase(interfaceIt);
        delete interface;
    }
}

void IGMPv3::deleteHostGroupData(InterfaceEntry *ie, const IPv4Address &group)
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

void IGMPv3::deleteRouterGroupData(InterfaceEntry *ie, const IPv4Address &group)
{
    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);
    GroupToRouterDataMap::iterator it = interfaceData->groups.find(group);
    if(it != interfaceData->groups.end())
    {
        RouterGroupData *data = it->second;
        interfaceData->groups.erase(it);
        delete data;
    }
}

void IGMPv3::deleteSourceRecord(InterfaceEntry *ie, const IPv4Address &group, const IPv4Address &source)
{
    RouterGroupData *groupData = getRouterGroupData(ie, group);
    SourceToGroupDataMap::iterator it = groupData->sources.find(source);
    if(it != groupData->sources.end())
    {
        SourceRecord *record = it->second;
        groupData->sources.erase(it);
        delete record;
    }
}



void IGMPv3::initialize(int stage)
{
    if (stage == 0)
    {
        ift = InterfaceTableAccess().get();
        rt = IPv4RoutingTableAccess().get();
        nb = NotificationBoardAccess().get();

        nb->subscribe(this, NF_INTERFACE_DELETED);
        nb->subscribe(this, NF_IPv4_MCAST_JOIN);
        nb->subscribe(this, NF_IPv4_MCAST_LEAVE);

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
        lastMemberQueryTime = lastMemberQueryInterval * lastMemberQueryCount;   //todo checknut ci je to takto..
        unsolicitedReportInterval = par("unsolicitedReportInterval");

        numGroups = 0;
        numHostGroups = 0;
        numRouterGroups = 0;

        numQueriesSent = 0;
        numQueriesRecv = 0;
        numGeneralQueriesSent = 0;
        numGeneralQueriesRecv = 0;
        numGroupSpecificQueriesSent = 0;
        numGroupSpecificQueriesRecv = 0;
        numGroupAndSourceSpecificQueriesSent = 0;
        numGroupAndSourceSpecificQueriesRecv = 0;
        numReportsSent = 0;
        numReportsRecv = 0;

        WATCH(numGroups);
        WATCH(numHostGroups);
        WATCH(numRouterGroups);

        WATCH(numQueriesSent);
        WATCH(numQueriesRecv);
        WATCH(numGeneralQueriesSent);
        WATCH(numGeneralQueriesRecv);
        WATCH(numGroupSpecificQueriesSent);
        WATCH(numGroupSpecificQueriesRecv);
        WATCH(numGroupAndSourceSpecificQueriesSent);
        WATCH(numGroupAndSourceSpecificQueriesRecv);
        WATCH(numReportsSent);
        WATCH(numReportsRecv);

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
    }
    else if (stage == 2) // ipv4Data() created in stage 1
    {
        // in multicast routers: join to ALL_IGMPv3_ROUTERS_MCAST address on all interfaces
        if (enabled && rt->isMulticastForwardingEnabled())
        {
            for (int i = 0; i < (int)ift->getNumInterfaces(); ++i)
            {
                InterfaceEntry *ie = ift->getInterface(i);
                if (ie->isMulticast())
                        ie->ipv4Data()->joinMulticastGroup(IPv4Address::ALL_IGMPV3_ROUTERS_MCAST);
            }
        }
    }
}

IGMPv3::~IGMPv3()
{
    while (!hostData.empty())
        deleteHostInterfaceData(hostData.begin()->first);
    while (!routerData.empty())
        deleteRouterInterfaceData(routerData.begin()->first);
}

void IGMPv3::receiveChangeNotification(int category, const cPolymorphic *details)
{
    Enter_Method_Silent();

    InterfaceEntry *ie;
    int interfaceId;
    const IPv4MulticastGroupInfo *info;
    switch (category)
    {
        case NF_INTERFACE_CREATED:
            ie = const_cast<InterfaceEntry*>(check_and_cast<const InterfaceEntry*>(details));
            if (ie->isMulticast())
                configureInterface(ie);
            break;
        case NF_INTERFACE_DELETED:
            ie = const_cast<InterfaceEntry*>(check_and_cast<const InterfaceEntry*>(details));
            if (ie->isMulticast())
            {
                interfaceId = ie->getInterfaceId();
                deleteHostInterfaceData(interfaceId);
                deleteRouterInterfaceData(interfaceId);
            }
            break;
        case NF_IPv4_MCAST_JOIN:
            info = check_and_cast<const IPv4MulticastGroupInfo*>(details);
            multicastGroupJoined(info->ie, info->groupAddress);
            break;
        case NF_IPv4_MCAST_LEAVE:
            info = check_and_cast<const IPv4MulticastGroupInfo*>(details);
            multicastGroupLeft(info->ie, info->groupAddress);
            break;
    }
}

void IGMPv3::configureInterface(InterfaceEntry *ie)
{
    if (enabled && rt->isMulticastForwardingEnabled())
    {
        // start querier on this interface
        cMessage *timer = new cMessage("IGMPv3 General Query timer", IGMPV3_R_GENERAL_QUERY_TIMER);
        std::vector<IPv4Address> sources;
        sources.clear();
        timer->setContextPointer(ie);
        RouterInterfaceData *routerData = getRouterInterfaceData(ie);
        routerData->generalQueryTimer = timer;
        routerData->state = IGMPV3_RS_QUERIER;
        sendQuery(ie, IPv4Address(), sources, queryResponseInterval); // general query
        startTimer(timer, startupQueryInterval);
    }
}

void IGMPv3::handleMessage(cMessage *msg)
{
    if (!enabled)
    {
        if (!msg->isSelfMessage())
        {
            EV << "IGMPv3 disabled, dropping packet.\n";
            delete msg;
        }
        return;
    }

    if (msg->isSelfMessage())
    {
        switch (msg->getKind())
        {
            case IGMPV3_R_GENERAL_QUERY_TIMER:
                processRouterGeneralQueryTimer(msg);
                break;
            case IGMPV3_R_GROUP_TIMER:
                processRouterGroupTimer(msg);
                break;
            case IGMPV3_R_SOURCE_TIMER:
                processRouterSourceTimer(msg);
                break;
            case IGMPV3_H_GENERAL_QUERY_TIMER:
                processHostGeneralQueryTimer(msg);
                break;
            case IGMPV3_H_GROUP_TIMER:
                processHostGroupQueryTimer(msg);
                break;
            default:
                ASSERT(false);
                break;
        }
    }
    else if (dynamic_cast<IGMPv3Query *>(msg))
        processQuery((IGMPv3Query *)msg);
    else if (dynamic_cast<IGMPv3Report *>(msg))
        processReport((IGMPv3Report *)msg);
    else if (dynamic_cast<SocketMessage *>(msg))
        processSocketChange((SocketMessage *)msg);
    else
        ASSERT(false);
}

void IGMPv3::multicastGroupJoined(InterfaceEntry *ie, const IPv4Address& groupAddr)
{
    ASSERT(ie && ie->isMulticast());
    ASSERT(groupAddr.isMulticast());

    if (enabled && !groupAddr.isLinkLocalMulticast())
    {
        HostGroupData *groupData = createHostGroupData(ie, groupAddr);
        numGroups++;
        numHostGroups++;
        std::vector<IPv4Address> sources = std::vector<IPv4Address>();
        sendGroupReport(ie, groupData->groupAddr, IGMPV3_RT_IS_EX, sources);    //sending Join Report message
        groupData->state = IGMPV3_HGS_DELAYING_MEMBER;
    }
}

void IGMPv3::multicastGroupLeft(InterfaceEntry *ie, const IPv4Address& groupAddr)
{
    ASSERT(ie && ie->isMulticast());
    ASSERT(groupAddr.isMulticast());

    if (enabled && !groupAddr.isLinkLocalMulticast())
    {
        HostGroupData *groupData = getHostGroupData(ie, groupAddr);
        if (groupData)
        {
            if (groupData->timer->isScheduled())
                cancelEvent(groupData->timer);
        }
        deleteHostGroupData(ie, groupAddr);
        numHostGroups--;
        numGroups--;
    }
}

void IGMPv3::startTimer(cMessage *timer, double interval)
{
    ASSERT(timer);
    cancelEvent(timer);
    scheduleAt(simTime() + interval, timer);
}

void IGMPv3::startHostGeneralTimer(InterfaceEntry *ie, HostInterfaceData* ifc, double maxRespTime)
{
    if (!ifc->generalQueryTimer)
    {
        ifc->generalQueryTimer = new cMessage("IGMPv3 Host General timer", IGMPV3_H_GENERAL_QUERY_TIMER);
    }

    double delay = uniform(0.0, maxRespTime);
    EV << "setting host General timer for " << ie->getName() << " to " << delay << "\n";
    startTimer(ifc->generalQueryTimer, delay);
}

void IGMPv3::startHostGroupTimer(InterfaceEntry *ie, HostGroupData* group, double maxRespTime)
{
    if (!group->timer)
    {
        group->timer = new cMessage("IGMPv3 Host Group timer", IGMPV3_H_GROUP_TIMER);
        group->timer->setContextPointer(new IGMPV3HostTimerGroupContext(ie, group));
    }

    double delay = uniform(0.0, maxRespTime);
    EV << "setting host timer for " << ie->getName() << " and group " << group->groupAddr.str() << " to " << delay << "\n";
    startTimer(group->timer, delay);
}

void IGMPv3::startHostSourceTimer(InterfaceEntry *ie, HostGroupData* group, double maxRespTime, std::vector<IPv4Address> sourceList)
{
    if (!group->timer)
    {
        group->timer = new cMessage("IGMPv3 Host Group and Source timer", IGMPV3_H_SOURCE_TIMER);
        group->timer->setContextPointer(new IGMPV3HostTimerSourceContext(ie, group, sourceList));
    }

    double delay = uniform(0.0, maxRespTime);
    EV << "setting host timer for " << ie->getName() << " and group " << group->groupAddr.str() << "and source to " << delay << "\n";
    startTimer(group->timer, delay);
}

void IGMPv3::sendQuery(InterfaceEntry *ie, const IPv4Address& groupAddr,std::vector<IPv4Address> sources, double maxRespTime)
{
    ASSERT(groupAddr.isUnspecified() || (groupAddr.isMulticast() && !groupAddr.isLinkLocalMulticast()));

    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);

    if (interfaceData->state == IGMPV3_RS_QUERIER)
    {
        if (groupAddr.isUnspecified() && sources.empty())
            EV << "IGMPv3: sending General Membership Query on iface=" << ie->getName() << "\n";
        else if(!groupAddr.isUnspecified() && sources.empty())
            EV << "IGMPv3: sending Group Specific Membership Query for group=" << groupAddr << " on iface=" << ie->getName() << "\n";
        else
            EV << "IGMPv3: sending Group and Source Specific Membership Query for group=" << groupAddr << " on iface=" << ie->getName() << "\n";

        IGMPv3Query *msg = new IGMPv3Query("IGMPv3 query");
        msg->setType(IGMP_MEMBERSHIP_QUERY);
        msg->setGroupAddress(groupAddr);
        msg->setMaxRespCode((int)(maxRespTime * 10.0));
        msg->setNumOfSources(sources.size());
        msg->setSourceList(sources);
        msg->setByteLength(12 + (4 * sources.size()));
        sendQueryToIP(msg, ie, groupAddr.isUnspecified() ? IPv4Address::ALL_HOSTS_MCAST : groupAddr);

        numQueriesSent++;
        if (groupAddr.isUnspecified() && sources.empty())
            numGeneralQueriesSent++;
        else if (!groupAddr.isUnspecified() && sources.empty())
            numGroupSpecificQueriesSent++;
        else
            numGroupAndSourceSpecificQueriesSent++;
    }
}

void IGMPv3::processRouterGeneralQueryTimer(cMessage *msg)
{
    InterfaceEntry *ie = (InterfaceEntry*)msg->getContextPointer();
    ASSERT(ie);
    EV << "IGMPv3: Router General Query timer expired, iface=" << ie->getName() << "\n";
    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);
    RouterState state = interfaceData->state;
    std::vector<IPv4Address> sources;
    sources.clear();
    if (state == IGMPV3_RS_QUERIER || state == IGMPV3_RS_NON_QUERIER)
    {
        interfaceData->state = IGMPV3_RS_QUERIER;
        sendQuery(ie, IPv4Address(), sources, queryResponseInterval);
        startTimer(msg, queryInterval);
    }
}

void IGMPv3::sendReportToIP(IGMPv3Report *msg, InterfaceEntry *ie, const IPv4Address& dest)
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

void IGMPv3::sendQueryToIP(IGMPv3Query *msg, InterfaceEntry *ie, const IPv4Address& dest)
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

void IGMPv3::processHostGeneralQueryTimer(cMessage *msg)
{
    IGMPV3HostGeneralTimerContext *ctx = (IGMPV3HostGeneralTimerContext*)msg->getContextPointer();
    InterfaceEntry *ie = ctx->ie;
    HostInterfaceData *interfaceData = ctx->interfaceData;
    ASSERT(ie);
    //HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    IGMPv3Report *report = new IGMPv3Report("IGMPv3 report");
    report->setType(IGMPV3_MEMBERSHIP_REPORT);
    int counter = 0;
    report->setGroupRecordArraySize(interfaceData->groups.size());
    /*
     * creating GroupRecord for each group on interface
     */
    for(GroupToHostDataMap::iterator it = interfaceData->groups.begin(); it != interfaceData->groups.end(); ++it)
    {
        GroupRecord gr;
        if(it->second->filter == IGMPV3_FM_INITIAL)
        {
            gr.recordType = IGMPV3_RT_IS_EX;
        }
        if(it->second->filter == IGMPV3_FM_INCLUDE)
        {
            gr.recordType = IGMPV3_RT_IS_IN;
        }
        else if(it->second->filter == IGMPV3_FM_EXCLUDE)
        {
            gr.recordType = IGMPV3_RT_IS_EX;
        }
        gr.groupAddress = it->second->groupAddr;
        gr.numOfSources = it->second->sourceAddressList.size();
        gr.SourceList = IGMPv3::IpVector();
        gr.SourceList = it->second->sourceAddressList;
        report->setGroupRecord(counter++,gr);
    }
    if(counter != 0)    //if no record created, dont need to send report
    {
        report->setNumGroupRecords(counter);
        EV << "IGMPv3: Host General Query Timer expired on iface=" << ie->getName() << "\n";
        sendReportToIP(report, ie, IPv4Address::ALL_IGMPV3_ROUTERS_MCAST);
        numReportsSent++;
    }
}

void IGMPv3::processHostGroupQueryTimer(cMessage *msg)
{
    IGMPV3HostTimerSourceContext *ctx = (IGMPV3HostTimerSourceContext*)msg->getContextPointer();
    EV << "IGMPv3: Host Group Timer expired for group=" << ctx->hostGroup->groupAddr << " on iface=" << ctx->ie->getName() << "\n";
    //checking if query is group or group-and-source specific
    if(ctx->sourceList.empty())
    {
        if(ctx->hostGroup->filter == IGMPV3_FM_EXCLUDE)
        {
            sendGroupReport(ctx->ie, ctx->hostGroup->groupAddr, IGMPV3_RT_IS_EX, ctx->hostGroup->sourceAddressList);
        }
        else if(ctx->hostGroup->filter == IGMPV3_FM_INCLUDE)
        {
            sendGroupReport(ctx->ie, ctx->hostGroup->groupAddr, IGMPV3_RT_IS_IN, ctx->hostGroup->sourceAddressList);
        }
    }
    else
    {
        if(ctx->hostGroup->filter == IGMPV3_FM_INCLUDE)
        {
            sendGroupReport(ctx->ie, ctx->hostGroup->groupAddr, IGMPV3_RT_IS_IN, IpIntersection(ctx->hostGroup->sourceAddressList, ctx->sourceList));
        }
        else if(ctx->hostGroup->filter == IGMPV3_FM_EXCLUDE)
        {
            sendGroupReport(ctx->ie, ctx->hostGroup->groupAddr, IGMPV3_RT_IS_IN, IpComplement(ctx->sourceList, ctx->hostGroup->sourceAddressList));
        }
    }

    ctx->sourceList.clear();
}
/**
 * Function for processing data from future Multicast application
 * This function is sending report message if interface state was changed.
 */
void IGMPv3::processSocketChange(SocketMessage *msg)
{
    InterfaceEntry *ie = (InterfaceEntry*)msg->getContextPointer();
    ASSERT(ie);
    EV << "IGMPv3: Host received socket change on iface=" << ie->getName() << "\n";
    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    GroupToHostDataMap::iterator it = interfaceData->groups.find(msg->getGroupAddr());
    HostGroupData *groupData = it->second;
    FilterMode filter;
    if(msg->getFilter())
        filter = IGMPV3_FM_INCLUDE;
    else
        filter = IGMPV3_FM_EXCLUDE;

    //Chec if IF state is different
    if(!(groupData->filter == filter) || !(groupData->sourceAddressList == msg->getSourceAddressList()))
    {
        //OldState: INCLUDE(A) NewState: INCLUDE(B) StateChangeRecordSent: ALLOW(B-A) BLOCK(A-B)
        if(groupData->filter == IGMPV3_FM_INCLUDE && filter == IGMPV3_FM_INCLUDE && groupData->sourceAddressList != msg->getSourceAddressList())
        {
            sendGroupReport(ie, msg->getGroupAddr(), IGMPV3_RT_ALLOW, IpComplement(msg->getSourceAddressList(), groupData->sourceAddressList));
            sendGroupReport(ie, msg->getGroupAddr(), IGMPV3_RT_BLOCK, IpComplement(groupData->sourceAddressList, msg->getSourceAddressList()));
            groupData->sourceAddressList = msg->getSourceAddressList();
        }
        else if(groupData->filter == IGMPV3_FM_EXCLUDE && filter == IGMPV3_FM_EXCLUDE && groupData->sourceAddressList != msg->getSourceAddressList())
        {
            sendGroupReport(ie, msg->getGroupAddr(), IGMPV3_RT_ALLOW, IpComplement(groupData->sourceAddressList, msg->getSourceAddressList()));
            sendGroupReport(ie, msg->getGroupAddr(), IGMPV3_RT_BLOCK, IpComplement(msg->getSourceAddressList(), groupData->sourceAddressList));
            groupData->sourceAddressList = msg->getSourceAddressList();
        }
        else if(groupData->filter == IGMPV3_FM_INCLUDE && filter == IGMPV3_FM_EXCLUDE)
        {
            sendGroupReport(ie, msg->getGroupAddr(), IGMPV3_RT_TO_EX, msg->getSourceAddressList());
            groupData->filter = filter;
            groupData->sourceAddressList = msg->getSourceAddressList();
        }
        else if(groupData->filter == IGMPV3_FM_EXCLUDE && filter == IGMPV3_FM_INCLUDE)
        {
            sendGroupReport(ie, msg->getGroupAddr(), IGMPV3_RT_TO_IN, msg->getSourceAddressList());
            groupData->filter = filter;
            groupData->sourceAddressList = msg->getSourceAddressList();
        }
    }

    delete msg;
}

void IGMPv3::processQuery(IGMPv3Query *msg)
{
    IPv4ControlInfo *controlInfo = (IPv4ControlInfo *)msg->getControlInfo();
    InterfaceEntry *ie = ift->getInterfaceById(controlInfo->getInterfaceId());

    ASSERT(ie->isMulticast());

    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    if(!interfaceData->generalQueryTimer)
    {
        interfaceData->generalQueryTimer = new cMessage("IGMPv3 Host General Timer", IGMPV3_H_GENERAL_QUERY_TIMER);
        interfaceData->generalQueryTimer->setContextPointer(new IGMPV3HostGeneralTimerContext(ie, interfaceData));
    }

    numQueriesRecv++;

    IPv4Address &groupAddr = msg->getGroupAddress();
    IpVector sources = msg->getSourceList();

    double maxRespTime = countMaxResponseTime((double)msg->getMaxRespCode()) /10.0;
    double delay = uniform(0.0, maxRespTime);



    // Rules from RFC page 22
    if(interfaceData->generalQueryTimer->isScheduled() && interfaceData->generalQueryTimer->getArrivalTime() < simTime() + delay)
    {
        //rule 1.
    }
    else if(groupAddr.isUnspecified() && msg->getSourceList().empty())
    {
        //rule 2.
        if(interfaceData->generalQueryTimer->isScheduled())
        {
            cancelEvent(interfaceData->generalQueryTimer);
        }
        scheduleAt(simTime() + delay, interfaceData->generalQueryTimer);
    }
    if(!groupAddr.isUnspecified())
    {
        HostGroupData *groupData = getHostGroupData(ie, groupAddr);
        if(!groupData->timer)
        {
            groupData->timer = new cMessage("IGMPv3 Host Group Timer", IGMPV3_H_GROUP_TIMER);
        }
        else if(!groupAddr.isUnspecified() && !groupData->timer->isScheduled())
        {
            //rule 3.
            groupData->timer->setContextPointer(new IGMPV3HostTimerSourceContext(ie, groupData, msg->getSourceList()));
            scheduleAt(simTime() + delay, groupData->timer);
        }
        else if(!groupAddr.isUnspecified() && groupData->timer->isScheduled())
        {
            IGMPV3HostTimerSourceContext *ctx = (IGMPV3HostTimerSourceContext*)groupData->timer->getContextPointer();
            //rule 4.
            if(msg->getSourceList().empty())
            {
                groupData->timer->setContextPointer(new IGMPV3HostTimerSourceContext(ie, groupData, msg->getSourceList()));
                if(groupData->timer->getArrivalTime() > simTime() + delay)
                {
                    cancelEvent(groupData->timer);
                    scheduleAt(simTime() + delay, groupData->timer);
                }
            }
            else
            {
                //rule 5.
                if(groupData->timer->getArrivalTime() > simTime() + delay)
                {
                    IGMPv3::IpVector combinedSources;
                    combinedSources.reserve(msg->getSourceList().size() + ctx->sourceList.size());
                    combinedSources.insert(combinedSources.end(), msg->getSourceList().begin(), msg->getSourceList().end());
                    combinedSources.insert(combinedSources.end(),ctx->sourceList.begin(),ctx->sourceList.end());
                    cancelEvent(groupData->timer);
                    scheduleAt(simTime() + delay, groupData->timer);
                }
            }
        }
    }

// Router part | Querier Election
    if (rt->isMulticastForwardingEnabled())
    {

        //Querier Election
        RouterInterfaceData *routerInterfaceData = getRouterInterfaceData(ie);
        if (controlInfo->getSrcAddr() < ie->ipv4Data()->getIPAddress())
        {
            startTimer(routerInterfaceData->generalQueryTimer, otherQuerierPresentInterval);
            routerInterfaceData->state = IGMPV3_RS_NON_QUERIER;
        }

        if (!groupAddr.isUnspecified() && routerInterfaceData->state == IGMPV3_RS_NON_QUERIER) // group specific query
        {
            RouterGroupData *groupData = getRouterGroupData(ie, groupAddr);
            if (groupData->state == IGMPV3_RGS_MEMBERS_PRESENT)
            {
                double maxResponseTime = maxRespTime;
                startTimer(groupData->timer, maxResponseTime * lastMemberQueryCount);
                groupData->state = IGMPV3_RGS_CHECKING_MEMBERSHIP;
            }
        }
    }
    delete msg;
}

double IGMPv3::countMaxResponseTime(double maxRespCode)
{
    double maxRespTime;
    if(maxRespCode < 128)
        maxRespTime = maxRespCode;
    else
    {
        unsigned long mantis;
        unsigned long exp;

        bitset<8> bvalue((unsigned long)maxRespCode);
        bitset<8> bmantis(string("00001111"));
        bitset<8> bexp(string("01110000"));
        bmantis = bmantis & bvalue;
        mantis = bmantis.to_ulong();
        bexp = bexp & bvalue;
        bexp>>3;
        exp = bexp.to_ulong();
        maxRespTime =  (double)mantis * pow(10,(double)exp);
    }
    return maxRespTime;
}

IGMPv3::IpVector IGMPv3::IpComplement(IGMPv3::IpVector first, IGMPv3::IpVector second)
{
    IGMPv3::IpVector complement;
    IGMPv3::IpVector::iterator it;

    std::sort(first.begin(), first.end());
    std::sort(second.begin(), second.end());

    it = set_difference(first.begin(), first.end(), second.begin(), second.end(), complement.begin());
    complement.resize(it-complement.begin());
    return complement;
}

IGMPv3::IpVector IGMPv3::IpIntersection(IGMPv3::IpVector first, IGMPv3::IpVector second)
{
    IGMPv3::IpVector intersection;
    IGMPv3::IpVector::iterator it;

    std::sort(first.begin(), first.end());
    std::sort(second.begin(), second.end());

    it = set_intersection(first.begin(), first.end(), second.begin(), second.end(), intersection.begin());
    intersection.resize(it-intersection.begin());
    return intersection;
}

IGMPv3::IpVector IGMPv3::IpUnion(IGMPv3::IpVector first, IGMPv3::IpVector second)
{
    IGMPv3::IpVector intersection;
    IGMPv3::IpVector::iterator it;

    std::sort(first.begin(), first.end());
    std::sort(second.begin(), second.end());

    it = set_union(first.begin(), first.end(), second.begin(), second.end(), intersection.begin());
    intersection.resize(it-intersection.begin());
    return intersection;
}


void IGMPv3::sendGroupReport(InterfaceEntry *ie, IPv4Address group, ReportType type, IpVector sources)
{
    ASSERT(group.isMulticast() && !group.isLinkLocalMulticast());

    EV << "IGMPv3: sending Membership Report for group=" << group << " on iface=" << ie->getName() << "\n";
    IGMPv3Report *msg = new IGMPv3Report("IGMPv3 report");
    msg->setType(IGMPV3_MEMBERSHIP_REPORT);
    GroupRecord gr;
    gr.SourceList = IGMPv3::IpVector();
    gr.SourceList = sources;
    gr.groupAddress = group;
    gr.numOfSources = sources.size();
    gr.recordType = type;
    msg->setGroupRecordArraySize(1);
    msg->setGroupRecord(0,gr);
    msg->setNumGroupRecords(1);
    sendReportToIP(msg, ie, group);
    numReportsSent++;
}

void IGMPv3::processReport(IGMPv3Report *msg)
{
    IPv4ControlInfo *controlInfo = (IPv4ControlInfo *)msg->getControlInfo();
    InterfaceEntry *ie = ift->getInterfaceById(controlInfo->getInterfaceId());
    ASSERT(ie->isMulticast());

    EV << "IGMPv3: received Membership Report on iface=" << ie->getName() << "\n";

    numReportsRecv++;

    if (rt->isMulticastForwardingEnabled())
    {
        for(int i = 0; i < msg->getNumGroupRecords(); i++)
        {
            GroupRecord gr = msg->getGroupRecord(i);
            RouterGroupData *groupData = getRouterGroupData(ie, gr.groupAddress);
            if(!groupData)
            {
                groupData = createRouterGroupData(ie, gr.groupAddress);
                groupData->filter = IGMPV3_FM_EXCLUDE;
                // notify IPv4InterfaceData to update its listener list
                ie->ipv4Data()->addMulticastListener(groupData->groupAddr);
                // notify routing
                IPv4MulticastGroupInfo info(ie, groupData->groupAddr);
                nb->fireChangeNotification(NF_IPv4_MCAST_REGISTERED, &info);
                numRouterGroups++;
                numGroups++;
            }


            //Current State Record part
            if(gr.recordType == IGMPV3_RT_IS_IN)
            {
                if(groupData->filter == IGMPV3_FM_INCLUDE)
                {
                    for(IGMPv3::IpVector::iterator it = gr.SourceList.begin(); it != gr.SourceList.end(); ++it)
                    {
                        SourceRecord *record = getSourceRecord(ie, groupData->groupAddr, *it);
                        if(!record)
                        {
                            record = createSourceRecord(ie, groupData->groupAddr, *it);
                            cMessage *timer = new cMessage("IGMPv3 router source timer", IGMPV3_R_SOURCE_TIMER);
                            timer->setContextPointer(new IGMPV3RouterSourceTimerContext(ie, groupData, *it));
                            record->sourceTimer = timer;
                        }
                            startTimer(record->sourceTimer, groupMembershipInterval);
                    }
                }
                else if(groupData->filter == IGMPV3_FM_EXCLUDE)
                {
                    for(IGMPv3::IpVector::iterator it = gr.SourceList.begin(); it != gr.SourceList.end(); ++it)
                    {
                        SourceRecord *record = getSourceRecord(ie, groupData->groupAddr, *it);
                        if(!record)
                        {
                            record = createSourceRecord(ie, groupData->groupAddr, *it);
                            cMessage *timer = new cMessage("IGMPv3 router source timer", IGMPV3_R_SOURCE_TIMER);
                            timer->setContextPointer(new IGMPV3RouterSourceTimerContext(ie, groupData, *it));
                            record->sourceTimer = timer;
                        }
                        startTimer(record->sourceTimer, groupMembershipInterval);
                    }
                }
            }
            else if(gr.recordType == IGMPV3_RT_IS_EX)
            {
                if(groupData->filter == IGMPV3_FM_INCLUDE)
                {
                    //grouptimer = gmi
                    if(!groupData->timer)
                    {
                        cMessage *timer = new cMessage("IGMPv3 router group timer", IGMPV3_R_GROUP_TIMER);
                        timer->setContextPointer(new IGMPV3RouterTimerContext(ie,groupData));
                        groupData->timer = timer;
                    }
                    startTimer(groupData->timer, groupMembershipInterval);
                    //change to mode exclude
                    groupData->filter = IGMPV3_FM_EXCLUDE;
                    //A*B, B-A ; B-A=0
                    for(IGMPv3::IpVector::iterator it = gr.SourceList.begin(); it != gr.SourceList.end(); ++ it)
                    {
                        SourceRecord *record = getSourceRecord(ie, groupData->groupAddr, *it);
                        if(!record)
                        {
                            record = createSourceRecord(ie, groupData->groupAddr, *it);
                            cMessage *timer = new cMessage("IGMPv3 router source timer", IGMPV3_R_SOURCE_TIMER);
                            timer->setContextPointer(new IGMPV3RouterSourceTimerContext(ie, groupData, *it));
                            record->sourceTimer = timer;
                        }
                    }
                    //delete A-B
                    for(SourceToGroupDataMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        //IGMPv3::IpVector::iterator itIp;
                        if(std::find(gr.SourceList.begin(), gr.SourceList.end(), it->first) == gr.SourceList.end())
                        {
                            deleteSourceRecord(ie, groupData->groupAddr, it->first);
                        }
                    }


                }
                else if(groupData->filter == IGMPV3_FM_EXCLUDE)
                {
                    //grouptimer = gmi
                    if(!groupData->timer)
                    {
                        cMessage *timer = new cMessage("IGMPv3 router group timer", IGMPV3_R_GROUP_TIMER);
                        timer->setContextPointer(new IGMPV3RouterTimerContext(ie, groupData));
                        groupData->timer = timer;
                    }
                    startTimer(groupData->timer, groupMembershipInterval);

                    //Delete X-A
                    //Delete Y-A
                    for(SourceToGroupDataMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        if(it->second->sourceTimer->isScheduled())
                        {
                            if(std::find(gr.SourceList.begin(), gr.SourceList.end(), it->first)== gr.SourceList.end())
                            {
                                deleteSourceRecord(ie, groupData->groupAddr, it->first);
                            }
                        }
                        else
                        {
                            if(std::find(gr.SourceList.begin(), gr.SourceList.end(), it->first)==gr.SourceList.end())
                            {
                                deleteSourceRecord(ie, groupData->groupAddr, it->first);
                            }
                        }

                    }
                    //A-X-Y = GMI
                    for(IGMPv3::IpVector::iterator it = gr.SourceList.begin(); it != gr.SourceList.end(); ++it)
                    {
                        SourceRecord *record = getSourceRecord(ie, groupData->groupAddr, *it);
                        if(!record)
                        {
                            record = createSourceRecord(ie, groupData->groupAddr, *it);
                            cMessage *timer = new cMessage("IGMPv3 router source timer", IGMPV3_R_SOURCE_TIMER);
                            timer->setContextPointer(new IGMPV3RouterSourceTimerContext(ie, groupData, *it));
                            record->sourceTimer = timer;
                            startTimer(record->sourceTimer, groupMembershipInterval);
                        }
                    }
                }
            }
            //Filter Mode change and Source List change part
            else if(gr.recordType == IGMPV3_RT_ALLOW)
            {
                if(groupData->filter == IGMPV3_FM_INCLUDE)
                {
                    //A+B; B=gmi
                    for(IGMPv3::IpVector::iterator it = gr.SourceList.begin(); it != gr.SourceList.end(); ++it)
                    {
                        SourceRecord *record = getSourceRecord(ie, groupData->groupAddr, *it);
                        if(!record)
                        {
                            record = createSourceRecord(ie, groupData->groupAddr, *it);     //todo neviem ci to takto moze byt
                            cMessage *timer = new cMessage("IGMPv3 router source timer", IGMPV3_R_SOURCE_TIMER);
                            timer->setContextPointer(new IGMPV3RouterSourceTimerContext(ie, groupData, *it));
                            record->sourceTimer = timer;
                        }
                            startTimer(record->sourceTimer, groupMembershipInterval);
                    }
                }
                else if(groupData->filter == IGMPV3_FM_EXCLUDE)
                {
                    //Exclude X+A,Y-A ; A=gmi
                    for(IGMPv3::IpVector::iterator it = gr.SourceList.begin(); it != gr.SourceList.end(); ++it)
                    {
                        SourceRecord *record = getSourceRecord(ie, groupData->groupAddr, *it);
                        if(!record)
                        {
                            record = createSourceRecord(ie, groupData->groupAddr, *it);
                            cMessage *timer = new cMessage("IGMPv3 router source timer", IGMPV3_R_SOURCE_TIMER);
                            timer->setContextPointer(new IGMPV3RouterSourceTimerContext(ie, groupData, *it));
                            record->sourceTimer = timer;
                        }
                        startTimer(record->sourceTimer, groupMembershipInterval);
                    }
                }
            }
            else if(gr.recordType == IGMPV3_RT_BLOCK)
            {
                if(groupData->filter == IGMPV3_FM_INCLUDE)
                {
                    //send q(G, A*B)
                    //include A
                    IGMPv3::IpVector mapSources;
                    for(SourceToGroupDataMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        mapSources.push_back(it->first);
                    }
                    sendQuery(ie, groupData->groupAddr, IpIntersection(mapSources,gr.SourceList), queryResponseInterval);
                }
                else if(groupData->filter == IGMPV3_FM_EXCLUDE)
                {
                    //Exclude x+(a-y), y to je pokial to necham tak jak to je a vytvorim ten jeden dole
                    //A-X-Y=GroupTimer
                    for(SourceToGroupDataMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        SourceRecord *record = getSourceRecord(ie,groupData->groupAddr, it->first);
                        if(!record)
                        {
                            record = createSourceRecord(ie, groupData->groupAddr, it->first);
                            cMessage *timer = new cMessage("IGMPv3 router source timer", IGMPV3_R_SOURCE_TIMER);
                            timer->setContextPointer(new IGMPV3RouterSourceTimerContext(ie, groupData, it->first));
                            record->sourceTimer = timer;
                            double grouptimertime = groupData->timer->getArrivalTime().dbl() - simTime().dbl();
                            startTimer(record->sourceTimer, grouptimertime);
                        }
                    }
                    //Send q(G,A-Y)
                    IGMPv3::IpVector mapSourcesY;
                    for(SourceToGroupDataMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        if(!it->second->sourceTimer->isScheduled())
                        {
                            mapSourcesY.push_back(it->first);
                        }
                    }
                    sendQuery(ie, groupData->groupAddr, IpComplement(gr.SourceList, mapSourcesY), queryResponseInterval);
                }
            }
            else if(gr.recordType == IGMPV3_RT_TO_IN)
            {
                if(groupData->filter == IGMPV3_FM_INCLUDE)
                {
                    //A+B; B=gmi
                    for(IGMPv3::IpVector::iterator it = gr.SourceList.begin(); it != gr.SourceList.end(); ++it)
                    {
                        SourceRecord *record = getSourceRecord(ie, groupData->groupAddr, *it);
                        if(!record)
                        {
                            record = createSourceRecord(ie, groupData->groupAddr, *it);     //todo neviem ci to takto moze byt
                            cMessage *timer = new cMessage("IGMPv3 router source timer", IGMPV3_R_SOURCE_TIMER);
                            timer->setContextPointer(new IGMPV3RouterSourceTimerContext(ie, groupData, *it));
                            record->sourceTimer = timer;
                        }
                            startTimer(record->sourceTimer, groupMembershipInterval);
                    }
                    //send q(G,A-B)
                    IGMPv3::IpVector sourcesA;
                    for(SourceToGroupDataMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        sourcesA.push_back(it->first);
                    }
                    sendQuery(ie, groupData->groupAddr, IpComplement(sourcesA, gr.SourceList), queryResponseInterval);
                }
                else if(groupData->filter == IGMPV3_FM_EXCLUDE)
                {
                    //exclude X+A Y-A A=gmi
                    for(IGMPv3::IpVector::iterator it = gr.SourceList.begin(); it != gr.SourceList.end(); ++it)
                    {
                        SourceRecord *record = getSourceRecord(ie, groupData->groupAddr, *it);
                        if(!record)
                        {
                            record = createSourceRecord(ie, groupData->groupAddr, *it);
                            cMessage *timer = new cMessage("IGMPv3 router source timer", IGMPV3_R_SOURCE_TIMER);
                            timer->setContextPointer(new IGMPV3RouterSourceTimerContext(ie, groupData, *it));
                            record->sourceTimer = timer;
                        }
                        startTimer(record->sourceTimer, groupMembershipInterval);
                    }
                    //send q(g,x-A)
                    IGMPv3::IpVector sourcesX;
                    for(SourceToGroupDataMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        if(it->second->sourceTimer->isScheduled())
                        {
                            sourcesX.push_back(it->first);
                        }
                    }
                    sendQuery(ie, groupData->groupAddr, IpComplement(sourcesX, gr.SourceList), queryResponseInterval);
                    //send q(g)
                    IGMPv3::IpVector emptySources;
                    emptySources.clear();
                    sendQuery(ie,groupData->groupAddr, emptySources, queryResponseInterval);
                }
            }
            else if(gr.recordType == IGMPV3_RT_TO_EX)
            {
                if(groupData->filter == IGMPV3_FM_INCLUDE)
                {
                    //grouptimer = gmi
                    if(!groupData->timer)
                    {
                        cMessage *timer = new cMessage("IGMPv3 router group timer", IGMPV3_R_GROUP_TIMER);
                        timer->setContextPointer(new IGMPV3RouterTimerContext(ie, groupData));
                        groupData->timer = timer;
                    }
                    startTimer(groupData->timer, groupMembershipInterval);
                    //change to mode exclude
                    groupData->filter = IGMPV3_FM_EXCLUDE;
                    //A*B, B-A ; B-A=0
                    for(IGMPv3::IpVector::iterator it = gr.SourceList.begin(); it != gr.SourceList.end(); ++ it)
                    {
                        SourceRecord *record = getSourceRecord(ie, groupData->groupAddr, *it);
                        if(!record)
                        {
                            record = createSourceRecord(ie, groupData->groupAddr, *it);     //todo neviem ci to takto moze byt
                            cMessage *timer = new cMessage("IGMPv3 router source timer", IGMPV3_R_SOURCE_TIMER);
                            timer->setContextPointer(new IGMPV3RouterSourceTimerContext(ie, groupData, *it));
                            record->sourceTimer = timer;
                        }
                    }
                    //delete A-B
                    for(SourceToGroupDataMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        //IGMPv3::IpVector::iterator itIp;
                        if(std::find(gr.SourceList.begin(), gr.SourceList.end(), it->first) == gr.SourceList.end())
                        {
                            deleteSourceRecord(ie, groupData->groupAddr, it->first);
                        }
                    }
                    //send q(g,a*b)
                    IGMPv3::IpVector mapSources;
                    for(SourceToGroupDataMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        mapSources.push_back(it->first);
                    }
                    sendQuery(ie, groupData->groupAddr, IpIntersection(mapSources,gr.SourceList), queryResponseInterval);
                }
                else if(groupData->filter == IGMPV3_FM_EXCLUDE)
                {
                    //grouptimer = gmi
                    if(!groupData->timer)
                    {
                        cMessage *timer = new cMessage("IGMPv3 router group timer", IGMPV3_R_GROUP_TIMER);
                        timer->setContextPointer(new IGMPV3RouterTimerContext(ie, groupData));
                        groupData->timer = timer;
                    }
                    startTimer(groupData->timer, groupMembershipInterval);

                    //Delete X-A
                    //Delete Y-A
                    for(SourceToGroupDataMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        if(it->second->sourceTimer->isScheduled())
                        {
                            if(std::find(gr.SourceList.begin(), gr.SourceList.end(), it->first)== gr.SourceList.end())
                            {
                                deleteSourceRecord(ie, groupData->groupAddr, it->first);
                            }
                        }
                        else
                        {
                            if(std::find(gr.SourceList.begin(), gr.SourceList.end(), it->first)==gr.SourceList.end())
                            {
                                deleteSourceRecord(ie, groupData->groupAddr, it->first);
                            }
                        }
                    }
                    //A-X-Y = GMI
                    for(IGMPv3::IpVector::iterator it = gr.SourceList.begin(); it != gr.SourceList.end(); ++it)
                    {
                        SourceRecord *record = getSourceRecord(ie, groupData->groupAddr, *it);
                        if(!record)
                        {
                            record = createSourceRecord(ie, groupData->groupAddr, *it);
                            cMessage *timer = new cMessage("IGMPv3 router source timer", IGMPV3_R_SOURCE_TIMER);
                            timer->setContextPointer(new IGMPV3RouterSourceTimerContext(ie, groupData, *it));
                            record->sourceTimer = timer;
                            startTimer(record->sourceTimer, groupMembershipInterval);
                        }
                    }

                    //send q(g,a-y)
                    IGMPv3::IpVector mapSourcesY;
                    for(SourceToGroupDataMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        if(!it->second->sourceTimer->isScheduled())
                        {
                            mapSourcesY.push_back(it->first);
                        }
                    }
                    sendQuery(ie, groupData->groupAddr, IpComplement(gr.SourceList, mapSourcesY), queryResponseInterval);
                }
            }
        }

    }
    delete msg;
}

/**
 * Function for switching EXCLUDE filter mode back to INCLUDE
 * If at least one source timer is still runing, it will switch to Include mode.
 * Else if no source timer is running, group record is deleted.
 */
void IGMPv3::processRouterGroupTimer(cMessage *msg)
{
    IGMPV3RouterTimerContext *ctx = (IGMPV3RouterTimerContext*)msg->getContextPointer();
    RouterGroupData *groupData = ctx->routerGroup;
    if(groupData->filter == IGMPV3_FM_EXCLUDE)
    {
        bool timerRunning = false;
        for(SourceToGroupDataMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
        {
            if(!it->second->sourceTimer->isScheduled())
            {
                deleteSourceRecord(ctx->ie, groupData->groupAddr, it->first);
            }
            else
            {
                timerRunning = true;
            }
        }
        groupData->filter = IGMPV3_FM_INCLUDE;
        if(!timerRunning)
        {
            ctx->ie->ipv4Data()->removeMulticastListener(ctx->routerGroup->groupAddr);
            IPv4MulticastGroupInfo info(ctx->ie, ctx->routerGroup->groupAddr);
            nb->fireChangeNotification(NF_IPv4_MCAST_UNREGISTERED, &info);
            deleteRouterGroupData(ctx->ie, ctx->routerGroup->groupAddr);
            numRouterGroups--;
            numGroups--;
        }
    }
}
/**
 * Function for checking expired source timers if group is in INCLUDE filter mode.
 */
void IGMPv3::processRouterSourceTimer(cMessage *msg)
{
    IGMPV3RouterSourceTimerContext *ctx = (IGMPV3RouterSourceTimerContext*)msg->getContextPointer();
    RouterGroupData *groupData = ctx->routerGroup;
    bool last = true;
    if(groupData->filter == IGMPV3_FM_INCLUDE)
    {
        deleteSourceRecord(ctx->ie, groupData->groupAddr, ctx->sourceAddr);
    }
    for(SourceToGroupDataMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
    {
        if(it->second->sourceTimer->isScheduled())
        {
            last = false;
        }
    }
    if(last)
    {
        ctx->ie->ipv4Data()->removeMulticastListener(ctx->routerGroup->groupAddr);
        IPv4MulticastGroupInfo info(ctx->ie, ctx->routerGroup->groupAddr);
        nb->fireChangeNotification(NF_IPv4_MCAST_UNREGISTERED, &info);
        deleteRouterGroupData(ctx->ie, ctx->routerGroup->groupAddr);
        numRouterGroups--;
        numGroups--;
    }
}
