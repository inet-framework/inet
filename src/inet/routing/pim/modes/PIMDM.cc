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

#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/common/ModuleAccess.h"
#include "inet/routing/pim/modes/PIMDM.h"

namespace inet {
Define_Module(PIMDM);

using namespace std;

simsignal_t PIMDM::sentGraftPkSignal = registerSignal("sentGraftPk");
simsignal_t PIMDM::rcvdGraftPkSignal = registerSignal("rcvdGraftPk");
simsignal_t PIMDM::sentGraftAckPkSignal = registerSignal("sentGraftAckPk");
simsignal_t PIMDM::rcvdGraftAckPkSignal = registerSignal("rcvdGraftAckPk");
simsignal_t PIMDM::sentJoinPrunePkSignal = registerSignal("sentJoinPrunePk");
simsignal_t PIMDM::rcvdJoinPrunePkSignal = registerSignal("rcvdJoinPrunePk");
simsignal_t PIMDM::sentAssertPkSignal = registerSignal("sentAssertPk");
simsignal_t PIMDM::rcvdAssertPkSignal = registerSignal("rcvdAssertPk");
simsignal_t PIMDM::sentStateRefreshPkSignal = registerSignal("sentStateRefreshPk");
simsignal_t PIMDM::rcvdStateRefreshPkSignal = registerSignal("rcvdStateRefreshPk");

// for logging
ostream& operator<<(ostream& out, const PIMDM::Route *route)
{
    out << "(S=" << route->source << ", G=" << route->group << ")";
    return out;
}

PIMDM::~PIMDM()
{
    for (auto & elem : routes)
        delete elem.second;
    routes.clear();
}

void PIMDM::initialize(int stage)
{
    PIMBase::initialize(stage);

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

bool PIMDM::handleNodeStart(IDoneCallback *doneCallback)
{
    bool done = PIMBase::handleNodeStart(doneCallback);

    // subscribe for notifications
    if (isEnabled) {
        cModule *host = findContainingNode(this);
        if (!host)
            throw cRuntimeError("PIMDM: containing node not found.");
        host->subscribe(NF_IPv4_NEW_MULTICAST, this);
        host->subscribe(NF_IPv4_MCAST_REGISTERED, this);
        host->subscribe(NF_IPv4_MCAST_UNREGISTERED, this);
        host->subscribe(NF_IPv4_DATA_ON_NONRPF, this);
        host->subscribe(NF_IPv4_DATA_ON_RPF, this);
        host->subscribe(NF_ROUTE_ADDED, this);
        host->subscribe(NF_INTERFACE_STATE_CHANGED, this);
    }

    return done;
}

bool PIMDM::handleNodeShutdown(IDoneCallback *doneCallback)
{
    // TODO send PIM Hellos to neighbors with 0 HoldTime
    stopPIMRouting();
    return PIMBase::handleNodeShutdown(doneCallback);
}

void PIMDM::handleNodeCrash()
{
    stopPIMRouting();
    PIMBase::handleNodeCrash();
}

void PIMDM::stopPIMRouting()
{
    if (isEnabled) {
        cModule *host = findContainingNode(this);
        if (!host)
            throw cRuntimeError("PIMDM: containing node not found.");
        host->unsubscribe(NF_IPv4_NEW_MULTICAST, this);
        host->unsubscribe(NF_IPv4_MCAST_REGISTERED, this);
        host->unsubscribe(NF_IPv4_MCAST_UNREGISTERED, this);
        host->unsubscribe(NF_IPv4_DATA_ON_NONRPF, this);
        host->unsubscribe(NF_IPv4_DATA_ON_RPF, this);
        host->unsubscribe(NF_ROUTE_ADDED, this);
        host->unsubscribe(NF_INTERFACE_STATE_CHANGED, this);
    }

    clearRoutes();
}

void PIMDM::handleMessageWhenUp(cMessage *msg)
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
                throw cRuntimeError("PIMDM: unknown self message: %s (%s)", msg->getName(), msg->getClassName());
        }
    }
    else if (dynamic_cast<PIMPacket *>(msg)) {
        if (!isEnabled) {
            EV_DETAIL << "PIM-DM is disabled, dropping packet.\n";
            delete msg;
            return;
        }

        PIMPacket *pkt = static_cast<PIMPacket *>(msg);
        switch (pkt->getType()) {
            case Hello:
                processHelloPacket(check_and_cast<PIMHello *>(pkt));
                break;

            case JoinPrune:
                processJoinPrunePacket(check_and_cast<PIMJoinPrune *>(pkt));
                break;

            case Assert:
                processAssertPacket(check_and_cast<PIMAssert *>(pkt));
                break;

            case Graft:
                processGraftPacket(check_and_cast<PIMGraft *>(pkt));
                break;

            case GraftAck:
                processGraftAckPacket(check_and_cast<PIMGraftAck *>(pkt));
                break;

            case StateRefresh:
                processStateRefreshPacket(check_and_cast<PIMStateRefresh *>(pkt));
                break;

            default:
                EV_WARN << "Dropping packet " << pkt->getName() << ".\n";
                delete pkt;
                break;
        }
    }
    else
        throw cRuntimeError("PIMDM: received unknown message: %s (%s).", msg->getName(), msg->getClassName());
}

