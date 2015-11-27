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
 * @author Adam Malik(towdie13@gmail.com), Vladimir Vesely (ivesely@fit.vutbr.cz), Tamas Borbely (tomi@omnetpp.org)
 * @date 12.5.2013
 */

#include "inet/networklayer/ipv4/IGMPv3.h"
#include "inet/networklayer/common/IPSocket.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/ipv4/IPv4RoutingTable.h"
#include "inet/common/ModuleAccess.h"

#include <algorithm>
#include <bitset>

namespace inet {

using namespace std;

Define_Module(IGMPv3);

static bool isSorted(const IPv4AddressVector& v)
{
    int n = (int)v.size();
    for (int i = 0; i < n - 1; ++i)
        if (v[i] > v[i + 1])
            return false;

    return true;
}

static IPv4AddressVector set_complement(const IPv4AddressVector& first, const IPv4AddressVector& second)
{
    ASSERT(isSorted(first));
    ASSERT(isSorted(second));

    IPv4AddressVector complement(first.size());

    auto it = set_difference(first.begin(), first.end(), second.begin(), second.end(), complement.begin());
    complement.resize(it - complement.begin());
    return complement;
}

static IPv4AddressVector set_intersection(const IPv4AddressVector& first, const IPv4AddressVector& second)
{
    ASSERT(isSorted(first));
    ASSERT(isSorted(second));

    IPv4AddressVector intersection(min(first.size(), second.size()));

    auto it = set_intersection(first.begin(), first.end(), second.begin(), second.end(), intersection.begin());
    intersection.resize(it - intersection.begin());
    return intersection;
}

static IPv4AddressVector set_union(const IPv4AddressVector& first, const IPv4AddressVector& second)
{
    ASSERT(isSorted(first));
    ASSERT(isSorted(second));

    IPv4AddressVector result(first.size() + second.size());

    auto it = set_union(first.begin(), first.end(), second.begin(), second.end(), result.begin());
    result.resize(it - result.begin());
    return result;
}

// handy definition for logging
static ostream& operator<<(ostream& out, const IPv4AddressVector addresses)
{
    out << "(";
    for (int i = 0; i < (int)addresses.size(); i++)
        out << (i > 0 ? "," : "") << addresses[i];
    out << ")";
    return out;
}

IGMPv3::HostGroupData::HostGroupData(HostInterfaceData *parent, IPv4Address group)
    : parent(parent), groupAddr(group), filter(IGMPV3_FM_INCLUDE), state(IGMPV3_HGS_NON_MEMBER)
{
    ASSERT(parent);
    ASSERT(groupAddr.isMulticast());

    timer = new cMessage("IGMPv3 Host Group Timer", IGMPV3_H_GROUP_TIMER);
    timer->setContextPointer(this);
}

IGMPv3::HostGroupData::~HostGroupData()
{
    parent->owner->cancelAndDelete(timer);
}

string IGMPv3::HostGroupData::getStateInfo() const
{
    ostringstream out;
    switch (filter) {
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
    : parent(parent), groupAddr(group), filter(IGMPV3_FM_INCLUDE), state(IGMPV3_RGS_NO_MEMBERS_PRESENT)
{
    ASSERT(parent);
    ASSERT(groupAddr.isMulticast());

    timer = new cMessage("IGMPv3 router group timer", IGMPV3_R_GROUP_TIMER);
    timer->setContextPointer(this);
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

IGMPv3::SourceRecord *IGMPv3::RouterGroupData::getOrCreateSourceRecord(IPv4Address source)
{
    auto it = sources.find(source);
    if (it != sources.end())
        return it->second;
    return createSourceRecord(source);
}

void IGMPv3::RouterGroupData::deleteSourceRecord(IPv4Address source)
{
    auto it = sources.find(source);
    if (it != sources.end()) {
        SourceRecord *record = it->second;
        sources.erase(it);
        delete record;
    }
}

string IGMPv3::RouterGroupData::getStateInfo() const
{
    ostringstream out;
    switch (filter) {
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
void IGMPv3::RouterGroupData::collectForwardedSources(IPv4MulticastSourceList& result) const
{
    switch (filter) {
        case IGMPV3_FM_INCLUDE:
            result.filterMode = MCAST_INCLUDE_SOURCES;
            result.sources.clear();
            for (const auto & elem : sources) {
                if (elem.second->sourceTimer && elem.second->sourceTimer->isScheduled())
                    result.sources.push_back(elem.first);
            }
            break;

        case IGMPV3_FM_EXCLUDE:
            result.filterMode = MCAST_EXCLUDE_SOURCES;
            result.sources.clear();
            for (const auto & elem : sources) {
                if (!elem.second->sourceTimer || !elem.second->sourceTimer->isScheduled())
                    result.sources.push_back(elem.first);
            }
            break;
    }
}

void IGMPv3::RouterGroupData::printSourceList(ostream& out, bool withRunningTimer) const
{
    bool first = true;
    for (const auto & elem : sources) {
        bool timerIsRunning = elem.second->sourceTimer && elem.second->sourceTimer->isScheduled();
        if (withRunningTimer == timerIsRunning) {
            if (!first)
                out << ",";
            first = false;
            out << elem.first;
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

    for (auto & elem : groups)
        delete elem.second;
}

IGMPv3::HostGroupData *IGMPv3::HostInterfaceData::getOrCreateGroupData(IPv4Address group)
{
    auto it = groups.find(group);
    if (it != groups.end())
        return it->second;

    HostGroupData *data = new HostGroupData(this, group);
    groups[group] = data;
    owner->numGroups++;
    owner->numHostGroups++;
    return data;
}

void IGMPv3::HostInterfaceData::deleteGroupData(IPv4Address group)
{
    auto it = groups.find(group);
    if (it != groups.end()) {
        HostGroupData *data = it->second;
        groups.erase(it);
        delete data;
        owner->numHostGroups--;
        owner->numGroups--;
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

    for (auto & elem : groups)
        delete elem.second;
}

IGMPv3::RouterGroupData *IGMPv3::RouterInterfaceData::getOrCreateGroupData(IPv4Address group)
{
    auto it = groups.find(group);
    if (it != groups.end())
        return it->second;

    RouterGroupData *data = new RouterGroupData(this, group);
    groups[group] = data;
    owner->numGroups++;
    owner->numRouterGroups++;
    return data;
}

void IGMPv3::RouterInterfaceData::deleteGroupData(IPv4Address group)
{
    auto it = groups.find(group);
    if (it != groups.end()) {
        RouterGroupData *data = it->second;
        groups.erase(it);
        delete data;
        owner->numRouterGroups--;
        owner->numGroups--;
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
    auto it = hostData.find(interfaceId);
    if (it != hostData.end())
        return it->second;

    HostInterfaceData *data = createHostInterfaceData(ie);
    hostData[interfaceId] = data;
    return data;
}

IGMPv3::RouterInterfaceData *IGMPv3::getRouterInterfaceData(InterfaceEntry *ie)
{
    int interfaceId = ie->getInterfaceId();
    auto it = routerData.find(interfaceId);
    if (it != routerData.end())
        return it->second;

    RouterInterfaceData *data = createRouterInterfaceData(ie);
    routerData[interfaceId] = data;
    return data;
}

void IGMPv3::deleteHostInterfaceData(int interfaceId)
{
    auto interfaceIt = hostData.find(interfaceId);
    if (interfaceIt != hostData.end()) {
        HostInterfaceData *interface = interfaceIt->second;
        hostData.erase(interfaceIt);
        delete interface;
    }
}

void IGMPv3::deleteRouterInterfaceData(int interfaceId)
{
    auto interfaceIt = routerData.find(interfaceId);
    if (interfaceIt != routerData.end()) {
        RouterInterfaceData *interface = interfaceIt->second;
        routerData.erase(interfaceIt);
        delete interface;
    }
}

void IGMPv3::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        rt = getModuleFromPar<IIPv4RoutingTable>(par("routingTableModule"), this);

        cModule *host = getContainingNode(this);
        host->subscribe(NF_INTERFACE_DELETED, this);
        host->subscribe(NF_IPv4_MCAST_CHANGE, this);

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
        lastMemberQueryTime = lastMemberQueryInterval * lastMemberQueryCount;    //todo checknut ci je to takto..
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
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        for (int i = 0; i < (int)ift->getNumInterfaces(); ++i) {
            InterfaceEntry *ie = ift->getInterface(i);
            if (ie->isMulticast())
                configureInterface(ie);
        }

        cModule *host = getContainingNode(this);
        host->subscribe(NF_INTERFACE_CREATED, this);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER_2) {    // ipv4Data() created in INITSTAGE_NETWORK_LAYER
        // in multicast routers: join to ALL_IGMPv3_ROUTERS_MCAST address on all interfaces
        if (enabled && rt->isMulticastForwardingEnabled()) {
            for (int i = 0; i < (int)ift->getNumInterfaces(); ++i) {
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

void IGMPv3::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG)
{
    Enter_Method_Silent();

    InterfaceEntry *ie;
    int interfaceId;
    const IPv4MulticastGroupSourceInfo *info;
    if (signalID == NF_INTERFACE_CREATED) {
        ie = const_cast<InterfaceEntry *>(check_and_cast<const InterfaceEntry *>(obj));
        if (ie->isMulticast())
            configureInterface(ie);
    }
    else if (signalID == NF_INTERFACE_DELETED) {
        ie = const_cast<InterfaceEntry *>(check_and_cast<const InterfaceEntry *>(obj));
        if (ie->isMulticast()) {
            interfaceId = ie->getInterfaceId();
            deleteHostInterfaceData(interfaceId);
            deleteRouterInterfaceData(interfaceId);
        }
    }
    else if (signalID == NF_IPv4_MCAST_CHANGE) {
        info = check_and_cast<const IPv4MulticastGroupSourceInfo *>(obj);
        multicastSourceListChanged(info->ie, info->groupAddress, info->sourceList);
    }
}

void IGMPv3::configureInterface(InterfaceEntry *ie)
{
    if (enabled && rt->isMulticastForwardingEnabled()) {
        // start querier on this interface
        EV_INFO << "Sending General Query on interface '" << ie->getName() << "', and scheduling next Query to '"
                << (simTime() + startupQueryInterval) << "'.\n";
        RouterInterfaceData *routerData = getRouterInterfaceData(ie);
        routerData->state = IGMPV3_RS_QUERIER;

        sendGeneralQuery(routerData, queryResponseInterval);
        startTimer(routerData->generalQueryTimer, startupQueryInterval);
    }
}

void IGMPv3::handleMessage(cMessage *msg)
{
    if (!enabled) {
        if (!msg->isSelfMessage()) {
            EV << "IGMPv3 disabled, dropping packet.\n";
            delete msg;
        }
        return;
    }

    if (msg->isSelfMessage()) {
        switch (msg->getKind()) {
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
    else
        processIgmpMessage(check_and_cast<IGMPMessage *>(msg));
}

void IGMPv3::processIgmpMessage(IGMPMessage *msg)
{
    switch (msg->getType()) {
        case IGMP_MEMBERSHIP_QUERY:
            if (dynamic_cast<IGMPv3Query *>(msg))
                processQuery((IGMPv3Query *)msg);
            else
                /* TODO process v1 and v2 queries*/;
            break;

        case IGMPV3_MEMBERSHIP_REPORT:
            processReport(check_and_cast<IGMPv3Report *>(msg));
            break;

        // TODO process v1/v2 reports
        default:
            //delete msg;
            throw cRuntimeError("IGMPv3: Unhandled message type (%dq)", msg->getType());
    }
}

void IGMPv3::startTimer(cMessage *timer, double interval)
{
    ASSERT(timer);
    cancelEvent(timer);
    scheduleAt(simTime() + interval, timer);
}

void IGMPv3::sendGeneralQuery(RouterInterfaceData *interfaceData, double maxRespTime)
{
    if (interfaceData->state == IGMPV3_RS_QUERIER) {
        IGMPv3Query *msg = new IGMPv3Query("IGMPv3 query");
        msg->setType(IGMP_MEMBERSHIP_QUERY);
        msg->setMaxRespCode((int)(maxRespTime * 10.0));
        msg->setByteLength(12);
        sendQueryToIP(msg, interfaceData->ie, IPv4Address::ALL_HOSTS_MCAST);

        numQueriesSent++;
        numGeneralQueriesSent++;
    }
}

// See RFC 3376 6.6.3.1.
// Also from RFC 3376 6.4.2):
// ..., when a router queries a specific group, it lowers its
// group timer for that group to a small interval of Last Member Query
// Time seconds.
void IGMPv3::sendGroupSpecificQuery(RouterGroupData *groupData)
{
    RouterInterfaceData *interfaceData = groupData->parent;
    bool suppressFlag = groupData->timer->isScheduled() && groupData->timer->getArrivalTime() > simTime() + lastMemberQueryTime;

    // Set group timer to LMQT
    startTimer(groupData->timer, lastMemberQueryTime);

    if (interfaceData->state == IGMPV3_RS_QUERIER) {
        IGMPv3Query *msg = new IGMPv3Query("IGMPv3 query");
        msg->setType(IGMP_MEMBERSHIP_QUERY);
        msg->setGroupAddress(groupData->groupAddr);
        msg->setMaxRespCode((int)(lastMemberQueryInterval * 10.0));    // FIXME compute code
        msg->setSuppressRouterProc(suppressFlag);
        msg->setByteLength(12);
        sendQueryToIP(msg, interfaceData->ie, groupData->groupAddr);

        numQueriesSent++;
        numGroupSpecificQueriesSent++;
    }

    // TODO retransmission [Last Member Query Count]-1 times
}

// See RFC 3376 6.6.3.2.
void IGMPv3::sendGroupAndSourceSpecificQuery(RouterGroupData *groupData, const IPv4AddressVector& sources)
{
    ASSERT(!sources.empty());

    RouterInterfaceData *interfaceData = groupData->parent;

    if (interfaceData->state == IGMPV3_RS_QUERIER) {
        IGMPv3Query *msg = new IGMPv3Query("IGMPv3 query");
        msg->setType(IGMP_MEMBERSHIP_QUERY);
        msg->setGroupAddress(groupData->groupAddr);
        msg->setMaxRespCode((int)(lastMemberQueryInterval * 10.0));    // FIXME compute code
        msg->setSourceList(sources);
        msg->setByteLength(12 + (4 * sources.size()));
        sendQueryToIP(msg, interfaceData->ie, groupData->groupAddr);

        numQueriesSent++;
        numGroupAndSourceSpecificQueriesSent++;
    }
}

void IGMPv3::processRouterGeneralQueryTimer(cMessage *msg)
{
    RouterInterfaceData *interfaceData = (RouterInterfaceData *)msg->getContextPointer();
    InterfaceEntry *ie = interfaceData->ie;
    ASSERT(ie);
    EV_INFO << "General Query timer expired on interface='" << ie->getName() << "'.\n";
    RouterState state = interfaceData->state;
    if (state == IGMPV3_RS_QUERIER || state == IGMPV3_RS_NON_QUERIER) {
        EV_INFO << "Sending General Query on interface '" << ie->getName() << "', and scheduling next Query to '"
                << (simTime() + queryInterval) << "'.\n";
        interfaceData->state = IGMPV3_RS_QUERIER;
        sendGeneralQuery(interfaceData, queryResponseInterval);
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
    HostInterfaceData *interfaceData = (HostInterfaceData *)msg->getContextPointer();
    InterfaceEntry *ie = interfaceData->ie;
    ASSERT(ie);

    EV_INFO << "Response timer to a General Query on interface '" << ie->getName() << "' has expired.\n";

    //HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    IGMPv3Report *report = new IGMPv3Report("IGMPv3 report");
    unsigned int byteLength = 8;   // IGMPv3Report header size
    report->setType(IGMPV3_MEMBERSHIP_REPORT);
    int counter = 0;
    report->setGroupRecordArraySize(interfaceData->groups.size());

    // FIXME Do not create reports larger than MTU of the interface
    //

    /*
     * creating GroupRecord for each group on interface
     */
    for (auto & elem : interfaceData->groups) {
        GroupRecord gr;
        if (elem.second->filter == IGMPV3_FM_INCLUDE) {
            gr.recordType = IGMPV3_RT_IS_IN;
        }
        else if (elem.second->filter == IGMPV3_FM_EXCLUDE) {
            gr.recordType = IGMPV3_RT_IS_EX;
        }
        gr.groupAddress = elem.second->groupAddr;
        gr.sourceList = elem.second->sourceAddressList;
        report->setGroupRecord(counter++, gr);
        byteLength += 8 + gr.sourceList.size() * 4;    // 8 byte header + n * 4 byte (IPv4Address)
    }
    report->setByteLength(byteLength);

    if (counter != 0) {    //if no record created, dont need to send report
        EV_INFO << "Sending response to a General Query on interface '" << ie->getName() << "'.\n";
        sendReportToIP(report, ie, IPv4Address::ALL_IGMPV3_ROUTERS_MCAST);
        numReportsSent++;
    }
    else {
        EV_INFO << "There are no multicast listeners, no response is sent to a General Query on interface '" << ie->getName() << "'.\n";
        delete report;
    }
}

// RFC 3376 5.2  report generation, point 2. and 3.
void IGMPv3::processHostGroupQueryTimer(cMessage *msg)
{
    HostGroupData *group = (HostGroupData *)msg->getContextPointer();
    InterfaceEntry *ie = group->parent->ie;

    vector<GroupRecord> records(1);

    //checking if query is group or group-and-source specific
    if (group->queriedSources.empty()) {
        // Send report for a Group-Specific Query
        EV_INFO << "Response timer for a Group-Specific Query for group '" << group->groupAddr << "' on interface '" << ie->getName() << "'\n";

        records[0].groupAddress = group->groupAddr;
        records[0].recordType = group->filter == IGMPV3_FM_INCLUDE ? IGMPV3_RT_IS_IN : IGMPV3_RT_IS_EX;
        records[0].sourceList = group->sourceAddressList;
        sendGroupReport(ie, records);
    }
    else {
        // Send report for a Group-and-Source-Specific Query
        EV_INFO << "Response timer for a Group-and-Source-Specific Query for group '" << group->groupAddr << "' on interface '" << ie->getName() << "'\n";

        records[0].groupAddress = group->groupAddr;
        records[0].recordType = IGMPV3_RT_IS_IN;
        records[0].sourceList = group->filter == IGMPV3_FM_INCLUDE ? set_intersection(group->sourceAddressList, group->queriedSources) :
            set_complement(group->queriedSources, group->sourceAddressList);
        sendGroupReport(ie, records);
    }

    group->queriedSources.clear();
}

static bool isEmptyRecord(const GroupRecord& record)
{
    return record.sourceList.empty();
}

/**
 * This function is sending report message if interface state was changed.
 * See RFC 3376 5.1.
 */
void IGMPv3::multicastSourceListChanged(InterfaceEntry *ie, IPv4Address group, const IPv4MulticastSourceList& sourceList)
{
    ASSERT(ie);
    ASSERT(isSorted(sourceList.sources));

    if (!enabled || group.isLinkLocalMulticast())
        return;

    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    HostGroupData *groupData = interfaceData->getOrCreateGroupData(group);

    FilterMode filter = sourceList.filterMode == MCAST_INCLUDE_SOURCES ? IGMPV3_FM_INCLUDE : IGMPV3_FM_EXCLUDE;

    EV_DETAIL_C("test") << "State of group '" << group << "' on interface '" << ie->getName() << "' has changed:\n";
    EV_DETAIL_C("test") << "    Old state: " << groupData->getStateInfo() << ".\n";
    EV_DETAIL_C("test") << "    New state: " << (filter == IGMPV3_FM_INCLUDE ? "INCLUDE" : "EXCLUDE") << sourceList.sources << ".\n";

    // Check if IF state is different
    if (!(groupData->filter == filter) || !(groupData->sourceAddressList == sourceList.sources)) {
        // INCLUDE(A) -> INCLUDE(B): Send ALLOW(B-A), BLOCK(A-B)
        if (groupData->filter == IGMPV3_FM_INCLUDE && filter == IGMPV3_FM_INCLUDE && groupData->sourceAddressList != sourceList.sources) {
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
        }
        // EXCLUDE(A) -> EXCLUDE(B): Send ALLOW(A-B), BLOCK(B-A)
        else if (groupData->filter == IGMPV3_FM_EXCLUDE && filter == IGMPV3_FM_EXCLUDE && groupData->sourceAddressList != sourceList.sources) {
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
        }
        // INCLUDE(A) -> EXCLUDE(B): Send TO_EX(B)
        else if (groupData->filter == IGMPV3_FM_INCLUDE && filter == IGMPV3_FM_EXCLUDE) {
            EV_DETAIL << "Sending TO_EX report.\n";
            vector<GroupRecord> records(1);
            records[0].groupAddress = group;
            records[0].recordType = IGMPV3_RT_TO_EX;
            records[0].sourceList = sourceList.sources;
            sendGroupReport(ie, records);
        }
        // EXCLUDE(A) -> INCLUDE(B): Send TO_IN(B)
        else if (groupData->filter == IGMPV3_FM_EXCLUDE && filter == IGMPV3_FM_INCLUDE) {
            EV_DETAIL << "Sending TO_IN report.\n";
            vector<GroupRecord> records(1);
            records[0].groupAddress = group;
            records[0].recordType = IGMPV3_RT_TO_IN;
            records[0].sourceList = sourceList.sources;
            sendGroupReport(ie, records);
        }

        // Go to new state
        groupData->filter = filter;
        groupData->sourceAddressList = sourceList.sources;
        sort(groupData->sourceAddressList.begin(), groupData->sourceAddressList.end());
    }

    // FIXME missing: the report  is retransmitted [Robustness Variable] - 1 more times,
    //       at intervals chosen at random from the range (0, [Unsolicited Report Interval])

    // FIXME if an interface change occured when there is a pending report, then
    //       the groups of the old report and the new report are to be merged.
}

// RFC 3376 5.2
void IGMPv3::processQuery(IGMPv3Query *msg)
{
    IPv4ControlInfo *controlInfo = (IPv4ControlInfo *)msg->getControlInfo();
    InterfaceEntry *ie = ift->getInterfaceById(controlInfo->getInterfaceId());
    IPv4Address groupAddr = msg->getGroupAddress();
    IPv4AddressVector& queriedSources = msg->getSourceList();
    double maxRespTime = decodeTime(msg->getMaxRespCode());

    ASSERT(ie->isMulticast());

    EV_INFO << "Received IGMPv3 query on interface '" << ie->getName() << "' for group '" << groupAddr << "'.\n";

    HostInterfaceData *interfaceData = getHostInterfaceData(ie);

    numQueriesRecv++;

    double delay = uniform(0.0, maxRespTime);

    // Rules from RFC page 22:
    if (interfaceData->generalQueryTimer->isScheduled() && interfaceData->generalQueryTimer->getArrivalTime() < simTime() + delay) {
        // 1. If there is a pending response to a previous General Query
        //    scheduled sooner than the selected delay, no additional response
        //    needs to be scheduled.
        EV_DETAIL << "There is a pending response to a previous General Query, no further response is scheduled.\n";
    }
    else if (groupAddr.isUnspecified() && queriedSources.empty()) {
        // 2. If the received Query is a General Query, the interface timer is
        //    used to schedule a response to the General Query after the
        //    selected delay.  Any previously pending response to a General
        //    Query is canceled.
        EV_DETAIL << "Received a General Query, scheduling report with delay=" << delay << ".\n";
        startTimer(interfaceData->generalQueryTimer, delay);
    }
    else if (!groupAddr.isUnspecified()) {
        HostGroupData *groupData = interfaceData->getOrCreateGroupData(groupAddr);

        if (!groupData->timer->isScheduled()) {
            // 3. If the received Query is a Group-Specific Query or a Group-and-
            //    Source-Specific Query and there is no pending response to a
            //    previous Query for this group, then the group timer is used to
            //    schedule a report.  If the received Query is a Group-and-Source-
            //    Specific Query, the list of queried sources is recorded to be used
            //    when generating a response.
            EV_DETAIL << "Received Group" << (queriedSources.empty() ? "" : "-and-Source") << "-Specific Query, "
                      << "scheduling report with delay=" << delay << ".\n";

            sort(queriedSources.begin(), queriedSources.end());
            groupData->queriedSources = queriedSources;
            startTimer(groupData->timer, delay);
        }
        else if (queriedSources.empty()) {
            //4. If there already is a pending response to a previous Query
            //   scheduled for this group, and either the new Query is a Group-
            //   Specific Query or the recorded source-list associated with the
            //   group is empty, then the group source-list is cleared and a single
            //   response is scheduled using the group timer.  The new response is
            //   scheduled to be sent at the earliest of the remaining time for the
            //   pending report and the selected delay.
            EV_DETAIL << "Received Group-Specific Query, scheduling report with delay="
                      << min(delay, SIMTIME_DBL(groupData->timer->getArrivalTime() - simTime())) << ".\n";

            sort(queriedSources.begin(), queriedSources.end());
            groupData->queriedSources = queriedSources;
            if (groupData->timer->getArrivalTime() > simTime() + delay)
                startTimer(groupData->timer, delay);
        }
        else {
            // 5. If the received Query is a Group-and-Source-Specific Query and
            //    there is a pending response for this group with a non-empty
            //    source-list, then the group source list is augmented to contain
            //    the list of sources in the new Query and a single response is
            //    scheduled using the group timer.  The new response is scheduled to
            //    be sent at the earliest of the remaining time for the pending
            //    report and the selected delay.
            EV_DETAIL << "Received Group-and-Source-Specific Query, combining sources with the sources of pending report, "
                      << "and scheduling a new report with delay="
                      << min(delay, SIMTIME_DBL(groupData->timer->getArrivalTime() - simTime())) << ".\n";

            if (groupData->timer->getArrivalTime() > simTime() + delay) {
                sort(queriedSources.begin(), queriedSources.end());
                groupData->queriedSources = set_union(groupData->queriedSources, queriedSources);
                startTimer(groupData->timer, delay);
            }
        }
    }

// Router part | Querier Election
    if (rt->isMulticastForwardingEnabled()) {
        //Querier Election
        RouterInterfaceData *routerInterfaceData = getRouterInterfaceData(ie);
        if (controlInfo->getSrcAddr() < ie->ipv4Data()->getIPAddress()) {
            startTimer(routerInterfaceData->generalQueryTimer, otherQuerierPresentInterval);
            routerInterfaceData->state = IGMPV3_RS_NON_QUERIER;
        }

        if (!groupAddr.isUnspecified() && routerInterfaceData->state == IGMPV3_RS_NON_QUERIER) {    // group specific query
            RouterGroupData *groupData = routerInterfaceData->getOrCreateGroupData(groupAddr);
            if (groupData->state == IGMPV3_RGS_MEMBERS_PRESENT) {
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
    if (code < 128)
        time = code;
    else {
        unsigned mantis = code & 0x15;
        unsigned exp = (code >> 4) & 0x07;
        time = (mantis | 0x10) << (exp + 3);
    }

    return (double)time / 10.0;
}

void IGMPv3::sendGroupReport(InterfaceEntry *ie, const vector<GroupRecord>& records)
{
    EV << "IGMPv3: sending Membership Report on iface=" << ie->getName() << "\n";
    IGMPv3Report *msg = new IGMPv3Report("IGMPv3 report");
    unsigned int byteLength = 8;   // IGMPv3Report header size
    msg->setType(IGMPV3_MEMBERSHIP_REPORT);
    msg->setGroupRecordArraySize(records.size());
    for (int i = 0; i < (int)records.size(); ++i) {
        IPv4Address group = records[i].groupAddress;
        ASSERT(group.isMulticast() && !group.isLinkLocalMulticast());
        msg->setGroupRecord(i, records[i]);
        byteLength += 8 + records[i].sourceList.size() * 4;    // 8 byte header + n * 4 byte (IPv4Address)
    }
    msg->setByteLength(byteLength);

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

    if (rt->isMulticastForwardingEnabled()) {
        RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);

        for (unsigned int i = 0; i < msg->getGroupRecordArraySize(); i++) {
            GroupRecord gr = msg->getGroupRecord(i);
            EV_DETAIL << "Found a record for group '" << gr.groupAddress << "'.\n";

            // ensure that the received sources are sorted
            sort(gr.sourceList.begin(), gr.sourceList.end());
            IPv4AddressVector& receivedSources = gr.sourceList;    // sorted

            RouterGroupData *groupData = interfaceData->getOrCreateGroupData(gr.groupAddress);

            IPv4MulticastSourceList oldSourceList;
            groupData->collectForwardedSources(oldSourceList);

            EV_DETAIL << "Router State is " << groupData->getStateInfo() << ".\n";

            // RFC 3376 6.4.1: Reception of Current State Record
            if (gr.recordType == IGMPV3_RT_IS_IN) {
                EV_DETAIL << "Received IS_IN" << receivedSources << " report.\n";
                // INCLUDE(A)   -> IS_IN(B) -> INCLUDE(A+B)    : (B) = GMI
                // EXCLUDE(X,Y) -> IS_IN(A) -> EXCLUDE(X+A,Y-A): (A) = GMI
                for (auto & receivedSource : receivedSources) {
                    EV_DETAIL << "Setting source timer of '" << receivedSource << "' to '" << groupMembershipInterval << "'.\n";
                    SourceRecord *record = groupData->getOrCreateSourceRecord(receivedSource);
                    startTimer(record->sourceTimer, groupMembershipInterval);
                }
            }
            else if (gr.recordType == IGMPV3_RT_IS_EX) {
                EV_DETAIL << "Received IS_EX" << receivedSources << " report.\n";

                // Group Timer = GMI
                EV_DETAIL << "Setting group timer to '" << groupMembershipInterval << "'.\n";
                startTimer(groupData->timer, groupMembershipInterval);

                // INCLUDE(A)   -> IS_EX(B) -> EXCLUDE(A*B,B-A): Delete (A-B)
                // EXCLUDE(X,Y) -> IS_EX(A) -> EXCLUDE(A-Y,Y*A): Delete (X-A) Delete (Y-A)
                for (auto it = groupData->sources.begin(); it != groupData->sources.end(); ++it) {
                    if (find(receivedSources.begin(), receivedSources.end(), it->first) == receivedSources.end()) {
                        EV_DETAIL << "Deleting source record of '" << it->first << "'.\n";
                        groupData->deleteSourceRecord(it->first);
                    }
                }

                // Router State == INCLUDE(A),   Report == IS_EX(B): (B-A) = 0
                // Router State == EXCLUDE(X,Y), Report == IS_EX(A): (A-X-Y) = GMI
                for (auto & receivedSource : receivedSources) {
                    if (!groupData->hasSourceRecord(receivedSource)) {
                        SourceRecord *record = groupData->createSourceRecord(receivedSource);
                        double timerValue = groupData->filter == IGMPV3_FM_INCLUDE ? 0.0 : groupMembershipInterval;
                        EV_DETAIL << "Setting source timer of '" << receivedSource << "' to '" << timerValue << "'.\n";
                        if (timerValue > 0)
                            startTimer(record->sourceTimer, groupMembershipInterval);
                    }
                }

                groupData->filter = IGMPV3_FM_EXCLUDE;
            }
            // RFC 3376 6.4.2: Reception of Filter-Mode-Change and Source-List-Change Records
            else if (gr.recordType == IGMPV3_RT_ALLOW) {
                EV_DETAIL << "Received ALLOW" << receivedSources << " report.\n";

                // INCLUDE(A)   -> ALLOW(B) -> INCLUDE(A+B):     (B) = GMI
                // EXCLUDE(X,Y) -> ALLOW(A) -> EXCLUDE(X+A,Y-A): (A) = GMI
                for (auto & receivedSource : receivedSources) {
                    EV_DETAIL << "Setting source timer of '" << receivedSource << "' to '" << groupMembershipInterval << "'.\n";
                    SourceRecord *record = groupData->getOrCreateSourceRecord(receivedSource);
                    startTimer(record->sourceTimer, groupMembershipInterval);
                }
            }
            else if (gr.recordType == IGMPV3_RT_BLOCK) {
                EV_DETAIL << "Received BLOCK" << receivedSources << " report.\n";

                if (groupData->filter == IGMPV3_FM_INCLUDE) {
                    // INCLUDE(A) -> BLOCK(B) -> INCLUDE(A): Send Q(G,A*B)
                    IPv4AddressVector sourcesA;
                    for (auto & elem : groupData->sources)
                        sourcesA.push_back(elem.first);

                    IPv4AddressVector aIntersectB = set_intersection(sourcesA, receivedSources);
                    if (!aIntersectB.empty()) {
                        EV_INFO << "Sending Group-and-Source-Specific Query for group '" << groupData->groupAddr
                                << "' on interface '" << ie->getName() << "'.\n";
                        sendGroupAndSourceSpecificQuery(groupData, aIntersectB);
                    }
                }
                else if (groupData->filter == IGMPV3_FM_EXCLUDE) {
                    // EXCLUDE (X,Y) -> BLOCK (A) -> EXCLUDE (X+(A-Y),Y): (A-X-Y)=Group Timer
                    for (auto it = groupData->sources.begin(); it != groupData->sources.end(); ++it) {
                        if (!groupData->hasSourceRecord(it->first)) {
                            SourceRecord *record = groupData->createSourceRecord(it->first);
                            double grouptimertime = groupData->timer->getArrivalTime().dbl() - simTime().dbl();
                            EV_DETAIL << "Setting source timer of '" << it->first << "' to '" << grouptimertime << "'.\n";
                            startTimer(record->sourceTimer, grouptimertime);
                        }
                    }
                    // Send Q(G,A-Y)
                    IPv4AddressVector ySources;
                    for (auto & elem : groupData->sources) {
                        if (!elem.second->sourceTimer->isScheduled()) {
                            ySources.push_back(elem.first);
                        }
                    }
                    IPv4AddressVector aMinusY = set_complement(receivedSources, ySources);
                    if (!aMinusY.empty()) {
                        EV_INFO << "Sending Group-and-Source-Specific Query for group '" << groupData->groupAddr
                                << "' on interface '" << ie->getName() << "'.\n";
                        sendGroupAndSourceSpecificQuery(groupData, aMinusY);
                    }
                }
            }
            else if (gr.recordType == IGMPV3_RT_TO_IN) {
                EV_DETAIL << "Received TO_IN" << receivedSources << " report.\n";

                if (groupData->filter == IGMPV3_FM_INCLUDE) {
                    // INCLUDE(A) -> TO_IN (B) -> INCLUDE (A+B): (B)=GMI
                    for (auto & receivedSource : receivedSources) {
                        EV_DETAIL << "Setting source timer of '" << receivedSource << "' to '" << groupMembershipInterval << "'.\n";
                        SourceRecord *record = groupData->getOrCreateSourceRecord(receivedSource);
                        startTimer(record->sourceTimer, groupMembershipInterval);
                    }
                    // Send Q(G,A-B)
                    IPv4AddressVector sourcesA;
                    for (auto & elem : groupData->sources)
                        sourcesA.push_back(elem.first);
                    IPv4AddressVector aMinusB = set_complement(sourcesA, receivedSources);
                    if (!aMinusB.empty()) {
                        EV_INFO << "Sending Group-and-Source-Specific Query for group '" << groupData->groupAddr
                                << "' on interface '" << ie->getName() << "'.\n";
                        sendGroupAndSourceSpecificQuery(groupData, aMinusB);
                    }
                }
                else if (groupData->filter == IGMPV3_FM_EXCLUDE) {
                    // compute X before modifying the state
                    IPv4AddressVector sourcesX;
                    for (auto & elem : groupData->sources) {
                        if (elem.second->sourceTimer->isScheduled())
                            sourcesX.push_back(elem.first);
                    }

                    // EXCLUDE(X,Y) -> TO_IN(A) -> EXCLUDE(X+A,Y-A): (A) = GMI
                    for (auto & receivedSource : receivedSources) {
                        EV_DETAIL << "Setting source timer of '" << receivedSource << "' to '" << groupMembershipInterval << "'.\n";
                        SourceRecord *record = groupData->getOrCreateSourceRecord(receivedSource);
                        startTimer(record->sourceTimer, groupMembershipInterval);
                    }

                    // Send Q(G,X-A)
                    IPv4AddressVector xMinusA = set_complement(sourcesX, receivedSources);
                    if (!xMinusA.empty()) {
                        EV_INFO << "Sending Group-and-Source-Specific Query for group '" << groupData->groupAddr
                                << "' on interface '" << ie->getName() << "'.\n";
                        sendGroupAndSourceSpecificQuery(groupData, xMinusA);
                    }

                    // Send Q(G)
                    EV_INFO << "Sending Group-Specific Query for group '" << groupData->groupAddr
                            << "' on interface '" << ie->getName() << "'.\n";
                    sendGroupSpecificQuery(groupData);
                }
            }
            else if (gr.recordType == IGMPV3_RT_TO_EX) {
                EV_DETAIL << "Received TO_EX" << receivedSources << " report.\n";

                if (groupData->filter == IGMPV3_FM_INCLUDE) {
                    // INCLUDE (A) -> TO_EX(B) -> EXCLUDE (A*B,B-A): Group Timer = GMI
                    EV_DETAIL << "Setting group timer to '" << groupMembershipInterval << "'.\n";
                    startTimer(groupData->timer, groupMembershipInterval);

                    //change to mode exclude
                    groupData->filter = IGMPV3_FM_EXCLUDE;

                    // save A
                    IPv4AddressVector sourcesA;
                    for (auto & elem : groupData->sources)
                        sourcesA.push_back(elem.first);

                    // (B-A) = 0
                    for (auto & receivedSource : receivedSources) {
                        if (!groupData->hasSourceRecord(receivedSource)) {
                            EV_DETAIL << "Setting source timer of '" << receivedSource << "' to '0'.\n";
                            groupData->createSourceRecord(receivedSource);
                        }
                    }

                    // Delete A-B
                    for (auto & elem : sourcesA) {
                        if (find(receivedSources.begin(), receivedSources.end(), elem) == receivedSources.end()) {
                            EV_DETAIL << "Deleting source record of '" << elem << "'.\n";
                            groupData->deleteSourceRecord(elem);
                        }
                    }

                    // Send Q(G,A*B)
                    IPv4AddressVector aIntersectB = set_intersection(sourcesA, receivedSources);
                    if (!aIntersectB.empty()) {
                        EV_INFO << "Sending Group-Specific Query for group '" << groupData->groupAddr
                                << "' on interface '" << ie->getName() << "'.\n";
                        sendGroupAndSourceSpecificQuery(groupData, aIntersectB);
                    }
                }
                else if (groupData->filter == IGMPV3_FM_EXCLUDE) {
                    // EXCLUDE (X,Y) -> TO_EX (A) -> EXCLUDE (A-Y,Y*A): Group Timer = GMI
                    EV_DETAIL << "Setting group timer to '" << groupMembershipInterval << "'.\n";
                    startTimer(groupData->timer, groupMembershipInterval);

                    // save Y
                    IPv4AddressVector sourcesY;
                    for (auto & elem : groupData->sources) {
                        if (!elem.second->sourceTimer->isScheduled())
                            sourcesY.push_back(elem.first);
                    }

                    // Delete (X-A) Delete (Y-A)
                    for (auto it = groupData->sources.begin(); it != groupData->sources.end(); ) {
                        auto rec = it->first;
                        ++it; // let's advance the iterator now because the deleteSourcerecord call will invalidate it and we wont be able to increment it after that
                        if (find(receivedSources.begin(), receivedSources.end(), it->first) == receivedSources.end()) {
                            EV_DETAIL << "Deleting source record of '" << rec << "'.\n";
                            groupData->deleteSourceRecord(rec);
                        }
                    }

                    // (A-X-Y) = GMI FIXME should be set to Group Timer
                    for (auto & receivedSource : receivedSources) {
                        if (!groupData->hasSourceRecord(receivedSource)) {
                            EV_DETAIL << "Setting source timer of '" << receivedSource << "' to '" << groupMembershipInterval << "'.\n";
                            SourceRecord *record = groupData->createSourceRecord(receivedSource);
                            startTimer(record->sourceTimer, groupMembershipInterval);
                        }
                    }

                    // Send Q(G,A-Y)
                    IPv4AddressVector aMinusY = set_complement(receivedSources, sourcesY);
                    if (!aMinusY.empty()) {
                        EV_INFO << "Sending Group-Specific Query for group '" << groupData->groupAddr
                                << "' on interface '" << ie->getName() << "'.\n";
                        sendGroupAndSourceSpecificQuery(groupData, aMinusY);
                    }
                }
            }

            EV_DETAIL << "New Router State is " << groupData->getStateInfo() << ".\n";

            // update interface state
            IPv4MulticastSourceList newSourceList;
            groupData->collectForwardedSources(newSourceList);

            if (newSourceList != oldSourceList) {
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
    RouterGroupData *groupData = (RouterGroupData *)msg->getContextPointer();
    InterfaceEntry *ie = groupData->parent->ie;

    EV_INFO << "Group Timer for group '" << groupData->groupAddr << "' on interface '" << ie->getName() << "' has expired.\n";
    EV_DETAIL << "Router State is " << groupData->getStateInfo() << ".\n";

    if (groupData->filter == IGMPV3_FM_EXCLUDE) {
        bool timerRunning = false;
        for (auto it = groupData->sources.begin(); it != groupData->sources.end(); ++it) {
            if (!it->second->sourceTimer->isScheduled()) {
                EV_DETAIL << "Deleting source record of '" << it->first << "'.\n";
                groupData->deleteSourceRecord(it->first);
            }
            else {
                timerRunning = true;
            }
        }
        groupData->filter = IGMPV3_FM_INCLUDE;
        if (!timerRunning) {
            EV_DETAIL << "Deleting multicast listener for group '" << groupData->groupAddr << "' from the interface table.\n";
            ie->ipv4Data()->removeMulticastListener(groupData->groupAddr);
            groupData->parent->deleteGroupData(groupData->groupAddr);

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
    SourceRecord *sourceRecord = (SourceRecord *)msg->getContextPointer();
    RouterGroupData *groupData = sourceRecord->parent;
    InterfaceEntry *ie = groupData->parent->ie;

    EV_INFO << "Source timer for group '" << groupData->groupAddr << "' and source '" << sourceRecord->sourceAddr
            << "' on interface '" << ie->getName() << "' has expired.\n";

    bool last = true;
    if (groupData->filter == IGMPV3_FM_INCLUDE) {
        groupData->deleteSourceRecord(sourceRecord->sourceAddr);
    }
    for (auto & elem : groupData->sources) {
        if (elem.second->sourceTimer->isScheduled()) {
            last = false;
        }
    }
    if (last) {
        ie->ipv4Data()->removeMulticastListener(groupData->groupAddr);
        groupData->parent->deleteGroupData(groupData->groupAddr);
    }
}

}    // namespace inet

