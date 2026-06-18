//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/icmpv6/Mldv2.h"

#include <algorithm>

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/checksum/ChecksumMode_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/icmpv6/Icmpv6.h"
#include "inet/networklayer/icmpv6/MldMessage_m.h" // MLDv1 MldQuery/MldReport/MldDone for older-version interop
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"

namespace inet {

using namespace std;

Define_Module(Mldv2);

static bool isEmptyRecord(const Mldv2MulticastAddressRecord& record)
{
    return record.getSourceList().empty();
}

static bool isSorted(const Ipv6AddressVector& v)
{
    int n = (int)v.size();
    for (int i = 0; i < n - 1; ++i)
        if (v[i + 1] < v[i])
            return false;

    return true;
}

Mldv2::~Mldv2()
{
    while (!hostData.empty())
        deleteHostInterfaceData(hostData.begin()->first);

    while (!routerData.empty())
        deleteRouterInterfaceData(routerData.begin()->first);
}

void Mldv2::initialize(int stage)
{
    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        enabled = par("enabled");
        // robustnessVariable also seeds the NED default() expressions of
        // groupMembershipInterval / otherQuerierPresentInterval / startupQueryCount /
        // lastMemberQueryCount; in addition it controls how many times a host
        // (re)transmits a State-Change Report (RFC 3810 6.1).
        robustnessVariable = par("robustnessVariable");
        queryInterval = par("queryInterval");
        queryResponseInterval = par("queryResponseInterval");
        groupMembershipInterval = par("groupMembershipInterval");
        otherQuerierPresentInterval = par("otherQuerierPresentInterval");
        startupQueryInterval = par("startupQueryInterval");
        startupQueryCount = par("startupQueryCount");
        lastMemberQueryInterval = par("lastMemberQueryInterval");
        lastMemberQueryCount = par("lastMemberQueryCount");
        lastMemberQueryTime = lastMemberQueryInterval * lastMemberQueryCount;
        unsolicitedReportInterval = par("unsolicitedReportInterval");
        const char *checksumModeString = par("checksumMode");
        checksumMode = parseChecksumMode(checksumModeString, false);

        addWatches();
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);

        cModule *host = getContainingNode(this);

        // Register Protocol::mld so the lp dispatcher delivers MLD ICMPv6 messages to us
        registerProtocol(Protocol::mld, gate("ipOut"), gate("ipIn"));

        for (int i = 0; i < ift->getNumInterfaces(); ++i) {
            NetworkInterface *ie = ift->getInterface(i);
            if (ie->isMulticast()) {
                configureInterface(ie);
                if (auto ipv6interfaceData = ie->findProtocolDataForUpdate<Ipv6InterfaceData>()) {
                    int n = ipv6interfaceData->getNumOfJoinedMulticastGroups();
                    for (int j = 0; j < n; j++) {
                        auto groupAddress = ipv6interfaceData->getJoinedMulticastGroup(j);
                        const auto& sourceList = ipv6interfaceData->getJoinedMulticastSources(j);
                        multicastSourceListChanged(ie, groupAddress, sourceList);
                    }
                }
            }
        }

        host->subscribe(interfaceCreatedSignal, this);
        host->subscribe(interfaceDeletedSignal, this);
        host->subscribe(ipv6MulticastChangeSignal, this);

        // in multicast routers: join the all-MLDv2-routers address on all interfaces
        if (enabled && rt->isMulticastForwardingEnabled()) {
            for (int i = 0; i < ift->getNumInterfaces(); ++i) {
                NetworkInterface *ie = ift->getInterface(i);
                if (ie->isMulticast())
                    ie->getProtocolDataForUpdate<Ipv6InterfaceData>()->joinMulticastGroup(Ipv6Address::ALL_MLDV2_ROUTERS);
            }
        }
    }
}

void Mldv2::addWatches()
{
    WATCH(lastMemberQueryTime);
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

    WATCH(hostData);
    WATCH(routerData);
}

void Mldv2::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    NetworkInterface *ie;
    int interfaceId;
    const Ipv6MulticastGroupSourceInfo *info;

    if (signalID == ipv6MulticastChangeSignal) {
        info = check_and_cast<const Ipv6MulticastGroupSourceInfo *>(obj);
        multicastSourceListChanged(info->ie, info->groupAddress, info->sourceList);
    }
    else if (signalID == interfaceCreatedSignal) {
        ie = check_and_cast<NetworkInterface *>(obj);
        if (ie->isMulticast())
            configureInterface(ie);
    }
    else if (signalID == interfaceDeletedSignal) {
        ie = check_and_cast<NetworkInterface *>(obj);
        if (ie->isMulticast()) {
            interfaceId = ie->getInterfaceId();
            deleteHostInterfaceData(interfaceId);
            deleteRouterInterfaceData(interfaceId);
        }
    }
}

/**
 * This function sends a report message if the interface state has changed.
 * See RFC 3810 §6.1 (mirrors RFC 3376 §5.1 for IGMPv3).
 */
void Mldv2::multicastSourceListChanged(NetworkInterface *ie, const Ipv6Address& group, const Ipv6MulticastSourceList& sourceList)
{
    ASSERT(ie);
    ASSERT(isSorted(sourceList.sources));

    // The link-scope all-nodes group (ff02::1) is never reported (RFC 3810),
    // mirroring how IGMPv3 skips the link-local 224.0.0.0/24 range.
    if (!enabled || group == Ipv6Address::ALL_NODES_2)
        return;

    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    HostGroupData *groupData = interfaceData->getOrCreateGroupData(group);

    FilterMode filter = sourceList.filterMode == MCAST_INCLUDE_SOURCES ? MLDV2_FM_INCLUDE : MLDV2_FM_EXCLUDE;

    EV_DETAIL_C("test") << "State of group '" << group << "' on interface '" << ie->getInterfaceName() << "' has changed:\n";
    EV_DETAIL_C("test") << "    Old state: " << groupData->getStateInfo() << ".\n";
    EV_DETAIL_C("test") << "    New state: " << (filter == MLDV2_FM_INCLUDE ? "INCLUDE" : "EXCLUDE") << sourceList.sources << ".\n";

    // RFC 3810 8.2.1: while in MLDv1 compatibility on this interface, send MLDv1
    // Reports/Dones instead of MLDv2 State-Change Reports. The MLDv2 INCLUDE/EXCLUDE
    // bookkeeping below is still kept up to date so that reverting to MLDv2 on timer
    // expiry resumes from the correct state.
    if (interfaceData->olderVersionPresent) {
        bool wasJoined = groupData->filter == MLDV2_FM_EXCLUDE || !groupData->sourceAddressList.empty();
        bool nowJoined = filter == MLDV2_FM_EXCLUDE || !sourceList.sources.empty();
        if (!wasJoined && nowJoined)
            sendOlderVersionReport(ie, group);
        else if (wasJoined && !nowJoined)
            sendOlderVersionDone(ie, group);
        else if (nowJoined)
            sendOlderVersionReport(ie, group); // refresh membership
        groupData->filter = filter;
        groupData->sourceAddressList = sourceList.sources;
        sort(groupData->sourceAddressList.begin(), groupData->sourceAddressList.end());
        return;
    }

    // Check if IF state is different
    if (!(groupData->filter == filter) || !(groupData->sourceAddressList == sourceList.sources)) {
        vector<Mldv2MulticastAddressRecord> records;
        // INCLUDE(A) -> INCLUDE(B): Send ALLOW(B-A), BLOCK(A-B)
        if (groupData->filter == MLDV2_FM_INCLUDE && filter == MLDV2_FM_INCLUDE && groupData->sourceAddressList != sourceList.sources) {
            EV_DETAIL << "Sending ALLOW/BLOCK report.\n";
            records.resize(2);
            records[0].setGroupAddress(group);
            records[0].setRecordType(MLD_ALLOW_NEW_SOURCES);
            records[0].setSourceList(set_complement(sourceList.sources, groupData->sourceAddressList));
            records[1].setGroupAddress(group);
            records[1].setRecordType(MLD_BLOCK_OLD_SOURCES);
            records[1].setSourceList(set_complement(groupData->sourceAddressList, sourceList.sources));
            records.erase(remove_if(records.begin(), records.end(), isEmptyRecord), records.end());
        }
        // EXCLUDE(A) -> EXCLUDE(B): Send ALLOW(A-B), BLOCK(B-A)
        else if (groupData->filter == MLDV2_FM_EXCLUDE && filter == MLDV2_FM_EXCLUDE && groupData->sourceAddressList != sourceList.sources) {
            EV_DETAIL << "Sending ALLOW/BLOCK report.\n";
            records.resize(2);
            records[0].setGroupAddress(group);
            records[0].setRecordType(MLD_ALLOW_NEW_SOURCES);
            records[0].setSourceList(set_complement(groupData->sourceAddressList, sourceList.sources));
            records[1].setGroupAddress(group);
            records[1].setRecordType(MLD_BLOCK_OLD_SOURCES);
            records[1].setSourceList(set_complement(sourceList.sources, groupData->sourceAddressList));
            records.erase(remove_if(records.begin(), records.end(), isEmptyRecord), records.end());
        }
        // INCLUDE(A) -> EXCLUDE(B): Send TO_EX(B)
        else if (groupData->filter == MLDV2_FM_INCLUDE && filter == MLDV2_FM_EXCLUDE) {
            EV_DETAIL << "Sending TO_EX report.\n";
            records.resize(1);
            records[0].setGroupAddress(group);
            records[0].setRecordType(MLD_CHANGE_TO_EXCLUDE_MODE);
            records[0].setSourceList(sourceList.sources);
        }
        // EXCLUDE(A) -> INCLUDE(B): Send TO_IN(B)
        else if (groupData->filter == MLDV2_FM_EXCLUDE && filter == MLDV2_FM_INCLUDE) {
            EV_DETAIL << "Sending TO_IN report.\n";
            records.resize(1);
            records[0].setGroupAddress(group);
            records[0].setRecordType(MLD_CHANGE_TO_INCLUDE_MODE);
            records[0].setSourceList(sourceList.sources);
        }

        if (!records.empty()) {
            sendGroupReport(ie, records);

            // RFC 3810 6.1: the State-Change Report is (re)transmitted [Robustness
            // Variable] times in total, i.e. robustnessVariable - 1 additional times,
            // at intervals chosen at random from (0, Unsolicited Report Interval].
            //
            // If a new change arrives while a retransmission is still pending, the
            // records above were recomputed from the current (just-updated) interface
            // state, so they already reflect the merged result: we simply replace the
            // pending records and restart the retransmission counter (this is the
            // 6.1 "merge by recomputation" of the old and new pending reports).
            groupData->pendingRecords = records;
            groupData->retransmitCount = robustnessVariable - 1;
            if (groupData->retransmitCount > 0)
                startTimer(groupData->retransmitTimer, uniform(0, unsolicitedReportInterval));
            else
                cancelEvent(groupData->retransmitTimer); // RV<=1: nothing to retransmit, drop any leftover schedule
        }

        // Go to new state
        groupData->filter = filter;
        groupData->sourceAddressList = sourceList.sources;
        sort(groupData->sourceAddressList.begin(), groupData->sourceAddressList.end());
    }
}