void PIMDM::processJoinPrunePacket(PIMJoinPrune *pkt)
{
    EV_INFO << "Received JoinPrune packet.\n";

    emit(rcvdJoinPrunePkSignal, pkt);

    IPv4ControlInfo *ctrlInfo = check_and_cast<IPv4ControlInfo *>(pkt->getControlInfo());
    InterfaceEntry *incomingInterface = ift->getInterfaceById(ctrlInfo->getInterfaceId());

    if (!incomingInterface) {
        delete pkt;
        return;
    }

    IPv4Address upstreamNeighborAddress = pkt->getUpstreamNeighborAddress();
    int numRpfNeighbors = pimNbt->getNumNeighbors(incomingInterface->getInterfaceId());

    for (unsigned int i = 0; i < pkt->getJoinPruneGroupsArraySize(); i++) {
        JoinPruneGroup group = pkt->getJoinPruneGroups(i);
        IPv4Address groupAddr = group.getGroupAddress();

        // go through list of joined sources
        for (unsigned int j = 0; j < group.getJoinedSourceAddressArraySize(); j++) {
            EncodedAddress& source = group.getJoinedSourceAddress(j);
            Route *route = findRoute(source.IPaddress, groupAddr);
            processJoin(route, incomingInterface->getInterfaceId(), numRpfNeighbors, upstreamNeighborAddress);
        }

        // go through list of pruned sources
        for (unsigned int j = 0; j < group.getPrunedSourceAddressArraySize(); j++) {
            EncodedAddress& source = group.getPrunedSourceAddress(j);
            Route *route = findRoute(source.IPaddress, groupAddr);
            processPrune(route, incomingInterface->getInterfaceId(), pkt->getHoldTime(), numRpfNeighbors, upstreamNeighborAddress);
        }
    }

    delete pkt;
}

/**
 * The method process PIM Prune packet. First the method has to find correct outgoing interface
 * where PIM Prune packet came to. The method also checks if there is still any forwarding outgoing
 * interface. Forwarding interfaces, where Prune packet come to, goes to prune state. If all outgoing
 * interfaces are pruned, the router will prune from multicast tree.
 */
void PIMDM::processPrune(Route *route, int intId, int holdTime, int numRpfNeighbors, IPv4Address upstreamNeighborField)
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
    if (upstreamNeighborField != downstream->ie->ipv4Data()->getIPAddress())
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

void PIMDM::processJoin(Route *route, int intId, int numRpfNeighbors, IPv4Address upstreamNeighborField)
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
    if (upstreamNeighborField != downstream->ie->ipv4Data()->getIPAddress())
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

void PIMDM::processGraftPacket(PIMGraft *pkt)
{
    EV_INFO << "Received Graft packet.\n";

    emit(rcvdGraftPkSignal, pkt);

    IPv4ControlInfo *ctrlInfo = check_and_cast<IPv4ControlInfo *>(pkt->getControlInfo());
    IPv4Address sender = ctrlInfo->getSrcAddr();
    InterfaceEntry *incomingInterface = ift->getInterfaceById(ctrlInfo->getInterfaceId());

    // does packet belong to this router?
    if (pkt->getUpstreamNeighborAddress() != incomingInterface->ipv4Data()->getIPAddress()) {
        delete pkt;
        return;
    }

    for (unsigned int i = 0; i < pkt->getJoinPruneGroupsArraySize(); i++) {
        JoinPruneGroup& group = pkt->getJoinPruneGroups(i);
        IPv4Address groupAddr = group.getGroupAddress();

        for (unsigned int j = 0; j < group.getJoinedSourceAddressArraySize(); j++) {
            EncodedAddress& source = group.getJoinedSourceAddress(j);
            processGraft(source.IPaddress, groupAddr, sender, incomingInterface->getInterfaceId());
        }
    }

    // Send GraftAck for this Graft message
    sendGraftAckPacket(pkt);

    delete pkt;
}

/**
 * The method is used to process PIMGraft packet. Packet means that downstream router wants to join to
 * multicast tree, so the packet cannot come to RPF interface. Router finds correct outgoig interface
 * towards downstream router. Change its state to forward if it was not before and cancel Prune Timer.
 * If route was in pruned state, router will send also Graft message to join multicast tree.
 */
