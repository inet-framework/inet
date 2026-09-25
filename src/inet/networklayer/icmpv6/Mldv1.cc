//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/icmpv6/Mldv1.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/checksum/ChecksumMode_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/icmpv6/Icmpv6.h"
#include "inet/networklayer/icmpv6/MldMessage_m.h"
#include "inet/networklayer/ipv6/Ipv6ExtHeaderTag_m.h"
#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"

namespace inet {

// true when the interface has a link-local address that Duplicate Address Detection has tested:
// every MLD Query leaves from such an address (RFC 3810 section 5.1.13, RFC 9777 section 5.1.14)
static bool hasValidLinkLocalAddress(const NetworkInterface *ie)
{
    auto ipv6Data = ie->findProtocolData<Ipv6InterfaceData>();
    if (ipv6Data == nullptr)
        return false;
    const Ipv6Address& linkLocalAddress = ipv6Data->getLinkLocalAddress();
    return !linkLocalAddress.isUnspecified() && !ipv6Data->isTentativeAddress(linkLocalAddress);
}

Define_Module(Mldv1);

// --- RouterGroupData ---

Mldv1::RouterGroupData::RouterGroupData(Mldv1 *owner, const Ipv6Address& group)
    : owner(owner), groupAddr(group)
{
    ASSERT(owner);
    ASSERT(groupAddr.isMulticast());
    state = MLD_RGS_NO_LISTENERS_PRESENT;
    timer = nullptr;
    rexmtTimer = nullptr;
}

Mldv1::RouterGroupData::~RouterGroupData()
{
    if (timer) {
        delete (MldRouterTimerContext *)timer->getContextPointer();
        owner->cancelAndDelete(timer);
    }
    if (rexmtTimer) {
        delete (MldRouterTimerContext *)rexmtTimer->getContextPointer();
        owner->cancelAndDelete(rexmtTimer);
    }
}

// --- RouterInterfaceData ---

Mldv1::RouterInterfaceData::RouterInterfaceData(Mldv1 *owner)
    : owner(owner)
{
    ASSERT(owner);
    mldQueryTimer = nullptr;
}

Mldv1::RouterInterfaceData::~RouterInterfaceData()
{
    owner->cancelAndDelete(mldQueryTimer);
    for (auto& elem : groups)
        delete elem.second;
}

// --- HostGroupData ---

Mldv1::HostGroupData::HostGroupData(Mldv1 *owner, const Ipv6Address& group)
    : owner(owner), groupAddr(group)
{
    ASSERT(owner);
    ASSERT(groupAddr.isMulticast());

    state = MLD_HGS_NON_LISTENER;
    flag = false;
    timer = nullptr;
}

Mldv1::HostGroupData::~HostGroupData()
{
    if (timer) {
        delete (MldHostTimerContext *)timer->getContextPointer();
        owner->cancelAndDelete(timer);
    }
}

// --- HostInterfaceData ---

Mldv1::HostInterfaceData::HostInterfaceData(Mldv1 *owner)
    : owner(owner)
{
    ASSERT(owner);
}

Mldv1::HostInterfaceData::~HostInterfaceData()
{
    for (auto& elem : groups)
        delete elem.second;
}

// --- Destructor ---

Mldv1::~Mldv1()
{
    while (!heldMessages.empty())
        deleteHeldMessages(heldMessages.begin()->first);
    while (!hostData.empty())
        deleteHostInterfaceData(hostData.begin()->first);
    while (!routerData.empty())
        deleteRouterInterfaceData(routerData.begin()->first);
}

// --- Initialize ---

void Mldv1::initialize(int stage)
{
    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        enabled = par("enabled");
        const char *checksumModeString = par("checksumMode");
        checksumMode = parseChecksumMode(checksumModeString, false);

        // Read router (querier) NED params
        queryInterval = par("queryInterval");
        queryResponseInterval = par("queryResponseInterval");
        multicastListenerInterval = par("multicastListenerInterval");
        startupQueryInterval = par("startupQueryInterval");
        startupQueryCount = par("startupQueryCount");
        lastListenerQueryInterval = par("lastListenerQueryInterval");
        lastListenerQueryCount = par("lastListenerQueryCount");
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);

