//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
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
// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/routing/pim/modes/PimDm.h"

namespace inet {

Define_Module(PimDm);

simsignal_t PimDm::sentGraftPkSignal        = registerSignal("sentGraftPk");
simsignal_t PimDm::rcvdGraftPkSignal        = registerSignal("rcvdGraftPk");
simsignal_t PimDm::sentGraftAckPkSignal     = registerSignal("sentGraftAckPk");
simsignal_t PimDm::rcvdGraftAckPkSignal     = registerSignal("rcvdGraftAckPk");
simsignal_t PimDm::sentJoinPrunePkSignal    = registerSignal("sentJoinPrunePk");
simsignal_t PimDm::rcvdJoinPrunePkSignal    = registerSignal("rcvdJoinPrunePk");
simsignal_t PimDm::sentAssertPkSignal       = registerSignal("sentAssertPk");
simsignal_t PimDm::rcvdAssertPkSignal       = registerSignal("rcvdAssertPk");
simsignal_t PimDm::sentStateRefreshPkSignal = registerSignal("sentStateRefreshPk");
simsignal_t PimDm::rcvdStateRefreshPkSignal = registerSignal("rcvdStateRefreshPk");

PimDm::~PimDm()
{
    for (auto & elem : routes)
        delete elem.second;
    routes.clear();
}

void PimDm::initialize(int stage)
{
    PimBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        pruneInterval = par("pruneInterval");
        pruneLimitInterval = par("pruneLimitInterval");
        overrideInterval = par("overrideInterval");
        propagationDelay = par("propagationDelay");
        graftRetryInterval = par("graftRetryInterval");
        sourceActiveInterval = par("sourceActiveInterval");
        stateRefreshInterval = par("stateRefreshInterval");
        assertTime = par("assertTime");
    }
}

void PimDm::handleStartOperation(LifecycleOperation *operation)
{
    PimBase::handleStartOperation(operation);

    // subscribe for notifications
    if (isEnabled) {
        cModule *host = findContainingNode(this);
        if (!host)
            throw cRuntimeError("PimDm: containing node not found.");
        host->subscribe(ipv4NewMulticastSignal, this);
        host->subscribe(ipv4MulticastGroupRegisteredSignal, this);
        host->subscribe(ipv4MulticastGroupUnregisteredSignal, this);
        host->subscribe(ipv4DataOnNonrpfSignal, this);
        host->subscribe(ipv4DataOnRpfSignal, this);
        host->subscribe(routeAddedSignal, this);
        host->subscribe(interfaceStateChangedSignal, this);

        WATCH_PTRMAP(routes);
    }
}

void PimDm::handleStopOperation(LifecycleOperation *operation)
{
    // TODO send PIM Hellos to neighbors with 0 HoldTime
    stopPIMRouting();
    PimBase::handleStopOperation(operation);
}

void PimDm::handleCrashOperation(LifecycleOperation *operation)
{
    stopPIMRouting();
    PimBase::handleCrashOperation(operation);
}

void PimDm::stopPIMRouting()
{
    if (isEnabled) {
        cModule *host = findContainingNode(this);
        if (!host)
            throw cRuntimeError("PimDm: containing node not found.");
        host->unsubscribe(ipv4NewMulticastSignal, this);
        host->unsubscribe(ipv4MulticastGroupRegisteredSignal, this);
        host->unsubscribe(ipv4MulticastGroupUnregisteredSignal, this);
        host->unsubscribe(ipv4DataOnNonrpfSignal, this);
        host->unsubscribe(ipv4DataOnRpfSignal, this);
        host->unsubscribe(routeAddedSignal, this);
        host->unsubscribe(interfaceStateChangedSignal, this);
    }

    clearRoutes();
}

void PimDm::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        switch (msg->getKind()) {
            case HelloTimer:
                processHelloTimer(msg);
                break;

            case AssertTimer:
                processAssertTimer(msg);
                break;

            case PruneTimer:
                processPruneTimer(msg);
                break;

            case PrunePendingTimer:
                processPrunePendingTimer(msg);
                break;

            case GraftRetryTimer:
                processGraftRetryTimer(msg);
                break;

            case UpstreamOverrideTimer:
                processOverrideTimer(msg);
                break;

            case PruneLimitTimer:
                break;

            case SourceActiveTimer:
                processSourceActiveTimer(msg);
                break;

            case StateRefreshTimer:
                processStateRefreshTimer(msg);
                break;

            default:
                throw cRuntimeError("PimDm: unknown self message: %s (%s)", msg->getName(), msg->getClassName());
        }
    }
    else {
        Packet *pk = check_and_cast<Packet *>(msg);
        const auto& pkt = pk->peekAtFront<PimPacket>();
        if (pkt == nullptr)
            throw cRuntimeError("PimDm: received unknown message: %s (%s).", msg->getName(), msg->getClassName());

        if (!isEnabled) {
            EV_DETAIL << "PIM-DM is disabled, dropping packet.\n";
            delete msg;
            return;
        }

        switch (pkt->getType()) {
            case Hello:
                processHelloPacket(pk);
                break;

            case JoinPrune:
                processJoinPrunePacket(pk);
                break;

            case Assert:
                processAssertPacket(pk);
                break;

            case Graft:
                processGraftPacket(pk);
                break;

            case GraftAck:
                processGraftAckPacket(pk);
                break;

            case StateRefresh:
                processStateRefreshPacket(pk);
                break;

            default:
                EV_WARN << "Dropping packet " << pk->getName() << ".\n";
                delete pk;
                break;
        }
    }
}

void PimDm::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();
    printSignalBanner(signalID, obj, details);
    const Ipv4Header *ipv4Header;
    PimInterface *pimInterface;

    // new multicast data appears in router
    // note: this signal is often emitted from Ipv4::forwardMulticastPacket method
    if (signalID == ipv4NewMulticastSignal) {
        ipv4Header = check_and_cast<const Ipv4Header *>(obj);
        pimInterface = getIncomingInterface(check_and_cast<InterfaceEntry *>(details));
        if (pimInterface && pimInterface->getMode() == PimInterface::DenseMode)
            unroutableMulticastPacketArrived(ipv4Header->getSrcAddress(), ipv4Header->getDestAddress(), ipv4Header->getTimeToLive());
    }
    // configuration of interface changed, it means some change from IGMP, address were added.
    else if (signalID == ipv4MulticastGroupRegisteredSignal) {
        const Ipv4MulticastGroupInfo *info = check_and_cast<const Ipv4MulticastGroupInfo *>(obj);
        pimInterface = pimIft->getInterfaceById(info->ie->getInterfaceId());
        if (pimInterface && pimInterface->getMode() == PimInterface::DenseMode)
            multicastReceiverAdded(pimInterface->getInterfacePtr(), info->groupAddress);
    }
    // configuration of interface changed, it means some change from IGMP, address were removed.
    else if (signalID == ipv4MulticastGroupUnregisteredSignal) {
        const Ipv4MulticastGroupInfo *info = check_and_cast<const Ipv4MulticastGroupInfo *>(obj);
        pimInterface = pimIft->getInterfaceById(info->ie->getInterfaceId());
        if (pimInterface && pimInterface->getMode() == PimInterface::DenseMode)
            multicastReceiverRemoved(pimInterface->getInterfacePtr(), info->groupAddress);
    }
    // data come to non-RPF interface
    else if (signalID == ipv4DataOnNonrpfSignal) {
        ipv4Header = check_and_cast<const Ipv4Header *>(obj);
        pimInterface = getIncomingInterface(check_and_cast<InterfaceEntry *>(details));
        if (pimInterface && pimInterface->getMode() == PimInterface::DenseMode)
            multicastPacketArrivedOnNonRpfInterface(ipv4Header->getDestAddress(), ipv4Header->getSrcAddress(), pimInterface->getInterfaceId());
    }
    // data come to RPF interface
    else if (signalID == ipv4DataOnRpfSignal) {
        ipv4Header = check_and_cast<const Ipv4Header *>(obj);
        pimInterface = getIncomingInterface(check_and_cast<InterfaceEntry *>(details));
        if (pimInterface && pimInterface->getMode() == PimInterface::DenseMode)
            multicastPacketArrivedOnRpfInterface(pimInterface->getInterfaceId(), ipv4Header->getDestAddress(), ipv4Header->getSrcAddress(), ipv4Header->getTimeToLive());
    }
    // RPF interface has changed
    else if (signalID == routeAddedSignal) {
        const Ipv4Route *entry = check_and_cast<const Ipv4Route *>(obj);
        for (int i = 0; i < rt->getNumMulticastRoutes(); i++) {
            // find multicast routes whose source are on the destination of the new unicast route
            Ipv4MulticastRoute *route = rt->getMulticastRoute(i);
            if (route->getSource() == this && Ipv4Address::maskedAddrAreEqual(route->getOrigin(), entry->getDestination()/*routeSource*/, entry->getNetmask()/*routeNetmask*/)) {
                Ipv4Address source = route->getOrigin();
                Ipv4Route *routeToSource = rt->findBestMatchingRoute(source);
                InterfaceEntry *newRpfInterface = routeToSource->getInterface();
                InterfaceEntry *oldRpfInterface = route->getInInterface()->getInterface();

                // is there any change?
                if (newRpfInterface != oldRpfInterface)
                    rpfInterfaceHasChanged(route, routeToSource);

                // TODO update metric
            }
        }
    }
}

// ---- handle timers ----

void PimDm::processAssertTimer(cMessage *timer)
{
    Interface *interfaceData = static_cast<Interface *>(timer->getContextPointer());
    ASSERT(timer == interfaceData->assertTimer);
    ASSERT(interfaceData->assertState != DownstreamInterface::NO_ASSERT_INFO);

    Route *route = check_and_cast<Route *>(interfaceData->owner);
    UpstreamInterface *upstream = route->upstreamInterface;
    EV_DETAIL << "AssertTimer" << route << " interface=" << interfaceData->ie->getInterfaceName() << " has expired.\n";

    //
    // Assert State Machine; event: AT(S,G,I) expires
    //

    // The Assert state machine MUST transition to NoInfo (NI) state.
    // The router MUST delete the Assert Winner's address and metric.
    // If CouldAssert == TRUE, the router MUST evaluate any possible
    // transitions to its Upstream(S,G) state machine.
    EV_DEBUG << "Going into NO_ASSERT_INFO state.\n";
    interfaceData->deleteAssertInfo();    // deletes timer

    // upstream state machine transition
    if (interfaceData != upstream) {
        bool isOlistNull = route->isOilistNull();
        if (upstream->graftPruneState == UpstreamInterface::PRUNED && !isOlistNull)
            processOlistNonEmptyEvent(route);
        else if (upstream->graftPruneState != UpstreamInterface::PRUNED && isOlistNull)
            processOlistEmptyEvent(route);
    }
}

