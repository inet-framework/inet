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
#include "IPSocket.h"
#include "IPv4RoutingTableAccess.h"
#include "InterfaceTableAccess.h"
#include "IPv4ControlInfo.h"
#include "IPv4InterfaceData.h"
#include "NotificationBoard.h"

#include <algorithm>
#include <bitset>

using namespace std;

Define_Module(IGMPv3);

static IPv4AddressVector set_complement(IPv4AddressVector first, IPv4AddressVector second)
{
    IPv4AddressVector complement(first.size());
    IPv4AddressVector::iterator it;

    std::sort(first.begin(), first.end());
    std::sort(second.begin(), second.end());

    it = set_difference(first.begin(), first.end(), second.begin(), second.end(), complement.begin());
    complement.resize(it-complement.begin());
    return complement;
}

static IPv4AddressVector set_intersection(IPv4AddressVector first, IPv4AddressVector second)
{
    IPv4AddressVector intersection(std::min(first.size(), second.size()));
    IPv4AddressVector::iterator it;

    std::sort(first.begin(), first.end());
    std::sort(second.begin(), second.end());

    it = set_intersection(first.begin(), first.end(), second.begin(), second.end(), intersection.begin());
    intersection.resize(it-intersection.begin());
    return intersection;
}

// handy definition for logging
static std::ostream &operator<<(std::ostream &out, const IPv4AddressVector addresses)
{
    out << "(";
    for (int i = 0; i < (int)addresses.size(); i++)
        out << (i>0?",":"") << addresses[i];
    out << ")";
    return out;
}

IGMPv3::HostGroupData::HostGroupData(HostInterfaceData *parent, IPv4Address group)
    : parent(parent), groupAddr(group), filter(IGMPV3_FM_INCLUDE), state(IGMPV3_HGS_NON_MEMBER), timer(NULL)
{
    ASSERT(parent);
    ASSERT(groupAddr.isMulticast());
}

IGMPv3::HostGroupData::~HostGroupData()
{
    if(timer)
    {
        delete (IGMPV3HostTimerSourceContext*)timer->getContextPointer();
        parent->owner->cancelAndDelete(timer);
    }
}

std::string IGMPv3::HostGroupData::getStateInfo() const
{
    std::ostringstream out;
    switch (filter)
    {
        case IGMPV3_FM_INCLUDE:
            out << "INCLUDE" << sourceAddressList;
            break;
        case IGMPV3_FM_EXCLUDE:
            out << "EXCLUDE" << sourceAddressList;
            break;
    }
    return out.str();
}

IGMPv3::RouterGroupData::RouterGroupData(RouterInterfaceData *parent, IPv4Address group)
    : parent(parent), groupAddr(group), filter(IGMPV3_FM_INCLUDE), state(IGMPV3_RGS_NO_MEMBERS_PRESENT), timer(NULL)
{
    ASSERT(parent);
    ASSERT(groupAddr.isMulticast());
}

IGMPv3::RouterGroupData::~RouterGroupData()
{
    parent->owner->cancelAndDelete(timer);
}

IGMPv3::SourceRecord *IGMPv3::RouterGroupData::createSourceRecord(IPv4Address source)
{
    ASSERT(sources.find(source) == sources.end());
    SourceRecord *record = new SourceRecord(this, source);
    sources[source] = record;
    return record;
}

IGMPv3::SourceRecord *IGMPv3::RouterGroupData::getSourceRecord(IPv4Address source)
{
    SourceToSourceRecordMap::iterator it = sources.find(source);
    return it != sources.end() ? it->second : NULL;
}

void IGMPv3::RouterGroupData::deleteSourceRecord(IPv4Address source)
{
    SourceToSourceRecordMap::iterator it = sources.find(source);
    if(it != sources.end())
    {
        SourceRecord *record = it->second;
        sources.erase(it);
        delete record;
    }
}

std::string IGMPv3::RouterGroupData::getStateInfo() const
{
    std::ostringstream out;
    switch (filter)
    {
        case IGMPV3_FM_INCLUDE:
            out << "INCLUDE(";
            printSourceList(out, true);
            out << ")";
            break;
        case IGMPV3_FM_EXCLUDE:
            out << "EXCLUDE(";
            printSourceList(out, true);
            out << ";";
            printSourceList(out, false);
            out << ")";
            break;
    }
    return out.str();
}

// See RFC 3376 6.3 for the description of forwarding rules
void IGMPv3::RouterGroupData::collectForwardedSources(IPv4MulticastSourceList &result) const
{
    switch (filter)
    {
        case IGMPV3_FM_INCLUDE:
            result.filterMode = MCAST_INCLUDE_SOURCES;
            result.sources.clear();
            for (SourceToSourceRecordMap::const_iterator it = sources.begin(); it != sources.end(); ++it)
            {
                if (it->second->sourceTimer && it->second->sourceTimer->isScheduled())
                    result.sources.push_back(it->first);
            }
            break;
        case IGMPV3_FM_EXCLUDE:
            result.filterMode = MCAST_EXCLUDE_SOURCES;
            result.sources.clear();
            for (SourceToSourceRecordMap::const_iterator it = sources.begin(); it != sources.end(); ++it)
            {
                if (!it->second->sourceTimer || !it->second->sourceTimer->isScheduled())
                    result.sources.push_back(it->first);
            }
            break;
    }
}


void IGMPv3::RouterGroupData::printSourceList(std::ostream &out, bool withRunningTimer) const
{
    bool first = true;
    for (IGMPv3::SourceToSourceRecordMap::const_iterator it = sources.begin(); it != sources.end(); ++it)
    {
        bool timerIsRunning = it->second->sourceTimer && it->second->sourceTimer->isScheduled();
        if (withRunningTimer == timerIsRunning)
        {
            if (!first)
                out << ",";
            first = false;
            out << it->first;
        }
    }
}

IGMPv3::SourceRecord::SourceRecord(RouterGroupData *parent, IPv4Address source)
    : parent(parent), sourceAddr(source)
{
    ASSERT(parent);

    sourceTimer = new cMessage("IGMPv3 router source timer", IGMPV3_R_SOURCE_TIMER);
    sourceTimer->setContextPointer(this);
}

IGMPv3::SourceRecord::~SourceRecord()
{
    parent->parent->owner->cancelAndDelete(sourceTimer);
}

