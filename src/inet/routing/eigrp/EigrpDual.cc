//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
/**
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#include "inet/routing/eigrp/EigrpDual.h"

#include <omnetpp.h>

#include "inet/routing/eigrp/pdms/EigrpMetricHelper.h"

#define EIGRP_DUAL_DEBUG
namespace inet {
namespace eigrp {
namespace eigrpDual {
// User messages
const char *userMsgs[] =
{
    // RECV_UPDATE
    "received Update",
    // RECV_QUERY
    "received Query",
    // RECV_REPLY
    "received Reply",
    // NEIGHBOR_DOWN
    "neighbor down",
    // INTERFACE_DOWN
    "interface down",
    //INTERFACE_UP
    "interface up",
    //LOST_ROUTE
    "route deletion",
};
}; // end of namespace eigrpDual

template<typename IPAddress>
void EigrpDual<IPAddress>::invalidateRoute(EigrpRouteSource<IPAddress> *routeSrc)
{
    if (routeSrc->isValid()) {
        routeSrc->setValid(false);
        EV_DEBUG << "DUAL: invalidate route via " << routeSrc->getNextHop() << " in TT" << endl;
    }
}

/**
 * @param event Type of input event
 * @param source Data from messages are stored into source. If the source is new,
 *        then it is not inserted into route.
 * @param route Contains actual metric and RD. Dij remains unchanged.
 */
template<typename IPAddress>
void EigrpDual<IPAddress>::processEvent(DualEvent event, EigrpRouteSource<IPAddress> *source, int neighborId, bool isSourceNew)
{
    EigrpRoute<IPAddress> *route = source->getRouteInfo();

    if (event == NEIGHBOR_DOWN || event == INTERFACE_DOWN)
        source->setUnreachableMetric(); // Must be there

    EV_DEBUG << "DUAL: " << eigrpDual::userMsgs[event];
    EV_DEBUG << " for route " << route->getRouteAddress() << " via " << source->getNextHop();
    EV_DEBUG << " (" << source->getMetric() << "/" << source->getRd() << ")" << endl;
    EV_DEBUG << "QueryOrigin je: " << route->getQueryOrigin() << " , replyStatusSum je: " << route->getReplyStatusSum() << endl;
    EV_DEBUG << "Event: " << event << endl;
    switch (route->getQueryOrigin()) {
        case 0: // active state
            processQo0(event, source, route, neighborId, isSourceNew);
            break;

        case 1:
            if (route->getReplyStatusSum() == 0) { // passive state
                processQo1Passive(event, source, route, neighborId, isSourceNew);
            }
            else { // active state
                processQo1Active(event, source, route, neighborId, isSourceNew);
            }
            break;

        case 2: // active state
            processQo2(event, source, route, neighborId, isSourceNew);
            break;

        case 3: // active state
            processQo3(event, source, route, neighborId, isSourceNew);
            break;

        default:
            // DUAL detected invalid state of route
            ASSERT(false);
            break;
    }
}

//
// States of DUAL
//