/*
 * The Prune Timer (PT(S,G,I)) expires, indicating that it is again
 * time to flood data from S addressed to group G onto interface I.
 * The Prune(S,G) Downstream state machine on interface I MUST
 * transition to the NoInfo (NI) state.  The router MUST evaluate
 * any possible transitions in the Upstream(S,G) state machine.
 */
void PimDm::processPruneTimer(cMessage *timer)
{
    DownstreamInterface *downstream = static_cast<DownstreamInterface *>(timer->getContextPointer());
    ASSERT(timer == downstream->pruneTimer);
    ASSERT(downstream->pruneState == DownstreamInterface::PRUNED);

    Route *route = downstream->route();
    EV_INFO << "PruneTimer" << route << " expired.\n";

    // state of interface is changed to forwarding
    downstream->stopPruneTimer();
    downstream->pruneState = DownstreamInterface::NO_INFO;

    // upstream state change if olist become non nullptr
    if (!route->isOilistNull())
        processOlistNonEmptyEvent(route);
}

// See RFC 3973 4.4.2.2
/*
   The PrunePending Timer (PPT(S,G,I)) expires, indicating that no
   neighbors have overridden the previous Prune(S,G) message.  The
   Prune(S,G) Downstream state machine on interface I MUST
   transition to the Pruned (P) state.  The Prune Timer (PT(S,G,I))
   is started and MUST be initialized to the received
   Prune_Hold_Time minus J/P_Override_Interval.  A PruneEcho(S,G)
   MUST be sent on I if I has more than one PIM neighbor.  A
   PruneEcho(S,G) is simply a Prune(S,G) message multicast by the
   upstream router to a LAN, with itself as the Upstream Neighbor.
   Its purpose is to add additional reliability so that if a Join
   that should have overridden the Prune is lost locally on the LAN,
   the PruneEcho(S,G) may be received and trigger a new Join
   message.  A PruneEcho(S,G) is OPTIONAL on an interface with only
   one PIM neighbor.  In addition, the router MUST evaluate any
   possible transitions in the Upstream(S,G) state machine.
 */
void PimDm::processPrunePendingTimer(cMessage *timer)
{
    DownstreamInterface *downstream = static_cast<DownstreamInterface *>(timer->getContextPointer());
    ASSERT(timer == downstream->prunePendingTimer);
    ASSERT(downstream->pruneState == DownstreamInterface::PRUNE_PENDING);

    delete timer;
    downstream->prunePendingTimer = nullptr;

    Route *route = downstream->route();
    EV_INFO << "PrunePendingTimer" << route << " has expired.\n";

    //
    // go to pruned state
    downstream->pruneState = DownstreamInterface::PRUNED;
    double holdTime = pruneInterval - overrideInterval - propagationDelay;    // XXX should be received HoldTime - computed override interval;
    downstream->startPruneTimer(holdTime);

    // TODO optionally send PruneEcho

    // upstream state change if olist become nullptr
    if (route->isOilistNull())
        processOlistEmptyEvent(route);
}

void PimDm::processGraftRetryTimer(cMessage *timer)
{
    EV_INFO << "GraftRetryTimer expired.\n";
    UpstreamInterface *upstream = static_cast<UpstreamInterface *>(timer->getContextPointer());
    ASSERT(upstream->graftPruneState == UpstreamInterface::ACK_PENDING);
    ASSERT(timer == upstream->graftRetryTimer);

    Route *route = upstream->route();
    EV_INFO << "GraftRetryTimer" << route << " expired.\n";

    // The Upstream(S,G) state machine stays in the AckPending (AP)
    // state.  Another Graft message for (S,G) SHOULD be unicast to
    // RPF'(S) and the GraftRetry Timer (GRT(S,G)) reset to
    // Graft_Retry_Period.  It is RECOMMENDED that the router retry a
    // configured number of times before ceasing retries.
    sendGraftPacket(upstream->rpfNeighbor(), route->source, route->group, upstream->getInterfaceId());
    scheduleAfter(graftRetryInterval, timer);
}

void PimDm::processOverrideTimer(cMessage *timer)
{
    UpstreamInterface *upstream = static_cast<UpstreamInterface *>(timer->getContextPointer());
    ASSERT(timer == upstream->overrideTimer);
    ASSERT(upstream->graftPruneState != UpstreamInterface::PRUNED);

    Route *route = upstream->route();
    EV_INFO << "OverrideTimer" << route << " expired.\n";

    // send a Join(S,G) to RPF'(S)
    sendJoinPacket(upstream->rpfNeighbor(), route->source, route->group, upstream->getInterfaceId());

    upstream->overrideTimer = nullptr;
    delete timer;
}

/*
 * SAT(S,G) Expires
       The router MUST cancel the SRT(S,G) timer and transition to the
       NotOriginator (NO) state.
 */
void PimDm::processSourceActiveTimer(cMessage *timer)
{
    UpstreamInterface *upstream = static_cast<UpstreamInterface *>(timer->getContextPointer());
    ASSERT(timer == upstream->sourceActiveTimer);
    ASSERT(upstream->originatorState == UpstreamInterface::ORIGINATOR);

    Route *route = upstream->route();
    EV_INFO << "SourceActiveTimer" << route << " expired.\n";

    upstream->originatorState = UpstreamInterface::NOT_ORIGINATOR;
    cancelAndDelete(upstream->stateRefreshTimer);
    upstream->stateRefreshTimer = nullptr;

    // delete the route, because there are no more packets
    Ipv4Address routeSource = route->source;
    Ipv4Address routeGroup = route->group;
    deleteRoute(routeSource, routeGroup);
    Ipv4MulticastRoute *ipv4Route = findIpv4MulticastRoute(routeGroup, routeSource);
    if (ipv4Route)
        rt->deleteMulticastRoute(ipv4Route);
}

/*
 * State Refresh Timer is used only on router which is connected directly to the source of multicast.
 *
 * SRT(S,G) Expires
 *      The router remains in the Originator (O) state and MUST reset
 *      SRT(S,G) to StateRefreshInterval.  The router MUST also generate
 *      State Refresh messages for transmission, as described in the
 *      State Refresh Forwarding rules (Section 4.5.1), except for the
 *      TTL.  If the TTL of data packets from S to G are being recorded,
 *      then the TTL of each State Refresh message is set to the highest
 *      recorded TTL.  Otherwise, the TTL is set to the configured State
 *      Refresh TTL.  Let I denote the interface over which a State
 *      Refresh message is being sent.  If the Prune(S,G) Downstream
 *      state machine is in the Pruned (P) state, then the Prune-
 *      Indicator bit MUST be set to 1 in the State Refresh message being
 *      sent over I. Otherwise, the Prune-Indicator bit MUST be set to 0.
 */
void PimDm::processStateRefreshTimer(cMessage *timer)
{
    UpstreamInterface *upstream = static_cast<UpstreamInterface *>(timer->getContextPointer());
    ASSERT(timer == upstream->stateRefreshTimer);
    ASSERT(upstream->originatorState == UpstreamInterface::ORIGINATOR);

    Route *route = upstream->route();

    EV_INFO << "StateRefreshTimer" << route << " expired.\n";

    EV_DETAIL << "Sending StateRefresh packets on downstream interfaces.\n";
    for (unsigned int i = 0; i < route->downstreamInterfaces.size(); i++) {
        DownstreamInterface *downstream = route->downstreamInterfaces[i];
        // TODO check ttl threshold and boundary
        if (downstream->assertState == DownstreamInterface::I_LOST_ASSERT)
            continue;

        // when sending StateRefresh in Pruned state, then restart PruneTimer
        bool isPruned = downstream->pruneState == DownstreamInterface::PRUNED;
        if (isPruned)
            restartTimer(downstream->pruneTimer, pruneInterval);

        // Send StateRefresh message downstream
        Ipv4Address originator = downstream->ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress();
        sendStateRefreshPacket(originator, route, downstream, upstream->maxTtlSeen);
    }

    scheduleAfter(stateRefreshInterval, timer);
}

// ---- process PIM Messages ----

void PimDm::processJoinPrunePacket(Packet *pk)
{
    const auto& pkt = pk->peekAtFront<PimJoinPrune>();
    EV_INFO << "Received JoinPrune packet.\n";

    emit(rcvdJoinPrunePkSignal, pk);

    auto ifTag = pk->getTag<InterfaceInd>();
    InterfaceEntry *incomingInterface = ift->getInterfaceById(ifTag->getInterfaceId());

    if (!incomingInterface) {
        delete pk;
        return;
    }

    Ipv4Address upstreamNeighborAddress = pkt->getUpstreamNeighborAddress().unicastAddress.toIpv4();
    int numRpfNeighbors = pimNbt->getNumNeighbors(incomingInterface->getInterfaceId());

    for (unsigned int i = 0; i < pkt->getJoinPruneGroupsArraySize(); i++) {
        JoinPruneGroup group = pkt->getJoinPruneGroups(i);
        Ipv4Address groupAddr = group.getGroupAddress().groupAddress.toIpv4();

        // go through list of joined sources
        for (unsigned int j = 0; j < group.getJoinedSourceAddressArraySize(); j++) {
            const auto& source = group.getJoinedSourceAddress(j);
            Route *route = findRoute(source.sourceAddress.toIpv4(), groupAddr);
            ASSERT(route);
            processJoin(route, incomingInterface->getInterfaceId(), numRpfNeighbors, upstreamNeighborAddress);
        }

        // go through list of pruned sources
        for (unsigned int j = 0; j < group.getPrunedSourceAddressArraySize(); j++) {
            const auto& source = group.getPrunedSourceAddress(j);
            Route *route = findRoute(source.sourceAddress.toIpv4(), groupAddr);
            ASSERT(route);
            processPrune(route, incomingInterface->getInterfaceId(), pkt->getHoldTime(), numRpfNeighbors, upstreamNeighborAddress);
        }
    }

    delete pk;
}

void PimDm::processJoin(Route *route, int intId, int numRpfNeighbors, Ipv4Address upstreamNeighborField)
{
    UpstreamInterface *upstream = route->upstreamInterface;

    // See join to RPF'(S,G) ?
    if (upstream->ie->getInterfaceId() == intId && numRpfNeighbors > 1) {
        // TODO check that destAddress == upstream->nextHop
        if (upstream->graftPruneState == UpstreamInterface::FORWARDING || upstream->graftPruneState == UpstreamInterface::ACK_PENDING) {
            // If the OT(S,G) is running, then it
            // means that the router had scheduled a Join to override a
            // previously received Prune.  Another router has responded more
            // quickly with a Join, so the local router SHOULD cancel its
            // OT(S,G), if it is running.
            cancelAndDelete(upstream->overrideTimer);
            upstream->overrideTimer = nullptr;
        }
    }

    //
    // Downstream Interface State Machine
    //
    DownstreamInterface *downstream = route->findDownstreamInterfaceByInterfaceId(intId);
    if (!downstream)
        return;

    // does packet belong to this router?
    if (upstreamNeighborField != downstream->ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress())
        return;

    if (downstream->pruneState == DownstreamInterface::PRUNE_PENDING)
        downstream->stopPrunePendingTimer();
    else if (downstream->pruneState == DownstreamInterface::PRUNED)
        downstream->stopPruneTimer();

    downstream->pruneState = DownstreamInterface::NO_INFO;

    if (upstream->graftPruneState == UpstreamInterface::PRUNED && !route->isOilistNull())
        processOlistNonEmptyEvent(route); // will send Graft upstream

    //
    // Assert State Machine; event: Receive Join(S,G)
    //
    if (downstream->assertState == DownstreamInterface::I_LOST_ASSERT) {
        // A Join(S,G) message was received on
        // interface I with its upstream neighbor address set to the
        // router's address on I.  The router MUST send an Assert(S,G) on
        // the receiving interface I to initiate an Assert negotiation.  The
        // Assert state machine remains in the Assert Loser(L) state.
        sendAssertPacket(route->source, route->group, route->metric, downstream->ie);
    }
}