IGMPv3::HostInterfaceData::HostInterfaceData(IGMPv3 *owner, InterfaceEntry *ie)
    : owner(owner), ie(ie)
{
    ASSERT(owner);
    ASSERT(ie);

    generalQueryTimer = new cMessage("IGMPv3 Host General Timer", IGMPV3_H_GENERAL_QUERY_TIMER);
    generalQueryTimer->setContextPointer(this);
}

IGMPv3::HostInterfaceData::~HostInterfaceData()
{
    owner->cancelAndDelete(generalQueryTimer);

    for (GroupToHostDataMap::iterator it = groups.begin(); it != groups.end(); ++it)
        delete it->second;
}

IGMPv3::HostGroupData *IGMPv3::HostInterfaceData::createGroupData(IPv4Address group)
{
    ASSERT(groups.find(group) == groups.end());
    HostGroupData *data = new HostGroupData(this, group);
    groups[group] = data;
    return data;
}

IGMPv3::HostGroupData *IGMPv3::HostInterfaceData::getGroupData(IPv4Address group)
{
    GroupToHostDataMap::iterator it = groups.find(group);
    return it != groups.end() ? it->second : NULL;
}

void IGMPv3::HostInterfaceData::deleteGroupData(IPv4Address group)
{
    GroupToHostDataMap::iterator it = groups.find(group);
    if (it != groups.end())
    {
        HostGroupData *data = it->second;
        groups.erase(it);
        delete data;
    }
}

IGMPv3::RouterInterfaceData::RouterInterfaceData(IGMPv3 *owner, InterfaceEntry *ie)
    : owner(owner), ie(ie)
{
    ASSERT(owner);
    ASSERT(ie);

    state = IGMPV3_RS_INITIAL;
    generalQueryTimer = new cMessage("IGMPv3 General Query timer", IGMPV3_R_GENERAL_QUERY_TIMER);
    generalQueryTimer->setContextPointer(this);
}

IGMPv3::RouterInterfaceData::~RouterInterfaceData()
{
    owner->cancelAndDelete(generalQueryTimer);

    for(GroupToRouterDataMap::iterator it = groups.begin(); it != groups.end(); ++it)
        delete it->second;
}

IGMPv3::RouterGroupData *IGMPv3::RouterInterfaceData::createGroupData(IPv4Address group)
{
    ASSERT(groups.find(group) == groups.end());
    RouterGroupData *data = new RouterGroupData(this, group);
    groups[group] = data;
    return data;
}

IGMPv3::RouterGroupData *IGMPv3::RouterInterfaceData::getGroupData(IPv4Address group)
{
    GroupToRouterDataMap::iterator it = groups.find(group);
    return it != groups.end() ? it->second : NULL;
}

void IGMPv3::RouterInterfaceData::deleteGroupData(IPv4Address group)
{
    GroupToRouterDataMap::iterator it = groups.find(group);
    if(it != groups.end())
    {
        RouterGroupData *data = it->second;
        groups.erase(it);
        delete data;
    }
}

IGMPv3::HostInterfaceData *IGMPv3::createHostInterfaceData(InterfaceEntry *ie)
{
    return new HostInterfaceData(this, ie);
}

IGMPv3::RouterInterfaceData *IGMPv3::createRouterInterfaceData(InterfaceEntry *ie)
{
    return new RouterInterfaceData(this, ie);
}

IGMPv3::HostInterfaceData *IGMPv3::getHostInterfaceData(InterfaceEntry *ie)
{
    int interfaceId = ie->getInterfaceId();
    InterfaceToHostDataMap::iterator it = hostData.find(interfaceId);
    if(it != hostData.end())
        return it->second;

    HostInterfaceData *data = createHostInterfaceData(ie);
    hostData[interfaceId] = data;
    return data;
}

IGMPv3::RouterInterfaceData *IGMPv3::getRouterInterfaceData(InterfaceEntry *ie)
{
    int interfaceId = ie->getInterfaceId();
    InterfaceToRouterDataMap::iterator it = routerData.find(interfaceId);
    if(it != routerData.end())
        return it->second;

    RouterInterfaceData *data = createRouterInterfaceData(ie);
    routerData[interfaceId] = data;
    return data;
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

void IGMPv3::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        ift = InterfaceTableAccess().get();
        rt = IPv4RoutingTableAccess().get();
        nb = NotificationBoardAccess().get();

        nb->subscribe(this, NF_INTERFACE_DELETED);
        nb->subscribe(this, NF_IPv4_MCAST_CHANGE);

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

        IPSocket ipSocket(gate("ipOut"));
        ipSocket.registerProtocol(IP_PROT_IGMP);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER)
    {
        for (int i = 0; i < (int)ift->getNumInterfaces(); ++i)
        {
            InterfaceEntry *ie = ift->getInterface(i);
            if (ie->isMulticast())
                configureInterface(ie);
        }
        nb->subscribe(this, NF_INTERFACE_CREATED);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER_2) // ipv4Data() created in INITSTAGE_NETWORK_LAYER
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
    const IPv4MulticastGroupSourceInfo *info;
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
        case NF_IPv4_MCAST_CHANGE:
            info = check_and_cast<const IPv4MulticastGroupSourceInfo*>(details);
            multicastSourceListChanged(info->ie, info->groupAddress, info->sourceList);
            break;
    }
}

void IGMPv3::configureInterface(InterfaceEntry *ie)
{
    if (enabled && rt->isMulticastForwardingEnabled())
    {
        // start querier on this interface
        EV_INFO << "Sending General Query on interface '" << ie->getName() << "', and scheduling next Query to '"
                << (simTime() + startupQueryInterval) << "'.\n";
        RouterInterfaceData *routerData = getRouterInterfaceData(ie);
        routerData->state = IGMPV3_RS_QUERIER;

        sendQuery(ie, IPv4Address::UNSPECIFIED_ADDRESS, IPv4AddressVector(), queryResponseInterval); // general query
        startTimer(routerData->generalQueryTimer, startupQueryInterval);
    }
}

// TODO accept v1/v2 messages too
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
            // XXX IGMPV3_H_SOURCE_TIMER?
            default:
                ASSERT(false);
                break;
        }
    }
    else
        processIgmpMessage(check_and_cast<IGMPMessage*>(msg));
}