        // Register Protocol::mld so the lp dispatcher delivers MLD ICMPv6 messages to us
        registerProtocol(Protocol::mld, gate("ipOut"), gate("ipIn"));

        // Read NED parameter
        unsolicitedReportInterval = par("unsolicitedReportInterval");

        // Replay already-joined groups on each interface (e.g. after restart)
        for (int i = 0; i < ift->getNumInterfaces(); ++i) {
            NetworkInterface *ie = ift->getInterface(i);
            if (ie->isMulticast()) {
                if (auto *ipv6data = ie->findProtocolData<Ipv6InterfaceData>()) {
                    for (int g = 0; g < ipv6data->getNumOfJoinedMulticastGroups(); ++g)
                        multicastGroupJoined(ie, ipv6data->getJoinedMulticastGroup(g));
                }
            }
        }

        // Activate router/querier role on each multicast interface (gated by isMulticastForwardingEnabled)
        for (int i = 0; i < ift->getNumInterfaces(); ++i) {
            NetworkInterface *ie = ift->getInterface(i);
            if (ie->isMulticast())
                configureInterface(ie);
        }

        // Subscribe to interface and multicast group signals at the containing node
        cModule *host = getContainingNode(this);
        host->subscribe(interfaceCreatedSignal, this);
        host->subscribe(interfaceDeletedSignal, this);
        host->subscribe(ipv6MulticastGroupJoinedSignal, this);
        host->subscribe(ipv6MulticastGroupLeftSignal, this);
        host->subscribe(interfaceIpv6ConfigChangedSignal, this);
    }
}

// --- handleMessageWhenUp ---

void Mldv1::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        switch (msg->getKind()) {
            case MLD_HOSTGROUP_TIMER:
                processHostGroupTimer(msg);
                break;
            case MLD_QUERY_TIMER:
                processQueryTimer(msg);
                break;
            case MLD_LEAVE_TIMER:
                processLeaveTimer(msg);
                break;
            case MLD_REXMT_TIMER:
                processRexmtTimer(msg);
                break;
            default:
                throw cRuntimeError("Unknown self-message kind %d: %s", msg->getKind(), msg->getName());
        }
    }
    else if (auto packet = dynamic_cast<Packet *>(msg)) {
        if (packet->getTag<PacketProtocolTag>()->getProtocol() == &Protocol::mld) {
            processMldMessage(packet);
        }
        else
            throw cRuntimeError("Received packet '%s' (%s) with unexpected protocol", packet->getName(), packet->getClassName());
    }
    else if (auto indication = dynamic_cast<Indication *>(msg)) {
        // The IPv6 layer reports an ICMPv6 error for an MLD message we sent;
        // discard it (no recovery logic in this skeleton phase).
        EV_WARN << "Received an error indication (" << indication->getName()
                << ") for an MLD message; ignoring it" << endl;
        delete indication;
    }
    else
        throw cRuntimeError("Unknown message '%s' (%s) received", msg->getName(), msg->getClassName());
}

// --- Signal handler ---

void Mldv1::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    NetworkInterface *ie;
    int interfaceId;
    const Ipv6MulticastGroupInfo *info;

    if (signalID == ipv6MulticastGroupJoinedSignal) {
        info = check_and_cast<const Ipv6MulticastGroupInfo *>(obj);
        multicastGroupJoined(info->ie, info->groupAddress);
    }
    else if (signalID == ipv6MulticastGroupLeftSignal) {
        info = check_and_cast<const Ipv6MulticastGroupInfo *>(obj);
        multicastGroupLeft(info->ie, info->groupAddress);
    }
    else if (signalID == interfaceCreatedSignal) {
        ie = check_and_cast<NetworkInterface *>(obj);
        if (ie->isMulticast())
            configureInterface(ie);
    }
    else if (signalID == interfaceIpv6ConfigChangedSignal) {
        // a router interface that waited for a tested link-local address starts its querier
        ie = const_cast<NetworkInterface *>(check_and_cast<const NetworkInterfaceChangeDetails *>(obj)->getNetworkInterface());
        if (hasValidLinkLocalAddress(ie))
            sendHeldMessages(ie);
        auto it = routerData.find(ie->getInterfaceId());
        if (it != routerData.end() && it->second->waitsForLinkLocalAddress && hasValidLinkLocalAddress(ie)) {
            it->second->waitsForLinkLocalAddress = false;
            EV_INFO << "Mldv1: iface=" << ie->getInterfaceName() << " has a valid link-local address, starting the querier\n";
            sendQuery(ie, Ipv6Address::UNSPECIFIED_ADDRESS, queryResponseInterval);
            startTimer(it->second->mldQueryTimer, startupQueryInterval);
        }
    }
    else if (signalID == interfaceDeletedSignal) {
        ie = check_and_cast<NetworkInterface *>(obj);
        if (ie->isMulticast()) {
            interfaceId = ie->getInterfaceId();
            deleteHostInterfaceData(interfaceId);
            deleteRouterInterfaceData(interfaceId);    // mirror Igmpv2: also free router state + General-Query timer
            deleteHeldMessages(interfaceId);
        }
    }
}