void Mldv2::configureInterface(NetworkInterface *ie)
{
    if (enabled && rt->isMulticastForwardingEnabled()) {
        // start querier on this interface
        EV_INFO << "Sending General Query on interface '" << ie->getInterfaceName() << "', and scheduling next Query to '"
                << (simTime() + startupQueryInterval) << "'.\n";
        RouterInterfaceData *routerData = getRouterInterfaceData(ie);
        routerData->state = MLDV2_RS_QUERIER;

        sendGeneralQuery(routerData, queryResponseInterval);
        startTimer(routerData->generalQueryTimer, startupQueryInterval);
    }
}

void Mldv2::deleteHostInterfaceData(int interfaceId)
{
    auto interfaceIt = hostData.find(interfaceId);
    if (interfaceIt != hostData.end()) {
        HostInterfaceData *interface = interfaceIt->second;
        hostData.erase(interfaceIt);
        delete interface;
    }
}

void Mldv2::deleteRouterInterfaceData(int interfaceId)
{
    auto interfaceIt = routerData.find(interfaceId);
    if (interfaceIt != routerData.end()) {
        RouterInterfaceData *interface = interfaceIt->second;
        routerData.erase(interfaceIt);
        delete interface;
    }
}

void Mldv2::handleMessageWhenUp(cMessage *msg)
{
    if (!enabled) {
        if (!msg->isSelfMessage()) {
            EV << "Mldv2 disabled, dropping packet.\n";
            delete msg;
        }
        return;
    }

    if (msg->isSelfMessage()) {
        switch (msg->getKind()) {
            case MLDV2_R_GENERAL_QUERY_TIMER:
                processRouterGeneralQueryTimer(msg);
                break;

            case MLDV2_R_GROUP_TIMER:
                processRouterGroupTimer(msg);
                break;

            case MLDV2_R_SOURCE_TIMER:
                processRouterSourceTimer(msg);
                break;

            case MLDV2_R_REXMT_TIMER:
                processRexmtTimer(msg);
                break;

            case MLDV2_R_OLDER_VERSION_TIMER:
                processRouterOlderVersionTimer(msg);
                break;

            case MLDV2_H_GENERAL_QUERY_TIMER:
                processHostGeneralQueryTimer(msg);
                break;

            case MLDV2_H_GROUP_TIMER:
                processHostGroupQueryTimer(msg);
                break;

            case MLDV2_H_STATE_CHANGE_TIMER:
                processHostStateChangeTimer(msg);
                break;

            case MLDV2_H_OLDER_VERSION_TIMER:
                processHostOlderVersionTimer(msg);
                break;

            default:
                throw cRuntimeError("Mldv2: Unknown self-message kind %d: %s", msg->getKind(), msg->getName());
        }
    }
    else if (auto packet = dynamic_cast<Packet *>(msg)) {
        if (packet->getTag<PacketProtocolTag>()->getProtocol() == &Protocol::mld)
            processMldMessage(packet);
        else
            throw cRuntimeError("Mldv2: Unknown message type received.");
    }
    else if (auto indication = dynamic_cast<Indication *>(msg)) {
        // The IPv6 layer reports an ICMPv6 error for an MLD message we sent; discard it.
        EV_WARN << "Received an error indication (" << indication->getName()
                << ") for an MLD message; ignoring it" << endl;
        delete indication;
    }
    else
        throw cRuntimeError("Mldv2: Unknown message '%s' (%s) received", msg->getName(), msg->getClassName());
}

// --- Lifecycle ---

void Mldv2::handleStopOperation(LifecycleOperation *operation)
{
    // Clear per-interface state and cancel timers. Signal subscriptions are NOT
    // removed here: like Mldv1/Igmpv2, MLD subscribes once in initialize() and
    // keeps the subscription for the module's lifetime, so a stop/start cycle does
    // not leave the module deaf to membership-change signals.
    while (!hostData.empty())
        deleteHostInterfaceData(hostData.begin()->first);
    while (!routerData.empty())
        deleteRouterInterfaceData(routerData.begin()->first);
}

void Mldv2::handleCrashOperation(LifecycleOperation *operation)
{
    while (!hostData.empty())
        deleteHostInterfaceData(hostData.begin()->first);
    while (!routerData.empty())
        deleteRouterInterfaceData(routerData.begin()->first);
}

// --- Methods for handling self messages ---

void Mldv2::processRouterGeneralQueryTimer(cMessage *msg)
{
    RouterInterfaceData *interfaceData = (RouterInterfaceData *)msg->getContextPointer();
    NetworkInterface *ie = interfaceData->ie;
    ASSERT(ie);
    EV_INFO << "General Query timer expired on interface='" << ie->getInterfaceName() << "'.\n";
    RouterState state = interfaceData->state;
    if (state == MLDV2_RS_QUERIER || state == MLDV2_RS_NON_QUERIER) {
        EV_INFO << "Sending General Query on interface '" << ie->getInterfaceName() << "', and scheduling next Query to '"
                << (simTime() + queryInterval) << "'.\n";
        interfaceData->state = MLDV2_RS_QUERIER;
        sendGeneralQuery(interfaceData, queryResponseInterval);
        startTimer(msg, queryInterval);
    }
}

/**
 * Switch EXCLUDE filter mode back to INCLUDE. If at least one source timer is
 * still running, it switches to INCLUDE mode; otherwise the group record is deleted.
 */
void Mldv2::processRouterGroupTimer(cMessage *msg)
{
    RouterGroupData *groupData = (RouterGroupData *)msg->getContextPointer();
    NetworkInterface *ie = groupData->parent->ie;

    EV_INFO << "Group Timer for group '" << groupData->groupAddr << "' on interface '" << ie->getInterfaceName() << "' has expired.\n";
    EV_DETAIL << "Router State is " << groupData->getStateInfo() << ".\n";

    if (groupData->filter == MLDV2_FM_EXCLUDE) {
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
        groupData->filter = MLDV2_FM_INCLUDE;
        if (!timerRunning) {
            EV_DETAIL << "Deleting multicast listener for group '" << groupData->groupAddr << "' from the interface table.\n";
            ie->getProtocolDataForUpdate<Ipv6InterfaceData>()->removeMulticastListener(groupData->groupAddr);
            groupData->parent->deleteGroupData(groupData->groupAddr);

            EV_DETAIL << "New Router State is <deleted>.\n";
            return;
        }
    }

    EV_DETAIL << "New Router State is " << groupData->getStateInfo() << ".\n";
}

/**
 * Check expired source timers when the group is in INCLUDE filter mode.
 */
void Mldv2::processRouterSourceTimer(cMessage *msg)
{
    SourceRecord *sourceRecord = (SourceRecord *)msg->getContextPointer();
    RouterGroupData *groupData = sourceRecord->parent;
    NetworkInterface *ie = groupData->parent->ie;

    EV_INFO << "Source timer for group '" << groupData->groupAddr << "' and source '" << sourceRecord->sourceAddr
            << "' on interface '" << ie->getInterfaceName() << "' has expired.\n";

    bool last = true;
    if (groupData->filter == MLDV2_FM_INCLUDE) {
        groupData->deleteSourceRecord(sourceRecord->sourceAddr);
    }
    for (auto& elem : groupData->sources) {
        if (elem.second->sourceTimer->isScheduled()) {
            last = false;
        }
    }
    if (last) {
        ie->getProtocolDataForUpdate<Ipv6InterfaceData>()->removeMulticastListener(groupData->groupAddr);
        groupData->parent->deleteGroupData(groupData->groupAddr);
    }
}