/**
 * The method process PIM Prune packet. First the method has to find correct outgoing interface
 * where PIM Prune packet came to. The method also checks if there is still any forwarding outgoing
 * interface. Forwarding interfaces, where Prune packet come to, goes to prune state. If all outgoing
 * interfaces are pruned, the router will prune from multicast tree.
 */
void PimDm::processPrune(Route *route, int intId, int holdTime, int numRpfNeighbors, Ipv4Address upstreamNeighborField)
{
    EV_INFO << "Processing Prune" << route << ".\n";

    //
    // Upstream Interface State Machine; event: See Prune(S,G)
    //

    // See Prune(S,G) AND S is NOT directly connected ?
    UpstreamInterface *upstream = route->upstreamInterface;
    if (upstream->ie->getInterfaceId() == intId && !upstream->isSourceDirectlyConnected()) {
        // This event is only relevant if RPF_interface(S) is a shared
        // medium.  This router sees another router on RPF_interface(S) send
        // a Prune(S,G).  When this router is in Forwarding/AckPending state, it must
        // override the Prune after a short random interval.  If OT(S,G) is
        // not running, the router MUST set OT(S,G) to t_override seconds.
        // The Upstream(S,G) state machine remains in Forwarding/AckPending state.
        if (upstream->graftPruneState == UpstreamInterface::FORWARDING || upstream->graftPruneState == UpstreamInterface::ACK_PENDING) {
            if (!upstream->overrideTimer)
                upstream->startOverrideTimer();
        }
    }

    // we find correct outgoing interface
    DownstreamInterface *downstream = route->findDownstreamInterfaceByInterfaceId(intId);
    if (!downstream)
        return;

    // does packet belong to this router?
    if (upstreamNeighborField != downstream->ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress())
        return;

    //
    // Downstream Interface State Machine; event: Receive Prune(S,G)
    //

    // A Prune(S,G) is received on interface I with the upstream
    // neighbor field set to the router's address on I.
    if (downstream->pruneState == DownstreamInterface::NO_INFO) {
        // The Prune(S,G) Downstream state machine on interface I MUST transition to the
        // PrunePending (PP) state.  The PrunePending Timer (PPT(S,G,I))
        // MUST be set to J/P_Override_Interval if the router has more than
        // one neighbor on I.  If the router has only one neighbor on
        // interface I, then it SHOULD set the PPT(S,G,I) to zero,
        // effectively transitioning immediately to the Pruned (P) state.
        double prunePendingInterval = numRpfNeighbors > 1 ? overrideInterval + propagationDelay : 0;
        downstream->startPrunePendingTimer(prunePendingInterval);
        downstream->pruneState = DownstreamInterface::PRUNE_PENDING;
    }
    else if (downstream->pruneState == DownstreamInterface::PRUNED) {
        // The Prune(S,G) Downstream state machine on interface I remains in the Pruned (P)
        // state.  The Prune Timer (PT(S,G,I)) SHOULD be reset to the
        // holdtime contained in the Prune(S,G) message if it is greater
        // than the current value.
        EV << "Outgoing interface is already pruned, restart Prune Timer." << endl;
        if (downstream->pruneTimer->getArrivalTime() < simTime() + holdTime)
            restartTimer(downstream->pruneTimer, holdTime);
    }

    //
    // Assert state machine; event: Receive Prune(S,G)
    //
    if (downstream->assertState == DownstreamInterface::I_LOST_ASSERT) {
        // Receive Prune(S,G)
        //     A Prune(S,G) message was received on
        //     interface I with its upstream neighbor address set to the
        //     router's address on I.  The router MUST send an Assert(S,G) on
        //     the receiving interface I to initiate an Assert negotiation.  The
        //     Assert state machine remains in the Assert Loser(L) state.
        sendAssertPacket(route->source, route->group, route->metric, downstream->ie);
    }
}

void PimDm::processAssertPacket(Packet *pk)
{
    const auto& pkt = pk->peekAtFront<PimAssert>();
    int incomingInterfaceId = pk->getTag<InterfaceInd>()->getInterfaceId();
    Ipv4Address srcAddrFromTag = pk->getTag<L3AddressInd>()->getSrcAddress().toIpv4();
    Ipv4Address source = pkt->getSourceAddress().unicastAddress.toIpv4();
    Ipv4Address group = pkt->getGroupAddress().groupAddress.toIpv4();
    AssertMetric receivedMetric = AssertMetric(pkt->getMetricPreference(), pkt->getMetric(), srcAddrFromTag);
    Route *route = findRoute(source, group);
    ASSERT(route);    // XXX create S,G state?
    Interface *incomingInterface = route->upstreamInterface->getInterfaceId() == incomingInterfaceId ?
        static_cast<Interface *>(route->upstreamInterface) :
        static_cast<Interface *>(route->findDownstreamInterfaceByInterfaceId(incomingInterfaceId));
    ASSERT(incomingInterface);

    EV_INFO << "Received Assert(S=" << source << ", G=" << group
            << ") packet on interface '" << incomingInterface->ie->getInterfaceName() << "'.\n";

    emit(rcvdAssertPkSignal, pk);

    processAssert(incomingInterface, receivedMetric, 0);

    delete pk;
}

void PimDm::processGraftPacket(Packet *pk)
{
    const auto& pkt = pk->peekAtFront<PimGraft>();
    EV_INFO << "Received Graft packet.\n";

    emit(rcvdGraftPkSignal, pk);

    Ipv4Address sender = pk->getTag<L3AddressInd>()->getSrcAddress().toIpv4();
    InterfaceEntry *incomingInterface = ift->getInterfaceById(pk->getTag<InterfaceInd>()->getInterfaceId());

    // does packet belong to this router?
    if (pkt->getUpstreamNeighborAddress().unicastAddress != incomingInterface->getProtocolData<Ipv4InterfaceData>()->getIPAddress()) {
        delete pk;
        return;
    }

    for (unsigned int i = 0; i < pkt->getJoinPruneGroupsArraySize(); i++) {
        const JoinPruneGroup& group = pkt->getJoinPruneGroups(i);
        Ipv4Address groupAddr = group.getGroupAddress().groupAddress.toIpv4();

        for (unsigned int j = 0; j < group.getJoinedSourceAddressArraySize(); j++) {
            const auto& source = group.getJoinedSourceAddress(j);
            processGraft(source.sourceAddress.toIpv4(), groupAddr, sender, incomingInterface->getInterfaceId());
        }
    }

    // Send GraftAck for this Graft message
    sendGraftAckPacket(pk, pkt);

    delete pk;
}

/**
 * The method is used to process PimGraft packet. Packet means that downstream router wants to join to
 * multicast tree, so the packet cannot come to RPF interface. Router finds correct outgoig interface
 * towards downstream router. Change its state to forward if it was not before and cancel Prune Timer.
 * If route was in pruned state, router will send also Graft message to join multicast tree.
 */
void PimDm::processGraft(Ipv4Address source, Ipv4Address group, Ipv4Address sender, int incomingInterfaceId)
{
    EV_DEBUG << "Processing Graft(S=" << source << ", G=" << group << "), sender=" << sender << "incoming if=" << incomingInterfaceId << endl;

    Route *route = findRoute(source, group);
    ASSERT(route);

    UpstreamInterface *upstream = route->upstreamInterface;

    // check if message come to non-RPF interface
    if (upstream->ie->getInterfaceId() == incomingInterfaceId) {
        EV << "ERROR: Graft message came to RPF interface." << endl;
        return;
    }

    DownstreamInterface *downstream = route->findDownstreamInterfaceByInterfaceId(incomingInterfaceId);
    if (!downstream)
        return;

    //
    // Downstream Interface State Machine
    //
    // Note: GraftAck is sent in processGraftPacket()
    bool olistChanged = false;
    switch (downstream->pruneState) {
        case DownstreamInterface::NO_INFO:
            // do nothing
            break;

        case DownstreamInterface::PRUNE_PENDING:
            downstream->stopPrunePendingTimer();
            downstream->pruneState = DownstreamInterface::NO_INFO;
            break;

        case DownstreamInterface::PRUNED:
            EV << "Interface " << downstream->ie->getInterfaceId() << " transit to forwarding state (Graft)." << endl;
            downstream->stopPruneTimer();
            downstream->pruneState = DownstreamInterface::NO_INFO;
            olistChanged = downstream->isInOlist();
            break;
    }

    if (olistChanged)
        processOlistNonEmptyEvent(route);

    //
    // Assert State Machine; event: Receive Graft(S,G)
    //
    if (downstream->assertState == DownstreamInterface::I_LOST_ASSERT) {
        // A Graft(S,G) message was received on
        // interface I with its upstream neighbor address set to the
        // router's address on I.  The router MUST send an Assert(S,G) on
        // the receiving interface I to initiate an Assert negotiation. The
        // Assert state machine remains in the Assert Loser(L) state.
        // The router MUST respond with a GraftAck(S,G).
        sendAssertPacket(route->source, route->group, route->metric, downstream->ie);
    }
}

void PimDm::processGraftAckPacket(Packet *pk)
{
    const auto& pkt = pk->peekAtFront<PimGraft>();
    EV_INFO << "Received GraftAck packet.\n";

    emit(rcvdGraftAckPkSignal, pk);

    Ipv4Address destAddress = pk->getTag<L3AddressInd>()->getDestAddress().toIpv4();

    for (unsigned int i = 0; i < pkt->getJoinPruneGroupsArraySize(); i++) {
        const JoinPruneGroup& group = pkt->getJoinPruneGroups(i);
        Ipv4Address groupAddr = group.getGroupAddress().groupAddress.toIpv4();

        for (unsigned int j = 0; j < group.getJoinedSourceAddressArraySize(); j++) {
            const auto& source = group.getJoinedSourceAddress(j);
            Route *route = findRoute(source.sourceAddress.toIpv4(), groupAddr);
            ASSERT(route);
            UpstreamInterface *upstream = route->upstreamInterface;

            // If the destination address of the GraftAck packet is not
            // the address of the upstream interface, then no state transition occur.
            if (destAddress != upstream->ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress())
                continue;

            // upstream state transition
            // event: Receive GraftAck(S,G) from RPF'(S)
            if (upstream->graftPruneState == UpstreamInterface::ACK_PENDING) {
                // A GraftAck is received from  RPF'(S).  The GraftRetry Timer MUST
                // be cancelled, and the Upstream(S,G) state machine MUST transition
                // to the Forwarding(F) state.
                ASSERT(upstream->graftRetryTimer);
                cancelAndDelete(upstream->graftRetryTimer);
                upstream->graftRetryTimer = nullptr;
                upstream->graftPruneState = UpstreamInterface::FORWARDING;
            }
        }
    }

    delete pk;
}