void IGMPv3::processIgmpMessage(IGMPMessage *msg)
{
    switch (msg->getType())
    {
        case IGMP_MEMBERSHIP_QUERY:
            if (dynamic_cast<IGMPv3Query *>(msg))
                    processQuery((IGMPv3Query *)msg);
            else
                /* TODO process v1 and v2 queries*/;
            break;
        case IGMPV3_MEMBERSHIP_REPORT:
            processReport(check_and_cast<IGMPv3Report*>(msg));
            break;
        // TODO process v1/v2 reports
        default:
            delete msg;
            throw cRuntimeError("IGMPv2: Unhandled message type (%dq)", msg->getType());
    }
}

void IGMPv3::startTimer(cMessage *timer, double interval)
{
    ASSERT(timer);
    cancelEvent(timer);
    scheduleAt(simTime() + interval, timer);
}

void IGMPv3::sendQuery(InterfaceEntry *ie, IPv4Address groupAddr, const IPv4AddressVector &sources, double maxRespTime)
{
    ASSERT(groupAddr.isUnspecified() || (groupAddr.isMulticast() && !groupAddr.isLinkLocalMulticast()));

    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);

    if (interfaceData->state == IGMPV3_RS_QUERIER)
    {
        IGMPv3Query *msg = new IGMPv3Query("IGMPv3 query");
        msg->setType(IGMP_MEMBERSHIP_QUERY);
        msg->setGroupAddress(groupAddr);
        msg->setMaxRespCode((int)(maxRespTime * 10.0));
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
    RouterInterfaceData *interfaceData = (RouterInterfaceData*)msg->getContextPointer();
    InterfaceEntry *ie = interfaceData->ie;
    ASSERT(ie);
    EV_INFO << "General Query timer expired on interface='" << ie->getName() << "'.\n";
    RouterState state = interfaceData->state;
    if (state == IGMPV3_RS_QUERIER || state == IGMPV3_RS_NON_QUERIER)
    {
        EV_INFO << "Sending General Query on interface '" << ie->getName() << "', and scheduling next Query to '"
                << (simTime() + queryInterval) << "'.\n";
        interfaceData->state = IGMPV3_RS_QUERIER;
        sendQuery(ie, IPv4Address::UNSPECIFIED_ADDRESS, IPv4AddressVector(), queryResponseInterval);
        startTimer(msg, queryInterval);
    }
}

// TODO add Router Alert option, set Type of Service to 0xc0
void IGMPv3::sendReportToIP(IGMPv3Report *msg, InterfaceEntry *ie, IPv4Address dest)
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

void IGMPv3::sendQueryToIP(IGMPv3Query *msg, InterfaceEntry *ie, IPv4Address dest)
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

// RFC3376 5.2  report generation, point 1.
void IGMPv3::processHostGeneralQueryTimer(cMessage *msg)
{
    HostInterfaceData *interfaceData = (HostInterfaceData*)msg->getContextPointer();
    InterfaceEntry *ie = interfaceData->ie;
    ASSERT(ie);

    EV_INFO << "Response timer to a General Query on interface '" << ie->getName() << "' has expired.\n";

    //HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    IGMPv3Report *report = new IGMPv3Report("IGMPv3 report");
    report->setType(IGMPV3_MEMBERSHIP_REPORT);
    int counter = 0;
    report->setGroupRecordArraySize(interfaceData->groups.size());

    // FIXME Do not create reports larger than MTU of the interface
    //

    /*
     * creating GroupRecord for each group on interface
     */
    for(GroupToHostDataMap::iterator it = interfaceData->groups.begin(); it != interfaceData->groups.end(); ++it)
    {
        GroupRecord gr;
        if(it->second->filter == IGMPV3_FM_INCLUDE)
        {
            gr.recordType = IGMPV3_RT_IS_IN;
        }
        else if(it->second->filter == IGMPV3_FM_EXCLUDE)
        {
            gr.recordType = IGMPV3_RT_IS_EX;
        }
        gr.groupAddress = it->second->groupAddr;
        gr.sourceList = it->second->sourceAddressList;
        report->setGroupRecord(counter++,gr);
    }
    if(counter != 0)    //if no record created, dont need to send report
    {
        EV_INFO << "Sending response to a General Query on interface '" << ie->getName() << "'.\n";
        sendReportToIP(report, ie, IPv4Address::ALL_IGMPV3_ROUTERS_MCAST);
        numReportsSent++;
    }
    else
        EV_INFO << "There are no multicast listeners, no response is sent to a General Query on interface '" << ie->getName() << "'.\n";
}

// RFC3376 5.2  report generation, point 2. and 3.
void IGMPv3::processHostGroupQueryTimer(cMessage *msg)
{
    IGMPV3HostTimerSourceContext *ctx = (IGMPV3HostTimerSourceContext*)msg->getContextPointer();

    vector<GroupRecord> records(1);

    //checking if query is group or group-and-source specific
    if(ctx->sourceList.empty())
    {
        // Send report for a Group-Specific Query
        EV_INFO << "Response timer for a Group-Specific Query for group '" << ctx->hostGroup->groupAddr << "' on interface '" << ctx->ie->getName() << "'\n";

        if(ctx->hostGroup->filter == IGMPV3_FM_EXCLUDE)
        {
            records[0].groupAddress = ctx->hostGroup->groupAddr;
            records[0].recordType = IGMPV3_RT_IS_EX;
            records[0].sourceList = ctx->hostGroup->sourceAddressList;
            sendGroupReport(ctx->ie, records);
        }
        else if(ctx->hostGroup->filter == IGMPV3_FM_INCLUDE)
        {
            records[0].groupAddress = ctx->hostGroup->groupAddr;
            records[0].recordType = IGMPV3_RT_IS_IN;
            records[0].sourceList = ctx->hostGroup->sourceAddressList;
            sendGroupReport(ctx->ie, records);
        }
    }
    else
    {
        // Send report for a Group-and-Source-Specific Query
        EV_INFO << "Response timer for a Group-and-Source-Specific Query for group '" << ctx->hostGroup->groupAddr << "' on interface '" << ctx->ie->getName() << "'\n";

        if(ctx->hostGroup->filter == IGMPV3_FM_INCLUDE)
        {
            records[0].groupAddress = ctx->hostGroup->groupAddr;
            records[0].recordType = IGMPV3_RT_IS_IN;
            records[0].sourceList = set_intersection(ctx->hostGroup->sourceAddressList, ctx->sourceList);
            sendGroupReport(ctx->ie, records);
        }
        else if(ctx->hostGroup->filter == IGMPV3_FM_EXCLUDE)
        {
            records[0].groupAddress = ctx->hostGroup->groupAddr;
            records[0].recordType = IGMPV3_RT_IS_IN;
            records[0].sourceList = set_complement(ctx->sourceList, ctx->hostGroup->sourceAddressList);
            sendGroupReport(ctx->ie, records);
        }
    }

    ctx->sourceList.clear();
}