// RFC 3810 7.6.3: retransmit a pending Multicast-Address-Specific or
// -and-Source-Specific Query. The Query is sent [Last Listener Query Count] times in
// total; this handler covers the extra transmissions after the initial one in
// sendGroup[AndSource]SpecificQuery().
void Mldv2::processRexmtTimer(cMessage *msg)
{
    RouterGroupData *groupData = (RouterGroupData *)msg->getContextPointer();
    NetworkInterface *ie = groupData->parent->ie;
    ASSERT(groupData->rexmtCount > 0);

    Ipv6AddressVector sourcesToQuery;
    if (groupData->rexmtGroupAndSource) {
        // RFC 3810 7.6.3: on retransmission, query only the sources that are still
        // being queried, i.e. those with a source timer still running above LMQT.
        // A Report that moved a source back (raising its timer) or away (deleting it)
        // thus drops it from the retransmitted Query; if none remain, stop.
        simtime_t lmqt = simTime() + lastMemberQueryTime;
        for (auto& src : groupData->rexmtSources) {
            auto it = groupData->sources.find(src);
            if (it != groupData->sources.end() && it->second->sourceTimer->isScheduled()
                && it->second->sourceTimer->getArrivalTime() > lmqt)
                sourcesToQuery.push_back(src);
        }
        if (sourcesToQuery.empty()) {
            EV_INFO << "No sources are still being queried for group '" << groupData->groupAddr
                    << "', stopping Multicast-Address-and-Source-Specific Query retransmission.\n";
            groupData->rexmtCount = 0;
            groupData->rexmtSources.clear();
            return;
        }
        EV_INFO << "Retransmitting Multicast-Address-and-Source-Specific Query for group '" << groupData->groupAddr
                << "' on interface '" << ie->getInterfaceName() << "' (" << groupData->rexmtCount
                << " transmission(s) left).\n";
    }
    else {
        EV_INFO << "Retransmitting Multicast-Address-Specific Query for group '" << groupData->groupAddr
                << "' on interface '" << ie->getInterfaceName() << "' (" << groupData->rexmtCount
                << " transmission(s) left).\n";
    }

    RouterInterfaceData *interfaceData = groupData->parent;
    if (interfaceData->state == MLDV2_RS_QUERIER) {
        Packet *packet = new Packet("Mldv2 query");
        const auto& query = makeShared<Mldv2Query>();
        query->setType(ICMPv6_MLD_QUERY);
        query->setMulticastAddress(groupData->groupAddr);
        query->setMaxRespDelay(codeMaxRespCode((uint16_t)(1000.0 * lastMemberQueryInterval))); // milliseconds
        if (groupData->rexmtGroupAndSource) {
            query->setSourceList(sourcesToQuery);
            query->setChunkLength(B(28 + (16 * sourcesToQuery.size())));
        }
        else {
            // suppressRouterProc is set on retransmissions: listeners' reports keep the
            // group/source timers up, but the querier already lowered its own timers.
            query->setSuppressRouterProc(true);
            query->setChunkLength(B(28));
        }
        Icmpv6::insertChecksum(checksumMode, query, packet);
        packet->insertAtFront(query);
        sendQueryToIPv6(packet, ie, groupData->groupAddr);

        numQueriesSent++;
        if (groupData->rexmtGroupAndSource)
            numGroupAndSourceSpecificQueriesSent++;
        else
            numGroupSpecificQueriesSent++;
    }

    if (--groupData->rexmtCount > 0)
        startTimer(groupData->rexmtTimer, lastMemberQueryInterval);
    else
        groupData->rexmtSources.clear();
}

// RFC 3810 §6.1  report generation, point 1.
void Mldv2::processHostGeneralQueryTimer(cMessage *msg)
{
    HostInterfaceData *interfaceData = (HostInterfaceData *)msg->getContextPointer();
    NetworkInterface *ie = interfaceData->ie;
    ASSERT(ie);

    EV_INFO << "Response timer to a General Query on interface '" << ie->getInterfaceName() << "' has expired.\n";

    Packet *outPacket = new Packet("Mldv2 report");
    const auto& report = makeShared<Mldv2Report>();
    unsigned int byteLength = 8; // Mldv2Report header size
    report->setType(ICMPv6_MLDv2_REPORT);
    int counter = 0;
    report->setMulticastAddressRecordArraySize(interfaceData->groups.size());

    // FIXME Do not create reports larger than the interface MTU.

    // Create a Multicast Address Record for each group on the interface.
    for (auto& elem : interfaceData->groups) {
        Mldv2MulticastAddressRecord gr;
        if (elem.second->filter == MLDV2_FM_INCLUDE) {
            gr.setRecordType(MLD_MODE_IS_INCLUDE);
        }
        else if (elem.second->filter == MLDV2_FM_EXCLUDE) {
            gr.setRecordType(MLD_MODE_IS_EXCLUDE);
        }
        gr.setGroupAddress(elem.second->groupAddr);
        gr.setSourceList(elem.second->sourceAddressList);
        report->setMulticastAddressRecord(counter++, gr);
        byteLength += 20 + gr.getSourceList().size() * 16; // 20 byte record header + n * 16 byte (Ipv6Address)
    }
    report->setChunkLength(B(byteLength));

    if (counter != 0) { // if no record was created, no report is sent
        EV_INFO << "Sending response to a General Query on interface '" << ie->getInterfaceName() << "'.\n";
        Icmpv6::insertChecksum(checksumMode, report, outPacket);
        outPacket->insertAtFront(report);
        sendReportToIPv6(outPacket, ie, Ipv6Address::ALL_MLDV2_ROUTERS);
        numReportsSent++;
    }
    else {
        EV_INFO << "There are no multicast listeners, no response is sent to a General Query on interface '" << ie->getInterfaceName() << "'.\n";
        delete outPacket;
    }
}

// RFC 3810 §6.1  report generation, point 2. and 3.
void Mldv2::processHostGroupQueryTimer(cMessage *msg)
{
    HostGroupData *group = (HostGroupData *)msg->getContextPointer();
    NetworkInterface *ie = group->parent->ie;

    vector<Mldv2MulticastAddressRecord> records(1);

    // checking if query is multicast-address-specific or -and-source-specific
    if (group->queriedSources.empty()) {
        // Send report for a Multicast-Address-Specific Query
        EV_INFO << "Response timer for a Multicast-Address-Specific Query for group '" << group->groupAddr << "' on interface '" << ie->getInterfaceName() << "'\n";

        records[0].setGroupAddress(group->groupAddr);
        records[0].setRecordType(group->filter == MLDV2_FM_INCLUDE ? MLD_MODE_IS_INCLUDE : MLD_MODE_IS_EXCLUDE);
        records[0].setSourceList(group->sourceAddressList);
        sendGroupReport(ie, records);
    }
    else {
        // Send report for a Multicast-Address-and-Source-Specific Query
        EV_INFO << "Response timer for a Multicast-Address-and-Source-Specific Query for group '" << group->groupAddr << "' on interface '" << ie->getInterfaceName() << "'\n";

        records[0].setGroupAddress(group->groupAddr);
        records[0].setRecordType(MLD_MODE_IS_INCLUDE);
        records[0].setSourceList(group->filter == MLDV2_FM_INCLUDE ? set_intersection(group->sourceAddressList, group->queriedSources) :
            set_complement(group->queriedSources, group->sourceAddressList));
        sendGroupReport(ie, records);
    }

    group->queriedSources.clear();
}

// RFC 3810 6.1: retransmit a pending State-Change Report. The report is sent
// [Robustness Variable] times in total; this handler covers the extra
// transmissions after the initial one done in multicastSourceListChanged().
void Mldv2::processHostStateChangeTimer(cMessage *msg)
{
    HostGroupData *group = (HostGroupData *)msg->getContextPointer();
    NetworkInterface *ie = group->parent->ie;
    ASSERT(group->retransmitCount > 0);
    ASSERT(!group->pendingRecords.empty());

    EV_INFO << "Retransmitting State-Change Report for group '" << group->groupAddr
            << "' on interface '" << ie->getInterfaceName() << "' (" << group->retransmitCount
            << " transmission(s) left).\n";

    sendGroupReport(ie, group->pendingRecords);

    if (--group->retransmitCount > 0)
        startTimer(group->retransmitTimer, uniform(0, unsolicitedReportInterval));
    else
        group->pendingRecords.clear();
}

void Mldv2::startTimer(cMessage *timer, double interval)
{
    ASSERT(timer);
    rescheduleAfter(interval, timer);
}

// --- Methods for processing MLD messages ---

void Mldv2::processMldMessage(Packet *packet)
{
    const auto& icmp = packet->peekAtFront<Icmpv6Header>();
    NetworkInterface *ie = ift->getInterfaceById(packet->getTag<InterfaceInd>()->getInterfaceId());
    switch (icmp->getType()) {
        case ICMPv6_MLD_QUERY:
            // Type 130 is shared by MLDv1 (24-byte MldQuery) and MLDv2 (>=28-byte
            // Mldv2Query). Distinguish by chunk length: an MLDv1 General Query makes a
            // v2 host fall back to MLDv1 on the receiving interface (RFC 3810 8.2.1).
            if (icmp->getChunkLength() < B(28))
                processOlderVersionQuery(ie, packet);
            else
                processQuery(packet);
            break;

        case ICMPv6_MLDv2_REPORT:
            processReport(packet);
            break;

        // RFC 3810 8.3.2: an MLDv1 (type 131) Report or (type 132) Done puts the
        // addressed group into older-version-host-present compatibility on the router.
        case ICMPv6_MLD_REPORT:
            processOlderVersionReport(ie, packet);
            break;

        case ICMPv6_MLD_DONE:
            processOlderVersionDone(ie, packet);
            break;

        default:
            EV_WARN << "Mldv2: unexpected MLD message type " << (int)icmp->getType() << ", dropping.\n";
            delete packet;
            break;
    }
}