/**
 * The method is used to process PimStateRefresh packet. The method checks if there is route in mroute
 * and that packet has came to RPF interface. Then it goes through all outgoing interfaces. If the
 * interface is pruned, it resets Prune Timer. For each interface State Refresh message is copied and
 * correct prune indicator is set according to state of outgoing interface (pruned/forwarding).
 *
 * State Refresh message is used to stop flooding of network each 3 minutes.
 */
void PimDm::processStateRefreshPacket(Packet *pk)
{
    const auto& pkt = pk->peekAtFront<PimStateRefresh>();
    EV << "pimDM::processStateRefreshPacket" << endl;

    emit(rcvdStateRefreshPkSignal, pk);

    // first check if there is route for given group address and source
    Route *route = findRoute(pkt->getSourceAddress().unicastAddress.toIpv4(), pkt->getGroupAddress().groupAddress.toIpv4());
    if (route == nullptr) {
        delete pk;
        return;
    }

    // check if State Refresh msg has came from RPF neighbor
    auto ifTag = pk->getTag<InterfaceInd>();
    Ipv4Address srcAddr = pk->getTag<L3AddressInd>()->getSrcAddress().toIpv4();
    UpstreamInterface *upstream = route->upstreamInterface;
    if (ifTag->getInterfaceId() != upstream->getInterfaceId() || upstream->rpfNeighbor() != srcAddr) {
        delete pk;
        return;
    }

    // upstream state transitions
    bool pruneIndicator = pkt->getP();
    switch (upstream->graftPruneState) {
        case UpstreamInterface::FORWARDING:
            if (pruneIndicator) {
                upstream->startOverrideTimer();
            }
            break;

        case UpstreamInterface::PRUNED:
            if (!pruneIndicator) {
                if (!upstream->isPruneLimitTimerRunning()) {
                    sendPrunePacket(upstream->rpfNeighbor(), route->source, route->group, pruneInterval, upstream->getInterfaceId());
                    upstream->startPruneLimitTimer();
                }
            }
            else
                upstream->startPruneLimitTimer();
            break;

        case UpstreamInterface::ACK_PENDING:
            if (pruneIndicator) {
                if (!upstream->overrideTimer)
                    upstream->startOverrideTimer();
            }
            else {
                cancelAndDelete(upstream->graftRetryTimer);
                upstream->graftRetryTimer = nullptr;
                upstream->graftPruneState = UpstreamInterface::FORWARDING;
            }
            break;
    }

    //
    // Forward StateRefresh message downstream
    //

    // TODO check StateRefreshRateLimit(S,G)

    if (pkt->getTtl() == 0) {
        delete pk;
        return;
    }

    // go through all outgoing interfaces, reser Prune Timer and send out State Refresh msg
    for (unsigned int i = 0; i < route->downstreamInterfaces.size(); i++) {
        DownstreamInterface *downstream = route->downstreamInterfaces[i];
        // TODO check ttl threshold and boundary
        if (downstream->assertState == DownstreamInterface::I_LOST_ASSERT)
            continue;

        if (downstream->pruneState == DownstreamInterface::PRUNED) {
            // reset PT
            restartTimer(downstream->pruneTimer, pruneInterval);
        }
        sendStateRefreshPacket(pkt->getOriginatorAddress().unicastAddress.toIpv4(), route, downstream, pkt->getTtl() - 1);

        //
        // Assert State Machine; event: Send State Refresh
        //
        if (downstream->assertState == DownstreamInterface::I_WON_ASSERT) {
            // The router is sending a State Refresh(S,G) message on interface I.
            // The router MUST set the Assert Timer (AT(S,G,I)) to three
            // times the State Refresh Interval contained in the State Refresh(S,G) message.
            restartTimer(downstream->assertTimer, 3 * stateRefreshInterval);
        }
    }

    //
    // Assert State Machine; event: Receive State Refresh
    //
    AssertMetric receivedMetric(pkt->getMetricPreference(), pkt->getMetric(), srcAddr);
    processAssert(upstream, receivedMetric, pkt->getInterval());

    delete pk;
}

/*
 * This method called when we received the assert metrics of a neighbor, either in an Assert or in a StateRefresh message.
 */
void PimDm::processAssert(Interface *incomingInterface, AssertMetric receivedMetric, int stateRefreshInterval)
{
    Route *route = check_and_cast<Route *>(incomingInterface->owner);
    UpstreamInterface *upstream = route->upstreamInterface;

    //
    // Assert State Machine
    //
    AssertMetric currentMetric = incomingInterface->assertState == Interface::NO_ASSERT_INFO ?
        route->metric.setAddress(incomingInterface->ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress()) :
        incomingInterface->winnerMetric;
    bool isEqual = receivedMetric == currentMetric;
    bool isBetter = receivedMetric < currentMetric;
    bool couldAssert = incomingInterface != upstream;

    // event: Received Preferred Assert
    if (isBetter || isEqual) {
        if (incomingInterface->assertState == Interface::NO_ASSERT_INFO) {
            // The received Assert or State Refresh has a better metric than
            // this router's, and therefore the Assert state machine MUST
            // transition to the "I am Assert Loser" state and store the Assert
            // Winner's address and metric.  If the metric was received in an
            // Assert, the router MUST set the Assert Timer (AT(S,G,I)) to
            // Assert_Time.  If the metric was received in a State Refresh, the
            // router MUST set the Assert Timer (AT(S,G,I)) to three times the
            // received State Refresh Interval.  If CouldAssert(S,G,I) == TRUE,
            // the router MUST also multicast a Prune(S,G) to the Assert winner
            // with a Prune Hold Time equal to the Assert Timer and evaluate any
            // changes in its Upstream(S,G) state machine.
            ASSERT(isBetter);
            EV_DEBUG << "Received better metrics, going to I_LOST_ASSERT state.\n";
            incomingInterface->assertState = Interface::I_LOST_ASSERT;
            incomingInterface->winnerMetric = receivedMetric;
            double assertTime = stateRefreshInterval > 0 ? 3 * stateRefreshInterval : this->assertTime;
            incomingInterface->startAssertTimer(assertTime);
            if (couldAssert)
                sendPrunePacket(incomingInterface->winnerMetric.address, route->source, route->group, assertTime, incomingInterface->ie->getInterfaceId());

            // upstream state machine
            if (upstream->graftPruneState != UpstreamInterface::PRUNED && route->isOilistNull())
                processOlistEmptyEvent(route);
        }
        else if (incomingInterface->assertState == Interface::I_WON_ASSERT) {
            // An (S,G) Assert is received that has a better
            // metric than this router's metric for S on interface I. The
            // Assert state machine MUST transition to "I am Assert Loser" state
            // and store the new Assert Winner's address and metric. The router MUST set the Assert
            // Timer (AT(S,G,I)) to Assert_Time. The router MUST also
            // multicast a Prune(S,G) to the Assert winner, with a Prune Hold
            // Time equal to the Assert Timer, and evaluate any changes in its
            // Upstream(S,G) state machine.
            ASSERT(isBetter);
            EV_DEBUG << "Received better metrics, going to I_LOST_ASSERT state.\n";
            incomingInterface->assertState = DownstreamInterface::I_LOST_ASSERT;
            incomingInterface->winnerMetric = receivedMetric;
            restartTimer(incomingInterface->assertTimer, assertTime);
            sendPrunePacket(incomingInterface->winnerMetric.address, route->source, route->group, assertTime, incomingInterface->ie->getInterfaceId());

            // upstream state machine
            if (upstream->graftPruneState != UpstreamInterface::PRUNED && route->isOilistNull())
                processOlistEmptyEvent(route);
        }
        else if (incomingInterface->assertState == Interface::I_LOST_ASSERT) {
            // An Assert is received that has a metric better
            // than or equal to that of the current Assert winner.  The Assert
            // state machine remains in Loser (L) state.  If the metric was
            // received in an Assert, the router MUST set the Assert Timer
            // (AT(S,G,I)) to Assert_Time.  The router MUST set the Assert Timer (AT(S,G,I))
            // to three times the received State Refresh Interval.  If the
            // metric is better than the current Assert Winner, the router MUST
            // store the address and metric of the new Assert Winner, and if
            // CouldAssert(S,G,I) == TRUE, the router MUST multicast a
            // Prune(S,G) to the new Assert winner.
            EV_DEBUG << "Received better metrics, stay in I_LOST_ASSERT state.\n";
            restartTimer(incomingInterface->assertTimer, stateRefreshInterval > 0 ? 3 * stateRefreshInterval : assertTime);
            if (isBetter) {
                incomingInterface->winnerMetric = receivedMetric;
                if (couldAssert)
                    sendPrunePacket(incomingInterface->winnerMetric.address, route->source, route->group, assertTime, incomingInterface->ie->getInterfaceId());
            }
        }
    }
    // event: Receive Inferior Assert from Assert Winner
    else if (receivedMetric.address == incomingInterface->winnerMetric.address) {
        if (incomingInterface->assertState == Interface::I_LOST_ASSERT) {
            // An Assert is received from the current Assert
            // winner that is worse than this router's metric for S (typically,
            // the winner's metric became worse).  The Assert state machine MUST
            // transition to NoInfo (NI) state and cancel AT(S,G,I).  The router
            // MUST delete the previous Assert Winner's address and metric and
            // evaluate any possible transitions to its Upstream(S,G) state
            // machine.  Usually this router will eventually re-assert and win
            // when data packets from S have started flowing again.
            EV_DEBUG << "Assert winner lost best route, going to NO_ASSERT_INFO state.\n";
            incomingInterface->deleteAssertInfo();
            // upstream state machine
            if (upstream->graftPruneState == UpstreamInterface::PRUNED && !route->isOilistNull())
                processOlistNonEmptyEvent(route);
        }
    }
    // event: Receive Inferior Assert from non-Assert Winner AND CouldAssert==TRUE
    else if (couldAssert) {
        if (incomingInterface->assertState == Interface::NO_ASSERT_INFO) {
            // An Assert or State Refresh is received for (S,G) that is inferior
            // to our own assert metric on interface I. The Assert state machine
            // MUST transition to the "I am Assert Winner" state, send an
            // Assert(S,G) to interface I, store its own address and metric as
            // the Assert Winner, and set the Assert Timer (AT(S,G,I)) to
            // Assert_Time.
            EV_DEBUG << "Received inferior assert metrics, going to I_WON_ASSERT state.\n";
            incomingInterface->assertState = DownstreamInterface::I_WON_ASSERT;
            sendAssertPacket(route->source, route->group, route->metric, incomingInterface->ie);
            incomingInterface->startAssertTimer(assertTime);
        }
        else if (incomingInterface->assertState == DownstreamInterface::I_WON_ASSERT) {
            // An (S,G) Assert is received containing a metric for S that is
            // worse than this router's metric for S.  Whoever sent the Assert
            // is in error.  The router MUST send an Assert(S,G) to interface I
            // and reset the Assert Timer (AT(S,G,I)) to Assert_Time.
            EV_DEBUG << "Received inferior assert metrics, stay in I_WON_ASSERT state.\n";
            sendAssertPacket(route->source, route->group, route->metric, incomingInterface->ie);
            restartTimer(incomingInterface->assertTimer, assertTime);
        }
    }
}