static bool isEmptyRecord(const GroupRecord &record)
{
    return record.sourceList.empty();
}

/**
 * This function is sending report message if interface state was changed.
 */
void IGMPv3::multicastSourceListChanged(InterfaceEntry *ie, IPv4Address group, const IPv4MulticastSourceList &sourceList)
{
    ASSERT(ie);

    if (!enabled || group.isLinkLocalMulticast())
        return;

    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    HostGroupData *groupData = interfaceData->getGroupData(group);

    if (!groupData)
    {
        // If no interface state existed for that multicast address before the change,
        // then the "non-existent" state is considered to have a filter mode of INCLUDE
        // and an empty source list.
        groupData = interfaceData->createGroupData(group);
        numGroups++;
        numHostGroups++;
    }

    FilterMode filter = sourceList.filterMode == MCAST_INCLUDE_SOURCES ? IGMPV3_FM_INCLUDE : IGMPV3_FM_EXCLUDE;

    EV_DETAIL_C("test") << "State of group '" << group << "' on interface '" << ie->getName() << "' has changed:\n";
    EV_DETAIL_C("test") << "    Old state: " << groupData->getStateInfo() << ".\n";
    EV_DETAIL_C("test") << "    New state: " << (filter == IGMPV3_FM_INCLUDE?"INCLUDE":"EXCLUDE") << sourceList.sources << ".\n";

    //Check if IF state is different
    if(!(groupData->filter == filter) || !(groupData->sourceAddressList == sourceList.sources))
    {
        //OldState: INCLUDE(A) NewState: INCLUDE(B) StateChangeRecordSent: ALLOW(B-A) BLOCK(A-B)
        if(groupData->filter == IGMPV3_FM_INCLUDE && filter == IGMPV3_FM_INCLUDE && groupData->sourceAddressList != sourceList.sources)
        {
            EV_DETAIL << "Sending ALLOW/BLOCK report.\n";
            vector<GroupRecord> records(2);
            records[0].groupAddress = group;
            records[0].recordType = IGMPV3_RT_ALLOW;
            records[0].sourceList = set_complement(sourceList.sources, groupData->sourceAddressList);
            records[1].groupAddress = group;
            records[1].recordType = IGMPV3_RT_BLOCK;
            records[1].sourceList = set_complement(groupData->sourceAddressList, sourceList.sources);
            records.erase(remove_if(records.begin(), records.end(), isEmptyRecord), records.end());
            if (!records.empty())
                sendGroupReport(ie, records);

            groupData->sourceAddressList = sourceList.sources;
        }
        else if(groupData->filter == IGMPV3_FM_EXCLUDE && filter == IGMPV3_FM_EXCLUDE && groupData->sourceAddressList != sourceList.sources)
        {
            EV_DETAIL << "Sending ALLOW/BLOCK report.\n";
            vector<GroupRecord> records(2);
            records[0].groupAddress = group;
            records[0].recordType = IGMPV3_RT_ALLOW;
            records[0].sourceList = set_complement(groupData->sourceAddressList, sourceList.sources);
            records[1].groupAddress = group;
            records[1].recordType = IGMPV3_RT_BLOCK;
            records[1].sourceList = set_complement(sourceList.sources, groupData->sourceAddressList);
            records.erase(remove_if(records.begin(), records.end(), isEmptyRecord), records.end());
            if (!records.empty())
                sendGroupReport(ie, records);

            groupData->sourceAddressList = sourceList.sources;
        }
        else if(groupData->filter == IGMPV3_FM_INCLUDE && filter == IGMPV3_FM_EXCLUDE)
        {
            EV_DETAIL << "Sending TO_EX report.\n";
            vector<GroupRecord> records(1);
            records[0].groupAddress = group;
            records[0].recordType = IGMPV3_RT_TO_EX;
            records[0].sourceList = sourceList.sources;
            sendGroupReport(ie, records);

            groupData->filter = filter;
            groupData->sourceAddressList = sourceList.sources;
        }
        else if(groupData->filter == IGMPV3_FM_EXCLUDE && filter == IGMPV3_FM_INCLUDE)
        {
            EV_DETAIL << "Sending TO_IN report.\n";
            vector<GroupRecord> records(1);
            records[0].groupAddress = group;
            records[0].recordType = IGMPV3_RT_TO_IN;
            records[0].sourceList = sourceList.sources;
            sendGroupReport(ie, records);

            groupData->filter = filter;
            groupData->sourceAddressList = sourceList.sources;
        }
    }

    // FIXME missing: the report  is retransmitted [Robustness Variable] - 1 more times,
    //       at intervals chosen at random from the range (0, [Unsolicited Report Interval])

    // FIXME if an interface change occured when there is a pending report, then
    //       the groups of the old report and the new report are to be merged.
}