// RFC 3810 §6.2 / §7
void Mldv2::processQuery(Packet *packet)
{
    NetworkInterface *ie = ift->getInterfaceById(packet->getTag<InterfaceInd>()->getInterfaceId());
    const auto& msg = packet->peekAtFront<Mldv2Query>();
    ASSERT(msg != nullptr);

    Ipv6Address groupAddr = msg->getMulticastAddress();
    Ipv6AddressVector queriedSources = msg->getSourceList();
    // MLDv2 Maximum Response Code is in milliseconds (RFC 3810 §5.1.3).
    double maxRespTime = 0.001 * decodeMaxRespCode(msg->getMaxRespDelay());

    ASSERT(ie->isMulticast());

    EV_INFO << "Received Mldv2 query on interface '" << ie->getInterfaceName() << "' for group '" << groupAddr << "'.\n";

    HostInterfaceData *interfaceData = getHostInterfaceData(ie);

    numQueriesRecv++;

    double delay = uniform(0.0, maxRespTime);

    // Rules from RFC 3810 §6.2:
    if (interfaceData->generalQueryTimer->isScheduled() && interfaceData->generalQueryTimer->getArrivalTime() < simTime() + delay) {
        // 1. If there is a pending response to a previous General Query scheduled
        //    sooner than the selected delay, no additional response is scheduled.
        EV_DETAIL << "There is a pending response to a previous General Query, no further response is scheduled.\n";
    }
    else if (groupAddr.isUnspecified() && queriedSources.empty()) {
        // 2. General Query: schedule a response after the selected delay; any
        //    previously pending General Query response is canceled.
        EV_DETAIL << "Received a General Query, scheduling report with delay=" << delay << ".\n";
        startTimer(interfaceData->generalQueryTimer, delay);
    }
    else if (!groupAddr.isUnspecified()) {
        HostGroupData *groupData = interfaceData->getOrCreateGroupData(groupAddr);

        if (!groupData->timer->isScheduled()) {
            // 3. Multicast-Address-Specific or -and-Source-Specific Query with no
            //    pending response for this group: schedule a report. Record the
            //    queried sources for -and-Source-Specific Queries.
            EV_DETAIL << "Received Multicast-Address" << (queriedSources.empty() ? "" : "-and-Source") << "-Specific Query, "
                      << "scheduling report with delay=" << delay << ".\n";

            sort(queriedSources.begin(), queriedSources.end());
            groupData->queriedSources = queriedSources;
            startTimer(groupData->timer, delay);
        }
        else if (queriedSources.empty()) {
            // 4. Pending response and either a Multicast-Address-Specific Query or
            //    an empty recorded source-list: clear the source-list and schedule a
            //    single response at the earliest of the remaining time and the delay.
            EV_DETAIL << "Received Multicast-Address-Specific Query, scheduling report with delay="
                      << min(delay, SIMTIME_DBL(groupData->timer->getArrivalTime() - simTime())) << ".\n";

            sort(queriedSources.begin(), queriedSources.end());
            groupData->queriedSources = queriedSources;
            if (groupData->timer->getArrivalTime() > simTime() + delay)
                startTimer(groupData->timer, delay);
        }
        else {
            // 5. -and-Source-Specific Query with a pending non-empty source-list:
            //    augment the recorded sources and schedule a single response at the
            //    earliest of the remaining time and the selected delay.
            EV_DETAIL << "Received Multicast-Address-and-Source-Specific Query, combining sources with the sources of pending report, "
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
        RouterInterfaceData *routerInterfaceData = getRouterInterfaceData(ie);
        Ipv6Address srcAddr = packet->getTag<L3AddressInd>()->getSrcAddress().toIpv6();
        if (srcAddr < ie->getProtocolData<Ipv6InterfaceData>()->getLinkLocalAddress()) {
            startTimer(routerInterfaceData->generalQueryTimer, otherQuerierPresentInterval);
            routerInterfaceData->state = MLDV2_RS_NON_QUERIER;
        }

        if (!groupAddr.isUnspecified() && routerInterfaceData->state == MLDV2_RS_NON_QUERIER) { // multicast-address-specific query
            RouterGroupData *groupData = routerInterfaceData->getOrCreateGroupData(groupAddr);
            if (groupData->state == MLDV2_RGS_MEMBERS_PRESENT) {
                double maxResponseTime = maxRespTime;
                startTimer(groupData->timer, maxResponseTime * lastMemberQueryCount);
                groupData->state = MLDV2_RGS_CHECKING_MEMBERSHIP;
            }
        }
    }
    delete packet;
}

void Mldv2::processReport(Packet *packet)
{
    const auto& msg = packet->peekAtFront<Mldv2Report>();
    ASSERT(msg != nullptr);
    NetworkInterface *ie = ift->getInterfaceById(packet->getTag<InterfaceInd>()->getInterfaceId());

    ASSERT(ie->isMulticast());

    EV_INFO << "Received Mldv2 Multicast Listener Report on interface '" << ie->getInterfaceName() << "'.\n";

    numReportsRecv++;

    if (rt->isMulticastForwardingEnabled()) {
        RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);

        for (unsigned int i = 0; i < msg->getMulticastAddressRecordArraySize(); i++) {
            Mldv2MulticastAddressRecord gr = msg->getMulticastAddressRecord(i);
            EV_DETAIL << "Found a record for group '" << gr.getGroupAddress() << "'.\n";

            // ensure that the received sources are sorted
            sort(gr.getSourceListForUpdate().begin(), gr.getSourceListForUpdate().end());
            Ipv6AddressVector& receivedSources = gr.getSourceListForUpdate(); // sorted

            RouterGroupData *groupData = interfaceData->getOrCreateGroupData(gr.getGroupAddress());

            // RFC 3810 8.3.2: while an MLDv1 host is present for this group, the group is
            // treated as EXCLUDE{} and source-specific records from MLDv2 Reports for that
            // group are ignored. Skip all v2 per-source processing for this record.
            if (groupData->olderVersionPresent && groupData->olderVersionTimer->isScheduled()) {
                EV_DETAIL << "Ignoring v2 source state for group '" << gr.getGroupAddress()
                          << "': older-version (MLDv1) host present, group forwarded as EXCLUDE{}.\n";
                continue;
            }

            Ipv6MulticastSourceList oldSourceList;
            groupData->collectForwardedSources(oldSourceList);

            EV_DETAIL << "Router State is " << groupData->getStateInfo() << ".\n";

            // RFC 3810 7.6.3: a new Report for this group supersedes any in-progress
            // Last-Listener Query retransmission. Cancel it; if this record still needs
            // a Query, the send below re-arms the retransmission from scratch.
            if (groupData->rexmtCount > 0) {
                groupData->rexmtCount = 0;
                groupData->rexmtSources.clear();
                cancelEvent(groupData->rexmtTimer);
            }

            // RFC 3810 §7.4: Reception of Current State Record
            if (gr.getRecordType() == MLD_MODE_IS_INCLUDE) {
                EV_DETAIL << "Received IS_IN" << receivedSources << " report.\n";
                // INCLUDE(A)   -> IS_IN(B) -> INCLUDE(A+B)    : (B) = GMI
                // EXCLUDE(X,Y) -> IS_IN(A) -> EXCLUDE(X+A,Y-A): (A) = GMI
                for (auto& receivedSource : receivedSources) {
                    EV_DETAIL << "Setting source timer of '" << receivedSource << "' to '" << groupMembershipInterval << "'.\n";
                    SourceRecord *record = groupData->getOrCreateSourceRecord(receivedSource);
                    startTimer(record->sourceTimer, groupMembershipInterval);
                }
            }
            else if (gr.getRecordType() == MLD_MODE_IS_EXCLUDE) {
                EV_DETAIL << "Received IS_EX" << receivedSources << " report.\n";

                // Group Timer = GMI
                EV_DETAIL << "Setting group timer to '" << groupMembershipInterval << "'.\n";
                startTimer(groupData->timer, groupMembershipInterval);

                // INCLUDE(A)   -> IS_EX(B) -> EXCLUDE(A*B,B-A): Delete (A-B)
                // EXCLUDE(X,Y) -> IS_EX(A) -> EXCLUDE(A-Y,Y*A): Delete (X-A) Delete (Y-A)
                for (auto it = groupData->sources.begin(); it != groupData->sources.end(); ++it) {
                    if (!contains(receivedSources, it->first)) {
                        EV_DETAIL << "Deleting source record of '" << it->first << "'.\n";
                        groupData->deleteSourceRecord(it->first);
                    }
                }

                // Router State == INCLUDE(A),   Report == IS_EX(B): (B-A) = 0
                // Router State == EXCLUDE(X,Y), Report == IS_EX(A): (A-X-Y) = GMI
                for (auto& receivedSource : receivedSources) {
                    if (!groupData->hasSourceRecord(receivedSource)) {
                        SourceRecord *record = groupData->createSourceRecord(receivedSource);
                        double timerValue = groupData->filter == MLDV2_FM_INCLUDE ? 0.0 : groupMembershipInterval;
                        EV_DETAIL << "Setting source timer of '" << receivedSource << "' to '" << timerValue << "'.\n";
                        if (timerValue > 0)
                            startTimer(record->sourceTimer, groupMembershipInterval);
                    }
                }

                groupData->filter = MLDV2_FM_EXCLUDE;
            }
            // RFC 3810 §7.4: Reception of Filter-Mode-Change and Source-List-Change Records
            else if (gr.getRecordType() == MLD_ALLOW_NEW_SOURCES) {
                EV_DETAIL << "Received ALLOW" << receivedSources << " report.\n";

                // INCLUDE(A)   -> ALLOW(B) -> INCLUDE(A+B):     (B) = GMI
                // EXCLUDE(X,Y) -> ALLOW(A) -> EXCLUDE(X+A,Y-A): (A) = GMI
                for (auto& receivedSource : receivedSources) {
                    EV_DETAIL << "Setting source timer of '" << receivedSource << "' to '" << groupMembershipInterval << "'.\n";
                    SourceRecord *record = groupData->getOrCreateSourceRecord(receivedSource);
                    startTimer(record->sourceTimer, groupMembershipInterval);
                }
            }
            else if (gr.getRecordType() == MLD_BLOCK_OLD_SOURCES) {
                EV_DETAIL << "Received BLOCK" << receivedSources << " report.\n";

                if (groupData->filter == MLDV2_FM_INCLUDE) {
                    // INCLUDE(A) -> BLOCK(B) -> INCLUDE(A): Send Q(G,A*B)
                    Ipv6AddressVector sourcesA;
                    for (auto& elem : groupData->sources)
                        sourcesA.push_back(elem.first);

                    Ipv6AddressVector aIntersectB = set_intersection(sourcesA, receivedSources);
                    if (!aIntersectB.empty()) {
                        EV_INFO << "Sending Multicast-Address-and-Source-Specific Query for group '" << groupData->groupAddr
                                << "' on interface '" << ie->getInterfaceName() << "'.\n";
                        sendGroupAndSourceSpecificQuery(groupData, aIntersectB);
                    }
                }
                else if (groupData->filter == MLDV2_FM_EXCLUDE) {
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
                    Ipv6AddressVector ySources;
                    for (auto& elem : groupData->sources) {
                        if (!elem.second->sourceTimer->isScheduled()) {
                            ySources.push_back(elem.first);
                        }
                    }
                    Ipv6AddressVector aMinusY = set_complement(receivedSources, ySources);
                    if (!aMinusY.empty()) {
                        EV_INFO << "Sending Multicast-Address-and-Source-Specific Query for group '" << groupData->groupAddr
                                << "' on interface '" << ie->getInterfaceName() << "'.\n";
                        sendGroupAndSourceSpecificQuery(groupData, aMinusY);
                    }
                }
            }
            else if (gr.getRecordType() == MLD_CHANGE_TO_INCLUDE_MODE) {
                EV_DETAIL << "Received TO_IN" << receivedSources << " report.\n";

                if (groupData->filter == MLDV2_FM_INCLUDE) {
                    // INCLUDE(A) -> TO_IN (B) -> INCLUDE (A+B): (B)=GMI
                    for (auto& receivedSource : receivedSources) {
                        EV_DETAIL << "Setting source timer of '" << receivedSource << "' to '" << groupMembershipInterval << "'.\n";
                        SourceRecord *record = groupData->getOrCreateSourceRecord(receivedSource);
                        startTimer(record->sourceTimer, groupMembershipInterval);
                    }
                    // Send Q(G,A-B)
                    Ipv6AddressVector sourcesA;
                    for (auto& elem : groupData->sources)
                        sourcesA.push_back(elem.first);
                    Ipv6AddressVector aMinusB = set_complement(sourcesA, receivedSources);
                    if (!aMinusB.empty()) {
                        EV_INFO << "Sending Multicast-Address-and-Source-Specific Query for group '" << groupData->groupAddr
                                << "' on interface '" << ie->getInterfaceName() << "'.\n";
                        sendGroupAndSourceSpecificQuery(groupData, aMinusB);
                    }
                }
                else if (groupData->filter == MLDV2_FM_EXCLUDE) {
                    // compute X before modifying the state
                    Ipv6AddressVector sourcesX;
                    for (auto& elem : groupData->sources) {
                        if (elem.second->sourceTimer->isScheduled())
                            sourcesX.push_back(elem.first);
                    }

                    // EXCLUDE(X,Y) -> TO_IN(A) -> EXCLUDE(X+A,Y-A): (A) = GMI
                    for (auto& receivedSource : receivedSources) {
                        EV_DETAIL << "Setting source timer of '" << receivedSource << "' to '" << groupMembershipInterval << "'.\n";
                        SourceRecord *record = groupData->getOrCreateSourceRecord(receivedSource);
                        startTimer(record->sourceTimer, groupMembershipInterval);
                    }

                    // Send Q(G,X-A)
                    Ipv6AddressVector xMinusA = set_complement(sourcesX, receivedSources);
                    if (!xMinusA.empty()) {
                        EV_INFO << "Sending Multicast-Address-and-Source-Specific Query for group '" << groupData->groupAddr
                                << "' on interface '" << ie->getInterfaceName() << "'.\n";
                        sendGroupAndSourceSpecificQuery(groupData, xMinusA);
                    }

                    // Send Q(G)
                    EV_INFO << "Sending Multicast-Address-Specific Query for group '" << groupData->groupAddr
                            << "' on interface '" << ie->getInterfaceName() << "'.\n";
                    sendGroupSpecificQuery(groupData);
                }
            }
            else if (gr.getRecordType() == MLD_CHANGE_TO_EXCLUDE_MODE) {
                EV_DETAIL << "Received TO_EX" << receivedSources << " report.\n";

                if (groupData->filter == MLDV2_FM_INCLUDE) {
                    // INCLUDE (A) -> TO_EX(B) -> EXCLUDE (A*B,B-A): Group Timer = GMI
                    EV_DETAIL << "Setting group timer to '" << groupMembershipInterval << "'.\n";
                    startTimer(groupData->timer, groupMembershipInterval);

                    // change to mode exclude
                    groupData->filter = MLDV2_FM_EXCLUDE;

                    // save A
                    Ipv6AddressVector sourcesA;
                    for (auto& elem : groupData->sources)
                        sourcesA.push_back(elem.first);

                    // (B-A) = 0
                    for (auto& receivedSource : receivedSources) {
                        if (!groupData->hasSourceRecord(receivedSource)) {
                            EV_DETAIL << "Setting source timer of '" << receivedSource << "' to '0'.\n";
                            groupData->createSourceRecord(receivedSource);
                        }
                    }

                    // Delete A-B
                    for (auto& elem : sourcesA) {
                        if (!contains(receivedSources, elem)) {
                            EV_DETAIL << "Deleting source record of '" << elem << "'.\n";
                            groupData->deleteSourceRecord(elem);
                        }
                    }

                    // Send Q(G,A*B)
                    Ipv6AddressVector aIntersectB = set_intersection(sourcesA, receivedSources);
                    if (!aIntersectB.empty()) {
                        EV_INFO << "Sending Multicast-Address-Specific Query for group '" << groupData->groupAddr
                                << "' on interface '" << ie->getInterfaceName() << "'.\n";
                        sendGroupAndSourceSpecificQuery(groupData, aIntersectB);
                    }
                }
                else if (groupData->filter == MLDV2_FM_EXCLUDE) {
                    // EXCLUDE (X,Y) -> TO_EX (A) -> EXCLUDE (A-Y,Y*A): Group Timer = GMI
                    EV_DETAIL << "Setting group timer to '" << groupMembershipInterval << "'.\n";
                    startTimer(groupData->timer, groupMembershipInterval);

                    // save Y
                    Ipv6AddressVector sourcesY;
                    for (auto& elem : groupData->sources) {
                        if (!elem.second->sourceTimer->isScheduled())
                            sourcesY.push_back(elem.first);
                    }

                    // Delete (X-A) Delete (Y-A)
                    for (auto it = groupData->sources.begin(); it != groupData->sources.end();) {
                        auto rec = it->first;
                        ++it; // advance now: deleteSourceRecord invalidates the current iterator
                        if (!contains(receivedSources, rec)) {
                            EV_DETAIL << "Deleting source record of '" << rec << "'.\n";
                            groupData->deleteSourceRecord(rec);
                        }
                    }

                    // (A-X-Y) = GMI FIXME should be set to Group Timer
                    for (auto& receivedSource : receivedSources) {
                        if (!groupData->hasSourceRecord(receivedSource)) {
                            EV_DETAIL << "Setting source timer of '" << receivedSource << "' to '" << groupMembershipInterval << "'.\n";
                            SourceRecord *record = groupData->createSourceRecord(receivedSource);
                            startTimer(record->sourceTimer, groupMembershipInterval);
                        }
                    }

                    // Send Q(G,A-Y)
                    Ipv6AddressVector aMinusY = set_complement(receivedSources, sourcesY);
                    if (!aMinusY.empty()) {
                        EV_INFO << "Sending Multicast-Address-Specific Query for group '" << groupData->groupAddr
                                << "' on interface '" << ie->getInterfaceName() << "'.\n";
                        sendGroupAndSourceSpecificQuery(groupData, aMinusY);
                    }
                }
            }

            EV_DETAIL << "New Router State is " << groupData->getStateInfo() << ".\n";

            // update interface state
            Ipv6MulticastSourceList newSourceList;
            groupData->collectForwardedSources(newSourceList);

            if (newSourceList != oldSourceList) {
                ie->getProtocolDataForUpdate<Ipv6InterfaceData>()->setMulticastListeners(groupData->groupAddr, newSourceList.filterMode, newSourceList.sources);
            }
        }
    }
    delete packet;
}