// ---- handle signals ----

/**
 * The method process notification about new multicast data stream. It goes through all PIM
 * interfaces and tests them if they can be added to the list of outgoing interfaces. If there
 * is no interface on the list at the end, the router will prune from the multicast tree.
 */
void PimDm::unroutableMulticastPacketArrived(Ipv4Address source, Ipv4Address group, unsigned short ttl)
{
    ASSERT(!source.isUnspecified());
    ASSERT(group.isMulticast());

    EV_DETAIL << "New multicast source observed: source=" << source << ", group=" << group << ".\n";

    Ipv4Route *routeToSrc = rt->findBestMatchingRoute(source);
    if (!routeToSrc || !routeToSrc->getInterface()) {
        EV << "ERROR: PimDm::newMulticast(): cannot find RPF interface, routing information is missing.";
        return;
    }

    PimInterface *rpfInterface = pimIft->getInterfaceById(routeToSrc->getInterface()->getInterfaceId());
    if (!rpfInterface || rpfInterface->getMode() != PimInterface::DenseMode)
        return;

    // gateway is unspecified for directly connected destinations
    bool isSourceDirectlyConnected = routeToSrc->getSourceType() == Ipv4Route::IFACENETMASK;
    Ipv4Address rpfNeighbor = routeToSrc->getGateway().isUnspecified() ? source : routeToSrc->getGateway();

    Route *route = new Route(this, source, group);
    routes[SourceAndGroup(source, group)] = route;
    route->metric = AssertMetric(routeToSrc->getAdminDist(), routeToSrc->getMetric(), Ipv4Address::UNSPECIFIED_ADDRESS);
    route->upstreamInterface = new UpstreamInterface(route, rpfInterface->getInterfacePtr(), rpfNeighbor, isSourceDirectlyConnected);

    // if the source is directly connected, then go to the Originator state
    if (isSourceDirectlyConnected) {
        route->upstreamInterface->originatorState = UpstreamInterface::ORIGINATOR;
        if (rpfInterface->getSR())
            route->upstreamInterface->startStateRefreshTimer();
        route->upstreamInterface->startSourceActiveTimer();
        route->upstreamInterface->maxTtlSeen = ttl;
    }

    bool allDownstreamInterfacesArePruned = true;

    // insert all PIM interfaces except rpf int
    for (int i = 0; i < pimIft->getNumInterfaces(); i++) {
        PimInterface *pimInterface = pimIft->getInterface(i);

        //check if PIM-DM interface and it is not RPF interface
        if (pimInterface == rpfInterface || pimInterface->getMode() != PimInterface::DenseMode) // XXX original code added downstream if data for PIM-SM interfaces too
            continue;

        bool hasPIMNeighbors = pimNbt->getNumNeighbors(pimInterface->getInterfaceId()) > 0;
        bool hasConnectedReceivers = pimInterface->getInterfacePtr()->getProtocolData<Ipv4InterfaceData>()->hasMulticastListener(group);

        // if there are neighbors on interface, we will forward
        if (hasPIMNeighbors || hasConnectedReceivers) {
            // create new outgoing interface
            DownstreamInterface *downstream = route->createDownstreamInterface(pimInterface->getInterfacePtr());
            downstream->setHasConnectedReceivers(hasConnectedReceivers);
            allDownstreamInterfacesArePruned = false;
        }
    }

    // if there is no outgoing interface, prune from multicast tree
    if (allDownstreamInterfacesArePruned) {
        EV_DETAIL << "There is no outgoing interface for multicast, will send Prune message to upstream.\n";
        route->upstreamInterface->graftPruneState = UpstreamInterface::PRUNED;

        // Prune message is sent from the forwarding hook (ipv4DataOnRpfSignal), see multicastPacketArrivedOnRpfInterface()
    }

    // create new multicast route
    Ipv4MulticastRoute *newRoute = new Ipv4MulticastRoute();
    newRoute->setOrigin(source);
    newRoute->setOriginNetmask(Ipv4Address::ALLONES_ADDRESS);
    newRoute->setMulticastGroup(group);
    newRoute->setSourceType(IMulticastRoute::PIM_DM);
    newRoute->setSource(this);
    newRoute->setInInterface(new IMulticastRoute::InInterface(route->upstreamInterface->ie));
    for (auto & elem : route->downstreamInterfaces) {
        DownstreamInterface *downstream = elem;
        newRoute->addOutInterface(new PimDmOutInterface(downstream->ie, downstream));
    }

    rt->addMulticastRoute(newRoute);
    EV_DETAIL << "New route was added to the multicast routing table.\n";
}

/*
 * The method process notification about new multicast groups aasigned to interface. For each
 * new address it tries to find route. If there is route, it finds interface in list of outgoing
 * interfaces. If the interface is not in the list it will be added. if the router was pruned
 * from multicast tree, join again.
 */
void PimDm::multicastReceiverAdded(InterfaceEntry *ie, Ipv4Address group)
{
    EV_DETAIL << "Multicast receiver added for group " << group << ".\n";

    for (int i = 0; i < rt->getNumMulticastRoutes(); i++) {
        Ipv4MulticastRoute *ipv4Route = rt->getMulticastRoute(i);

        // check group
        if (ipv4Route->getSource() != this || ipv4Route->getMulticastGroup() != group)
            continue;

        Route *route = findRoute(ipv4Route->getOrigin(), group);
        ASSERT(route);

        // check on RPF interface
        UpstreamInterface *upstream = route->upstreamInterface;
        if (upstream->ie == ie)
            continue;

        // is interface in list of outgoing interfaces?
        DownstreamInterface *downstream = route->findDownstreamInterfaceByInterfaceId(ie->getInterfaceId());
        if (downstream) {
            EV << "Interface is already on list of outgoing interfaces" << endl;
            if (downstream->pruneState == DownstreamInterface::PRUNED)
                downstream->pruneState = DownstreamInterface::NO_INFO;
        }
        else {
            // create new downstream data
            EV << "Interface is not on list of outgoing interfaces yet, it will be added" << endl;
            downstream = route->createDownstreamInterface(ie);
            ipv4Route->addOutInterface(new PimDmOutInterface(ie, downstream));
        }

        downstream->setHasConnectedReceivers(true);

        // fire upstream state machine event
        if (upstream->graftPruneState == UpstreamInterface::PRUNED && downstream->isInOlist())
            processOlistNonEmptyEvent(route);
    }
}

/**
 * The method process notification about multicast groups removed from interface. For each
 * old address it tries to find route. If there is route, it finds interface in list of outgoing
 * interfaces. If the interface is in the list it will be removed. If the router was not pruned
 * and there is no outgoing interface, the router will prune from the multicast tree.
 */
void PimDm::multicastReceiverRemoved(InterfaceEntry *ie, Ipv4Address group)
{
    EV_DETAIL << "No more receiver for group " << group << " on interface '" << ie->getInterfaceName() << "'.\n";

    // delete pimInt from outgoing interfaces of multicast routes for group
    for (int i = 0; i < rt->getNumMulticastRoutes(); i++) {
        Ipv4MulticastRoute *ipv4Route = rt->getMulticastRoute(i);
        if (ipv4Route->getSource() == this && ipv4Route->getMulticastGroup() == group) {
            Route *route = findRoute(ipv4Route->getOrigin(), group);
            ASSERT(route);

            // remove pimInt from the list of outgoing interfaces
            DownstreamInterface *downstream = route->findDownstreamInterfaceByInterfaceId(ie->getInterfaceId());
            if (downstream) {
                bool wasInOlist = downstream->isInOlist();
                downstream->setHasConnectedReceivers(false);
                if (wasInOlist && !downstream->isInOlist()) {
                    EV_DEBUG << "Removed interface '" << ie->getInterfaceName() << "' from the outgoing interface list of route " << route << ".\n";

                    // fire upstream state machine event
                    if (route->isOilistNull())
                        processOlistEmptyEvent(route);
                }
            }
        }
    }
}

/**
 * The method has to solve the problem when multicast data appears on non-RPF interface. It can
 * happen when there is loop in the network. In this case, router has to prune from the neighbor,
 * so it sends Prune message.
 */