// --- Host join/leave handlers ---

void Mldv1::multicastGroupJoined(NetworkInterface *ie, const Ipv6Address& groupAddr)
{
    ASSERT(ie && ie->isMulticast());
    ASSERT(groupAddr.isMulticast());

    // RFC 2710 §5: "An MLD message is never sent for the link-scope all-nodes address (FF02::1)"
    if (enabled && groupAddr != Ipv6Address::ALL_NODES_2) {
        HostGroupData *groupData = createHostGroupData(ie, groupAddr);
        numGroups++;
        numHostGroups++;

        sendReport(ie, groupData);
        groupData->flag = true;
        startHostTimer(ie, groupData, unsolicitedReportInterval);
        groupData->state = MLD_HGS_DELAYING_LISTENER;
    }
}

void Mldv1::multicastGroupLeft(NetworkInterface *ie, const Ipv6Address& groupAddr)
{
    ASSERT(ie && ie->isMulticast());
    ASSERT(groupAddr.isMulticast());

    if (enabled && groupAddr != Ipv6Address::ALL_NODES_2) {
        HostGroupData *groupData = getHostGroupData(ie, groupAddr);
        if (groupData) {
            if (groupData->state == MLD_HGS_DELAYING_LISTENER)
                cancelEvent(groupData->timer);

            if (groupData->flag)
                sendDone(ie, groupData);

            deleteHostGroupData(ie, groupAddr);
            numHostGroups--;
            numGroups--;
        }
    }
}

// --- Host timer handlers ---

void Mldv1::processHostGroupTimer(cMessage *msg)
{
    MldHostTimerContext *ctx = (MldHostTimerContext *)msg->getContextPointer();
    EV_DEBUG << "Mldv1: Host Timer expired for group=" << ctx->hostGroup->groupAddr
             << " iface=" << ctx->ie->getInterfaceName() << "\n";
    sendReport(ctx->ie, ctx->hostGroup);
    ctx->hostGroup->flag = true;
    ctx->hostGroup->state = MLD_HGS_IDLE_LISTENER;
}

void Mldv1::startTimer(cMessage *timer, double interval)
{
    ASSERT(timer);
    rescheduleAfter(interval, timer);
}

void Mldv1::startHostTimer(NetworkInterface *ie, HostGroupData *group, double maxRespTime)
{
    if (!group->timer) {
        group->timer = new cMessage("Mldv1 group timer", MLD_HOSTGROUP_TIMER);
        group->timer->setContextPointer(new MldHostTimerContext(ie, group));
    }

    double delay = uniform(0.0, maxRespTime);
    EV_DEBUG << "setting host timer for " << ie->getInterfaceName() << " and group "
             << group->groupAddr.str() << " to " << delay << "\n";
    startTimer(group->timer, delay);
}

// --- Inbound MLD message processing ---