void PIMDM::processGraft(IPv4Address source, IPv4Address group, IPv4Address sender, int incomingInterfaceId)
{
    EV_DEBUG << "Processing Graft(S=" << source << ", G=" << group << "), sender=" << sender << "incoming if=" << incomingInterfaceId << endl;

    Route *route = findRoute(source, group);

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

// The set of interfaces defined by the olist(S,G) macro becomes
// null, indicating that traffic from S addressed to group G should
// no longer be forwarded.
void PIMDM::processOlistEmptyEvent(Route *route)
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

void PIMDM::processOlistNonEmptyEvent(Route *route)
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

void PIMDM::processGraftAckPacket(PIMGraftAck *pkt)
{
    EV_INFO << "Received GraftAck packet.\n";

    emit(rcvdGraftAckPkSignal, pkt);

    IPv4ControlInfo *ctrlInfo = check_and_cast<IPv4ControlInfo *>(pkt->getControlInfo());
    IPv4Address destAddress = ctrlInfo->getDestAddr();

    for (unsigned int i = 0; i < pkt->getJoinPruneGroupsArraySize(); i++) {
        JoinPruneGroup& group = pkt->getJoinPruneGroups(i);
        IPv4Address groupAddr = group.getGroupAddress();

        for (unsigned int j = 0; j < group.getJoinedSourceAddressArraySize(); j++) {
            EncodedAddress& source = group.getJoinedSourceAddress(j);
            Route *route = findRoute(source.IPaddress, groupAddr);
            UpstreamInterface *upstream = route->upstreamInterface;

            // If the destination address of the GraftAck packet is not
            // the address of the upstream interface, then no state transition occur.
            if (destAddress != upstream->ie->ipv4Data()->getIPAddress())
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

    delete pkt;
}

/**
 * The method is used to process PIMStateRefresh packet. The method checks if there is route in mroute
 * and that packet has came to RPF interface. Then it goes through all outgoing interfaces. If the
 * interface is pruned, it resets Prune Timer. For each interface State Refresh message is copied and
 * correct prune indicator is set according to state of outgoing interface (pruned/forwarding).
 *
 * State Refresh message is used to stop flooding of network each 3 minutes.
 */
void PIMDM::processStateRefreshPacket(PIMStateRefresh *pkt)
{
    EV << "pimDM::processStateRefreshPacket" << endl;

    emit(rcvdStateRefreshPkSignal, pkt);

    // first check if there is route for given group address and source
    Route *route = findRoute(pkt->getSourceAddress(), pkt->getGroupAddress());
    if (route == nullptr) {
        delete pkt;
        return;
    }

    // check if State Refresh msg has came from RPF neighbor
    IPv4ControlInfo *ctrlInfo = check_and_cast<IPv4ControlInfo *>(pkt->getControlInfo());
    UpstreamInterface *upstream = route->upstreamInterface;
    if (ctrlInfo->getInterfaceId() != upstream->getInterfaceId() || upstream->rpfNeighbor() != ctrlInfo->getSrcAddr()) {
        delete pkt;
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
        delete pkt;
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
        sendStateRefreshPacket(pkt->getOriginatorAddress(), route, downstream, pkt->getTtl() - 1);

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
    AssertMetric receivedMetric(pkt->getMetricPreference(), pkt->getMetric(), ctrlInfo->getSrcAddr());
    processAssert(upstream, receivedMetric, pkt->getInterval());

    delete pkt;
}

void PIMDM::processAssertPacket(PIMAssert *pkt)
{
    IPv4ControlInfo *ctrlInfo = check_and_cast<IPv4ControlInfo *>(pkt->getControlInfo());
    int incomingInterfaceId = ctrlInfo->getInterfaceId();
    IPv4Address source = pkt->getSourceAddress();
    IPv4Address group = pkt->getGroupAddress();
    AssertMetric receivedMetric = AssertMetric(pkt->getMetricPreference(), pkt->getMetric(), ctrlInfo->getSrcAddr());
    Route *route = findRoute(source, group);
    ASSERT(route);    // XXX create S,G state?
    Interface *incomingInterface = route->upstreamInterface->getInterfaceId() == incomingInterfaceId ?
        static_cast<Interface *>(route->upstreamInterface) :
        static_cast<Interface *>(route->findDownstreamInterfaceByInterfaceId(incomingInterfaceId));
    ASSERT(incomingInterface);

    EV_INFO << "Received Assert(S=" << source << ", G=" << group
            << ") packet on interface '" << incomingInterface->ie->getName() << "'.\n";

    emit(rcvdAssertPkSignal, pkt);

    processAssert(incomingInterface, receivedMetric, 0);

    delete pkt;
}

/*
 * This method called when we received the assert metrics of a neighbor, either in an Assert or in a StateRefresh message.
 */
void PIMDM::processAssert(Interface *incomingInterface, AssertMetric receivedMetric, int stateRefreshInterval)
{
    Route *route = check_and_cast<Route *>(incomingInterface->owner);
    UpstreamInterface *upstream = route->upstreamInterface;

    //
    // Assert State Machine
    //
    AssertMetric currentMetric = incomingInterface->assertState == Interface::NO_ASSERT_INFO ?
        route->metric.setAddress(incomingInterface->ie->ipv4Data()->getIPAddress()) :
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

/*
 * The Prune Timer (PT(S,G,I)) expires, indicating that it is again
 * time to flood data from S addressed to group G onto interface I.
 * The Prune(S,G) Downstream state machine on interface I MUST
 * transition to the NoInfo (NI) state.  The router MUST evaluate
 * any possible transitions in the Upstream(S,G) state machine.
 */
void PIMDM::processPruneTimer(cMessage *timer)
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
void PIMDM::processPrunePendingTimer(cMessage *timer)
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

void PIMDM::processGraftRetryTimer(cMessage *timer)
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
    scheduleAt(simTime() + graftRetryInterval, timer);
}

void PIMDM::processOverrideTimer(cMessage *timer)
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
void PIMDM::processSourceActiveTimer(cMessage *timer)
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
    deleteRoute(route->source, route->group);
    IPv4MulticastRoute *ipv4Route = findIPv4MulticastRoute(route->group, route->source);
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
void PIMDM::processStateRefreshTimer(cMessage *timer)
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
        IPv4Address originator = downstream->ie->ipv4Data()->getIPAddress();
        sendStateRefreshPacket(originator, route, downstream, upstream->maxTtlSeen);
    }

    scheduleAt(simTime() + stateRefreshInterval, timer);
}

void PIMDM::processAssertTimer(cMessage *timer)
{
    Interface *interfaceData = static_cast<Interface *>(timer->getContextPointer());
    ASSERT(timer == interfaceData->assertTimer);
    ASSERT(interfaceData->assertState != DownstreamInterface::NO_ASSERT_INFO);

    Route *route = check_and_cast<Route *>(interfaceData->owner);
    UpstreamInterface *upstream = route->upstreamInterface;
    EV_DETAIL << "AssertTimer" << route << " interface=" << interfaceData->ie->getName() << " has expired.\n";

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

void PIMDM::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG)
{
    Enter_Method_Silent();
    printNotificationBanner(signalID, obj);
    IPv4Datagram *datagram;
    PIMInterface *pimInterface;

    // new multicast data appears in router
    if (signalID == NF_IPv4_NEW_MULTICAST) {
        datagram = check_and_cast<IPv4Datagram *>(obj);
        pimInterface = getIncomingInterface(datagram);
        if (pimInterface && pimInterface->getMode() == PIMInterface::DenseMode)
            unroutableMulticastPacketArrived(datagram->getSrcAddress(), datagram->getDestAddress(), datagram->getTimeToLive());
    }
    // configuration of interface changed, it means some change from IGMP, address were added.
    else if (signalID == NF_IPv4_MCAST_REGISTERED) {
        IPv4MulticastGroupInfo *info = check_and_cast<IPv4MulticastGroupInfo *>(obj);
        pimInterface = pimIft->getInterfaceById(info->ie->getInterfaceId());
        if (pimInterface && pimInterface->getMode() == PIMInterface::DenseMode)
            multicastReceiverAdded(pimInterface->getInterfacePtr(), info->groupAddress);
    }
    // configuration of interface changed, it means some change from IGMP, address were removed.
    else if (signalID == NF_IPv4_MCAST_UNREGISTERED) {
        IPv4MulticastGroupInfo *info = check_and_cast<IPv4MulticastGroupInfo *>(obj);
        pimInterface = pimIft->getInterfaceById(info->ie->getInterfaceId());
        if (pimInterface && pimInterface->getMode() == PIMInterface::DenseMode)
            multicastReceiverRemoved(pimInterface->getInterfacePtr(), info->groupAddress);
    }
    // data come to non-RPF interface
    else if (signalID == NF_IPv4_DATA_ON_NONRPF) {
        datagram = check_and_cast<IPv4Datagram *>(obj);
        pimInterface = getIncomingInterface(datagram);
        if (pimInterface && pimInterface->getMode() == PIMInterface::DenseMode)
            multicastPacketArrivedOnNonRpfInterface(datagram->getDestAddress(), datagram->getSrcAddress(), pimInterface->getInterfaceId());
    }
    // data come to RPF interface
    else if (signalID == NF_IPv4_DATA_ON_RPF) {
        datagram = check_and_cast<IPv4Datagram *>(obj);
        pimInterface = getIncomingInterface(datagram);
        if (pimInterface && pimInterface->getMode() == PIMInterface::DenseMode)
            multicastPacketArrivedOnRpfInterface(pimInterface->getInterfaceId(),
                    datagram->getDestAddress(), datagram->getSrcAddress(), datagram->getTimeToLive());
    }
    // RPF interface has changed
    else if (signalID == NF_ROUTE_ADDED) {
        IPv4Route *entry = check_and_cast<IPv4Route *>(obj);
        IPv4Address routeSource = entry->getDestination();
        IPv4Address routeNetmask = entry->getNetmask();

        int numRoutes = rt->getNumMulticastRoutes();
        for (int i = 0; i < numRoutes; i++) {
            // find multicast routes whose source are on the destination of the new unicast route
            IPv4MulticastRoute *route = rt->getMulticastRoute(i);
            if (route->getSource() == this && IPv4Address::maskedAddrAreEqual(route->getOrigin(), routeSource, routeNetmask)) {
                IPv4Address source = route->getOrigin();
                IPv4Route *routeToSource = rt->findBestMatchingRoute(source);
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

/**
 * The method process notification about new multicast data stream. It goes through all PIM
 * interfaces and tests them if they can be added to the list of outgoing interfaces. If there
 * is no interface on the list at the end, the router will prune from the multicast tree.
 */
void PIMDM::unroutableMulticastPacketArrived(IPv4Address source, IPv4Address group, unsigned short ttl)
{
    ASSERT(!source.isUnspecified());
    ASSERT(group.isMulticast());

    EV_DETAIL << "New multicast source observed: source=" << source << ", group=" << group << ".\n";

    IPv4Route *routeToSrc = rt->findBestMatchingRoute(source);
    if (!routeToSrc || !routeToSrc->getInterface()) {
        EV << "ERROR: PIMDM::newMulticast(): cannot find RPF interface, routing information is missing.";
        return;
    }

    PIMInterface *rpfInterface = pimIft->getInterfaceById(routeToSrc->getInterface()->getInterfaceId());
    if (!rpfInterface || rpfInterface->getMode() != PIMInterface::DenseMode)
        return;

    // gateway is unspecified for directly connected destinations
    bool isSourceDirectlyConnected = routeToSrc->getSourceType() == IPv4Route::IFACENETMASK;
    IPv4Address rpfNeighbor = routeToSrc->getGateway().isUnspecified() ? source : routeToSrc->getGateway();

    Route *route = new Route(this, source, group);
    routes[SourceAndGroup(source, group)] = route;
    route->metric = AssertMetric(routeToSrc->getAdminDist(), routeToSrc->getMetric(), IPv4Address::UNSPECIFIED_ADDRESS);
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
        PIMInterface *pimInterface = pimIft->getInterface(i);

        //check if PIM-DM interface and it is not RPF interface
        if (pimInterface == rpfInterface || pimInterface->getMode() != PIMInterface::DenseMode) // XXX original code added downstream if data for PIM-SM interfaces too
            continue;

        bool hasPIMNeighbors = pimNbt->getNumNeighbors(pimInterface->getInterfaceId()) > 0;
        bool hasConnectedReceivers = pimInterface->getInterfacePtr()->ipv4Data()->hasMulticastListener(group);

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

        // Prune message is sent from the forwarding hook (NF_IPv4_DATA_ON_RPF), see multicastPacketArrivedOnRpfInterface()
    }

    // create new multicast route
    IPv4MulticastRoute *newRoute = new IPv4MulticastRoute();
    newRoute->setOrigin(source);
    newRoute->setOriginNetmask(IPv4Address::ALLONES_ADDRESS);
    newRoute->setMulticastGroup(group);
    newRoute->setSourceType(IMulticastRoute::PIM_DM);
    newRoute->setSource(this);
    newRoute->setInInterface(new IMulticastRoute::InInterface(route->upstreamInterface->ie));
    for (auto & elem : route->downstreamInterfaces) {
        DownstreamInterface *downstream = elem;
        newRoute->addOutInterface(new PIMDMOutInterface(downstream->ie, downstream));
    }

    rt->addMulticastRoute(newRoute);
    EV_DETAIL << "New route was added to the multicast routing table.\n";
}

void PIMDM::multicastPacketArrivedOnRpfInterface(int interfaceId, IPv4Address group, IPv4Address source, unsigned short ttl)
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
            PIMInterface *pimInterface = pimIft->getInterfaceById(upstream->ie->getInterfaceId());
            if (pimInterface && pimInterface->getSR())
                upstream->startStateRefreshTimer();
        }
        restartTimer(upstream->sourceActiveTimer, sourceActiveInterval);

        // record max TTL seen, it is used in StateRefresh messages
        upstream->maxTtlSeen = max(upstream->maxTtlSeen, ttl);
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
 * The method has to solve the problem when multicast data appears on non-RPF interface. It can
 * happen when there is loop in the network. In this case, router has to prune from the neighbor,
 * so it sends Prune message.
 */
void PIMDM::multicastPacketArrivedOnNonRpfInterface(IPv4Address group, IPv4Address source, int interfaceId)
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
        IPv4Address nextHop = (pimNbt->getNeighbor(interfaceId, 0))->getAddress();
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
        downstream->winnerMetric = route->metric.setAddress(downstream->ie->ipv4Data()->getIPAddress());
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

/*
 * The method process notification about new multicast groups aasigned to interface. For each
 * new address it tries to find route. If there is route, it finds interface in list of outgoing
 * interfaces. If the interface is not in the list it will be added. if the router was pruned
 * from multicast tree, join again.
 */
void PIMDM::multicastReceiverAdded(InterfaceEntry *ie, IPv4Address group)
{
    EV_DETAIL << "Multicast receiver added for group " << group << ".\n";

    int numRoutes = rt->getNumMulticastRoutes();
    for (int i = 0; i < numRoutes; i++) {
        IPv4MulticastRoute *ipv4Route = rt->getMulticastRoute(i);

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
            ipv4Route->addOutInterface(new PIMDMOutInterface(ie, downstream));
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
void PIMDM::multicastReceiverRemoved(InterfaceEntry *ie, IPv4Address group)
{
    EV_DETAIL << "No more receiver for group " << group << " on interface '" << ie->getName() << "'.\n";

    // delete pimInt from outgoing interfaces of multicast routes for group
    int numRoutes = rt->getNumMulticastRoutes();
    for (int i = 0; i < numRoutes; i++) {
        IPv4MulticastRoute *ipv4Route = rt->getMulticastRoute(i);
        if (ipv4Route->getSource() == this && ipv4Route->getMulticastGroup() == group) {
            Route *route = findRoute(ipv4Route->getOrigin(), group);

            // remove pimInt from the list of outgoing interfaces
            DownstreamInterface *downstream = route->findDownstreamInterfaceByInterfaceId(ie->getInterfaceId());
            if (downstream) {
                bool wasInOlist = downstream->isInOlist();
                downstream->setHasConnectedReceivers(false);
                if (wasInOlist && !downstream->isInOlist()) {
                    EV_DEBUG << "Removed interface '" << ie->getName() << "' from the outgoing interface list of route " << route << ".\n";

                    // fire upstream state machine event
                    if (route->isOilistNull())
                        processOlistEmptyEvent(route);
                }
            }
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
void PIMDM::rpfInterfaceHasChanged(IPv4MulticastRoute *ipv4Route, IPv4Route *routeToSource)
{
    InterfaceEntry *newRpf = routeToSource->getInterface();
    IPv4Address source = ipv4Route->getOrigin();
    IPv4Address group = ipv4Route->getMulticastGroup();
    int rpfId = newRpf->getInterfaceId();

    EV_DETAIL << "New RPF interface for group=" << group << " source=" << source << " is " << newRpf->getName() << endl;

    Route *route = findRoute(source, group);
    ASSERT(route);

    // delete old upstream interface data
    UpstreamInterface *oldUpstreamInterface = route->upstreamInterface;
    InterfaceEntry *oldRpfInterface = oldUpstreamInterface ? oldUpstreamInterface->ie : nullptr;
    delete oldUpstreamInterface;
    delete ipv4Route->getInInterface();
    ipv4Route->setInInterface(nullptr);

    // set new upstream interface data
    bool isSourceDirectlyConnected = routeToSource->getSourceType() == IPv4Route::IFACENETMASK;
    IPv4Address newRpfNeighbor = pimNbt->getNeighbor(rpfId, 0)->getAddress();    // XXX what happens if no neighbors?
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
        ipv4Route->addOutInterface(new PIMDMOutInterface(oldRpfInterface, downstream));
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

void PIMDM::sendPrunePacket(IPv4Address nextHop, IPv4Address src, IPv4Address grp, int holdTime, int intId)
{
    ASSERT(!src.isUnspecified());
    ASSERT(grp.isMulticast());

    EV_INFO << "Sending Prune(S=" << src << ", G=" << grp << ") message to neighbor '" << nextHop << "' on interface '" << intId << "'\n";

    PIMJoinPrune *packet = new PIMJoinPrune("PIMPrune");
    packet->setUpstreamNeighborAddress(nextHop);
    packet->setHoldTime(holdTime);

    // set multicast groups
    packet->setJoinPruneGroupsArraySize(1);
    JoinPruneGroup& group = packet->getJoinPruneGroups(0);
    group.setGroupAddress(grp);
    group.setPrunedSourceAddressArraySize(1);
    EncodedAddress& address = group.getPrunedSourceAddress(0);
    address.IPaddress = src;

    packet->setByteLength(PIM_HEADER_LENGTH + 8 + ENCODED_GROUP_ADDRESS_LENGTH + 4 + ENCODED_SOURCE_ADDRESS_LENGTH);

    emit(sentJoinPrunePkSignal, packet);

    sendToIP(packet, IPv4Address::UNSPECIFIED_ADDRESS, ALL_PIM_ROUTERS_MCAST, intId);
}

void PIMDM::sendJoinPacket(IPv4Address nextHop, IPv4Address src, IPv4Address grp, int intId)
{
    ASSERT(!src.isUnspecified());
    ASSERT(grp.isMulticast());

    EV_INFO << "Sending Join(S=" << src << ", G=" << grp << ") message to neighbor '" << nextHop << "' on interface '" << intId << "'\n";

    PIMJoinPrune *packet = new PIMJoinPrune("PIMJoin");
    packet->setUpstreamNeighborAddress(nextHop);
    packet->setHoldTime(0);    // ignored by the receiver

    // set multicast groups
    packet->setJoinPruneGroupsArraySize(1);
    JoinPruneGroup& group = packet->getJoinPruneGroups(0);
    group.setGroupAddress(grp);
    group.setJoinedSourceAddressArraySize(1);
    EncodedAddress& address = group.getJoinedSourceAddress(0);
    address.IPaddress = src;

    packet->setByteLength(PIM_HEADER_LENGTH + 8 + ENCODED_GROUP_ADDRESS_LENGTH + 4 + ENCODED_SOURCE_ADDRESS_LENGTH);

    emit(sentJoinPrunePkSignal, packet);

    sendToIP(packet, IPv4Address::UNSPECIFIED_ADDRESS, ALL_PIM_ROUTERS_MCAST, intId);
}

/*
 * PIM Graft messages use the same format as Join/Prune messages, except
 * that the Type field is set to 6.  The source address MUST be in the
 * Join section of the message.  The Hold Time field SHOULD be zero and
 * SHOULD be ignored when a Graft is received.
 */
void PIMDM::sendGraftPacket(IPv4Address nextHop, IPv4Address src, IPv4Address grp, int intId)
{
    EV_INFO << "Sending Graft(S=" << src << ", G=" << grp << ") message to neighbor '" << nextHop << "' on interface '" << intId << "'\n";

    PIMGraft *msg = new PIMGraft("PIMGraft");
    msg->setHoldTime(0);
    msg->setUpstreamNeighborAddress(nextHop);

    msg->setJoinPruneGroupsArraySize(1);
    JoinPruneGroup& group = msg->getJoinPruneGroups(0);
    group.setGroupAddress(grp);
    group.setJoinedSourceAddressArraySize(1);
    EncodedAddress& address = group.getJoinedSourceAddress(0);
    address.IPaddress = src;

    msg->setByteLength(PIM_HEADER_LENGTH + 8 + ENCODED_GROUP_ADDRESS_LENGTH + 4 + ENCODED_SOURCE_ADDRESS_LENGTH);

    emit(sentGraftPkSignal, msg);

    sendToIP(msg, IPv4Address::UNSPECIFIED_ADDRESS, nextHop, intId);
}

/*
 * PIM Graft Ack messages are identical in format to the received Graft
 * message, except that the Type field is set to 7.  The Upstream
 * Neighbor Address field SHOULD be set to the sender of the Graft
 * message and SHOULD be ignored upon receipt.
 */
void PIMDM::sendGraftAckPacket(PIMGraft *graftPacket)
{
    EV_INFO << "Sending GraftAck message.\n";

    IPv4ControlInfo *oldCtrl = check_and_cast<IPv4ControlInfo *>(graftPacket->removeControlInfo());
    IPv4Address destAddr = oldCtrl->getSrcAddr();
    IPv4Address srcAddr = oldCtrl->getDestAddr();
    int outInterfaceId = oldCtrl->getInterfaceId();
    delete oldCtrl;

    PIMGraftAck *msg = new PIMGraftAck();
    *((PIMGraft *)msg) = *graftPacket;
    msg->setName("PIMGraftAck");
    msg->setType(GraftAck);

    emit(sentGraftAckPkSignal, msg);

    sendToIP(msg, srcAddr, destAddr, outInterfaceId);
}

void PIMDM::sendStateRefreshPacket(IPv4Address originator, Route *route, DownstreamInterface *downstream, unsigned short ttl)
{
    EV_INFO << "Sending StateRefresh(S=" << route->source << ", G=" << route->group
            << ") message on interface '" << downstream->ie->getName() << "'\n";

    PIMStateRefresh *msg = new PIMStateRefresh("PIMStateRefresh");
    msg->setGroupAddress(route->group);
    msg->setSourceAddress(route->source);
    msg->setOriginatorAddress(originator);
    msg->setInterval(stateRefreshInterval);
    msg->setTtl(ttl);
    msg->setP(downstream->pruneState == DownstreamInterface::PRUNED);
    // TODO set metric

    msg->setByteLength(PIM_HEADER_LENGTH
            + ENCODED_GROUP_ADDRESS_LENGTH
            + ENCODED_UNICODE_ADDRESS_LENGTH
            + ENCODED_UNICODE_ADDRESS_LENGTH
            + 12);

    emit(sentStateRefreshPkSignal, msg);

    sendToIP(msg, IPv4Address::UNSPECIFIED_ADDRESS, ALL_PIM_ROUTERS_MCAST, downstream->ie->getInterfaceId());
}

void PIMDM::sendAssertPacket(IPv4Address source, IPv4Address group, AssertMetric metric, InterfaceEntry *ie)
{
    EV_INFO << "Sending Assert(S= " << source << ", G= " << group << ") message on interface '" << ie->getName() << "'\n";

    PIMAssert *pkt = new PIMAssert("PIMAssert");
    pkt->setGroupAddress(group);
    pkt->setSourceAddress(source);
    pkt->setR(false);
    pkt->setMetricPreference(metric.preference);
    pkt->setMetric(metric.metric);

    pkt->setByteLength(PIM_HEADER_LENGTH
            + ENCODED_GROUP_ADDRESS_LENGTH
            + ENCODED_UNICODE_ADDRESS_LENGTH
            + 8);

    emit(sentAssertPkSignal, pkt);

    sendToIP(pkt, IPv4Address::UNSPECIFIED_ADDRESS, ALL_PIM_ROUTERS_MCAST, ie->getInterfaceId());
}

void PIMDM::sendToIP(PIMPacket *packet, IPv4Address srcAddr, IPv4Address destAddr, int outInterfaceId)
{
    IPv4ControlInfo *ctrl = new IPv4ControlInfo();
    ctrl->setSrcAddr(srcAddr);
    ctrl->setDestAddr(destAddr);
    ctrl->setProtocol(IP_PROT_PIM);
    ctrl->setTimeToLive(1);
    ctrl->setInterfaceId(outInterfaceId);
    packet->setControlInfo(ctrl);
    send(packet, "ipOut");
}

//----------------------------------------------------------------------------
//           Helpers
//----------------------------------------------------------------------------

void PIMDM::restartTimer(cMessage *timer, double interval)
{
    cancelEvent(timer);
    scheduleAt(simTime() + interval, timer);
}

void PIMDM::cancelAndDeleteTimer(cMessage *& timer)
{
    cancelAndDelete(timer);
    timer = nullptr;
}

PIMInterface *PIMDM::getIncomingInterface(IPv4Datagram *datagram)
{
    cGate *g = datagram->getArrivalGate();
    if (g) {
        InterfaceEntry *ie = ift->getInterfaceByNetworkLayerGateIndex(g->getIndex());
        if (ie)
            return pimIft->getInterfaceById(ie->getInterfaceId());
    }
    return nullptr;
}

IPv4MulticastRoute *PIMDM::findIPv4MulticastRoute(IPv4Address group, IPv4Address source)
{
    int numRoutes = rt->getNumMulticastRoutes();
    for (int i = 0; i < numRoutes; i++) {
        IPv4MulticastRoute *route = rt->getMulticastRoute(i);
        if (route->getSource() == this && route->getMulticastGroup() == group && route->getOrigin() == source)
            return route;
    }
    return nullptr;
}

PIMDM::Route *PIMDM::findRoute(IPv4Address source, IPv4Address group)
{
    auto it = routes.find(SourceAndGroup(source, group));
    return it != routes.end() ? it->second : nullptr;
}

void PIMDM::deleteRoute(IPv4Address source, IPv4Address group)
{
    auto it = routes.find(SourceAndGroup(source, group));
    if (it != routes.end()) {
        delete it->second;
        routes.erase(it);
    }
}

void PIMDM::clearRoutes()
{
    // delete IPv4 routes
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < rt->getNumMulticastRoutes(); i++) {
            IPv4MulticastRoute *ipv4Route = rt->getMulticastRoute(i);
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

PIMDM::Route::~Route()
{
    delete upstreamInterface;
    for (auto & elem : downstreamInterfaces)
        delete elem;
    downstreamInterfaces.clear();
}

PIMDM::DownstreamInterface *PIMDM::Route::findDownstreamInterfaceByInterfaceId(int interfaceId) const
{
    for (auto & elem : downstreamInterfaces)
        if (elem->ie->getInterfaceId() == interfaceId)
            return elem;

    return nullptr;
}

PIMDM::DownstreamInterface *PIMDM::Route::createDownstreamInterface(InterfaceEntry *ie)
{
    DownstreamInterface *downstream = new DownstreamInterface(this, ie);
    downstreamInterfaces.push_back(downstream);
    return downstream;
}

PIMDM::DownstreamInterface *PIMDM::Route::removeDownstreamInterface(int interfaceId)
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

bool PIMDM::Route::isOilistNull()
{
    for (auto & elem : downstreamInterfaces) {
        if (elem->isInOlist())
            return false;
    }
    return true;
}

PIMDM::UpstreamInterface::~UpstreamInterface()
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
void PIMDM::UpstreamInterface::startGraftRetryTimer()
{
    graftRetryTimer = new cMessage("PIMGraftRetryTimer", GraftRetryTimer);
    graftRetryTimer->setContextPointer(this);
    pimdm()->scheduleAt(simTime() + pimdm()->graftRetryInterval, graftRetryTimer);
}

void PIMDM::UpstreamInterface::startOverrideTimer()
{
    overrideTimer = new cMessage("PIMOverrideTimer", UpstreamOverrideTimer);
    overrideTimer->setContextPointer(this);
    pimdm()->scheduleAt(simTime() + pimdm()->overrideInterval, overrideTimer);
}

/**
 * The method is used to create PIMSourceActive timer. The timer is set when source of multicast is
 * connected directly to the router.  If timer expires, router will remove the route from multicast
 * routing table. It is set to (S,G).
 */
void PIMDM::UpstreamInterface::startSourceActiveTimer()
{
    sourceActiveTimer = new cMessage("PIMSourceActiveTimer", SourceActiveTimer);
    sourceActiveTimer->setContextPointer(this);
    pimdm()->scheduleAt(simTime() + pimdm()->sourceActiveInterval, sourceActiveTimer);
}

/**
 * The method is used to create PIMStateRefresh timer. The timer is set when source of multicast is
 * connected directly to the router. If timer expires, router will send StateRefresh message, which
 * will propagate through all network and wil reset Prune Timer. It is set to (S,G).
 */
void PIMDM::UpstreamInterface::startStateRefreshTimer()
{
    stateRefreshTimer = new cMessage("PIMStateRefreshTimer", StateRefreshTimer);
    sourceActiveTimer->setContextPointer(this);
    pimdm()->scheduleAt(simTime() + pimdm()->stateRefreshInterval, stateRefreshTimer);
}

PIMDM::DownstreamInterface::~DownstreamInterface()
{
    owner->owner->cancelAndDelete(pruneTimer);
    owner->owner->cancelAndDelete(prunePendingTimer);
}

/**
 * The method is used to create PIMPrune timer. The timer is set when outgoing interface
 * goes to pruned state. After expiration (usually 3min) interface goes back to forwarding
 * state. It is set to (S,G,I).
 */
void PIMDM::DownstreamInterface::startPruneTimer(double holdTime)
{
    cMessage *timer = new cMessage("PimPruneTimer", PruneTimer);
    timer->setContextPointer(this);
    owner->owner->scheduleAt(simTime() + holdTime, timer);
    pruneTimer = timer;
}

void PIMDM::DownstreamInterface::stopPruneTimer()
{
    if (pruneTimer) {
        if (pruneTimer->isScheduled())
            owner->owner->cancelEvent(pruneTimer);
        delete pruneTimer;
        pruneTimer = nullptr;
    }
}

void PIMDM::DownstreamInterface::startPrunePendingTimer(double overrideInterval)
{
    ASSERT(!prunePendingTimer);
    cMessage *timer = new cMessage("PimPrunePendingTimer", PrunePendingTimer);
    timer->setContextPointer(this);
    owner->owner->scheduleAt(simTime() + overrideInterval, timer);
    prunePendingTimer = timer;
}

void PIMDM::DownstreamInterface::stopPrunePendingTimer()
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
bool PIMDM::DownstreamInterface::isInOlist() const
{
    // TODO check boundary
    bool hasNeighbors = pimdm()->pimNbt->getNumNeighbors(ie->getInterfaceId()) > 0;
    return ((hasNeighbors && pruneState != PRUNED) || hasConnectedReceivers()) && assertState != I_LOST_ASSERT;
}
}    // namespace inet