void PimDm::multicastPacketArrivedOnNonRpfInterface(Ipv4Address group, Ipv4Address source, int interfaceId)
{
    EV_DETAIL << "Received multicast datagram (source=" << source << ", group=" << group << ") on non-RPF interface: " << interfaceId << ".\n";

    Route *route = findRoute(source, group);
    ASSERT(route);

    UpstreamInterface *upstream = route->upstreamInterface;
    DownstreamInterface *downstream = route->findDownstreamInterfaceByInterfaceId(interfaceId);
    if (!downstream)
        return;

    // in case of p2p link, send prune
    // FIXME There should be better indicator of P2P link
    if (pimNbt->getNumNeighbors(interfaceId) == 1) {
        // send Prune msg to the neighbor who sent these multicast data
        Ipv4Address nextHop = (pimNbt->getNeighbor(interfaceId, 0))->getAddress();
        sendPrunePacket(nextHop, source, group, pruneInterval, interfaceId);

        // the incoming interface has to change its state to Pruned
        if (downstream->pruneState == DownstreamInterface::NO_INFO) {
            downstream->pruneState = DownstreamInterface::PRUNED;
            downstream->startPruneTimer(pruneInterval);

            // if there is no outgoing interface, Prune msg has to be sent on upstream
            if (route->isOilistNull()) {
                EV << "Route is not forwarding any more, send Prune to upstream" << endl;
                upstream->graftPruneState = UpstreamInterface::PRUNED;
                if (!upstream->isSourceDirectlyConnected()) {
                    sendPrunePacket(upstream->rpfNeighbor(), source, group, pruneInterval, upstream->getInterfaceId());
                }
            }
        }
        return;
    }

    //
    // Assert State Machine; event: An (S,G) data packet arrives on downstream interface I
    //
    if (downstream->assertState == DownstreamInterface::NO_ASSERT_INFO) {
        // An (S,G) data packet arrived on a downstream interface.  It is
        // optimistically assumed that this router will be the Assert winner
        // for this (S,G).  The Assert state machine MUST transition to the
        // "I am Assert Winner" state, send an Assert(S,G) to interface I,
        // store its own address and metric as the Assert Winner, and set
        // the Assert_Timer (AT(S,G,I) to Assert_Time, thereby initiating
        // the Assert negotiation for (S,G).
        downstream->assertState = DownstreamInterface::I_WON_ASSERT;
        downstream->winnerMetric = route->metric.setAddress(downstream->ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
        sendAssertPacket(source, group, route->metric, downstream->ie);
        downstream->startAssertTimer(assertTime);
    }
    else if (downstream->assertState == DownstreamInterface::I_WON_ASSERT) {
        // An (S,G) data packet arrived on a downstream interface.  The
        // Assert state machine remains in the "I am Assert Winner" state.
        // The router MUST send an Assert(S,G) to interface I and set the
        // Assert Timer (AT(S,G,I) to Assert_Time.
        sendAssertPacket(source, group, route->metric, downstream->ie);
        restartTimer(downstream->assertTimer, assertTime);
    }
}

void PimDm::multicastPacketArrivedOnRpfInterface(int interfaceId, Ipv4Address group, Ipv4Address source, unsigned short ttl)
{
    EV_DETAIL << "Multicast datagram arrived: source=" << source << ", group=" << group << ".\n";

    Route *route = findRoute(source, group);
    ASSERT(route);
    UpstreamInterface *upstream = route->upstreamInterface;

    // RFC 3973 4.5.2.2
    //
    // Receive Data Packet from S addressed to G
    //      The router remains in the Originator (O) state and MUST reset
    //      SAT(S,G) to SourceLifetime.  The router SHOULD increase its
    //      recorded TTL to match the TTL of the packet, if the packet's TTL
    //      is larger than the previously recorded TTL.  A router MAY record
    //      the TTL based on an implementation specific sampling policy to
    //      avoid examining the TTL of every multicast packet it handles.

    // Is source directly connected?
    if (upstream->isSourceDirectlyConnected()) {
        // State Refresh Originator state machine event: Receive Data from S AND S directly connected
        if (upstream->originatorState == UpstreamInterface::NOT_ORIGINATOR) {
            upstream->originatorState = UpstreamInterface::ORIGINATOR;
            PimInterface *pimInterface = pimIft->getInterfaceById(upstream->ie->getInterfaceId());
            if (pimInterface && pimInterface->getSR())
                upstream->startStateRefreshTimer();
        }
        restartTimer(upstream->sourceActiveTimer, sourceActiveInterval);

        // record max TTL seen, it is used in StateRefresh messages
        upstream->maxTtlSeen = std::max(upstream->maxTtlSeen, ttl);
    }

    // upstream state transition

    // Data Packet arrives on RPF_Interface(S) AND olist(S,G) == nullptr AND S is NOT directly connected ?
    if (upstream->ie->getInterfaceId() == interfaceId && route->isOilistNull() && !upstream->isSourceDirectlyConnected()) {
        EV_DETAIL << "Route does not have any outgoing interface and source is not directly connected.\n";

        switch (upstream->graftPruneState) {
            case UpstreamInterface::FORWARDING:
                // The Upstream(S,G) state machine MUST transition to the Pruned (P)
                // state, send a Prune(S,G) to RPF'(S), and set PLT(S,G) to t_limit seconds.
                sendPrunePacket(upstream->rpfNeighbor(), source, group, pruneInterval, upstream->getInterfaceId());
                upstream->startPruneLimitTimer();
                upstream->graftPruneState = UpstreamInterface::PRUNED;
                break;

            case UpstreamInterface::PRUNED:
                // Either another router on the LAN desires traffic from S addressed
                // to G or a previous Prune was lost.  To prevent generating a
                // Prune(S,G) in response to every data packet, the PruneLimit Timer
                // (PLT(S,G)) is used.  Once the PLT(S,G) expires, the router needs
                // to send another prune in response to a data packet not received
                // directly from the source.  A Prune(S,G) MUST be sent to RPF'(S),
                // and the PLT(S,G) MUST be set to t_limit.
                //
                // if GRT is running now, do not send Prune msg
                if (!upstream->isPruneLimitTimerRunning()) {
                    sendPrunePacket(upstream->rpfNeighbor(), source, group, pruneInterval, upstream->getInterfaceId());
                    upstream->startPruneLimitTimer();
                }
                break;

            case UpstreamInterface::ACK_PENDING:
                break;
        }
    }
}

/**
 * The method process notification about interface change. Multicast routing table will be
 * changed if RPF interface has changed. New RPF interface is set to route and is removed
 * from outgoing interfaces. On the other hand, old RPF interface is added to outgoing
 * interfaces. If route was not pruned, the router has to join to the multicast tree again
 * (by different path).
 */
// XXX Assert state causes RPF'(S) to change
void PimDm::rpfInterfaceHasChanged(Ipv4MulticastRoute *ipv4Route, Ipv4Route *routeToSource)
{
    InterfaceEntry *newRpf = routeToSource->getInterface();
    Ipv4Address source = ipv4Route->getOrigin();
    Ipv4Address group = ipv4Route->getMulticastGroup();
    int rpfId = newRpf->getInterfaceId();

    EV_DETAIL << "New RPF interface for group=" << group << " source=" << source << " is " << newRpf->getInterfaceName() << endl;

    Route *route = findRoute(source, group);
    ASSERT(route);

    // delete old upstream interface data
    UpstreamInterface *oldUpstreamInterface = route->upstreamInterface;
    InterfaceEntry *oldRpfInterface = oldUpstreamInterface ? oldUpstreamInterface->ie : nullptr;
    delete oldUpstreamInterface;
    delete ipv4Route->getInInterface();
    ipv4Route->setInInterface(nullptr);

    // set new upstream interface data
    bool isSourceDirectlyConnected = routeToSource->getSourceType() == Ipv4Route::IFACENETMASK;
    Ipv4Address newRpfNeighbor = pimNbt->getNeighbor(rpfId, 0)->getAddress();    // XXX what happens if no neighbors?
    UpstreamInterface *upstream = route->upstreamInterface = new UpstreamInterface(route, newRpf, newRpfNeighbor, isSourceDirectlyConnected);
    ipv4Route->setInInterface(new IMulticastRoute::InInterface(newRpf));

    // delete rpf interface from the downstream interfaces
    DownstreamInterface *oldDownstreamInterface = route->removeDownstreamInterface(newRpf->getInterfaceId());
    if (oldDownstreamInterface) {
        ipv4Route->removeOutInterface(newRpf);    // will delete downstream data, XXX method should be called deleteOutInterface()
        delete oldDownstreamInterface;
    }

    // old RPF interface should be now a downstream interface if it is not down
    if (oldRpfInterface && oldRpfInterface->isUp()) {
        DownstreamInterface *downstream = route->createDownstreamInterface(oldRpfInterface);
        ipv4Route->addOutInterface(new PimDmOutInterface(oldRpfInterface, downstream));
    }

    bool isOlistNull = route->isOilistNull();

    // upstream state transitions

    // RPF'(S) Changes AND olist(S,G) != nullptr AND S is NOT directly connected?
    if (!isOlistNull && !upstream->isSourceDirectlyConnected()) {
        // The Upstream(S,G) state
        // machine MUST transition to the AckPending (AP) state, unicast a
        // Graft to the new RPF'(S), and set the GraftRetry Timer (GRT(S,G))
        // to Graft_Retry_Period.

        if (upstream->graftPruneState == UpstreamInterface::PRUNED)
            upstream->stopPruneLimitTimer();

        // route was not pruned, join to the multicast tree again
        sendGraftPacket(upstream->rpfNeighbor(), source, group, rpfId);
        upstream->startGraftRetryTimer();
        upstream->graftPruneState = UpstreamInterface::ACK_PENDING;
    }
    // RPF'(S) Changes AND olist(S,G) == nullptr
    else if (isOlistNull) {
        if (upstream->graftPruneState == UpstreamInterface::PRUNED) {
            upstream->stopPruneLimitTimer();
        }
        else if (upstream->graftPruneState == UpstreamInterface::ACK_PENDING) {
            cancelAndDelete(upstream->graftRetryTimer);
            upstream->graftRetryTimer = nullptr;
        }

        upstream->graftPruneState = UpstreamInterface::PRUNED;
    }
}

// ---- Sending PIM Messages ----

void PimDm::sendPrunePacket(Ipv4Address nextHop, Ipv4Address src, Ipv4Address grp, int holdTime, int intId)
{
    ASSERT(!src.isUnspecified());
    ASSERT(grp.isMulticast());

    EV_INFO << "Sending Prune(S=" << src << ", G=" << grp << ") message to neighbor '" << nextHop << "' on interface '" << intId << "'\n";

    Packet *packet = new Packet("PIMPrune");
    const auto& msg = makeShared<PimJoinPrune>();
    msg->getUpstreamNeighborAddressForUpdate().unicastAddress = nextHop;
    msg->setHoldTime(holdTime);

    // set multicast groups
    msg->setJoinPruneGroupsArraySize(1);
    JoinPruneGroup& group = msg->getJoinPruneGroupsForUpdate(0);
    group.getGroupAddressForUpdate().groupAddress = grp;
    group.setPrunedSourceAddressArraySize(1);
    auto& address = group.getPrunedSourceAddressForUpdate(0);
    address.sourceAddress = src;

    msg->setChunkLength(PIM_HEADER_LENGTH + ENCODED_UNICODE_ADDRESS_LENGTH + B(4) + ENCODED_GROUP_ADDRESS_LENGTH + B(4) + ENCODED_SOURCE_ADDRESS_LENGTH);
    msg->setCrcMode(pimModule->getCrcMode());
    Pim::insertCrc(msg);
    packet->insertAtFront(msg);

    emit(sentJoinPrunePkSignal, packet);

    sendToIP(packet, Ipv4Address::UNSPECIFIED_ADDRESS, ALL_PIM_ROUTERS_MCAST, intId);
}

void PimDm::sendJoinPacket(Ipv4Address nextHop, Ipv4Address src, Ipv4Address grp, int intId)
{
    ASSERT(!src.isUnspecified());
    ASSERT(grp.isMulticast());

    EV_INFO << "Sending Join(S=" << src << ", G=" << grp << ") message to neighbor '" << nextHop << "' on interface '" << intId << "'\n";

    Packet *packet = new Packet("PIMJoin");
    const auto& msg = makeShared<PimJoinPrune>();
    msg->getUpstreamNeighborAddressForUpdate().unicastAddress = nextHop;
    msg->setHoldTime(0);    // ignored by the receiver

    // set multicast groups
    msg->setJoinPruneGroupsArraySize(1);
    JoinPruneGroup& group = msg->getJoinPruneGroupsForUpdate(0);
    group.getGroupAddressForUpdate().groupAddress = grp;
    group.setJoinedSourceAddressArraySize(1);
    auto& address = group.getJoinedSourceAddressForUpdate(0);
    address.sourceAddress = src;

    msg->setChunkLength(PIM_HEADER_LENGTH + ENCODED_UNICODE_ADDRESS_LENGTH + B(4) + ENCODED_GROUP_ADDRESS_LENGTH + B(4) + ENCODED_SOURCE_ADDRESS_LENGTH);
    msg->setCrcMode(pimModule->getCrcMode());
    Pim::insertCrc(msg);
    packet->insertAtFront(msg);

    emit(sentJoinPrunePkSignal, packet);

    sendToIP(packet, Ipv4Address::UNSPECIFIED_ADDRESS, ALL_PIM_ROUTERS_MCAST, intId);
}

/*
 * PIM Graft messages use the same format as Join/Prune messages, except
 * that the Type field is set to 6.  The source address MUST be in the
 * Join section of the message.  The Hold Time field SHOULD be zero and
 * SHOULD be ignored when a Graft is received.
 */
void PimDm::sendGraftPacket(Ipv4Address nextHop, Ipv4Address src, Ipv4Address grp, int intId)
{
    EV_INFO << "Sending Graft(S=" << src << ", G=" << grp << ") message to neighbor '" << nextHop << "' on interface '" << intId << "'\n";

    Packet *packet = new Packet("PimGraft");
    const auto& msg = makeShared<PimGraft>();
    msg->setHoldTime(0);
    msg->getUpstreamNeighborAddressForUpdate().unicastAddress = nextHop;

    msg->setJoinPruneGroupsArraySize(1);
    JoinPruneGroup& group = msg->getJoinPruneGroupsForUpdate(0);
    group.getGroupAddressForUpdate().groupAddress = grp;
    group.setJoinedSourceAddressArraySize(1);
    auto& address = group.getJoinedSourceAddressForUpdate(0);
    address.sourceAddress = src;

    msg->setChunkLength(PIM_HEADER_LENGTH + ENCODED_UNICODE_ADDRESS_LENGTH + B(4) + ENCODED_GROUP_ADDRESS_LENGTH + B(4) + ENCODED_SOURCE_ADDRESS_LENGTH);
    msg->setCrcMode(pimModule->getCrcMode());
    Pim::insertCrc(msg);
    packet->insertAtFront(msg);

    emit(sentGraftPkSignal, packet);

    sendToIP(packet, Ipv4Address::UNSPECIFIED_ADDRESS, nextHop, intId);
}

/*
 * PIM Graft Ack messages are identical in format to the received Graft
 * message, except that the Type field is set to 7.  The Upstream
 * Neighbor Address field SHOULD be set to the sender of the Graft
 * message and SHOULD be ignored upon receipt.
 */
void PimDm::sendGraftAckPacket(Packet *pk, const Ptr<const PimGraft>& graftPacket)
{
    EV_INFO << "Sending GraftAck message.\n";

    auto ifTag = pk->getTag<InterfaceInd>();
    auto addressInd = pk->getTag<L3AddressInd>();
    Ipv4Address destAddr = addressInd->getSrcAddress().toIpv4();
    Ipv4Address srcAddr = addressInd->getDestAddress().toIpv4();
    int outInterfaceId = ifTag->getInterfaceId();

    Packet *packet = new Packet("PIMGraftAck");
    auto msg = dynamicPtrCast<PimGraft>(graftPacket->dupShared());
    msg->setType(GraftAck);
    msg->setCrcMode(pimModule->getCrcMode());
    Pim::insertCrc(msg);
    packet->insertAtFront(msg);

    emit(sentGraftAckPkSignal, packet);

    sendToIP(packet, srcAddr, destAddr, outInterfaceId);
}

void PimDm::sendStateRefreshPacket(Ipv4Address originator, Route *route, DownstreamInterface *downstream, unsigned short ttl)
{
    EV_INFO << "Sending StateRefresh(S=" << route->source << ", G=" << route->group
            << ") message on interface '" << downstream->ie->getInterfaceName() << "'\n";

    Packet *packet = new Packet("PimStateRefresh");
    const auto& msg = makeShared<PimStateRefresh>();
    msg->getGroupAddressForUpdate().groupAddress = route->group;
    msg->getSourceAddressForUpdate().unicastAddress = route->source;
    msg->getOriginatorAddressForUpdate().unicastAddress = originator;
    msg->setInterval(stateRefreshInterval);
    msg->setTtl(ttl);
    msg->setP(downstream->pruneState == DownstreamInterface::PRUNED);
    // TODO set metric

    msg->setChunkLength(PIM_HEADER_LENGTH
            + ENCODED_GROUP_ADDRESS_LENGTH
            + ENCODED_UNICODE_ADDRESS_LENGTH
            + ENCODED_UNICODE_ADDRESS_LENGTH
            + B(12));
    msg->setCrcMode(pimModule->getCrcMode());
    Pim::insertCrc(msg);
    packet->insertAtFront(msg);

    emit(sentStateRefreshPkSignal, packet);

    sendToIP(packet, Ipv4Address::UNSPECIFIED_ADDRESS, ALL_PIM_ROUTERS_MCAST, downstream->ie->getInterfaceId());
}

void PimDm::sendAssertPacket(Ipv4Address source, Ipv4Address group, AssertMetric metric, InterfaceEntry *ie)
{
    EV_INFO << "Sending Assert(S= " << source << ", G= " << group << ") message on interface '" << ie->getInterfaceName() << "'\n";

    Packet *packet = new Packet("PimAssert");
    const auto& pkt = makeShared<PimAssert>();
    pkt->getGroupAddressForUpdate().groupAddress = group;
    pkt->getSourceAddressForUpdate().unicastAddress = source;
    pkt->setR(false);
    pkt->setMetricPreference(metric.preference);
    pkt->setMetric(metric.metric);

    pkt->setChunkLength(PIM_HEADER_LENGTH
            + ENCODED_GROUP_ADDRESS_LENGTH
            + ENCODED_UNICODE_ADDRESS_LENGTH
            + B(8));
    pkt->setCrcMode(pimModule->getCrcMode());
    Pim::insertCrc(pkt);
    packet->insertAtFront(pkt);

    emit(sentAssertPkSignal, packet);

    sendToIP(packet, Ipv4Address::UNSPECIFIED_ADDRESS, ALL_PIM_ROUTERS_MCAST, ie->getInterfaceId());
}

void PimDm::sendToIP(Packet *packet, Ipv4Address srcAddr, Ipv4Address destAddr, int outInterfaceId)
{
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::pim);
    packet->addTagIfAbsent<DispatchProtocolInd>()->setProtocol(&Protocol::pim);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
    packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(outInterfaceId);
    auto addresses = packet->addTagIfAbsent<L3AddressReq>();
    addresses->setSrcAddress(srcAddr);
    addresses->setDestAddress(destAddr);
    packet->addTagIfAbsent<HopLimitReq>()->setHopLimit(1);
    send(packet, "ipOut");
}