void Mldv1::processMldMessage(Packet *packet)
{
    if (!enabled) {
        // MLD administratively disabled on this node: ignore inbound MLD messages.
        EV_INFO << "MLD disabled, dropping received MLD message.\n";
        delete packet;
        return;
    }

    const auto& mldMsg = packet->peekAtFront<Icmpv6Header>();
    NetworkInterface *ie = ift->getInterfaceById(packet->getTag<InterfaceInd>()->getInterfaceId());
    EV_INFO << "Received MLD message, type=" << (int)mldMsg->getType() << endl;  // keep for MLD_smoke.test
    switch (mldMsg->getType()) {
        case ICMPv6_MLD_QUERY:
            processQuery(ie, packet);
            break;
        case ICMPv6_MLD_REPORT:
            processReport(ie, packet);
            break;
        case ICMPv6_MLD_DONE:
            processDone(ie, packet);
            break;
        case ICMPv6_MLDv2_REPORT:
            // an MLDv1 node does not know the type, so it discards the message silently
            // (RFC 4443 section 2.4 (b)); an MLDv2 host sends one before it hears an MLDv1 Query
            EV_INFO << "Ignoring an MLDv2 Report.\n";
            delete packet;
            break;
        default:
            throw cRuntimeError("Mldv1: Unknown MLD message type %d", (int)mldMsg->getType());
    }
}

void Mldv1::processQuery(NetworkInterface *ie, Packet *packet)
{
    ASSERT(ie->isMulticast());

    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    // a node accepts every Query of 24 octets or more (RFC 2710 section 5, RFC 9777 section 8.1),
    // so an MLDv2 Query too: its first 24 octets are the fields of an MLDv1 Query, and its
    // Maximum Response Code stands for the Maximum Response Delay
    const auto& mldQry = packet->peekAtFront<MldMessage>();

    // MLD maxRespDelay is in MILLISECONDS (uint16_t, RFC 2710 §3.4)
    // Guard against zero (would produce a negative/zero timer interval via uniform(0,0))
    uint16_t rawDelay = mldQry->getMaxRespDelay();
    simtime_t maxRespTime = (rawDelay > 0) ? SimTime(rawDelay, SIMTIME_MS) : SimTime(1, SIMTIME_MS);

    Ipv6Address groupAddr = mldQry->getMulticastAddress();

    if (groupAddr.isUnspecified()) {
        // General Listener Query: schedule a delayed Report for every joined group
        EV_INFO << "Mldv1: received General Listener Query on iface=" << ie->getInterfaceName() << "\n";
        for (auto& elem : interfaceData->groups)
            processGroupQuery(ie, elem.second, maxRespTime);
    }
    else {
        // Multicast-Address-Specific Query: only the addressed group responds
        EV_INFO << "Mldv1: received Multicast-Address-Specific Query for group=" << groupAddr
                << " iface=" << ie->getInterfaceName() << "\n";
        auto it = interfaceData->groups.find(groupAddr);
        if (it != interfaceData->groups.end())
            processGroupQuery(ie, it->second, maxRespTime);
    }

    delete packet;
}

void Mldv1::processGroupQuery(NetworkInterface *ie, HostGroupData *group, simtime_t maxRespTime)
{
    double maxRespTimeSecs = maxRespTime.dbl();

    if (group->state == MLD_HGS_DELAYING_LISTENER) {
        // RFC 2710 §5: only re-arm timer if the new max response time is sooner
        simtime_t maxAbsoluteRespTime = simTime() + maxRespTimeSecs;
        if (group->timer->isScheduled() && maxAbsoluteRespTime < group->timer->getArrivalTime())
            startHostTimer(ie, group, maxRespTimeSecs);
    }
    else if (group->state == MLD_HGS_IDLE_LISTENER) {
        // Idle Listener → Delaying Listener: arm the random-delay timer
        startHostTimer(ie, group, maxRespTimeSecs);
        group->state = MLD_HGS_DELAYING_LISTENER;
    }
    // else Non-Listener: ignore (not a member of this group)
}