// --- Older-version (MLDv1) interop (RFC 3810 8.2/8.3) ---

// RFC 3810 8.2.1: an MLDv1 General Query was received. Enter/refresh MLDv1
// compatibility on the interface and answer the Query in MLDv1 style (an MldReport for
// each joined group, just as an MLDv1 host would).
void Mldv2::processOlderVersionQuery(NetworkInterface *ie, Packet *packet)
{
    ASSERT(ie->isMulticast());
    numQueriesRecv++;

    const auto& query = packet->peekAtFront<MldQuery>();
    Ipv6Address groupAddr = query->getMulticastAddress();

    HostInterfaceData *interfaceData = getHostInterfaceData(ie);

    if (groupAddr.isUnspecified()) {
        if (!interfaceData->olderVersionPresent)
            EV_INFO << "Received older-version (MLDv1) General Query on interface '"
                    << ie->getInterfaceName() << "': entering MLDv1 compatibility.\n";
        else
            EV_INFO << "older-version querier present on interface '" << ie->getInterfaceName()
                    << "', refreshing MLDv1 compatibility.\n";
        interfaceData->olderVersionPresent = true;
        startTimer(interfaceData->olderVersionTimer, otherQuerierPresentInterval);

        // Answer the General Query in MLDv1 style for every joined group.
        for (auto& elem : interfaceData->groups) {
            HostGroupData *g = elem.second;
            bool joined = g->filter == MLDV2_FM_EXCLUDE || !g->sourceAddressList.empty();
            if (joined)
                sendOlderVersionReport(ie, g->groupAddr);
        }
    }
    else {
        // Multicast-Address-Specific MLDv1 Query: answer for the addressed group if joined.
        EV_INFO << "Received older-version (MLDv1) Multicast-Address-Specific Query for group '"
                << groupAddr << "' on interface '" << ie->getInterfaceName() << "'.\n";
        auto it = interfaceData->groups.find(groupAddr);
        if (it != interfaceData->groups.end()) {
            HostGroupData *g = it->second;
            bool joined = g->filter == MLDV2_FM_EXCLUDE || !g->sourceAddressList.empty();
            if (joined)
                sendOlderVersionReport(ie, groupAddr);
        }
    }

    // Router/Querier election still applies (an older-version querier may win).
    if (rt->isMulticastForwardingEnabled()) {
        RouterInterfaceData *routerInterfaceData = getRouterInterfaceData(ie);
        Ipv6Address srcAddr = packet->getTag<L3AddressInd>()->getSrcAddress().toIpv6();
        if (srcAddr < ie->getProtocolData<Ipv6InterfaceData>()->getLinkLocalAddress()) {
            startTimer(routerInterfaceData->generalQueryTimer, otherQuerierPresentInterval);
            routerInterfaceData->state = MLDV2_RS_NON_QUERIER;
        }
    }

    delete packet;
}