//----------------------------------------------------------------------------
//           Helpers
//----------------------------------------------------------------------------

// The set of interfaces defined by the olist(S,G) macro becomes
// null, indicating that traffic from S addressed to group G should
// no longer be forwarded.
void PimDm::processOlistEmptyEvent(Route *route)
{
    UpstreamInterface *upstream = route->upstreamInterface;

    // upstream state transitions

    // olist(S,G) -> nullptr AND S NOT directly connected?
    if (!upstream->isSourceDirectlyConnected()) {
        if (upstream->graftPruneState == UpstreamInterface::FORWARDING || upstream->graftPruneState == UpstreamInterface::ACK_PENDING) {
            // The Upstream(S,G) state machine MUST
            // transition to the Pruned (P) state.  A Prune(S,G) MUST be
            // multicast to the RPF_interface(S), with RPF'(S) named in the
            // upstream neighbor field.  The GraftRetry Timer (GRT(S,G)) MUST be
            // cancelled, and PLT(S,G) MUST be set to t_limit seconds.
            sendPrunePacket(upstream->rpfNeighbor(), route->source, route->group, pruneInterval, upstream->getInterfaceId());
            upstream->startPruneLimitTimer();
            if (upstream->graftPruneState == UpstreamInterface::ACK_PENDING) {
                cancelAndDelete(upstream->graftRetryTimer);
                upstream->graftRetryTimer = nullptr;
            }
        }
    }

    upstream->graftPruneState = UpstreamInterface::PRUNED;
}

void PimDm::processOlistNonEmptyEvent(Route *route)
{
    // upstream state transition: Pruned->AckPending if olist is not empty
    // if all route was pruned, remove prune flag
    // if upstrem is not source, send Graft message
    UpstreamInterface *upstream = route->upstreamInterface;
    if (upstream->graftPruneState == UpstreamInterface::PRUNED) {
        // olist(S,G)->non-nullptr AND S NOT directly connected
        //
        // The set of interfaces defined by the olist(S,G) macro becomes
        // non-empty, indicating that traffic from S addressed to group G
        // must be forwarded.  The Upstream(S,G) state machine MUST cancel
        // PLT(S,G), transition to the AckPending (AP) state and unicast a
        // Graft message to RPF'(S).  The Graft Retry Timer (GRT(S,G)) MUST
        // be set to Graft_Retry_Period.

        if (!upstream->isSourceDirectlyConnected()) {
            EV << "Route is not pruned any more, send Graft to upstream" << endl;
            sendGraftPacket(upstream->rpfNeighbor(), route->source, route->group, upstream->getInterfaceId());
            upstream->stopPruneLimitTimer();
            upstream->startGraftRetryTimer();
            upstream->graftPruneState = UpstreamInterface::ACK_PENDING;
        }
        else {
            upstream->graftPruneState = UpstreamInterface::FORWARDING;
        }
    }
}

void PimDm::restartTimer(cMessage *timer, double interval)
{
    cancelEvent(timer);
    scheduleAfter(interval, timer);
}

void PimDm::cancelAndDeleteTimer(cMessage *& timer)
{
    cancelAndDelete(timer);
    timer = nullptr;
}

PimInterface *PimDm::getIncomingInterface(InterfaceEntry *fromIE)
{
    if (fromIE)
        return pimIft->getInterfaceById(fromIE->getInterfaceId());
    return nullptr;
}

Ipv4MulticastRoute *PimDm::findIpv4MulticastRoute(Ipv4Address group, Ipv4Address source)
{
    int numRoutes = rt->getNumMulticastRoutes();
    for (int i = 0; i < numRoutes; i++) {
        Ipv4MulticastRoute *route = rt->getMulticastRoute(i);
        if (route->getSource() == this && route->getMulticastGroup() == group && route->getOrigin() == source)
            return route;
    }
    return nullptr;
}

PimDm::Route *PimDm::findRoute(Ipv4Address source, Ipv4Address group)
{
    auto it = routes.find(SourceAndGroup(source, group));
    return it != routes.end() ? it->second : nullptr;
}