void Mldv1::processReport(NetworkInterface *ie, Packet *packet)
{
    ASSERT(ie->isMulticast());

    const auto& msg = packet->peekAtFront<MldReport>();
    Ipv6Address groupAddr = msg->getMulticastAddress();

    EV_INFO << "Mldv1: received Multicast Listener Report for group=" << groupAddr
            << " iface=" << ie->getInterfaceName() << "\n";
    numReportsRecv++;

    // HOST-03: report suppression — hearing another node's Report cancels our pending Report
    HostGroupData *hostGroupData = getHostGroupData(ie, groupAddr);
    if (hostGroupData && hostGroupData->state == MLD_HGS_DELAYING_LISTENER) {
        cancelEvent(hostGroupData->timer);
        hostGroupData->flag = false;
        hostGroupData->state = MLD_HGS_IDLE_LISTENER;
    }

    // RTR-02: router-side listener recording
    if (rt->isMulticastForwardingEnabled()) {
        RouterGroupData *routerGroupData = getRouterGroupData(ie, groupAddr);
        if (!routerGroupData) {
            routerGroupData = createRouterGroupData(ie, groupAddr);
            numGroups++;
        }

        if (!routerGroupData->timer) {
            routerGroupData->timer = new cMessage("Mldv1 group timer", MLD_LEAVE_TIMER);
            routerGroupData->timer->setContextPointer(new MldRouterTimerContext(ie, routerGroupData));
        }
        if (!routerGroupData->rexmtTimer) {
            routerGroupData->rexmtTimer = new cMessage("Mldv1 rexmt timer", MLD_REXMT_TIMER);
            routerGroupData->rexmtTimer->setContextPointer(new MldRouterTimerContext(ie, routerGroupData));
        }

        if (routerGroupData->state == MLD_RGS_NO_LISTENERS_PRESENT) {
            ie->getProtocolDataForUpdate<Ipv6InterfaceData>()->addMulticastListener(groupAddr);
            numRouterGroups++;
            EV_INFO << "Mldv1: recorded listener for group=" << groupAddr << " on " << ie->getInterfaceName() << "\n";
        }
        else if (routerGroupData->state == MLD_RGS_CHECKING_LISTENERS)
            cancelEvent(routerGroupData->rexmtTimer);

        startTimer(routerGroupData->timer, multicastListenerInterval);
        routerGroupData->state = MLD_RGS_LISTENERS_PRESENT;
    }

    delete packet;
}

// --- Outbound send helpers ---

void Mldv1::sendReport(NetworkInterface *ie, HostGroupData *group)
{
    ASSERT(group->groupAddr.isMulticast());
    ASSERT(group->groupAddr != Ipv6Address::ALL_NODES_2);
    EV_INFO << "Mldv1: sending Multicast Listener Report for group=" << group->groupAddr
            << " on iface=" << ie->getInterfaceName() << "\n";
    Packet *packet = new Packet("Mldv1 report");
    const auto& msg = makeShared<MldReport>();
    msg->setMulticastAddress(group->groupAddr);
    msg->setChunkLength(B(24));
    Icmpv6::insertChecksum(checksumMode, msg, packet);
    packet->insertAtFront(msg);
    sendToIPv6(packet, ie, group->groupAddr); // dest = group address (RFC 2710 §5)
    numReportsSent++;
}

void Mldv1::sendDone(NetworkInterface *ie, HostGroupData *group)
{
    ASSERT(group->groupAddr.isMulticast());
    ASSERT(group->groupAddr != Ipv6Address::ALL_NODES_2);
    EV_INFO << "Mldv1: sending Multicast Listener Done for group=" << group->groupAddr
            << " on iface=" << ie->getInterfaceName() << "\n";
    Packet *packet = new Packet("Mldv1 done");
    const auto& msg = makeShared<MldDone>();
    msg->setMulticastAddress(group->groupAddr);
    msg->setChunkLength(B(24));
    Icmpv6::insertChecksum(checksumMode, msg, packet);
    packet->insertAtFront(msg);
    sendToIPv6(packet, ie, Ipv6Address::ALL_ROUTERS_2); // Done goes to ff02::2
    numDonesSent++;
}

// --- sendToIPv6 ---