template<typename IPAddress>
void EigrpDual<IPAddress>::processQo0(DualEvent event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId, bool isSourceNew)
{
    uint64_t dmin;
    bool hasReplyStatus;

    switch (event) {
        case INTERFACE_UP:
            processTransition7(event, source, route, neighborId);
            break;

        case RECV_UPDATE:
            processTransition7(event, source, route, neighborId);
            break;

        case RECV_QUERY:
            if (source->isSuccessor())
                processTransition5(event, source, route, neighborId);
            else
                processTransition6(event, source, route, neighborId);
            break;

        case RECV_REPLY:
        case NEIGHBOR_DOWN:
        case INTERFACE_DOWN:
            if ((hasReplyStatus = route->unsetReplyStatus(neighborId)) == true)
                EV_DEBUG << "     Clear handle, reply status summary = " << route->getReplyStatusSum() << endl;

            if (route->getReplyStatusSum() == 0) { // As last reply from neighbor
                // Check FC with FDij(t)
                if (pdm->hasFeasibleSuccessor(route, dmin))
                    processTransition14(event, source, route, dmin, neighborId);
                else
                    processTransition11(event, source, route, dmin, neighborId);
            }
            else if (hasReplyStatus) { // As not last reply from neighbor
                processTransition8(event, source, route, neighborId, isSourceNew);
            }
            // else do nothing (connected source)
            break;

        case LOST_ROUTE:
            // do nothing
            break;

        default:
            EV_DEBUG << "DUAL received invalid input event num. " << event << " in active state 0" << endl;
            ASSERT(false);
            break;
    }
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processQo1Passive(DualEvent event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId, bool isSourceNew)
{
    uint64_t dmin;

    switch (event) {
        case LOST_ROUTE:
        case INTERFACE_UP:
        case RECV_UPDATE:
            if (event == LOST_ROUTE) // Force loss of all sources of the route
                route->setFd(0);

            if (pdm->hasFeasibleSuccessor(route, dmin))
                processTransition2(event, source, route, dmin, neighborId);
            else
                processTransition4(event, source, route, dmin, neighborId);
            break;

        case NEIGHBOR_DOWN:
        case INTERFACE_DOWN:
            if (pdm->hasFeasibleSuccessor(route, dmin))
                processTransition2(event, source, route, dmin, neighborId);
            else
                processTransition4(event, source, route, dmin, neighborId);
            break;

        case RECV_QUERY:
            if (route->getNumSucc() == 0 || pdm->hasFeasibleSuccessor(route, dmin)) { // No successor to the destination or FC satisfied
                if (source->isSuccessor())
                    processTransition2(event, source, route, dmin, neighborId);
                else
                    processTransition1(event, source, route, dmin, neighborId);
            }
            else {
                if (source->isSuccessor())
                    processTransition3(event, source, route, dmin, neighborId);
                else
                    processTransition4(event, source, route, dmin, neighborId);
            }
            break;

        default:
            ASSERT(false);
            EV_DEBUG << "DUAL received invalid input event in passive state 0, skipped" << endl;
            break;
    }
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processQo1Active(DualEvent event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId, bool isSourceNew)
{
    bool hasReplyStatus;

    switch (event) {
        case INTERFACE_UP:
            processTransition17(event, source, route, neighborId);
            break;

        case RECV_UPDATE:
            if (source->isSuccessor())
                processTransition9(event, source, route, neighborId);
            else
                processTransition17(event, source, route, neighborId);
            break;

        case RECV_QUERY:
            if (source->isSuccessor())
                processTransition5(event, source, route, neighborId);
            else
                processTransition6(event, source, route, neighborId);
            break;

        case RECV_REPLY:
            if ((hasReplyStatus = route->unsetReplyStatus(neighborId)) == true)
                EV_DEBUG << "     Clear handle, reply status summary = " << route->getReplyStatusSum() << endl;

            if (route->getReplyStatusSum() == 0) // Last reply
                processTransition15(event, source, route, neighborId);
            else if (hasReplyStatus) // Not last reply
                processTransition18(event, source, route, neighborId, isSourceNew);
            // else do nothing
            break;

        case NEIGHBOR_DOWN:
        case INTERFACE_DOWN:
            if ((hasReplyStatus = route->unsetReplyStatus(neighborId)) == true)
                EV_DEBUG << "     Clear handle, reply status summary = " << route->getReplyStatusSum() << endl;

            // Transition 9 should take precedence over transition 15 (fail of link
            // to S (as last reply) versus fail of link to not S (as last reply)
            // TODO ověřit!
            if (source->isSuccessor())
                processTransition9(event, source, route, neighborId);

            if (route->getReplyStatusSum() == 0) { // As last reply from neighbor
                processTransition15(event, source, route, neighborId);
            }
            else if (hasReplyStatus) { // As not last reply from neighbor
                processTransition18(event, source, route, neighborId, isSourceNew);
            }
            // else do nothing
            break;

        case LOST_ROUTE:
            // do nothing
            break;

        default:
            ASSERT(false);
            EV_DEBUG << "DUAL received invalid input event in active state 1, skipped" << endl;
            break;
    }
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processQo2(DualEvent event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId, bool isSourceNew)
{
    uint64_t dmin;
    bool hasReplyStatus;

    switch (event) {
        case INTERFACE_UP:
            processTransition7(event, source, route, neighborId);
            break;

        case RECV_UPDATE:
            processTransition7(event, source, route, neighborId);
            break;

        case RECV_QUERY:
            if (!source->isSuccessor())
                processTransition6(event, source, route, neighborId);

            else {
                ASSERT(false);
                // do nothing (DUAL can not receive Query form S when it is in active state oij=2)
            }
            break;

        case RECV_REPLY:
        case NEIGHBOR_DOWN:
        case INTERFACE_DOWN:
            if ((hasReplyStatus = route->unsetReplyStatus(neighborId)) == true)
                EV_DEBUG << "     Clear handle, reply status summary = " << route->getReplyStatusSum() << endl;

            if (route->getReplyStatusSum() == 0) { // As last reply from neighbor
                // Check FC with FDij(t)
                if (pdm->hasFeasibleSuccessor(route, dmin))
                    processTransition16(event, source, route, dmin, neighborId);
                else
                    processTransition12(event, source, route, dmin, neighborId);
            }
            else if (hasReplyStatus) { // As not last reply from neighbor
                processTransition8(event, source, route, neighborId, isSourceNew);
            }
            // else do nothing (connected source)
            break;

        case LOST_ROUTE:
            // do nothing
            break;

        default:
            ASSERT(false);
            EV_DEBUG << "DUAL received invalid input event in active state 2, skipped" << endl;
            break;
    }
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processQo3(DualEvent event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId, bool isSourceNew)
{
    bool hasReplyStatus;

    switch (event) {
        case INTERFACE_UP:
            processTransition17(event, source, route, neighborId);
            break;

        case RECV_UPDATE:
            if (source->isSuccessor())
                processTransition10(event, source, route, neighborId);
            else
                processTransition17(event, source, route, neighborId);
            break;

        case RECV_QUERY:
            if (!source->isSuccessor())
                processTransition6(event, source, route, neighborId);
            else {
                ASSERT(false);
                // do nothing (DUAL can not receive Query form S when it is in active state oij=3)
            }
            break;

        case RECV_REPLY:
            if ((hasReplyStatus = route->unsetReplyStatus(neighborId)) == true)
                EV_DEBUG << "     Clear handle, reply status summary = " << route->getReplyStatusSum() << endl;

            if (route->getReplyStatusSum() == 0) // Last reply
                processTransition13(event, source, route, neighborId);
            else if (hasReplyStatus) // Not last reply
                processTransition18(event, source, route, neighborId, isSourceNew);
            // else do nothing
            break;

        case NEIGHBOR_DOWN:
        case INTERFACE_DOWN:
            if ((hasReplyStatus = route->unsetReplyStatus(neighborId)) == true)
                EV_DEBUG << "     Clear handle, reply status summary = " << route->getReplyStatusSum() << endl;

            // Transition 10 should take precedence over transition 13 (fail of link
            // to S (as last reply) versus fail of link to not S (as last reply)
            // TODO ověřit!
            if (source->isSuccessor())
                processTransition10(event, source, route, neighborId);

            if (route->getReplyStatusSum() == 0) { // As last reply from neighbor
                processTransition13(event, source, route, neighborId);
            }
            else if (hasReplyStatus) { // As not last reply from neighbor
                processTransition18(event, source, route, neighborId, isSourceNew);
            }
            // else do nothing
            break;

        case LOST_ROUTE:
            // do nothing
            break;

        default:
            ASSERT(false);
            EV_DEBUG << "DUAL received invalid input event in active state 3, skipped" << endl;
            break;
    }
}

//
// Transitions of DUAL
//
template<typename IPAddress>
void EigrpDual<IPAddress>::processTransition1(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, uint64_t dmin, int neighborId)
{
    EV_DEBUG << "DUAL: transit from oij=1 (passive) to oij=1 (passive) by transition 1" << endl;

    EigrpRouteSource<IPAddress> *successor = pdm->getBestSuccessor(route);
    if (successor == nullptr) { // There is no successor, reply with unreachable route
        pdm->sendReply(route, neighborId, source, false, true);
    }
    else
        pdm->sendReply(route, neighborId, successor);

    // Route will be removed after router receives Ack from neighbor for Reply
    if (source->isUnreachable())
        pdm->setDelayedRemove(neighborId, source);
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processTransition2(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, uint64_t dmin, int neighborId)
{
    EV_DEBUG << "DUAL: transit from oij=1 (passive) to oij=1 (passive) by transition 2" << endl;

    EigrpRouteSource<IPAddress> *successor;
    uint64_t oldDij; // Dij before the event
    bool rtableChanged = false;

    oldDij = route->getDij();

    // Find successors and update route in TT and RT
    successor = pdm->updateRoute(route, dmin, &rtableChanged);

    if (event == RECV_QUERY) {
        ASSERT(successor != nullptr);
        if (neighborId == successor->getNexthopId()) // Poison Reverse
            pdm->sendReply(route, neighborId, successor, true);
        else
            pdm->sendReply(route, neighborId, successor);
        // When Reply is sent, remove unreachable route after receiving Ack (According to Cisco EIGRP 10.0)
        if (source->isUnreachable())
            pdm->setDelayedRemove(neighborId, source);
    }
    else { // When Reply is not sent, remove unreachable route immediately
        if (source->isUnreachable())
            invalidateRoute(source);
    }

    // Send Update about new Successor
    if (successor != nullptr && pdm->hasNeighborForUpdate(successor)) {
        if (rtableChanged)
            pdm->sendUpdate(IEigrpPdm<IPAddress>::UNSPEC_RECEIVER, route, successor, true, "RT changed");
        // Nedavno zmeneno: pokud nastane zmena metriky a cesta neni connected, pak se uplatni Split Horizon (ne Poison Reverse), sem nepatri zmena z inf na non inf hodnotu (to pokryje zmena tabulky)
        else if (route->getDij() != oldDij) {
            bool forcePoisonRev = successor->getNexthopId() == IEigrpPdm<IPAddress>::CONNECTED_ROUTE;
            pdm->sendUpdate(IEigrpPdm<IPAddress>::UNSPEC_RECEIVER, route, successor, forcePoisonRev, "metric changed");
        }
    }
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processTransition3(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, uint64_t dmin, int neighborId)
{
    EV_DEBUG << "DUAL: transit from oij=1 (passive) to oij=3 (active) by transition 3" << endl;
    // Note: source is successor

    int numPeers = 0, numStubs = 0;
    bool gotoActive;
    route->setQueryOrigin(3);

    // Actualize distance of route and FD in TT
//    route->setDij(source->getMetric()); // not there (in transition to passive state DUAL can not detect change of Dij)
    route->setRdPar(source->getMetricParams());
    route->setFd(route->getDij());

    // Send Query with actual distance via successor to all peers
    gotoActive = pdm->setReplyStatusTable(route, source, false, &numPeers, &numStubs);
    EV_DEBUG << "DUAL: peers = " << numPeers << ", stubs = " << numStubs << endl;

    if (gotoActive) {
        pdm->sendQuery(IEigrpPdm<IPAddress>::UNSPEC_RECEIVER, route, source);
    }
    else { // Diffusion computation can not be performed, go back to passive state
        processTransition13(event, source, route, neighborId);
        return;
    }

    // Do not remove source
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processTransition4(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, uint64_t dmin, int neighborId)
{
    EV_DEBUG << "DUAL: transit from oij=1 (passive) to oij=1 (active) by transition 4" << endl;

    int numPeers = 0, numStubs = 0;
    bool gotoActive;
    EigrpRouteSource<IPAddress> *oldSuccessor;

    route->setQueryOrigin(1);

    // Get old successor
    oldSuccessor = pdm->getBestSuccessor(route);
    // Old successor may not be null
    if (oldSuccessor == nullptr) oldSuccessor = source;

    // Actualize distance of route in TT
    if (event == LOST_ROUTE) // Set distance and router RD to inf
        route->setUnreachable();
    else {
        route->setDij(oldSuccessor->getMetric());
        route->setRdPar(oldSuccessor->getMetricParams());
    }
    // Actualize FD in TT
    route->setFd(route->getDij());

    // Send Reply with RDij if input event is Query
    if (event == RECV_QUERY)
        pdm->sendReply(route, neighborId, oldSuccessor);

    // Start own diffusion computation
    gotoActive = pdm->setReplyStatusTable(route, source, true, &numPeers, &numStubs);
    EV_DEBUG << "DUAL: peers = " << numPeers << ", stubs = " << numStubs << endl;
    if (gotoActive) {
        pdm->sendQuery(IEigrpPdm<IPAddress>::UNSPEC_RECEIVER, route, oldSuccessor, true);
    }
    else { // Go to passive state
        processTransition15(event, source, route, neighborId);
        return;
    }

    // Do not remove source
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processTransition5(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId)
{
    EV_DEBUG << "DUAL: transit from oij=" << route->getQueryOrigin() << " (active) to oij=2 (active) by transition 5" << endl;

    route->setQueryOrigin(2);

    // Do not delete source
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processTransition6(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId)
{
    EV_DEBUG << "DUAL: transit from oij=" << route->getQueryOrigin() << " (active) to oij=" << route->getQueryOrigin() << " (active) by transition 6" << endl;

    EigrpRouteSource<IPAddress> *oldSuccessor = pdm->getBestSuccessor(route);
    ASSERT(oldSuccessor != nullptr); // Old successor must be available until transition to passive state
    /*if (oldSuccessor == nullptr) // Send route with unreachable distance (not Poison Reverse)
        pdm->sendReply(route, neighborId, source, false, true);
    else*/

    if (source->isSuccessor()) // Send route with unreachable distance to old Successor (Poison Reverse)
        pdm->sendReply(route, neighborId, oldSuccessor, true);
    else
        pdm->sendReply(route, neighborId, oldSuccessor);

    // Do not remove unreachable route (this will be done after transition to passive state)
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processTransition7(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId)
{
    EV_DEBUG << "DUAL: transit from oij=" << route->getQueryOrigin() << " (active) to oij=" << route->getQueryOrigin() << " (active) by transition 7" << endl;

    // Do not actualize Dij of route by new distance via successor (in transition to passive state DUAL can not detect change of Dij)
//    route->setDij(source->getMetric());

    // Do not remove unreachable route (this will be done after transition to passive state)
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processTransition8(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId, bool isSourceNew)
{
    EV_DEBUG << "DUAL: transit from oij=" << route->getQueryOrigin() << " (active) to oij=" << route->getQueryOrigin() << " (active) by transition 8" << endl;

    if (source->isUnreachable() && isSourceNew)
        invalidateRoute(source);
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processTransition9(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId)
{
    EV_DEBUG << "DUAL: transit from oij=1 (active) to oij=0 (active) by transition 9" << endl;

    route->setQueryOrigin(0);

    // Actualize Dij of route
    route->setDij(source->getMetric());

    if (route->getReplyStatusSum() == 0) { // Reply status table is empty, go to passive state or start new diffusing computation
        uint64_t dmin;
        if (pdm->hasFeasibleSuccessor(route, dmin))
            processTransition14(event, source, route, dmin, neighborId);
        else
            processTransition11(event, source, route, dmin, neighborId);
    }

    // Do not remove unreachable route (this will be done after transition to passive state)
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processTransition10(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId)
{
    EV_DEBUG << "DUAL: transit from oij=3 (active) to oij=2 (active) by transition 10" << endl;

    route->setQueryOrigin(2);

    // Actualize Dij of route
    route->setDij(source->getMetric());

    if (route->getReplyStatusSum() == 0) { // Reply status table is empty, go to passive state or start new diffusing computation
        uint64_t dmin;
        if (pdm->hasFeasibleSuccessor(route, dmin))
            processTransition16(event, source, route, dmin, neighborId);
        else
            processTransition12(event, source, route, dmin, neighborId);
    }

    // Do not remove unreachable route (this will be done after transition to passive state)
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processTransition11(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, uint64_t dmin, int neighborId)
{
    EV_DEBUG << "DUAL: transit from oij=0 (active) to oij=1 (active) by transition 11" << endl;

    int numPeers = 0, numStubs = 0;
    bool gotoActive;

    route->setQueryOrigin(1);

    gotoActive = pdm->setReplyStatusTable(route, source, false, &numPeers, &numStubs);
    EV_DEBUG << "DUAL: peers = " << numPeers << ", stubs = " << numStubs << endl;
    if (gotoActive) { // Start new diffusion computation
        int srcNeighbor = (neighborId != IEigrpPdm<IPAddress>::UNSPEC_RECEIVER) ? neighborId : IEigrpPdm<IPAddress>::UNSPEC_SENDER;
        pdm->sendQuery(IEigrpPdm<IPAddress>::UNSPEC_RECEIVER, route, source, srcNeighbor);
    }
    else { // Go to passive state
        processTransition15(event, source, route, neighborId);
        return;
    }

    // Do not remove unreachable route (this will be done after transition to passive state)
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processTransition12(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, uint64_t dmin, int neighborId)
{
    EV_DEBUG << "DUAL: transit from oij=2 (active) to oij=3 (active) by transition 12" << endl;

    int numPeers = 0, numStubs = 0;
    bool gotoActive;

    route->setQueryOrigin(3);

    gotoActive = pdm->setReplyStatusTable(route, source, false, &numPeers, &numStubs);
    EV_DEBUG << "DUAL: peers = " << numPeers << ", stubs = " << numStubs << endl;
    if (gotoActive) { // Start new diffusion computation
        pdm->sendQuery(IEigrpPdm<IPAddress>::UNSPEC_RECEIVER, route, source);
    }
    else { // Go to passive state
        processTransition13(event, source, route, neighborId);
        return;
    }

    // Do not remove unreachable route (this will be done after transition to passive state)
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processTransition13(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId)
{
    EV_DEBUG << "DUAL: transit from oij=3 (active) to oij=1 (passive) by transition 13" << endl;

    EigrpRouteSource<IPAddress> *successor;
    // Old successor is originator of Query
    EigrpRouteSource<IPAddress> *oldSuccessor = pdm->getBestSuccessor(route);
    uint64_t oldDij = route->getDij();
    uint64_t dmin;
    bool rtableChanged = false;

    route->setQueryOrigin(1);

    // Set FD to max
    route->setFd(EigrpMetricHelper::METRIC_INF);

    // Find min distance
    dmin = pdm->findRouteDMin(route);

    // Find successor and update distance of the route
    successor = pdm->updateRoute(route, dmin, &rtableChanged, true);
    if (source->isUnreachable())
        invalidateRoute(source);

    // Send Reply to old successor
    if (oldSuccessor != nullptr) {
        if (successor == nullptr) { // Send old Successor
            // Route will be removed after router receives Ack from neighbor for Reply
            if (oldSuccessor->isUnreachable())
                pdm->setDelayedRemove(oldSuccessor->getNexthopId(), oldSuccessor);
            pdm->sendReply(route, oldSuccessor->getNexthopId(), oldSuccessor, true);
        }
        else
            pdm->sendReply(route, oldSuccessor->getNexthopId(), successor, true);
    }

    // Send update about change of metric only if there is successor
    if (successor != nullptr && pdm->hasNeighborForUpdate(successor)) {
        if (rtableChanged)
            pdm->sendUpdate(IEigrpPdm<IPAddress>::UNSPEC_RECEIVER, route, successor, true, "RT changed");
        else if (route->getDij() != oldDij) {
            bool forcePoisonRev = successor->getNexthopId() == IEigrpPdm<IPAddress>::CONNECTED_ROUTE;
            pdm->sendUpdate(IEigrpPdm<IPAddress>::UNSPEC_RECEIVER, route, successor, forcePoisonRev, "metric changed");
        }
    }

    pdm->sendUpdateToStubs(successor, oldSuccessor, route);
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processTransition14(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, uint64_t dmin, int neighborId)
{
    EV_DEBUG << "DUAL: transit from oij=0 (active) to oij=1 (passive) by transition 14" << endl;

    EigrpRouteSource<IPAddress> *successor = nullptr, *oldSuccessor = nullptr;
    uint64_t oldDij = route->getDij();
    bool rtableChanged = false;

    route->setQueryOrigin(1);
    oldSuccessor = pdm->getBestSuccessor(route);

    // Find successor and update distance of the route
    successor = pdm->updateRoute(route, dmin, &rtableChanged, true);
    if (source->isUnreachable()) { // There is not necessary wait for Ack from neighbor before delete source (Reply is not sent)
        invalidateRoute(source);
    }

    // Do not send Reply (Reply was sent in transition 4)

    // Send update about change of metric
    if (successor != nullptr && pdm->hasNeighborForUpdate(successor)) {
        if (rtableChanged)
            pdm->sendUpdate(IEigrpPdm<IPAddress>::UNSPEC_RECEIVER, route, successor, true, "RT changed");
        else if (route->getDij() != oldDij) {
            bool forcePoisonRev = successor->getNexthopId() == IEigrpPdm<IPAddress>::CONNECTED_ROUTE;
            pdm->sendUpdate(IEigrpPdm<IPAddress>::UNSPEC_RECEIVER, route, successor, forcePoisonRev, "metric changed");
        }
    }

    pdm->sendUpdateToStubs(successor, oldSuccessor, route);
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processTransition15(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId)
{
    EV_DEBUG << "DUAL: transit from oij=1 (active) to oij=1 (passive) by transition 15" << endl;

    EigrpRouteSource<IPAddress> *successor = nullptr, *oldSuccessor = nullptr;
    uint64_t oldDij = route->getDij();
    uint64_t dmin;
    bool rtableChanged = false;

    // Set FD to max
    route->setFd(EigrpMetricHelper::METRIC_INF);

    oldSuccessor = pdm->getBestSuccessor(route);

    if (oldSuccessor == nullptr) { // could happen due to different order of signals (vs 5.0 ANSAversion)
        return;
    }

    // Find min distance
    dmin = pdm->findRouteDMin(route);

    successor = pdm->updateRoute(route, dmin, &rtableChanged, true);
    if (source->isUnreachable()) { // There is not necessary wait for Ack from neighbor before delete source (Reply is not sent)
        invalidateRoute(source);
    }

    // Do not send Reply (Reply was sent in transition 4)

    // Send update about change
    if (successor != nullptr && pdm->hasNeighborForUpdate(successor)) {
        if (rtableChanged)
            pdm->sendUpdate(IEigrpPdm<IPAddress>::UNSPEC_RECEIVER, route, successor, true, "RT changed");
        else if (route->getDij() != oldDij) {
            bool forcePoisonRev = successor->getNexthopId() == IEigrpPdm<IPAddress>::CONNECTED_ROUTE;
            pdm->sendUpdate(IEigrpPdm<IPAddress>::UNSPEC_RECEIVER, route, successor, forcePoisonRev, "metric changed");
        }
    }

    pdm->sendUpdateToStubs(successor, oldSuccessor, route);
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processTransition16(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, uint64_t dmin, int neighborId)
{
    EV_DEBUG << "DUAL: transit from oij=2 (active) to oij=1 (passive) by transition 16" << endl;

    EigrpRouteSource<IPAddress> *successor;
    // Old successor is originator of Query
    EigrpRouteSource<IPAddress> *oldSuccessor = pdm->getBestSuccessor(route);
    uint64_t oldDij = route->getDij();
    bool rtableChanged = false;

    route->setQueryOrigin(1);

    // Find successor and update distance of the route
    successor = pdm->updateRoute(route, dmin, &rtableChanged, true);
    ASSERT(successor != nullptr); // There must be successor

    if (source->isUnreachable()) { // There is not necessary wait for Ack from neighbor for Reply before delete source (successor may not be nullptr)
        invalidateRoute(source);
    }

    // Send Reply to old successor
    if (oldSuccessor != nullptr)
        pdm->sendReply(route, oldSuccessor->getNexthopId(), successor, true);

    // Send update about change of metric
    if (pdm->hasNeighborForUpdate(successor)) {
        if (rtableChanged)
            pdm->sendUpdate(IEigrpPdm<IPAddress>::UNSPEC_RECEIVER, route, successor, true, "RT changed");
        else if (route->getDij() != oldDij) {
            bool forcePoisonRev = successor->getNexthopId() == IEigrpPdm<IPAddress>::CONNECTED_ROUTE;
            pdm->sendUpdate(IEigrpPdm<IPAddress>::UNSPEC_RECEIVER, route, successor, forcePoisonRev, "metric changed");
        }
    }

    pdm->sendUpdateToStubs(successor, oldSuccessor, route);
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processTransition17(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId)
{
    EV_DEBUG << "DUAL: transit from oij=" << route->getQueryOrigin() << " (active) to oij=" << route->getQueryOrigin() << " (active) by transition 17" << endl;

    // Do not actualize Dij of route by new distance via successor (in transition to passive state DUAL can not detect change of Dij)
//    route->setDij(source->getMetric());
}

template<typename IPAddress>
void EigrpDual<IPAddress>::processTransition18(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId, bool isSourceNew)
{
    EV_DEBUG << "DUAL: transit from oij=" << route->getQueryOrigin() << " (active) to oij=" << route->getQueryOrigin() << " (active) by transition 18" << endl;

    if (source->isUnreachable() && isSourceNew)
        invalidateRoute(source);
}

template class EigrpDual<Ipv4Address>;

#ifndef DISABLE_EIGRP_IPV6
template class EigrpDual<Ipv6Address>;
#endif /* DISABLE_EIGRP_IPV6 */
} // eigrp
} // inet