void PimDm::deleteRoute(Ipv4Address source, Ipv4Address group)
{
    auto it = routes.find(SourceAndGroup(source, group));
    if (it != routes.end()) {
        delete it->second;
        routes.erase(it);
    }
}

void PimDm::clearRoutes()
{
    // delete Ipv4 routes
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < rt->getNumMulticastRoutes(); i++) {
            Ipv4MulticastRoute *ipv4Route = rt->getMulticastRoute(i);
            if (ipv4Route->getSource() == this) {
                rt->deleteMulticastRoute(ipv4Route);
                changed = true;
                break;
            }
        }
    }

    // clear local table
    for (auto & elem : routes)
        delete elem.second;
    routes.clear();
}

PimDm::Route::~Route()
{
    delete upstreamInterface;
    for (auto & elem : downstreamInterfaces)
        delete elem;
    downstreamInterfaces.clear();
}

PimDm::DownstreamInterface *PimDm::Route::findDownstreamInterfaceByInterfaceId(int interfaceId) const
{
    for (auto & elem : downstreamInterfaces)
        if (elem->ie->getInterfaceId() == interfaceId)
            return elem;

    return nullptr;
}

PimDm::DownstreamInterface *PimDm::Route::createDownstreamInterface(InterfaceEntry *ie)
{
    DownstreamInterface *downstream = new DownstreamInterface(this, ie);
    downstreamInterfaces.push_back(downstream);
    return downstream;
}

PimDm::DownstreamInterface *PimDm::Route::removeDownstreamInterface(int interfaceId)
{
    for (auto it = downstreamInterfaces.begin(); it != downstreamInterfaces.end(); ++it) {
        DownstreamInterface *downstream = *it;
        if (downstream->ie->getInterfaceId() == interfaceId) {
            downstreamInterfaces.erase(it);
            return downstream;
        }
    }
    return nullptr;
}

bool PimDm::Route::isOilistNull()
{
    for (auto & elem : downstreamInterfaces) {
        if (elem->isInOlist())
            return false;
    }
    return true;
}

PimDm::UpstreamInterface::~UpstreamInterface()
{
    owner->owner->cancelAndDelete(stateRefreshTimer);
    owner->owner->cancelAndDelete(graftRetryTimer);
    owner->owner->cancelAndDelete(sourceActiveTimer);
    owner->owner->cancelAndDelete(overrideTimer);
}

/**
 * The method is used to create PIMGraftRetry timer. The timer is set when router wants to join to
 * multicast tree again and send PIM Prune message to upstream. Router waits for Graft Retry Timer
 * (3 s) for PIM PruneAck message from upstream. If timer expires, router will send PIM Prune message
 * again. It is set to (S,G).
 */
void PimDm::UpstreamInterface::startGraftRetryTimer()
{
    graftRetryTimer = new cMessage("PIMGraftRetryTimer", GraftRetryTimer);
    graftRetryTimer->setContextPointer(this);
    pimdm()->scheduleAfter(pimdm()->graftRetryInterval, graftRetryTimer);
}

void PimDm::UpstreamInterface::startOverrideTimer()
{
    overrideTimer = new cMessage("PIMOverrideTimer", UpstreamOverrideTimer);
    overrideTimer->setContextPointer(this);
    pimdm()->scheduleAfter(pimdm()->overrideInterval, overrideTimer);
}

/**
 * The method is used to create PIMSourceActive timer. The timer is set when source of multicast is
 * connected directly to the router.  If timer expires, router will remove the route from multicast
 * routing table. It is set to (S,G).
 */
void PimDm::UpstreamInterface::startSourceActiveTimer()
{
    sourceActiveTimer = new cMessage("PIMSourceActiveTimer", SourceActiveTimer);
    sourceActiveTimer->setContextPointer(this);
    pimdm()->scheduleAfter(pimdm()->sourceActiveInterval, sourceActiveTimer);
}

/**
 * The method is used to create PimStateRefresh timer. The timer is set when source of multicast is
 * connected directly to the router. If timer expires, router will send StateRefresh message, which
 * will propagate through all network and wil reset Prune Timer. It is set to (S,G).
 */
void PimDm::UpstreamInterface::startStateRefreshTimer()
{
    stateRefreshTimer = new cMessage("PIMStateRefreshTimer", StateRefreshTimer);
    sourceActiveTimer->setContextPointer(this);
    pimdm()->scheduleAfter(pimdm()->stateRefreshInterval, stateRefreshTimer);
}

PimDm::DownstreamInterface::~DownstreamInterface()
{
    owner->owner->cancelAndDelete(pruneTimer);
    owner->owner->cancelAndDelete(prunePendingTimer);
}

/**
 * The method is used to create PIMPrune timer. The timer is set when outgoing interface
 * goes to pruned state. After expiration (usually 3min) interface goes back to forwarding
 * state. It is set to (S,G,I).
 */
void PimDm::DownstreamInterface::startPruneTimer(double holdTime)
{
    cMessage *timer = new cMessage("PimPruneTimer", PruneTimer);
    timer->setContextPointer(this);
    owner->owner->scheduleAfter(holdTime, timer);
    pruneTimer = timer;
}

void PimDm::DownstreamInterface::stopPruneTimer()
{
    if (pruneTimer) {
        if (pruneTimer->isScheduled())
            owner->owner->cancelEvent(pruneTimer);
        delete pruneTimer;
        pruneTimer = nullptr;
    }
}

void PimDm::DownstreamInterface::startPrunePendingTimer(double overrideInterval)
{
    ASSERT(!prunePendingTimer);
    cMessage *timer = new cMessage("PimPrunePendingTimer", PrunePendingTimer);
    timer->setContextPointer(this);
    owner->owner->scheduleAfter(overrideInterval, timer);
    prunePendingTimer = timer;
}

void PimDm::DownstreamInterface::stopPrunePendingTimer()
{
    if (prunePendingTimer) {
        if (prunePendingTimer->isScheduled())
            owner->owner->cancelEvent(prunePendingTimer);
        delete prunePendingTimer;
        prunePendingTimer = nullptr;
    }
}

//
// olist(S,G) = immediate_olist(S,G) (-) RPF_interface(S)
//
// immediate_olist(S,G) = pim_nbrs (-) prunes(S,G) (+)
//                        (pim_include(*,G) (-) pim_exclude(S,G) ) (+)
//                        pim_include(S,G) (-) lost_assert(S,G) (-)
//                        boundary(G)
//
// pim_nbrs is the set of all interfaces on which the router has at least one active PIM neighbor.
//
// prunes(S,G) = {all interfaces I such that DownstreamPState(S,G,I) is in Pruned state}
//
// lost_assert(S,G) = {all interfaces I such that lost_assert(S,G,I) == TRUE}
//
// bool lost_assert(S,G,I) {
//   if ( RPF_interface(S) == I ) {
//     return FALSE
//   } else {
//     return (AssertWinner(S,G,I) != me  AND
//            (AssertWinnerMetric(S,G,I) is better than
//             spt_assert_metric(S,G,I)))
//   }
// }
//
// boundary(G) = {all interfaces I with an administratively scoped boundary for group G}
bool PimDm::DownstreamInterface::isInOlist() const
{
    // TODO check boundary
    bool hasNeighbors = pimdm()->pimNbt->getNumNeighbors(ie->getInterfaceId()) > 0;
    return ((hasNeighbors && pruneState != PRUNED) || hasConnectedReceivers()) && assertState != I_LOST_ASSERT;
}

std::ostream& operator<<(std::ostream& out, const PimDm::Route& route)
{
    out << "Upstream interface ";
    if (!route.upstreamInterface)
        out << "(empty) ";
    else {
        out << "(name: " << route.upstreamInterface->ie->getInterfaceName() << " ";
        out << "RpfNeighbor: " << route.upstreamInterface->rpfNeighbor() << " ";
        out << "graftPruneState: " << PimDm::graftPruneStateString(route.upstreamInterface->getGraftPruneState()) << " ";
        auto t1 = route.upstreamInterface->getGraftRetryTimer();
        out << "graftRetryTimer: " << (t1 ? t1->getArrivalTime() : 0) << " ";
        auto t2 = route.upstreamInterface->getOverrideTimer();
        out << "overrideTimer: " << (t2 ? t2->getArrivalTime() : 0) << " ";
        out << "lastPruneSentTime: " << route.upstreamInterface->getLastPruneSentTime() << " ";
        out << "originatorState: " << PimDm::originatorStateString(route.upstreamInterface->getOriginatorState()) << " ";
        auto t3 = route.upstreamInterface->getSourceActiveTimer();
        out << "sourceActiveTimer: " << (t3 ? t3->getArrivalTime() : 0) << " ";
        auto t4 = route.upstreamInterface->getStateRefreshTimer();
        out << "stateRefreshTimer: " << (t4 ? t4->getArrivalTime() : 0) << " ";
        out << "maxTtlSeen: " << route.upstreamInterface->getMaxTtlSeen() << ") ";
    }

    out << "Downstream interfaces ";
    if(route.downstreamInterfaces.empty())
        out << "(empty) ";
    for (unsigned int i = 0; i < route.downstreamInterfaces.size(); ++i) {
        out << "(name: " << route.downstreamInterfaces[i]->ie->getInterfaceName() << " ";
        out << "pruneState: " << PimDm::pruneStateString(route.downstreamInterfaces[i]->getPruneState()) << " ";
        auto t1 = route.downstreamInterfaces[i]->getPruneTimer();
        out << "pruneTimer: " << (t1 ? t1->getArrivalTime() : 0) << " ";
        auto t2 = route.downstreamInterfaces[i]->getPrunePendingTimer();
        out << "prunePendingTimer: " << (t2 ? t2->getArrivalTime() : 0) << ") ";
    }

    return out;

}

const std::string PimDm::graftPruneStateString(UpstreamInterface::GraftPruneState ps)
{
    if(ps == UpstreamInterface::FORWARDING)
        return "FORWARDING";
    else if(ps == UpstreamInterface::PRUNED)
        return "PRUNED";
    else if(ps == UpstreamInterface::ACK_PENDING)
        return "ACK_PENDING";

    return "UNKNOWN";
}

const std::string PimDm::originatorStateString(UpstreamInterface::OriginatorState os)
{
    if(os == UpstreamInterface::NOT_ORIGINATOR)
        return "NOT_ORIGINATOR";
    else if(os == UpstreamInterface::ORIGINATOR)
        return "ORIGINATOR";

    return "UNKNOWN";
}

const std::string PimDm::pruneStateString(DownstreamInterface::PruneState ps)
{
    if(ps == DownstreamInterface::NO_INFO)
        return "NO_INFO";
    else if(ps == DownstreamInterface::PRUNE_PENDING)
        return "PRUNE_PENDING";
    else if(ps == DownstreamInterface::PRUNED)
        return "PRUNED";

    return "UNKNOWN";
}

}    // namespace inet