void Mldv1::sendToIPv6(Packet *msg, NetworkInterface *ie, const Ipv6Address& dest)
{
    ASSERT(ie->isMulticast());
    // MLD is an ICMPv6 sub-protocol (RFC 2710 §3); at the IP level it is carried
    // as ICMPv6 (protocol 58). Use Protocol::icmpv6 for the PacketProtocolTag so
    // that the IPv6 module encodes protocol number 58 in the IPv6 header, and
    // Protocol::ipv6 in DispatchProtocolReq so the lp dispatcher routes to Ipv6.
    msg->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::icmpv6);
    msg->addTagIfAbsent<DispatchProtocolInd>()->setProtocol(&Protocol::mld);
    msg->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv6);
    msg->addTagIfAbsent<InterfaceReq>()->setInterfaceId(ie->getInterfaceId());
    msg->addTagIfAbsent<L3AddressReq>()->setDestAddress(dest);
    msg->addTagIfAbsent<HopLimitReq>()->setHopLimit(1);   // RFC 2710 §3: hop limit = 1
    // every MLD message carries the Router Alert option in a Hop-by-Hop Options header (RFC 2710
    // section 3, RFC 9777 section 5)
    msg->addTagIfAbsent<Ipv6ExtHeaderReq>()->appendExtensionHeader(createIpv6RouterAlertHeader(IPv6_ROUTER_ALERT_MLD));
    // every MLD message from a link-local address (RFC 2710 section 3, RFC 9777 section 5). While
    // the interface has no tested one, a Query is not sent, and a Report or a Done waits for it
    if (hasValidLinkLocalAddress(ie))
        msg->addTagIfAbsent<L3AddressReq>()->setSrcAddress(ie->getProtocolData<Ipv6InterfaceData>()->getLinkLocalAddress());
    else if (msg->peekAtFront<Icmpv6Header>()->getType() == ICMPv6_MLD_QUERY) {
        EV_WARN << "Interface '" << ie->getInterfaceName() << "' has no valid link-local address, dropping the Query.\n";
        delete msg;
        return;
    }
    else {
        // RFC 3810 section 5.2.13 sends it from the unspecified address, which the IPv6 layer of
        // the model replaces with its preferred address; so it waits for the link-local address
        EV_INFO << "Interface '" << ie->getInterfaceName() << "' has no valid link-local address yet, holding the message.\n";
        heldMessages[ie->getInterfaceId()].push_back(msg);
        return;
    }
    send(msg, "ipOut");
}

// --- Lifecycle ---

void Mldv1::sendHeldMessages(NetworkInterface *ie)
{
    auto it = heldMessages.find(ie->getInterfaceId());
    if (it == heldMessages.end())
        return;
    std::vector<Packet *> messages = it->second;
    heldMessages.erase(it);
    for (auto msg : messages) {
        msg->addTagIfAbsent<L3AddressReq>()->setSrcAddress(ie->getProtocolData<Ipv6InterfaceData>()->getLinkLocalAddress());
        send(msg, "ipOut");
    }
}

void Mldv1::deleteHeldMessages(int interfaceId)
{
    auto it = heldMessages.find(interfaceId);
    if (it == heldMessages.end())
        return;
    for (auto msg : it->second)
        delete msg;
    heldMessages.erase(it);
}

void Mldv1::handleStopOperation(LifecycleOperation *operation)
{
    while (!heldMessages.empty())
        deleteHeldMessages(heldMessages.begin()->first);
    // Clear per-interface host state and cancel timers. Signal subscriptions are
    // NOT removed here: like Igmpv2, MLD subscribes once in initialize() and keeps
    // the subscription for the module's lifetime, so a stop/start lifecycle cycle
    // does not leave the module deaf to membership-change signals.
    while (!hostData.empty())
        deleteHostInterfaceData(hostData.begin()->first);
    while (!routerData.empty())
        deleteRouterInterfaceData(routerData.begin()->first);
}

void Mldv1::handleCrashOperation(LifecycleOperation *operation)
{
    while (!heldMessages.empty())
        deleteHeldMessages(heldMessages.begin()->first);
    while (!hostData.empty())
        deleteHostInterfaceData(hostData.begin()->first);
    while (!routerData.empty())
        deleteRouterInterfaceData(routerData.begin()->first);
}

// --- Host group-data CRUD helpers ---

Mldv1::HostGroupData *Mldv1::createHostGroupData(NetworkInterface *ie, const Ipv6Address& group)
{
    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    ASSERT(interfaceData->groups.find(group) == interfaceData->groups.end());
    HostGroupData *data = new HostGroupData(this, group);
    interfaceData->groups[group] = data;
    return data;
}

Mldv1::HostGroupData *Mldv1::getHostGroupData(NetworkInterface *ie, const Ipv6Address& group)
{
    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    auto it = interfaceData->groups.find(group);
    return it != interfaceData->groups.end() ? it->second : nullptr;
}