// Build/send an MLDv1 Report exactly as Mldv1 does (dest = group address).
void Mldv2::sendOlderVersionReport(NetworkInterface *ie, const Ipv6Address& group)
{
    ASSERT(group.isMulticast() && group != Ipv6Address::ALL_NODES_2);

    EV_INFO << "Mldv2: sending MLDv1 Multicast Listener Report for group '" << group
            << "' on interface '" << ie->getInterfaceName() << "'.\n";
    Packet *packet = new Packet("Mldv1 report");
    const auto& msg = makeShared<MldReport>();
    msg->setMulticastAddress(group);
    msg->setChunkLength(B(24));
    Icmpv6::insertChecksum(checksumMode, msg, packet);
    packet->insertAtFront(msg);
    sendToIPv6(packet, ie, group); // dest = group address (RFC 2710 §5)
    numReportsSent++;
}

// Build/send an MLDv1 Done exactly as Mldv1 does (dest = ALL_ROUTERS_2, ff02::2).
void Mldv2::sendOlderVersionDone(NetworkInterface *ie, const Ipv6Address& group)
{
    ASSERT(group.isMulticast() && group != Ipv6Address::ALL_NODES_2);

    EV_INFO << "Mldv2: sending MLDv1 Multicast Listener Done for group '" << group
            << "' on interface '" << ie->getInterfaceName() << "'.\n";
    Packet *packet = new Packet("Mldv1 done");
    const auto& msg = makeShared<MldDone>();
    msg->setMulticastAddress(group);
    msg->setChunkLength(B(24));
    Icmpv6::insertChecksum(checksumMode, msg, packet);
    packet->insertAtFront(msg);
    sendToIPv6(packet, ie, Ipv6Address::ALL_ROUTERS_2);
}

// RFC 3810 8.3.2: a received MLDv1 Report puts the addressed group into
// older-version-host-present mode on the router.
void Mldv2::processOlderVersionReport(NetworkInterface *ie, Packet *packet)
{
    ASSERT(ie->isMulticast());
    numReportsRecv++;

    Ipv6Address group = packet->peekAtFront<MldReport>()->getMulticastAddress();

    EV_INFO << "Mldv2: received MLDv1 Multicast Listener Report for group '" << group
            << "' on interface '" << ie->getInterfaceName() << "'.\n";

    if (rt->isMulticastForwardingEnabled() && group.isMulticast() && group != Ipv6Address::ALL_NODES_2) {
        RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);
        RouterGroupData *groupData = interfaceData->getOrCreateGroupData(group);
        enterRouterOlderVersionCompat(ie, groupData);
    }

    delete packet;
}

// RFC 3810 8.3.2: a received MLDv1 Done runs the normal last-listener query process
// (Feature 2 retransmission), exactly like an MLDv2 leave.
void Mldv2::processOlderVersionDone(NetworkInterface *ie, Packet *packet)
{
    ASSERT(ie->isMulticast());

    Ipv6Address group = packet->peekAtFront<MldDone>()->getMulticastAddress();

    EV_INFO << "Mldv2: received MLDv1 Multicast Listener Done for group '" << group
            << "' on interface '" << ie->getInterfaceName() << "'.\n";

    if (rt->isMulticastForwardingEnabled()) {
        RouterGroupData *groupData = getRouterGroupData(ie, group);
        if (groupData && groupData->state == MLDV2_RGS_MEMBERS_PRESENT) {
            EV_INFO << "Sending Multicast-Address-Specific Query for group '" << group
                    << "' on interface '" << ie->getInterfaceName() << "' in response to a Done.\n";
            groupData->state = MLDV2_RGS_CHECKING_MEMBERSHIP;
            sendGroupSpecificQuery(groupData);
        }
    }

    delete packet;
}

Mldv2::RouterGroupData *Mldv2::getRouterGroupData(NetworkInterface *ie, const Ipv6Address& group)
{
    auto it = routerData.find(ie->getInterfaceId());
    if (it == routerData.end())
        return nullptr;
    auto git = it->second->groups.find(group);
    return git != it->second->groups.end() ? git->second : nullptr;
}

// Put the group into older-version-host-present mode: forward as EXCLUDE{} (any-source),
// (re)start the per-group compatibility timer, and record the compat flag.
void Mldv2::enterRouterOlderVersionCompat(NetworkInterface *ie, RouterGroupData *groupData)
{
    bool entering = !groupData->olderVersionPresent || !groupData->olderVersionTimer->isScheduled();
    if (entering)
        EV_INFO << "v1 host present for group '" << groupData->groupAddr << "' on interface '"
                << ie->getInterfaceName() << "': entering MLDv1 compatibility, forwarding as EXCLUDE{}.\n";
    else
        EV_INFO << "Refreshing older-version-host-present timer for group '" << groupData->groupAddr
                << "' on interface '" << ie->getInterfaceName() << "'.\n";

    groupData->olderVersionPresent = true;
    startTimer(groupData->olderVersionTimer, groupMembershipInterval);

    // Force EXCLUDE{} forwarding and the v2 router group/filter state.
    Ipv6MulticastSourceList oldSourceList;
    groupData->collectForwardedSources(oldSourceList);

    groupData->filter = MLDV2_FM_EXCLUDE;
    groupData->state = MLDV2_RGS_MEMBERS_PRESENT;
    startTimer(groupData->timer, groupMembershipInterval);

    Ipv6MulticastSourceList newSourceList;
    newSourceList.filterMode = MCAST_EXCLUDE_SOURCES;
    newSourceList.sources.clear();

    if (newSourceList != oldSourceList || entering)
        ie->getProtocolDataForUpdate<Ipv6InterfaceData>()->setMulticastListeners(groupData->groupAddr, MCAST_EXCLUDE_SOURCES, newSourceList.sources);
}

// RFC 3810 8.2.1: the Older Version Querier Present timer expired; revert to MLDv2.
void Mldv2::processHostOlderVersionTimer(cMessage *msg)
{
    HostInterfaceData *interfaceData = (HostInterfaceData *)msg->getContextPointer();
    EV_INFO << "Older Version Querier Present timer expired on interface '"
            << interfaceData->ie->getInterfaceName() << "': reverting to MLDv2.\n";
    interfaceData->olderVersionPresent = false;
}

// RFC 3810 8.3.2: the Older Version Host Present timer for a group expired; revert that
// group to native MLDv2 processing.
void Mldv2::processRouterOlderVersionTimer(cMessage *msg)
{
    RouterGroupData *groupData = (RouterGroupData *)msg->getContextPointer();
    EV_INFO << "Older Version Host Present timer expired for group '" << groupData->groupAddr
            << "' on interface '" << groupData->parent->ie->getInterfaceName() << "': reverting to MLDv2.\n";
    groupData->olderVersionPresent = false;
}

// --- Methods for sending MLD messages ---

void Mldv2::sendGeneralQuery(RouterInterfaceData *interfaceData, double maxRespTime)
{
    if (interfaceData->state == MLDV2_RS_QUERIER) {
        Packet *packet = new Packet("Mldv2 query");
        const auto& msg = makeShared<Mldv2Query>();
        msg->setType(ICMPv6_MLD_QUERY);
        msg->setMaxRespDelay(codeMaxRespCode((uint16_t)(maxRespTime * 1000.0))); // milliseconds
        msg->setChunkLength(B(28));
        Icmpv6::insertChecksum(checksumMode, msg, packet);
        packet->insertAtFront(msg);
        sendQueryToIPv6(packet, interfaceData->ie, Ipv6Address::ALL_NODES_2);

        numQueriesSent++;
        numGeneralQueriesSent++;
    }
}