void IGMPv3::processQuery(IGMPv3Query *msg)
{
    IPv4ControlInfo *controlInfo = (IPv4ControlInfo *)msg->getControlInfo();
    InterfaceEntry *ie = ift->getInterfaceById(controlInfo->getInterfaceId());

    ASSERT(ie->isMulticast());

    EV_INFO << "Received IGMPv3 query on interface '" << ie->getName() << "' for group '" << msg->getGroupAddress() << "'.\n";

    HostInterfaceData *interfaceData = getHostInterfaceData(ie);

    numQueriesRecv++;

    IPv4Address &groupAddr = msg->getGroupAddress();
    IPv4AddressVector sources = msg->getSourceList();

    double maxRespTime = decodeTime(msg->getMaxRespCode());
    double delay = uniform(0.0, maxRespTime);

    // Rules from RFC page 22
    if(interfaceData->generalQueryTimer->isScheduled() && interfaceData->generalQueryTimer->getArrivalTime() < simTime() + delay)
    {
        // 1. If there is a pending response to a previous General Query
        //    scheduled sooner than the selected delay, no additional response
        //    needs to be scheduled.
        EV_DETAIL << "There is a pending response to a previous General Query, no further response is scheduled.\n";
    }
    else if(groupAddr.isUnspecified() && msg->getSourceList().empty())
    {
        // 2. If the received Query is a General Query, the interface timer is
        //    used to schedule a response to the General Query after the
        //    selected delay.  Any previously pending response to a General
        //    Query is canceled.
        EV_DETAIL << "Received a General Query, scheduling report with delay=" << delay << ".\n";

        if(interfaceData->generalQueryTimer->isScheduled())
        {
            cancelEvent(interfaceData->generalQueryTimer);
        }
        scheduleAt(simTime() + delay, interfaceData->generalQueryTimer);
    }
    if(!groupAddr.isUnspecified())
    {
        HostGroupData *groupData = interfaceData->getGroupData(groupAddr);
        if (!groupData) {
            groupData = interfaceData->createGroupData(groupAddr);
            numGroups++;
            numHostGroups++;
        }

        if(!groupData->timer)
        {
            groupData->timer = new cMessage("IGMPv3 Host Group Timer", IGMPV3_H_GROUP_TIMER);
        }
        else if(!groupAddr.isUnspecified() && !groupData->timer->isScheduled())
        {
            // 3. If the received Query is a Group-Specific Query or a Group-and-
            //    Source-Specific Query and there is no pending response to a
            //    previous Query for this group, then the group timer is used to
            //    schedule a report.  If the received Query is a Group-and-Source-
            //    Specific Query, the list of queried sources is recorded to be used
            //    when generating a response.
            EV_DETAIL << "Received Group" << (msg->getSourceList().empty()?"":"-and-Source") << "-Specific Query, "
                      << "scheduling report with delay=" << delay << ".\n";

            groupData->timer->setContextPointer(new IGMPV3HostTimerSourceContext(ie, groupData, msg->getSourceList()));
            scheduleAt(simTime() + delay, groupData->timer);
        }
        else if(!groupAddr.isUnspecified() && groupData->timer->isScheduled())
        {
            //4. If there already is a pending response to a previous Query
            //   scheduled for this group, and either the new Query is a Group-
            //   Specific Query or the recorded source-list associated with the
            //   group is empty, then the group source-list is cleared and a single
            //   response is scheduled using the group timer.  The new response is
            //   scheduled to be sent at the earliest of the remaining time for the
            //   pending report and the selected delay.
            if(msg->getSourceList().empty())
            {
                EV_DETAIL << "Received Group-Specific Query, scheduling report with delay="
                          << std::min(delay, SIMTIME_DBL(groupData->timer->getArrivalTime() - simTime())) << ".\n";

                groupData->timer->setContextPointer(new IGMPV3HostTimerSourceContext(ie, groupData, msg->getSourceList()));
                if(groupData->timer->getArrivalTime() > simTime() + delay)
                {
                    cancelEvent(groupData->timer);
                    scheduleAt(simTime() + delay, groupData->timer);
                }
            }
            else
            {
                // 5. If the received Query is a Group-and-Source-Specific Query and
                //    there is a pending response for this group with a non-empty
                //    source-list, then the group source list is augmented to contain
                //    the list of sources in the new Query and a single response is
                //    scheduled using the group timer.  The new response is scheduled to
                //    be sent at the earliest of the remaining time for the pending
                //    report and the selected delay.
                EV_DETAIL << "Received Group-and-Source-Specific Query, combining sources with the sources of pending report, "
                          << "and scheduling a new report with delay="
                          << std::min(delay, SIMTIME_DBL(groupData->timer->getArrivalTime() - simTime())) << ".\n";

                IGMPV3HostTimerSourceContext *ctx = (IGMPV3HostTimerSourceContext*)groupData->timer->getContextPointer();
                if(groupData->timer->getArrivalTime() > simTime() + delay)
                {
                    IPv4AddressVector combinedSources;
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
            RouterGroupData *groupData = routerInterfaceData->getGroupData(groupAddr);
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

double IGMPv3::decodeTime(unsigned char code)
{
    unsigned time;
    if(code < 128)
        time = code;
    else
    {
        unsigned mantis = code & 0x15;
        unsigned exp = (code >> 4) & 0x07;
        time = (mantis | 0x10) << (exp + 3);
    }

    return (double)time / 10.0;
}

void IGMPv3::sendGroupReport(InterfaceEntry *ie, const vector<GroupRecord> &records)
{
    EV << "IGMPv3: sending Membership Report on iface=" << ie->getName() << "\n";
    IGMPv3Report *msg = new IGMPv3Report("IGMPv3 report");
    msg->setType(IGMPV3_MEMBERSHIP_REPORT);
    msg->setGroupRecordArraySize(records.size());
    for (int i = 0; i < (int)records.size(); ++i)
    {
        IPv4Address group = records[i].groupAddress;
        ASSERT(group.isMulticast() && !group.isLinkLocalMulticast());
        msg->setGroupRecord(i, records[i]);
    }

    sendReportToIP(msg, ie, IPv4Address::ALL_IGMPV3_ROUTERS_MCAST);
    numReportsSent++;
}

void IGMPv3::processReport(IGMPv3Report *msg)
{
    IPv4ControlInfo *controlInfo = (IPv4ControlInfo *)msg->getControlInfo();
    InterfaceEntry *ie = ift->getInterfaceById(controlInfo->getInterfaceId());
    ASSERT(ie->isMulticast());

    EV_INFO << "Received IGMPv3 Membership Report on interface '" << ie->getName() << "'.\n";

    numReportsRecv++;

    if (rt->isMulticastForwardingEnabled())
    {
        RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);

        for(unsigned int i = 0; i < msg->getGroupRecordArraySize(); i++)
        {
            GroupRecord gr = msg->getGroupRecord(i);
            EV_DETAIL << "Found a record for group '" << gr.groupAddress << "'.\n";

            RouterGroupData *groupData = interfaceData->getGroupData(gr.groupAddress);
            if(!groupData)
            {
                groupData = interfaceData->createGroupData(gr.groupAddress);

                // FIXME the code below is clearly wrong.
                //       If the router did not received report for the group previously,
                //       then it must assume the INCLUDE() as the previous state.

//                groupData->filter = IGMPV3_FM_EXCLUDE;
//                // notify IPv4InterfaceData to update its listener list
//                ie->ipv4Data()->addMulticastListener(groupData->groupAddr);
//                // notify routing
//                IPv4MulticastGroupInfo info(ie, groupData->groupAddr);
//                nb->fireChangeNotification(NF_IPv4_MCAST_REGISTERED, &info);
//                numRouterGroups++;
//                numGroups++;
            }

            IPv4MulticastSourceList oldSourceList;
            groupData->collectForwardedSources(oldSourceList);

            EV_DETAIL << "Router State is " << groupData->getStateInfo() << ".\n";


            //Current State Record part
            if(gr.recordType == IGMPV3_RT_IS_IN)
            {
                EV_DETAIL << "Received IS_IN" << gr.sourceList << " report.\n";

                // XXX the two branch contain the same code
                if(groupData->filter == IGMPV3_FM_INCLUDE)
                {
                    for(IPv4AddressVector::iterator it = gr.sourceList.begin(); it != gr.sourceList.end(); ++it)
                    {
                        EV_DETAIL << "Setting source timer of '" << *it << "' to '" << groupMembershipInterval << "'.\n";

                        SourceRecord *record = groupData->getSourceRecord(*it);
                        if(!record)
                            record = groupData->createSourceRecord(*it);
                        startTimer(record->sourceTimer, groupMembershipInterval);
                    }
                }
                else if(groupData->filter == IGMPV3_FM_EXCLUDE)
                {
                    for(IPv4AddressVector::iterator it = gr.sourceList.begin(); it != gr.sourceList.end(); ++it)
                    {
                        EV_DETAIL << "Setting source timer of '" << *it << "' to '" << groupMembershipInterval << "'.\n";

                        SourceRecord *record = groupData->getSourceRecord(*it);
                        if(!record)
                            record = groupData->createSourceRecord(*it);
                        startTimer(record->sourceTimer, groupMembershipInterval);
                    }
                }
            }
            else if(gr.recordType == IGMPV3_RT_IS_EX)
            {
                EV_DETAIL << "Received IS_EX" << gr.sourceList << " report.\n";

                if(groupData->filter == IGMPV3_FM_INCLUDE)
                {
                    //grouptimer = gmi
                    EV_DETAIL << "Setting group timer to '" << groupMembershipInterval << "'.\n";
                    if(!groupData->timer)
                    {
                        cMessage *timer = new cMessage("IGMPv3 router group timer", IGMPV3_R_GROUP_TIMER);
                        timer->setContextPointer(groupData);
                        groupData->timer = timer;
                    }
                    startTimer(groupData->timer, groupMembershipInterval);
                    //change to mode exclude
                    groupData->filter = IGMPV3_FM_EXCLUDE;
                    //A*B, B-A ; B-A=0
                    for(IPv4AddressVector::iterator it = gr.sourceList.begin(); it != gr.sourceList.end(); ++ it)
                    {
                        SourceRecord *record = groupData->getSourceRecord(*it);
                        if(!record)
                        {
                            EV_DETAIL << "Setting source timer of '" << *it << "' to 0.\n";
                            record = groupData->createSourceRecord(*it);
                        }
                    }
                    //delete A-B
                    for(SourceToSourceRecordMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        //IGMPv3::IpVector::iterator itIp;
                        if(std::find(gr.sourceList.begin(), gr.sourceList.end(), it->first) == gr.sourceList.end())
                        {
                            EV_DETAIL << "Deleting source record of '" << it->first << "'.\n";
                            groupData->deleteSourceRecord(it->first);
                        }
                    }
                }
                else if(groupData->filter == IGMPV3_FM_EXCLUDE)
                {
                    //grouptimer = gmi
                    EV_DETAIL << "Setting group timer to '" << groupMembershipInterval << "'.\n";
                    if(!groupData->timer)
                    {
                        cMessage *timer = new cMessage("IGMPv3 router group timer", IGMPV3_R_GROUP_TIMER);
                        timer->setContextPointer(groupData);
                        groupData->timer = timer;
                    }
                    startTimer(groupData->timer, groupMembershipInterval);

                    //Delete X-A
                    //Delete Y-A
                    for(SourceToSourceRecordMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        if(std::find(gr.sourceList.begin(), gr.sourceList.end(), it->first) == gr.sourceList.end())
                        {
                            EV_DETAIL << "Deleting source record of '" << it->first << "'.\n";
                            groupData->deleteSourceRecord(it->first);
                        }
                    }

                    //A-X-Y = GMI
                    for(IPv4AddressVector::iterator it = gr.sourceList.begin(); it != gr.sourceList.end(); ++it)
                    {
                        SourceRecord *record = groupData->getSourceRecord(*it);
                        if(!record)
                        {
                            EV_DETAIL << "Setting source timer of '" << *it << "' to '" << groupMembershipInterval << "'.\n";
                            record = groupData->createSourceRecord(*it);
                            startTimer(record->sourceTimer, groupMembershipInterval);
                        }
                    }
                }
            }
            //Filter Mode change and Source List change part
            else if(gr.recordType == IGMPV3_RT_ALLOW)
            {
                EV_DETAIL << "Received ALLOW" << gr.sourceList << " report.\n";

                if(groupData->filter == IGMPV3_FM_INCLUDE)
                {
                    //A+B; B=gmi
                    for(IPv4AddressVector::iterator it = gr.sourceList.begin(); it != gr.sourceList.end(); ++it)
                    {
                        EV_DETAIL << "Setting source timer of '" << *it << "' to '" << groupMembershipInterval << "'.\n";
                        SourceRecord *record = groupData->getSourceRecord(*it);
                        if(!record)
                            record = groupData->createSourceRecord(*it);     //todo neviem ci to takto moze byt
                        startTimer(record->sourceTimer, groupMembershipInterval);
                    }
                }
                else if(groupData->filter == IGMPV3_FM_EXCLUDE)
                {
                    //Exclude X+A,Y-A ; A=gmi
                    for(IPv4AddressVector::iterator it = gr.sourceList.begin(); it != gr.sourceList.end(); ++it)
                    {
                        EV_DETAIL << "Setting source timer of '" << *it << "' to '" << groupMembershipInterval << "'.\n";
                        SourceRecord *record = groupData->getSourceRecord(*it);
                        if(!record)
                            record = groupData->createSourceRecord(*it);
                        startTimer(record->sourceTimer, groupMembershipInterval);
                    }
                }
            }
            else if(gr.recordType == IGMPV3_RT_BLOCK)
            {
                EV_DETAIL << "Received BLOCK" << gr.sourceList << " report.\n";

                if(groupData->filter == IGMPV3_FM_INCLUDE)
                {
                    //send q(G, A*B)
                    //include A
                    EV_INFO << "Sending Group-and-Source-Specific Query for group '" << groupData->groupAddr
                            << "' on interface '" << ie->getName() << "'.\n";
                    IPv4AddressVector mapSources;
                    for(SourceToSourceRecordMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        mapSources.push_back(it->first);
                    }
                    // FIXME do not send a query in the intersection is empty
                    sendQuery(ie, groupData->groupAddr, set_intersection(mapSources,gr.sourceList), queryResponseInterval);
                }
                else if(groupData->filter == IGMPV3_FM_EXCLUDE)
                {
                    //Exclude x+(a-y), y to je pokial to necham tak jak to je a vytvorim ten jeden dole
                    //A-X-Y=GroupTimer
                    for(SourceToSourceRecordMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        SourceRecord *record = groupData->getSourceRecord(it->first);
                        if(!record)
                        {
                            record = groupData->createSourceRecord(it->first);
                            double grouptimertime = groupData->timer->getArrivalTime().dbl() - simTime().dbl();
                            EV_DETAIL << "Setting source timer of '" << it->first << "' to '" << grouptimertime << "'.\n";
                            startTimer(record->sourceTimer, grouptimertime);
                        }
                    }
                    //Send q(G,A-Y)
                    EV_INFO << "Sending Group-and-Source-Specific Query for group '" << groupData->groupAddr
                            << "' on interface '" << ie->getName() << "'.\n";
                    IPv4AddressVector mapSourcesY;
                    for(SourceToSourceRecordMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        if(!it->second->sourceTimer->isScheduled())
                        {
                            mapSourcesY.push_back(it->first);
                        }
                    }
                    // FIXME do not send the query with empty source list
                    sendQuery(ie, groupData->groupAddr, set_complement(gr.sourceList, mapSourcesY), queryResponseInterval);
                }
            }
            else if(gr.recordType == IGMPV3_RT_TO_IN)
            {
                EV_DETAIL << "Received TO_IN" << gr.sourceList << " report.\n";

                if(groupData->filter == IGMPV3_FM_INCLUDE)
                {
                    //A+B; B=gmi
                    for(IPv4AddressVector::iterator it = gr.sourceList.begin(); it != gr.sourceList.end(); ++it)
                    {
                        EV_DETAIL << "Setting source timer of '" << *it << "' to '" << groupMembershipInterval << "'.\n";
                        SourceRecord *record = groupData->getSourceRecord(*it);
                        if(!record)
                            record = groupData->createSourceRecord(*it);     //todo neviem ci to takto moze byt
                        startTimer(record->sourceTimer, groupMembershipInterval);
                    }
                    //send q(G,A-B)
                    EV_INFO << "Sending Group-and-Source-Specific Query for group '" << groupData->groupAddr
                            << "' on interface '" << ie->getName() << "'.\n";
                    IPv4AddressVector sourcesA;
                    for(SourceToSourceRecordMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        sourcesA.push_back(it->first);
                    }
                    // FIXME do not send the query with empty source list
                    sendQuery(ie, groupData->groupAddr, set_complement(sourcesA, gr.sourceList), queryResponseInterval);
                }
                else if(groupData->filter == IGMPV3_FM_EXCLUDE)
                {
                    //exclude X+A Y-A A=gmi
                    for(IPv4AddressVector::iterator it = gr.sourceList.begin(); it != gr.sourceList.end(); ++it)
                    {
                        EV_DETAIL << "Setting source timer of '" << *it << "' to '" << groupMembershipInterval << "'.\n";
                        SourceRecord *record = groupData->getSourceRecord(*it);
                        if(!record)
                            record = groupData->createSourceRecord(*it);
                        startTimer(record->sourceTimer, groupMembershipInterval);
                    }
                    //send q(g,x-A)
                    EV_INFO << "Sending Group-and-Source-Specific Query for group '" << groupData->groupAddr
                            << "' on interface '" << ie->getName() << "'.\n";
                    IPv4AddressVector sourcesX;
                    for(SourceToSourceRecordMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        if(it->second->sourceTimer->isScheduled())
                        {
                            sourcesX.push_back(it->first);
                        }
                    }
                    // FIXME do not send the query with empty source list
                    sendQuery(ie, groupData->groupAddr, set_complement(sourcesX, gr.sourceList), queryResponseInterval);

                    // FIXME Missing (see RFC 3376 6.4.2):
                    // ..., when a router queries a specific group, it lowers its
                    // group timer for that group to a small interval of Last Member Query
                    // Time seconds.

                    //send q(g)
                    EV_INFO << "Sending Group-Specific Query for group '" << groupData->groupAddr
                            << "' on interface '" << ie->getName() << "'.\n";
                    IPv4AddressVector emptySources;
                    sendQuery(ie,groupData->groupAddr, emptySources, queryResponseInterval);
                }
            }
            else if(gr.recordType == IGMPV3_RT_TO_EX)
            {
                EV_DETAIL << "Received TO_EX" << gr.sourceList << " report.\n";

                if(groupData->filter == IGMPV3_FM_INCLUDE)
                {
                    //grouptimer = gmi
                    EV_DETAIL << "Setting group timer to '" << groupMembershipInterval << "'.\n";
                    if(!groupData->timer)
                    {
                        cMessage *timer = new cMessage("IGMPv3 router group timer", IGMPV3_R_GROUP_TIMER);
                        timer->setContextPointer(groupData);
                        groupData->timer = timer;
                    }
                    startTimer(groupData->timer, groupMembershipInterval);

                    //change to mode exclude
                    groupData->filter = IGMPV3_FM_EXCLUDE;

                    IPv4AddressVector oldSources;
                    for(SourceToSourceRecordMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                        oldSources.push_back(it->first);

                    //A*B, B-A ; B-A=0
                    for(IPv4AddressVector::iterator it = gr.sourceList.begin(); it != gr.sourceList.end(); ++ it)
                    {
                        SourceRecord *record = groupData->getSourceRecord(*it);
                        if(!record)
                        {
                            EV_DETAIL << "Setting source timer of '" << *it << "' to '0'.\n";
                            record = groupData->createSourceRecord(*it);     //todo neviem ci to takto moze byt
                        }
                    }

                    //delete A-B
                    for(SourceToSourceRecordMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        //IGMPv3::IpVector::iterator itIp;
                        if(std::find(gr.sourceList.begin(), gr.sourceList.end(), it->first) == gr.sourceList.end())
                        {
                            EV_DETAIL << "Deleting source record of '" << it->first << "'.\n";
                            groupData->deleteSourceRecord(it->first);
                        }
                    }

                    //send q(g,a*b)
                    EV_INFO << "Sending Group-Specific Query for group '" << groupData->groupAddr
                            << "' on interface '" << ie->getName() << "'.\n";

                    // FIXME do not send a query if the intersection is empty
                    sendQuery(ie, groupData->groupAddr, set_intersection(oldSources,gr.sourceList), queryResponseInterval);
                }
                else if(groupData->filter == IGMPV3_FM_EXCLUDE)
                {
                    //grouptimer = gmi
                    EV_DETAIL << "Setting group timer to '" << groupMembershipInterval << "'.\n";
                    if(!groupData->timer)
                    {
                        cMessage *timer = new cMessage("IGMPv3 router group timer", IGMPV3_R_GROUP_TIMER);
                        timer->setContextPointer(groupData);
                        groupData->timer = timer;
                    }
                    startTimer(groupData->timer, groupMembershipInterval);

                    //Delete X-A
                    //Delete Y-A
                    for(SourceToSourceRecordMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        if(std::find(gr.sourceList.begin(), gr.sourceList.end(), it->first)== gr.sourceList.end())
                        {
                            EV_DETAIL << "Deleting source record of '" << it->first << "'.\n";
                            groupData->deleteSourceRecord(it->first);
                        }
                    }

                    //A-X-Y = GMI
                    for(IPv4AddressVector::iterator it = gr.sourceList.begin(); it != gr.sourceList.end(); ++it)
                    {
                        SourceRecord *record = groupData->getSourceRecord(*it);
                        if(!record)
                        {
                            EV_DETAIL << "Setting source timer of '" << *it << "' to '" << groupMembershipInterval << "'.\n";
                            record = groupData->createSourceRecord(*it);
                            startTimer(record->sourceTimer, groupMembershipInterval);
                        }
                    }

                    //send q(g,a-y)
                    EV_INFO << "Sending Group-Specific Query for group '" << groupData->groupAddr
                            << "' on interface '" << ie->getName() << "'.\n";
                    IPv4AddressVector mapSourcesY;
                    for(SourceToSourceRecordMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
                    {
                        if(!it->second->sourceTimer->isScheduled())
                        {
                            mapSourcesY.push_back(it->first);
                        }
                    }
                    // FIXME do not send a query if the complement is empty
                    sendQuery(ie, groupData->groupAddr, set_complement(gr.sourceList, mapSourcesY), queryResponseInterval);
                }
            }

            EV_DETAIL << "New Router State is " << groupData->getStateInfo() << ".\n";


            // update interface state
            IPv4MulticastSourceList newSourceList;
            groupData->collectForwardedSources(newSourceList);

            if (newSourceList != oldSourceList)
            {
                ie->ipv4Data()->setMulticastListeners(groupData->groupAddr, newSourceList.filterMode, newSourceList.sources);
                // TODO notifications?
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
    RouterGroupData *groupData = (RouterGroupData*)msg->getContextPointer();
    InterfaceEntry *ie = groupData->parent->ie;

    EV_INFO << "Group Timer for group '" <<  groupData->groupAddr << "' on interface '" << ie->getName() << "' has expired.\n";
    EV_DETAIL << "Router State is " << groupData->getStateInfo() << ".\n";

    if(groupData->filter == IGMPV3_FM_EXCLUDE)
    {
        bool timerRunning = false;
        for(SourceToSourceRecordMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
        {
            if(!it->second->sourceTimer->isScheduled())
            {
                EV_DETAIL << "Deleting source record of '" << it->first << "'.\n";
                groupData->deleteSourceRecord(it->first);
            }
            else
            {
                timerRunning = true;
            }
        }
        groupData->filter = IGMPV3_FM_INCLUDE;
        if(!timerRunning)
        {
            EV_DETAIL << "Deleting multicast listener for group '" << groupData->groupAddr << "' from the interface table.\n";
            ie->ipv4Data()->removeMulticastListener(groupData->groupAddr);
            IPv4MulticastGroupInfo info(ie, groupData->groupAddr);
            nb->fireChangeNotification(NF_IPv4_MCAST_UNREGISTERED, &info);
            groupData->parent->deleteGroupData(groupData->groupAddr);
            numRouterGroups--;
            numGroups--;

            EV_DETAIL << "New Router State is <deleted>.\n";
            return;
        }
    }

    EV_DETAIL << "New Router State is " << groupData->getStateInfo() << ".\n";
}

/**
 * Function for checking expired source timers if group is in INCLUDE filter mode.
 */
void IGMPv3::processRouterSourceTimer(cMessage *msg)
{
    SourceRecord *sourceRecord = (SourceRecord*)msg->getContextPointer();
    RouterGroupData *groupData = sourceRecord->parent;
    InterfaceEntry *ie = groupData->parent->ie;

    EV_INFO << "Source timer for group '" << groupData->groupAddr << "' and source '" << sourceRecord->sourceAddr
            << "' on interface '" << ie->getName() << "' has expired.\n";

    bool last = true;
    if(groupData->filter == IGMPV3_FM_INCLUDE)
    {
        groupData->deleteSourceRecord(sourceRecord->sourceAddr);
    }
    for(SourceToSourceRecordMap::iterator it = groupData->sources.begin(); it != groupData->sources.end(); ++it)
    {
        if(it->second->sourceTimer->isScheduled())
        {
            last = false;
        }
    }
    if(last)
    {
        ie->ipv4Data()->removeMulticastListener(groupData->groupAddr);
        IPv4MulticastGroupInfo info(ie, groupData->groupAddr);
        nb->fireChangeNotification(NF_IPv4_MCAST_UNREGISTERED, &info);
        groupData->parent->deleteGroupData(groupData->groupAddr);
        numRouterGroups--;
        numGroups--;
    }
}