void Mldv1::deleteHostGroupData(NetworkInterface *ie, const Ipv6Address& group)
{
    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    auto it = interfaceData->groups.find(group);
    if (it != interfaceData->groups.end()) {
        HostGroupData *data = it->second;
        interfaceData->groups.erase(it);
        delete data;
    }
}

// --- Host interface-data CRUD helpers ---

Mldv1::HostInterfaceData *Mldv1::getHostInterfaceData(NetworkInterface *ie)
{
    int interfaceId = ie->getInterfaceId();
    auto it = hostData.find(interfaceId);
    if (it != hostData.end())
        return it->second;

    // Create on demand
    HostInterfaceData *data = createHostInterfaceData();
    hostData[interfaceId] = data;
    return data;
}

Mldv1::HostInterfaceData *Mldv1::createHostInterfaceData()
{
    return new HostInterfaceData(this);
}

void Mldv1::deleteHostInterfaceData(int interfaceId)
{
    auto interfaceIt = hostData.find(interfaceId);
    if (interfaceIt != hostData.end()) {
        HostInterfaceData *data = interfaceIt->second;
        hostData.erase(interfaceIt);
        delete data;
    }
}

// --- Router group-data CRUD helpers ---

Mldv1::RouterGroupData *Mldv1::createRouterGroupData(NetworkInterface *ie, const Ipv6Address& group)
{
    RouterInterfaceData *ifData = getRouterInterfaceData(ie);
    ASSERT(ifData->groups.find(group) == ifData->groups.end());
    RouterGroupData *data = new RouterGroupData(this, group);
    ifData->groups[group] = data;
    return data;
}

Mldv1::RouterGroupData *Mldv1::getRouterGroupData(NetworkInterface *ie, const Ipv6Address& group)
{
    RouterInterfaceData *ifData = getRouterInterfaceData(ie);
    auto it = ifData->groups.find(group);
    return it != ifData->groups.end() ? it->second : nullptr;
}

void Mldv1::deleteRouterGroupData(NetworkInterface *ie, const Ipv6Address& group)
{
    RouterInterfaceData *ifData = getRouterInterfaceData(ie);
    auto it = ifData->groups.find(group);
    if (it != ifData->groups.end()) {
        RouterGroupData *data = it->second;
        ifData->groups.erase(it);
        delete data;
    }
}

// --- Router interface-data CRUD helpers ---

Mldv1::RouterInterfaceData *Mldv1::getRouterInterfaceData(NetworkInterface *ie)
{
    int interfaceId = ie->getInterfaceId();
    auto it = routerData.find(interfaceId);
    if (it != routerData.end())
        return it->second;

    // Create on demand
    RouterInterfaceData *data = createRouterInterfaceData();
    routerData[interfaceId] = data;
    return data;
}

Mldv1::RouterInterfaceData *Mldv1::createRouterInterfaceData()
{
    return new RouterInterfaceData(this);
}

void Mldv1::deleteRouterInterfaceData(int interfaceId)
{
    auto it = routerData.find(interfaceId);
    if (it != routerData.end()) {
        RouterInterfaceData *data = it->second;
        routerData.erase(it);
        delete data;
    }
}

// --- Router (querier) behavior ---

void Mldv1::configureInterface(NetworkInterface *ie)
{
    if (enabled && rt->isMulticastForwardingEnabled()) {
        cMessage *timer = new cMessage("Mldv1 query timer", MLD_QUERY_TIMER);
        timer->setContextPointer(ie);
        RouterInterfaceData *routerIfData = getRouterInterfaceData(ie);
        routerIfData->mldQueryTimer = timer;
        if (!hasValidLinkLocalAddress(ie)) {
            EV_INFO << "Mldv1: iface=" << ie->getInterfaceName() << " has no valid link-local address yet; the querier starts when it has one\n";
            routerIfData->waitsForLinkLocalAddress = true;
            return;
        }
        sendQuery(ie, Ipv6Address::UNSPECIFIED_ADDRESS, queryResponseInterval);
        startTimer(timer, startupQueryInterval);
    }
}