// See RFC 3810 §7.6.3.1.
void Mldv2::sendGroupSpecificQuery(RouterGroupData *groupData)
{
    RouterInterfaceData *interfaceData = groupData->parent;
    bool suppressFlag = groupData->timer->isScheduled() && groupData->timer->getArrivalTime() > simTime() + lastMemberQueryTime;

    // Set group timer to LMQT
    startTimer(groupData->timer, lastMemberQueryTime);

    if (interfaceData->state == MLDV2_RS_QUERIER) {
        Packet *packet = new Packet("Mldv2 query");
        const auto& msg = makeShared<Mldv2Query>();
        msg->setType(ICMPv6_MLD_QUERY);
        msg->setMulticastAddress(groupData->groupAddr);
        msg->setMaxRespDelay(codeMaxRespCode((uint16_t)(1000.0 * lastMemberQueryInterval))); // milliseconds
        msg->setSuppressRouterProc(suppressFlag);
        msg->setChunkLength(B(28));
        Icmpv6::insertChecksum(checksumMode, msg, packet);
        packet->insertAtFront(msg);
        sendQueryToIPv6(packet, interfaceData->ie, groupData->groupAddr);

        numQueriesSent++;
        numGroupSpecificQueriesSent++;
    }

    // RFC 3810 7.6.3: the Multicast-Address-Specific Query is (re)transmitted [Last
    // Listener Query Count] times in total, lastMemberQueryInterval apart.
    groupData->rexmtGroupAndSource = false;
    groupData->rexmtSources.clear();
    groupData->rexmtCount = lastMemberQueryCount - 1;
    if (groupData->rexmtCount > 0 && lastMemberQueryInterval > 0)
        startTimer(groupData->rexmtTimer, lastMemberQueryInterval);
    else {
        groupData->rexmtCount = 0;
        cancelEvent(groupData->rexmtTimer); // nothing to retransmit, drop any leftover schedule
    }
}

void Mldv2::sendGroupReport(NetworkInterface *ie, const vector<Mldv2MulticastAddressRecord>& records)
{
    EV << "Mldv2: sending Multicast Listener Report on iface=" << ie->getInterfaceName() << "\n";
    Packet *packet = new Packet("Mldv2 report");
    const auto& msg = makeShared<Mldv2Report>();
    unsigned int byteLength = 8; // Mldv2Report header size
    msg->setType(ICMPv6_MLDv2_REPORT);
    msg->setMulticastAddressRecordArraySize(records.size());
    for (size_t i = 0; i < records.size(); ++i) {
        Ipv6Address group = records[i].getGroupAddress();
        ASSERT(group.isMulticast() && group != Ipv6Address::ALL_NODES_2);
        msg->setMulticastAddressRecord(i, records[i]);
        byteLength += 20 + records[i].getSourceList().size() * 16; // 20 byte record header + n * 16 byte (Ipv6Address)
    }
    msg->setChunkLength(B(byteLength));
    Icmpv6::insertChecksum(checksumMode, msg, packet);
    packet->insertAtFront(msg);
    sendReportToIPv6(packet, ie, Ipv6Address::ALL_MLDV2_ROUTERS);
    numReportsSent++;
}

// See RFC 3810 §7.6.3.2.
void Mldv2::sendGroupAndSourceSpecificQuery(RouterGroupData *groupData, const Ipv6AddressVector& sources)
{
    ASSERT(!sources.empty());

    RouterInterfaceData *interfaceData = groupData->parent;

    if (interfaceData->state == MLDV2_RS_QUERIER) {
        Packet *packet = new Packet("Mldv2 query");
        const auto& msg = makeShared<Mldv2Query>();
        msg->setType(ICMPv6_MLD_QUERY);
        msg->setMulticastAddress(groupData->groupAddr);
        msg->setMaxRespDelay(codeMaxRespCode((uint16_t)(1000.0 * lastMemberQueryInterval))); // milliseconds
        msg->setSourceList(sources);
        msg->setChunkLength(B(28 + (16 * sources.size())));
        Icmpv6::insertChecksum(checksumMode, msg, packet);
        packet->insertAtFront(msg);
        sendQueryToIPv6(packet, interfaceData->ie, groupData->groupAddr);

        numQueriesSent++;
        numGroupAndSourceSpecificQueriesSent++;
    }

    // RFC 3810 7.6.3: the Multicast-Address-and-Source-Specific Query is
    // (re)transmitted [Last Listener Query Count] times in total,
    // lastMemberQueryInterval apart. On each retransmission the set of queried
    // sources is recomputed (see processRexmtTimer), so remember them here.
    groupData->rexmtGroupAndSource = true;
    groupData->rexmtSources = sources;
    sort(groupData->rexmtSources.begin(), groupData->rexmtSources.end());
    groupData->rexmtCount = lastMemberQueryCount - 1;
    if (groupData->rexmtCount > 0 && lastMemberQueryInterval > 0)
        startTimer(groupData->rexmtTimer, lastMemberQueryInterval);
    else {
        groupData->rexmtCount = 0;
        cancelEvent(groupData->rexmtTimer); // nothing to retransmit, drop any leftover schedule
    }
}

void Mldv2::sendReportToIPv6(Packet *msg, NetworkInterface *ie, const Ipv6Address& dest)
{
    sendToIPv6(msg, ie, dest);
}

void Mldv2::sendQueryToIPv6(Packet *msg, NetworkInterface *ie, const Ipv6Address& dest)
{
    sendToIPv6(msg, ie, dest);
}

// MLD is an ICMPv6 sub-protocol carried as ICMPv6 (protocol 58). Use
// Protocol::icmpv6 for the PacketProtocolTag so the IPv6 module encodes protocol
// 58 in the IPv6 header, and Protocol::ipv6 in DispatchProtocolReq so the lp
// dispatcher routes to Ipv6. Hop limit is 1 (RFC 3810).
void Mldv2::sendToIPv6(Packet *msg, NetworkInterface *ie, const Ipv6Address& dest)
{
    ASSERT(ie->isMulticast());

    msg->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::icmpv6);
    msg->addTagIfAbsent<DispatchProtocolInd>()->setProtocol(&Protocol::mld);
    msg->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv6);
    msg->addTagIfAbsent<InterfaceReq>()->setInterfaceId(ie->getInterfaceId());
    msg->addTagIfAbsent<L3AddressReq>()->setDestAddress(dest);
    msg->addTagIfAbsent<HopLimitReq>()->setHopLimit(1);
    send(msg, "ipOut");
}

// --- Utility Methods for SourceRecord ---

Mldv2::SourceRecord::SourceRecord(RouterGroupData *parent, const Ipv6Address& source)
    : parent(parent), sourceAddr(source)
{
    ASSERT(parent);

    sourceTimer = new cMessage("Mldv2 router source timer", MLDV2_R_SOURCE_TIMER);
    sourceTimer->setContextPointer(this);
}

Mldv2::SourceRecord::~SourceRecord()
{
    parent->parent->owner->cancelAndDelete(sourceTimer);
}

// --- Utility Methods for Interface Data ---

Mldv2::RouterInterfaceData *Mldv2::getRouterInterfaceData(NetworkInterface *ie)
{
    int interfaceId = ie->getInterfaceId();
    auto it = routerData.find(interfaceId);
    if (it != routerData.end())
        return it->second;

    RouterInterfaceData *data = createRouterInterfaceData(ie);
    routerData[interfaceId] = data;
    return data;
}

Mldv2::RouterInterfaceData *Mldv2::createRouterInterfaceData(NetworkInterface *ie)
{
    return new RouterInterfaceData(this, ie);
}

Mldv2::HostInterfaceData *Mldv2::getHostInterfaceData(NetworkInterface *ie)
{
    int interfaceId = ie->getInterfaceId();
    auto it = hostData.find(interfaceId);
    if (it != hostData.end())
        return it->second;

    HostInterfaceData *data = createHostInterfaceData(ie);
    hostData[interfaceId] = data;
    return data;
}

Mldv2::HostInterfaceData *Mldv2::createHostInterfaceData(NetworkInterface *ie)
{
    return new HostInterfaceData(this, ie);
}

// --- HostGroupData structure implementation ---

Mldv2::HostGroupData::HostGroupData(HostInterfaceData *parent, const Ipv6Address& group)
    : parent(parent), groupAddr(group), filter(MLDV2_FM_INCLUDE), state(MLDV2_HGS_NON_MEMBER)
{
    ASSERT(parent);
    ASSERT(groupAddr.isMulticast());

    timer = new cMessage("Mldv2 Host Group Timer", MLDV2_H_GROUP_TIMER);
    timer->setContextPointer(this);

    retransmitTimer = new cMessage("Mldv2 Host State-Change Retransmit Timer", MLDV2_H_STATE_CHANGE_TIMER);
    retransmitTimer->setContextPointer(this);
}

Mldv2::HostGroupData::~HostGroupData()
{
    parent->owner->cancelAndDelete(timer);
    parent->owner->cancelAndDelete(retransmitTimer);
}

string Mldv2::HostGroupData::getStateInfo() const
{
    ostringstream out;
    switch (filter) {
        case MLDV2_FM_INCLUDE:
            out << "INCLUDE" << sourceAddressList;
            break;

        case MLDV2_FM_EXCLUDE:
            out << "EXCLUDE" << sourceAddressList;
            break;
    }
    return out.str();
}

// --- RouterGroupData structure implementation ---