void Mldv1::processQueryTimer(cMessage *msg)
{
    NetworkInterface *ie = (NetworkInterface *)msg->getContextPointer();
    ASSERT(ie);
    EV_DEBUG << "Mldv1: General Query timer expired, iface=" << ie->getInterfaceName() << "\n";
    sendQuery(ie, Ipv6Address::UNSPECIFIED_ADDRESS, queryResponseInterval);
    startTimer(msg, queryInterval);
}

void Mldv1::processLeaveTimer(cMessage *msg)
{
    MldRouterTimerContext *ctx = (MldRouterTimerContext *)msg->getContextPointer();
    EV_DEBUG << "Mldv1: Leave Timer expired, removing group=" << ctx->routerGroup->groupAddr
             << " from listener list of '" << ctx->ie->getInterfaceName() << "'\n";

    ctx->ie->getProtocolDataForUpdate<Ipv6InterfaceData>()->removeMulticastListener(ctx->routerGroup->groupAddr);
    numRouterGroups--;

    if (ctx->routerGroup->state == MLD_RGS_CHECKING_LISTENERS)
        cancelEvent(ctx->routerGroup->rexmtTimer);

    ctx->routerGroup->state = MLD_RGS_NO_LISTENERS_PRESENT;
    deleteRouterGroupData(ctx->ie, ctx->routerGroup->groupAddr);
    numGroups--;
}

void Mldv1::processRexmtTimer(cMessage *msg)
{
    MldRouterTimerContext *ctx = (MldRouterTimerContext *)msg->getContextPointer();
    EV_DEBUG << "Mldv1: Rexmt Timer expired for group=" << ctx->routerGroup->groupAddr
             << " iface=" << ctx->ie->getInterfaceName() << "\n";
    sendQuery(ctx->ie, ctx->routerGroup->groupAddr, lastListenerQueryInterval);
    startTimer(ctx->routerGroup->rexmtTimer, lastListenerQueryInterval);
    ctx->routerGroup->state = MLD_RGS_CHECKING_LISTENERS;
}

void Mldv1::sendQuery(NetworkInterface *ie, const Ipv6Address& groupAddr, double maxRespTime)
{
    if (groupAddr.isUnspecified())
        EV_INFO << "Mldv1: sending General Listener Query on iface=" << ie->getInterfaceName() << "\n";
    else
        EV_INFO << "Mldv1: sending Multicast-Address-Specific Query for group=" << groupAddr
                << " on iface=" << ie->getInterfaceName() << "\n";

    Packet *packet = new Packet("Mldv1 query");
    const auto& msg = makeShared<MldQuery>();
    msg->setMulticastAddress(groupAddr);                            // :: for General, group for MAS
    msg->setMaxRespDelay((uint16_t)(maxRespTime * 1000));           // MILLISECONDS (RFC 2710 §3.4)
    msg->setChunkLength(B(24));
    Icmpv6::insertChecksum(checksumMode, msg, packet);
    packet->insertAtFront(msg);
    Ipv6Address dest = groupAddr.isUnspecified() ? Ipv6Address::ALL_NODES_2 : groupAddr;
    sendToIPv6(packet, ie, dest);

    numQueriesSent++;
    if (groupAddr.isUnspecified())
        numGeneralQueriesSent++;
    else
        numGroupSpecificQueriesSent++;
}

void Mldv1::processDone(NetworkInterface *ie, Packet *packet)
{
    ASSERT(ie->isMulticast());

    const auto& msg = packet->peekAtFront<MldDone>();
    Ipv6Address groupAddr = msg->getMulticastAddress();

    EV_INFO << "Mldv1: received Multicast Listener Done for group=" << groupAddr
            << " iface=" << ie->getInterfaceName() << "\n";
    numDonesRecv++;

    if (rt->isMulticastForwardingEnabled()) {
        RouterGroupData *groupData = getRouterGroupData(ie, groupAddr);
        if (groupData && groupData->state == MLD_RGS_LISTENERS_PRESENT) {
            // No querier check — we are always the querier (single-querier decision)
            startTimer(groupData->timer, lastListenerQueryInterval * lastListenerQueryCount);
            startTimer(groupData->rexmtTimer, lastListenerQueryInterval);
            sendQuery(ie, groupAddr, lastListenerQueryInterval);
            groupData->state = MLD_RGS_CHECKING_LISTENERS;
        }
    }

    delete packet;
}

} // namespace inet