Mldv2::RouterGroupData::RouterGroupData(RouterInterfaceData *parent, const Ipv6Address& group)
    : parent(parent), groupAddr(group), filter(MLDV2_FM_INCLUDE), state(MLDV2_RGS_NO_MEMBERS_PRESENT)
{
    ASSERT(parent);
    ASSERT(groupAddr.isMulticast());

    timer = new cMessage("Mldv2 router group timer", MLDV2_R_GROUP_TIMER);
    timer->setContextPointer(this);

    rexmtTimer = new cMessage("Mldv2 router rexmt timer", MLDV2_R_REXMT_TIMER);
    rexmtTimer->setContextPointer(this);

    olderVersionTimer = new cMessage("Mldv2 Older Version Host Present Timer", MLDV2_R_OLDER_VERSION_TIMER);
    olderVersionTimer->setContextPointer(this);
}

Mldv2::RouterGroupData::~RouterGroupData()
{
    parent->owner->cancelAndDelete(timer);
    parent->owner->cancelAndDelete(rexmtTimer);
    parent->owner->cancelAndDelete(olderVersionTimer);
}

Mldv2::SourceRecord *Mldv2::RouterGroupData::createSourceRecord(const Ipv6Address& source)
{
    ASSERT(!containsKey(sources, source));
    SourceRecord *record = new SourceRecord(this, source);
    sources[source] = record;
    return record;
}

Mldv2::SourceRecord *Mldv2::RouterGroupData::getOrCreateSourceRecord(const Ipv6Address& source)
{
    auto it = sources.find(source);
    if (it != sources.end())
        return it->second;
    return createSourceRecord(source);
}

void Mldv2::RouterGroupData::deleteSourceRecord(const Ipv6Address& source)
{
    auto it = sources.find(source);
    if (it != sources.end()) {
        SourceRecord *record = it->second;
        sources.erase(it);
        delete record;
    }
}

string Mldv2::RouterGroupData::getStateInfo() const
{
    ostringstream out;
    switch (filter) {
        case MLDV2_FM_INCLUDE:
            out << "INCLUDE(";
            printSourceList(out, true);
            out << ")";
            break;

        case MLDV2_FM_EXCLUDE:
            out << "EXCLUDE(";
            printSourceList(out, true);
            out << ";";
            printSourceList(out, false);
            out << ")";
            break;
    }
    return out.str();
}

void Mldv2::RouterGroupData::printSourceList(ostream& out, bool withRunningTimer) const
{
    bool first = true;
    for (const auto& elem : sources) {
        bool timerIsRunning = elem.second->sourceTimer && elem.second->sourceTimer->isScheduled();
        if (withRunningTimer == timerIsRunning) {
            if (!first)
                out << ",";
            first = false;
            out << elem.first;
        }
    }
}

// See RFC 3810 §7.2 for the forwarding rules.
void Mldv2::RouterGroupData::collectForwardedSources(Ipv6MulticastSourceList& result) const
{
    switch (filter) {
        case MLDV2_FM_INCLUDE:
            result.filterMode = MCAST_INCLUDE_SOURCES;
            result.sources.clear();
            for (const auto& elem : sources) {
                if (elem.second->sourceTimer && elem.second->sourceTimer->isScheduled())
                    result.sources.push_back(elem.first);
            }
            break;

        case MLDV2_FM_EXCLUDE:
            result.filterMode = MCAST_EXCLUDE_SOURCES;
            result.sources.clear();
            for (const auto& elem : sources) {
                if (!elem.second->sourceTimer || !elem.second->sourceTimer->isScheduled())
                    result.sources.push_back(elem.first);
            }
            break;
    }
}

// --- HostInterfaceData structure implementation ---

Mldv2::HostInterfaceData::HostInterfaceData(Mldv2 *owner, NetworkInterface *ie)
    : owner(owner), ie(ie)
{
    ASSERT(owner);
    ASSERT(ie);

    generalQueryTimer = new cMessage("Mldv2 Host General Timer", MLDV2_H_GENERAL_QUERY_TIMER);
    generalQueryTimer->setContextPointer(this);

    olderVersionTimer = new cMessage("Mldv2 Older Version Querier Present Timer", MLDV2_H_OLDER_VERSION_TIMER);
    olderVersionTimer->setContextPointer(this);
}

Mldv2::HostInterfaceData::~HostInterfaceData()
{
    owner->cancelAndDelete(generalQueryTimer);
    owner->cancelAndDelete(olderVersionTimer);

    for (auto& elem : groups)
        delete elem.second;
}

Mldv2::HostGroupData *Mldv2::HostInterfaceData::getOrCreateGroupData(const Ipv6Address& group)
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

void Mldv2::HostInterfaceData::deleteGroupData(const Ipv6Address& group)
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

// --- RouterInterfaceData structure implementation ---

Mldv2::RouterInterfaceData::RouterInterfaceData(Mldv2 *owner, NetworkInterface *ie)
    : owner(owner), ie(ie)
{
    ASSERT(owner);
    ASSERT(ie);

    state = MLDV2_RS_INITIAL;
    generalQueryTimer = new cMessage("Mldv2 General Query timer", MLDV2_R_GENERAL_QUERY_TIMER);
    generalQueryTimer->setContextPointer(this);
}

Mldv2::RouterInterfaceData::~RouterInterfaceData()
{
    owner->cancelAndDelete(generalQueryTimer);

    for (auto& elem : groups)
        delete elem.second;
}

Mldv2::RouterGroupData *Mldv2::RouterInterfaceData::getOrCreateGroupData(const Ipv6Address& group)
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

void Mldv2::RouterInterfaceData::deleteGroupData(const Ipv6Address& group)
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

// --- Ipv6AddressVector set helpers ---

Ipv6AddressVector Mldv2::set_complement(const Ipv6AddressVector& first, const Ipv6AddressVector& second)
{
    ASSERT(isSorted(first));
    ASSERT(isSorted(second));

    Ipv6AddressVector complement(first.size());

    auto it = set_difference(first.begin(), first.end(), second.begin(), second.end(), complement.begin());
    complement.resize(it - complement.begin());
    return complement;
}

Ipv6AddressVector Mldv2::set_intersection(const Ipv6AddressVector& first, const Ipv6AddressVector& second)
{
    ASSERT(isSorted(first));
    ASSERT(isSorted(second));

    Ipv6AddressVector intersection(min(first.size(), second.size()));

    auto it = std::set_intersection(first.begin(), first.end(), second.begin(), second.end(), intersection.begin());
    intersection.resize(it - intersection.begin());
    return intersection;
}

Ipv6AddressVector Mldv2::set_union(const Ipv6AddressVector& first, const Ipv6AddressVector& second)
{
    ASSERT(isSorted(first));
    ASSERT(isSorted(second));

    Ipv6AddressVector result(first.size() + second.size());

    auto it = std::set_union(first.begin(), first.end(), second.begin(), second.end(), result.begin());
    result.resize(it - result.begin());
    return result;
}

// --- 16-bit MLDv2 Max Response Code / QQIC codec (RFC 3810 §5.1.3) ---

uint16_t Mldv2::decodeMaxRespCode(uint16_t code)
{
    if (code < 0x8000)
        return code;
    // floating point: 1 eee mmmm mmmm mmmm -> (0x1000 | mant) << (exp + 3)
    unsigned mant = code & 0x0FFF;
    unsigned exp = (code >> 12) & 0x07;
    return (uint16_t)((0x1000 | mant) << (exp + 3));
}

uint16_t Mldv2::codeMaxRespCode(uint16_t value)
{
    if (value < 0x8000)
        return value;
    // find the smallest exponent so that the mantissa fits in 12 bits
    unsigned exp = 0;
    uint32_t v = value;
    v >>= 3;
    while (v > 0x1FFF) {
        v >>= 1;
        exp++;
    }
    if (exp > 7) {
        // saturate at the maximum representable value
        exp = 7;
        v = 0x1FFF;
    }
    unsigned mant = v & 0x0FFF;
    return (uint16_t)(0x8000 | ((exp & 0x07) << 12) | mant);
}

const std::string Mldv2::getRouterStateString(Mldv2::RouterState rs)
{
    if (rs == MLDV2_RS_INITIAL)
        return "INITIAL";
    else if (rs == MLDV2_RS_QUERIER)
        return "QUERIER";
    else if (rs == MLDV2_RS_NON_QUERIER)
        return "NON_QUERIER";

    return "UNKNOWN";
}

const std::string Mldv2::getRouterGroupStateString(Mldv2::RouterGroupState rgs)
{
    if (rgs == MLDV2_RGS_NO_MEMBERS_PRESENT)
        return "NO_MEMBERS_PRESENT";
    else if (rgs == MLDV2_RGS_MEMBERS_PRESENT)
        return "MEMBERS_PRESENT";
    else if (rgs == MLDV2_RGS_CHECKING_MEMBERSHIP)
        return "CHECKING_MEMBERSHIP";

    return "UNKNOWN";
}

const std::string Mldv2::getHostGroupStateString(Mldv2::HostGroupState hgs)
{
    if (hgs == MLDV2_HGS_NON_MEMBER)
        return "NON_MEMBER";
    else if (hgs == MLDV2_HGS_DELAYING_MEMBER)
        return "DELAYING_MEMBER";
    else if (hgs == MLDV2_HGS_IDLE_MEMBER)
        return "IDLE_MEMBER";

    return "UNKNOWN";
}

const std::string Mldv2::getFilterModeString(Mldv2::FilterMode fm)
{
    if (fm == MLDV2_FM_INCLUDE)
        return "INCLUDE";
    else if (fm == MLDV2_FM_EXCLUDE)
        return "EXCLUDE";

    return "UNKNOWN";
}

} // namespace inet
