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
// Authors: Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz), Veronika Rybova, Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)

#include "IPv4ControlInfo.h"
#include "IPv4Datagram.h"
#include "IPv4InterfaceData.h"
#include "NotifierConsts.h"
#include "PIMSM.h"

Define_Module(PIMSM);

typedef IPv4MulticastRoute::OutInterface OutInterface;

PIMSM::Route::Route(PIMSM *owner, RouteType type, IPv4Address origin, IPv4Address group)
    : RouteEntry(owner, origin, group), type(type), rpAddr(IPv4Address::UNSPECIFIED_ADDRESS),
      rpRoute(NULL), gRoute(NULL), sgrptRoute(NULL),
      sequencenumber(0), keepAliveTimer(NULL), joinTimer(NULL), registerState(RS_NO_INFO), registerStopTimer(NULL),
      upstreamInterface(NULL)
{
}

PIMSM::Route::~Route()
{
    owner->cancelAndDelete(keepAliveTimer);
    owner->cancelAndDelete(registerStopTimer);
    owner->cancelAndDelete(joinTimer);
    delete upstreamInterface;
    for (DownstreamInterfaceVector::iterator it = downstreamInterfaces.begin(); it != downstreamInterfaces.end(); ++it)
        delete *it;
}

PIMSM::DownstreamInterface *PIMSM::Route::addNewDownstreamInterface(InterfaceEntry *ie)
{
    DownstreamInterface *downstream = new DownstreamInterface(this, ie, DownstreamInterface::NO_INFO);
    //downstream->startExpiryTimer(holdTime);
    addDownstreamInterface(downstream);

    IPv4MulticastRoute *ipv4Route = pimsm()->findIPv4Route(source, group);
    ipv4Route->addOutInterface(new PIMSMOutInterface(downstream));

    return downstream;
}


PIMSM::DownstreamInterface *PIMSM::Route::findDownstreamInterfaceByInterfaceId(int interfaceId)
{
    for (unsigned int i = 0; i < downstreamInterfaces.size(); i++)
    {
        DownstreamInterface *downstream = downstreamInterfaces[i];
        if (downstream->getInterfaceId() == interfaceId)
            return downstream;
    }
    return NULL;
}

int PIMSM::Route::findDownstreamInterface(InterfaceEntry *ie)
{
    for (unsigned int i = 0; i < downstreamInterfaces.size(); i++)
    {
        DownstreamInterface *downstream = downstreamInterfaces[i];
        if (downstream->ie == ie)
            return i;
    }
    return -1;
}

bool PIMSM::Route::isOilistNull()
{
    for (unsigned int i = 0; i < downstreamInterfaces.size(); i++)
        if (downstreamInterfaces[i]->isInOlist())
            return false;
    return true;
}

bool PIMSM::Route::isImmediateOlistNull()
{
    for (unsigned int i = 0; i < downstreamInterfaces.size(); i++)
        if (downstreamInterfaces[i]->isInImmediateOlist())
            return false;
    return true;
}

bool PIMSM::Route::isInheritedOlistNull()
{
    for (unsigned int i = 0; i < downstreamInterfaces.size(); i++)
        if (downstreamInterfaces[i]->isInInheritedOlist())
            return false;
    return true;
}

void PIMSM::Route::updateJoinDesired()
{
    bool oldValue = joinDesired();
    bool newValue = false;
    if (type == RP)
        newValue = !isImmediateOlistNull();
    if (type == G)
        newValue = !isImmediateOlistNull() /* || JoinDesired(*,*,RP(G)) AND AssertWinner(*,G,RPF_interface(RP(G))) != NULL */;
    else if (type == SG)
        newValue = !isImmediateOlistNull() || (keepAliveTimer && isInheritedOlistNull());

    if (newValue != oldValue)
    {
        setFlag(JOIN_DESIRED, newValue);
        pimsm()->joinDesiredChanged(this);
    }
}

PIMSM::~PIMSM()
{
    for (RoutingTable::iterator it = routes.begin(); it != routes.end(); ++it)
        delete it->second;
}

void PIMSM::handleMessage(cMessage *msg)
{
	if (msg->isSelfMessage())
	{
	   processPIMTimer(msg);
	}
	else if (dynamic_cast<PIMPacket *>(msg))
	{
	   PIMPacket *pkt = check_and_cast<PIMPacket *>(msg);
	   EV << "Version: " << pkt->getVersion() << ", type: " << pkt->getType() << endl;
	   processPIMPacket(pkt);
	}
	else
	   EV << "PIMSM::handleMessage: Wrong message" << endl;
}

void PIMSM::initialize(int stage)
{
    PIMBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        setRPAddress(par("RP").stdstringValue());
        setSPTthreshold(par("sptThreshold").stdstringValue());
        joinPrunePeriod = par("joinPrunePeriod");
        defaultOverrideInterval = par("defaultOverrideInterval");
        defaultPropagationDelay = par("defaultPropagationDelay");
        keepAlivePeriod = par("keepAlivePeriod");
        rpKeepAlivePeriod = par("rpKeepAlivePeriod");
        registerSuppressionTime = par("registerSuppressionTime");
        registerProbeTime = par("registerProbeTime");
        assertTime = par("assertTime");
        assertOverrideInterval = par("assertOverrideInterval");
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS)
    {
        // is PIM enabled?
        if (pimIft->getNumInterfaces() == 0)
        {
            EV << "PIM is NOT enabled on device " << endl;
            return;
        }
        else
        {
            // subscribe for notifications
            cModule *host = findContainingNode(this);
            if (host != NULL) {
                host->subscribe(NF_IPv4_NEW_MULTICAST, this);
                host->subscribe(NF_IPv4_MDATA_REGISTER, this);
                host->subscribe(NF_IPv4_DATA_ON_RPF, this);
                host->subscribe(NF_IPv4_DATA_ON_NONRPF, this);
                host->subscribe(NF_IPv4_MCAST_REGISTERED, this);
                host->subscribe(NF_IPv4_MCAST_UNREGISTERED, this);
            }
        }
    }
}

/**
 * DATA ON RPF INTERFACE
 *
 * The method process notification about data which appears on RPF interface. It means that source
 * is still active. The result is resetting of Keep Alive Timer. Also if first data packet arrive to
 * last hop router in RPT, switchover to SPT has to be considered.
 *
 * @param newRoute Pointer to new entry in the multicast routing table.
 * @see PIMkat()
 * @see getSPTthreshold()
 */
void PIMSM::multicastPacketArrivedOnRpfInterface(Route *route)
{
    if (route->type == SG)                          // (S,G) route
    {
        // set KeepAlive timer
        if (/*DirectlyConnected(route->source) ||*/
            (!route->isFlagSet(Route::PRUNED) && !route->isInheritedOlistNull()))
        {
            EV_DETAIL << "Data arrived on RPF interface, restarting KAT(" << route->source << ", " << route->group << ") timer.\n";

            if (!route->keepAliveTimer)
                route->startKeepAliveTimer();
            else
                restartTimer(route->keepAliveTimer, keepAlivePeriod);
        }

        // Update SPT bit

        /* TODO
             void
             Update_SPTbit(S,G,iif) {
               if ( iif == RPF_interface(S)
                     AND JoinDesired(S,G) == TRUE
                     AND ( DirectlyConnected(S) == TRUE
                           OR RPF_interface(S) != RPF_interface(RP(G))
                           OR inherited_olist(S,G,rpt) == NULL
                           OR ( ( RPF'(S,G) == RPF'(*,G) ) AND
                                ( RPF'(S,G) != NULL ) )
                           OR ( I_Am_Assert_Loser(S,G,iif) ) {
                  Set SPTbit(S,G) to TRUE
               }
             }
         */
         route->setFlags(Route::SPT_BIT);
    }

    // check switch from RP tree to the SPT
    if (route->isFlagSet(Route::SPT_BIT))
    {
        /*
             void
             CheckSwitchToSpt(S,G) {
               if ( ( pim_include(*,G) (-) pim_exclude(S,G)
                      (+) pim_include(S,G) != NULL )
                    AND SwitchToSptDesired(S,G) ) {
                      # Note: Restarting the KAT will result in the SPT switch
                      set KeepaliveTimer(S,G) to Keepalive_Period
               }
             }
         */
    }

    //TODO SPT threshold at last hop router
    if (route->isFlagSet(Route::CONNECTED))
    {
        if (this->getSPTthreshold() != "infinity")
            EV << "pimSM::dataOnRpf - Last hop router should to send Join(S,G)" << endl;
        else
            EV << "pimSM::dataOnRpf - SPT threshold set to infinity" << endl;

    }
}

/**
 * SET RP ADDRESS
 *
 * The method is used to set RP address from configuration.
 *
 * @param address in string format.
 * @see getRPAddress()
 */
void PIMSM::setRPAddress(std::string address)
{
    if (address != "")
    {
        std::string RP (address);
        rpAddr = IPv4Address(RP.c_str());
    }
    else
        EV << "PIMSM::setRPAddress: empty RP address" << endl;
}

/**
 * SET SPT threshold
 *
 * The method is used to set SPT threshold from configuration.
 *
 * @param threshold in string format.
 * @see getSPTAddress()
 */
void PIMSM::setSPTthreshold(std::string threshold)
{
    if (threshold != "")
        sptThreshold.append(threshold);
    else
        EV << "PIMSM::setSPTthreshold: bad SPTthreshold" << endl;
}

/**
 * The method is used to determine if router is Designed Router for given IP address.
 */
bool PIMSM::IamDR (IPv4Address source)
{
    for (int i=0; i < ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        IPv4Address localAddr = ie->ipv4Data()->getIPAddress();
        IPv4Address netmask = ie->ipv4Data()->getNetmask();

        if (IPv4Address::maskedAddrAreEqual(source, localAddr, netmask))
            return true;
    }

    return false;
}

/**
 * The method is used to process PIM Keep Alive Timer. It is (S,G) timer. When Keep Alive Timer expires,
 * route is removed from multicast routing table.
 */
void PIMSM::processKeepAliveTimer(cMessage *timer)
{
    EV << "pimSM::processKeepAliveTimer: route will be deleted" << endl;
    Route *route = static_cast<Route*>(timer->getContextPointer());
    ASSERT(route->type == SG);
    ASSERT(timer == route->keepAliveTimer);

    delete timer;
    route->keepAliveTimer = NULL;
    deleteMulticastRoute(route);
}

/**
 * The method is used to process PIM Register Stop Timer. RST is used to send
 * periodic Register-Null messages
 */
void PIMSM::processRegisterStopTimer(cMessage *timer)
{
    EV << "pimSM::processRegisterStopTimer: " << endl;
    Route *routeSG = static_cast<Route*>(timer->getContextPointer());
    ASSERT(timer == routeSG->registerStopTimer);
    ASSERT(routeSG->type == SG);

    delete timer;
    routeSG->registerStopTimer = NULL;

    if (routeSG->registerState == Route::RS_PRUNE)
    {
        routeSG->registerState = Route::RS_JOIN_PENDING;
        sendPIMRegisterNull(routeSG->source, routeSG->group);
        routeSG->startRegisterStopTimer(registerProbeTime);
    }
    else if (routeSG->registerState == Route::RS_JOIN_PENDING)
    {
        routeSG->registerState = Route::RS_JOIN;
    }
}

/**
 * PROCESS EXPIRY TIMER
 *
 * The method is used to process PIM Expiry Timer. It is timer for (S,G) and (*,G).
 * When Expiry timer expires,route is removed from multicast routing table.
 *
 * @param timer Pointer to Keep Alive Timer.
 * @see PIMet()
 */
void PIMSM::processExpiryTimer(cMessage *timer)
{
    EV << "pimSM::processExpiryTimer: " << endl;

    PimsmInterface *interface = static_cast<PimsmInterface*>(timer->getContextPointer());
    Route *route = interface->route();

    if (interface != route->upstreamInterface)
    {
        //
        // Downstream Join/Prune State Machine; event: ET expires
        //
        DownstreamInterface *downstream = check_and_cast<DownstreamInterface*>(interface);
        downstream->joinPruneState = DownstreamInterface::NO_INFO;
        cancelAndDeleteTimer(downstream->prunePendingTimer);
        downstream->expiryTimer = NULL;
        delete timer;

        // upstream state machine
        if (route->isOilistNull())
        {
            route->clearFlag(Route::CONNECTED);
            route->setFlags(Route::PRUNED);
            PIMNeighbor *RPFneighbor = pimNbt->getFirstNeighborOnInterface(route->upstreamInterface->getInterfaceId());
            if (route->type == G && !IamRP(route->rpAddr))
                sendPIMPrune(route->group, route->rpAddr, RPFneighbor->getAddress(), G);
            else if (route->type == SG)
                sendPIMPrune(route->group, route->source, RPFneighbor->getAddress(), SG);

            cancelAndDeleteTimer(route->joinTimer);
        }
    }
    if (route->upstreamInterface->expiryTimer && interface == route->upstreamInterface)
    {
        for(unsigned i=0; i<route->downstreamInterfaces.size();)
        {
            if (route->downstreamInterfaces[i]->expiryTimer)
                route->removeDownstreamInterface(i);
            else
                i++;
        }
        if (IamRP(this->getRPAddress()) && route->type == G)
        {
            EV << "ET for (*,G) route on RP expires - go to stopped" << endl;
            cancelAndDeleteTimer(route->upstreamInterface->expiryTimer);
        }
        else
            deleteMulticastRoute(route);
    }
}

/**
 * PROCESS JOIN TIMER
 *
 * The method is used to process PIM Join Timer. It is timer for (S,G) and (*,G).
 * When Join Timer expires, periodic Join message is sent to upstream neighbor.
 *
 * @param timer Pointer to Join Timer.
 * @see PIMjt()
 */
void PIMSM::processJoinTimer(cMessage *timer)
{
    EV << "pimSM::processJoinTimer:" << endl;

    Route *route = static_cast<Route*>(timer->getContextPointer());
    ASSERT(timer == route->joinTimer);
    IPv4Address joinAddr = route->type == G ? route->rpAddr : route->source;

    if (!route->isOilistNull())
    {
        sendPIMJoin(route->group, joinAddr, route->upstreamInterface->nextHop, route->type);
        restartTimer(route->joinTimer, joinPrunePeriod);
    }
    else
    {
        delete timer;
        route->joinTimer = NULL;
    }
}

/**
 * PROCESS PRUNE PENDING TIMER
 *
 * The method is used to process PIM Prune Pending Timer.
 * Prune Pending Timer is used for delaying of Prune message sending
 * (for possible overriding Join from another PIM neighbor)
 *
 * @param timer Pointer to Prune Pending Timer.
 * @see PIMppt()
 */
void PIMSM::processPrunePendingTimer(cMessage *timer)
{
    EV << "pimSM::processPrunePendingTimer:" << endl;
    DownstreamInterface *downstream = static_cast<DownstreamInterface*>(timer->getContextPointer());
    ASSERT(timer == downstream->prunePendingTimer);
    ASSERT(downstream->joinPruneState == DownstreamInterface::PRUNE_PENDING);
    Route *route = downstream->route();

    if (route->type == G || route->type == SG)
    {
        //
        // Downstream (*,G)/(S,G) Join/Prune State Machine; event: PPT expires
        //

        // go to NO_INFO state
        downstream->joinPruneState = DownstreamInterface::NO_INFO;
        cancelAndDeleteTimer(downstream->expiryTimer);
        downstream->prunePendingTimer = NULL;
        delete timer;

        // optionally send PruneEcho message
        if (pimNbt->getNumNeighborsOnInterface(downstream->ie->getInterfaceId()) > 1)
        {
            // A PruneEcho is simply a Prune message sent by the
            // upstream router on a LAN with its own address in the Upstream
            // Neighbor Address field.  Its purpose is to add additional
            // reliability so that if a Prune that should have been
            // overridden by another router is lost locally on the LAN, then
            // the PruneEcho may be received and cause the override to happen.
            IPv4Address pruneAddr = route->type == G ? route->rpAddr : route->source;
            IPv4Address upstreamNeighborField = downstream->ie->ipv4Data()->getIPAddress();
            sendPIMPrune(route->group, pruneAddr, upstreamNeighborField, route->type);
        }
    }
    else if (route->type == SGrpt)
    {
        //
        // Downstream (S,G,rpt) Join/Prune State Machine; event: PPT expires
        //

        // go to PRUNE state
        // TODO
    }

    // Now check upstream state transitions

    // old code
//    IPv4Address pruneAddr = route->type == G ? route->rpAddr : route->origin;
//    PIMNeighbor *neighbor = pimNbt->getFirstNeighborOnInterface(route->upstreamInterface->getInterfaceId()); // XXX why not nextHop?
//
//    if ((route->type == G && !IamRP(this->getRPAddress())) || route->type == SG)
//        sendPIMPrune(route->group, pruneAddr, neighbor->getAddress(), route->type);

}

void PIMSM::processAssertTimer(cMessage *timer)
{
    PimsmInterface *interfaceData = static_cast<PimsmInterface*>(timer->getContextPointer());
    ASSERT(timer == interfaceData->assertTimer);
    ASSERT(interfaceData->assertState != Interface::NO_ASSERT_INFO);

    Route *route = interfaceData->route();
    if (route->type == SG || route->type == G)
    {
        //
        // (S,G) Assert State Machine; event: AT(S,G,I) expires OR
        // (*,G) Assert State Machine; event: AT(*,G,I) expires
        //
        EV_DETAIL << "AssertTimer(" << (route->type==G?"*":route->source.str()) << ", "
                  << route->group << ", " << interfaceData->ie->getName() << ") has expired.\n";

        if (interfaceData->assertState == Interface::I_WON_ASSERT)
        {
            // The (S,G) or (*,G) Assert Timer expires.  As we're in the Winner state,
            // we must still have (S,G) or (*,G) forwarding state that is actively
            // being kept alive.  We resend the (S,G) or (*,G) Assert and restart the
            // Assert Timer.  Note that the assert
            // winner's Assert Timer is engineered to expire shortly before
            // timers on assert losers; this prevents unnecessary thrashing
            // of the forwarder and periodic flooding of duplicate packets.
            sendPIMAssert(route->source, route->group, route->metric, interfaceData->ie, route->type == G);
            interfaceData->startAssertTimer(assertTime - assertOverrideInterval);
        }
        else if (interfaceData->assertState == Interface::I_LOST_ASSERT)
        {
            // The (S,G) or (*,G) Assert Timer expires.  We transition to NoInfo
            // state, deleting the (S,G) or (*,G) assert information.
            EV_DEBUG << "Going into NO_ASSERT_INFO state.\n";
            interfaceData->deleteAssertInfo(); // deleted timer
            return;
        }
    }

    delete timer;
}


/**
 * The method is used to restart ET. ET is used for outgoing interfaces
 * and whole route in router. After ET expires, outgoing interface is
 * removed or if there aren't any outgoing interface, route is removed
 * after ET expires.
 */
void PIMSM::restartExpiryTimer(Route *route, InterfaceEntry *originIntf, int holdTime)
{
    EV << "pimSM::restartExpiryTimer: next ET @ " << simTime() + holdTime << " for type: ";

    if (route)
    {
        // ET for route
        if (route->upstreamInterface && route->upstreamInterface->expiryTimer)
            restartTimer(route->upstreamInterface->expiryTimer, holdTime);

        // ET for outgoing interfaces
        for (unsigned i=0; i< route->downstreamInterfaces.size(); i++)
        {   // if exist ET and for given interface
            DownstreamInterface *downstream = route->downstreamInterfaces[i];
            if (downstream->expiryTimer && (downstream->getInterfaceId() == originIntf->getInterfaceId()))
            {
                EV << /*timer->getStateType() << " , " <<*/ route->group << " , " << route->source << ", int: " << downstream->ie->getName() << endl;
                restartTimer(downstream->expiryTimer, holdTime);
                break;
            }
        }
    }
}

void PIMSM::processJoinPrunePacket(PIMJoinPrune *pkt)
{
    EV_INFO << "Received JoinPrune packet.\n";

    IPv4ControlInfo *ctrl = check_and_cast<IPv4ControlInfo*>(pkt->getControlInfo());
    InterfaceEntry *inInterface = ift->getInterfaceById(ctrl->getInterfaceId());
    int holdTime = pkt->getHoldTime();
    IPv4Address upstreamNeighbor = pkt->getUpstreamNeighborAddress();

    for (unsigned int i = 0; i < pkt->getMulticastGroupsArraySize(); i++)
    {
        MulticastGroup group = pkt->getMulticastGroups(i);
        IPv4Address groupAddr = group.getGroupAddress();

        // go through list of joined sources
        for (unsigned int j = 0; j < group.getJoinedSourceAddressArraySize(); j++)
        {
            EncodedAddress &source = group.getJoinedSourceAddress(j);
            if (source.S)
            {
                if (source.W)      // (*,G) Join
                    processJoinG(groupAddr, source.IPaddress, upstreamNeighbor, holdTime, inInterface);
                else if (source.R) // (S,G,rpt) Join
                    processJoinSGrpt(source.IPaddress, groupAddr, upstreamNeighbor, holdTime, inInterface);
                else               // (S,G) Join
                    processJoinSG(source.IPaddress, groupAddr, upstreamNeighbor, holdTime, inInterface);
            }
        }

        // go through list of pruned sources
        for(unsigned int j = 0; j < group.getPrunedSourceAddressArraySize(); j++)
        {
            EncodedAddress &source = group.getPrunedSourceAddress(j);
            if (source.S)
            {
                if (source.W)      // (*,G) Prune
                    processPruneG(groupAddr, upstreamNeighbor, inInterface);
                else if (source.R) // (S,G,rpt) Prune
                    processPruneSGrpt(source.IPaddress, groupAddr, upstreamNeighbor, inInterface);
                else               // (S,G) Prune
                    processPruneSG(source.IPaddress, groupAddr, upstreamNeighbor, inInterface);
            }
        }
    }

    delete pkt;
}

void PIMSM::processJoinG(IPv4Address group, IPv4Address rp, IPv4Address upstreamNeighborField, int holdTime, InterfaceEntry *inInterface)
{
    EV_DETAIL << "Processing Join(*," << group << ") received on interface '" << inInterface->getName() << "'.\n";

    // TODO RP check

    //
    // Downstream per-interface (*,G) state machine; event = Receive Join(*,G)
    //

    // check UpstreamNeighbor field
    if (upstreamNeighborField != inInterface->ipv4Data()->getIPAddress())
        return;

    bool newRoute = false;
    Route *routeG = findRouteG(group);
    if (!routeG)
    {
        routeG = createRouteG(group, Route::PRUNED);
        addRouteG(routeG);
        newRoute = true;
    }

    DownstreamInterface *downstream = routeG->findDownstreamInterfaceByInterfaceId(inInterface->getInterfaceId());
    if (!downstream && (!routeG->upstreamInterface || inInterface != routeG->upstreamInterface->ie))
        downstream = routeG->addNewDownstreamInterface(inInterface);

    if (downstream)
    {
        // A Join(*,G) is received on interface I with its Upstream
        // Neighbor Address set to the router's primary IP address on I.
        if (downstream->joinPruneState == DownstreamInterface::NO_INFO)
        {
            // The (*,G) downstream state machine on interface I transitions
            // to the Join state.  The Expiry Timer (ET) is started and set
            // to the HoldTime from the triggering Join/Prune message.
            downstream->joinPruneState = DownstreamInterface::JOIN;
            downstream->startExpiryTimer(holdTime);
        }
        else if (downstream->joinPruneState == DownstreamInterface::JOIN)
        {
            // The (*,G) downstream state machine on interface I remains in
            // Join state, and the Expiry Timer (ET) is restarted, set to
            // maximum of its current value and the HoldTime from the
            // triggering Join/Prune message.
            if (simTime() + holdTime > downstream->expiryTimer->getArrivalTime())
                restartTimer(downstream->expiryTimer, holdTime);
        }
        else if (downstream->joinPruneState == DownstreamInterface::PRUNE_PENDING)
        {
            // The (*,G) downstream state machine on interface I transitions
            // to the Join state.  The Prune-Pending Timer is canceled
            // (without triggering an expiry event).  The Expiry Timer is
            // restarted, set to maximum of its current value and the
            // HoldTime from the triggering Join/Prune message.
            cancelAndDeleteTimer(downstream->prunePendingTimer);
            if (simTime() + holdTime > downstream->expiryTimer->getArrivalTime())
                restartTimer(downstream->expiryTimer, holdTime);
        }
    }

    // XXX Join(*,G) messages can affect (S,G,rpt) downstream state machines too

    // check upstream state transition
    routeG->updateJoinDesired();

    if (!newRoute && !routeG->upstreamInterface) // I am RP
    {
        routeG->clearFlag(Route::REGISTER);

        for (RoutingTable::iterator it = routes.begin(); it != routes.end(); ++it)
        {
            Route *route = it->second;
            if (route->group == group && route->sequencenumber == 0)   // only check if route was installed
            {
                if (route->type == SG)
                {
                    // update flags
                    route->clearFlag(Route::PRUNED);
                    route->setFlags(Route::SPT_BIT);
                    route->startJoinTimer();

                    if (route->downstreamInterfaces.empty())          // Has route any outgoing interface?
                    {
                        downstream = route->addNewDownstreamInterface(inInterface);
                        downstream->joinPruneState = DownstreamInterface::JOIN;
                        downstream->startExpiryTimer(holdTime);
                        sendPIMJoin(group, route->source, route->upstreamInterface->nextHop, SG);
                    }
                }
                route->sequencenumber = 1;
            }
        }
    }



//    if (!routeG)                                // check if (*,G) exist
//    {
//        InterfaceEntry *rpfInterface = rt->getInterfaceForDestAddr(rp);
//        if (inInterface != rpfInterface)
//        {
//            Route *newRouteG = createRouteG(group, Route::NO_FLAG);
//
//            if (!IamRP(newRouteG->rpAddr))
//                newRouteG->startJoinTimer();              // periodic Join (*,G)
//
//            DownstreamInterface *downstream = new DownstreamInterface(newRouteG, inInterface, DownstreamInterface::JOIN);
//            downstream->startExpiryTimer(holdTime);
//            newRouteG->addDownstreamInterface(downstream);
//
//            addGRoute(newRouteG);
//
//            if (newRouteG->upstreamInterface) // XXX should always have expiryTimer
//            {
//                newRouteG->upstreamInterface->startExpiryTimer(holdTime);
//                sendPIMJoin(group, newRouteG->rpAddr, newRouteG->upstreamInterface->rpfNeighbor(), G); // triggered Join (*,G)
//            }
//        }
//    }
//    else            // (*,G) route exist
//    {
//        //if (!routeG->isRpf(inInterface->getInterfaceId()))
//        if (!routeG->upstreamInterface || routeG->upstreamInterface->ie != inInterface)
//        {
//            if (IamRP(rp)) // (*,G) route exists at RP
//            {
//                for (SGStateMap::iterator it = routes.begin(); it != routes.end(); ++it)
//                {
//                    Route *route = it->second;
//                    if (route->group == group && route->sequencenumber == 0)   // only check if route was installed
//                    {
//                        if (route->type == SG)
//                        {
//                            // update flags
//                            route->clearFlag(Route::P);
//                            route->setFlags(Route::T);
//                            route->startJoinTimer();
//
//                            if (route->downstreamInterfaces.empty())          // Has route any outgoing interface?
//                            {
//                                route->addNewDownstreamInterface(inInterface, holdTime);
//                                sendPIMJoin(group, route->origin, route->upstreamInterface->nextHop, SG);
//                            }
//                        }
//                        else if (route->type == G)
//                        {
//                            route->clearFlag(Route::P);
//                            route->clearFlag(Route::F);
//                            cancelAndDeleteTimer(route->keepAliveTimer);
//
//                            if (route->findDownstreamInterface(inInterface) < 0)
//                            {
//                                route->addNewDownstreamInterface(inInterface, holdTime);
//                                if (route->upstreamInterface) // XXX should always have expiryTimer
//                                    route->upstreamInterface->startExpiryTimer(holdTime);
//                            }
//                        }
//
//                        route->sequencenumber = 1;
//                    }
//                }
//            }
//            else        // (*,G) route exist somewhere in RPT
//            {
//                if (routeG->findDownstreamInterface(inInterface) < 0)
//                    routeG->addNewDownstreamInterface(inInterface, holdTime);
//            }
//
//            // restart ET for given interface - for (*,G) and also (S,G)
//            restartExpiryTimer(routeG, inInterface, holdTime);
//            for (SGStateMap::iterator it = routes.begin(); it != routes.end(); ++it)
//            {
//                Route *routeSG = it->second;
//                if (routeSG->group == group && !routeSG->origin.isUnspecified())
//                {
//                    //restart ET for (S,G)
//                    restartExpiryTimer(routeSG, inInterface, holdTime);
//                }
//            }
//        }
//    }
}

/**
 * The method is used to process (S,G) Join PIM message. SG Join is process in
 * source tree between RP and source DR. If (S,G) route doesn't exist is created
 * along with (*,G) route. Otherwise outgoing interface and JT are created.
 */
void PIMSM::processJoinSG(IPv4Address source, IPv4Address group, IPv4Address upstreamNeighborField, int holdTime, InterfaceEntry *inInterface)
{
    EV_DETAIL << "Processing Join(" << source << ", " << group <<") received on interface '" << inInterface->getName() << "'.'n";

    if (!IamDR(source))
    {
        Route *routeG = findRouteG(group);
        if (!routeG)        // create (*,G) route between RP and source DR
        {
            Route *newRouteG = createRouteG(group, Route::PRUNED);
            addRouteG(newRouteG);
        }
    }

    //
    // Downstream per-interface (S,G) state machine; event = Receive Join(S,G)
    //

    // check UpstreamNeighbor field
    if (upstreamNeighborField != inInterface->ipv4Data()->getIPAddress())
        return;

    Route *routeSG = findRouteSG(source, group);
    if (!routeSG)         // create (S,G) route between RP and source DR
    {
        routeSG = createRouteSG(source, group, Route::PRUNED);
        routeSG->startKeepAliveTimer();
        addRouteSG(routeSG);
    }

    DownstreamInterface *downstream = routeSG->findDownstreamInterfaceByInterfaceId(inInterface->getInterfaceId());
    if (!downstream && inInterface != routeSG->upstreamInterface->ie)
        downstream = routeSG->addNewDownstreamInterface(inInterface);

    if (downstream)
    {
        // A Join(S,G) is received on interface I with its Upstream
        // Neighbor Address set to the router's primary IP address on I.
        if (downstream->joinPruneState == DownstreamInterface::NO_INFO)
        {
            // The (S,G) downstream state machine on interface I transitions
            // to the Join state.  The Expiry Timer (ET) is started and set
            // to the HoldTime from the triggering Join/Prune message.
            downstream->joinPruneState = DownstreamInterface::JOIN;
            downstream->startExpiryTimer(holdTime);
        }
        else if (downstream->joinPruneState == DownstreamInterface::JOIN)
        {
            // The (S,G) downstream state machine on interface I remains in
            // Join state, and the Expiry Timer (ET) is restarted, set to
            // maximum of its current value and the HoldTime from the
            // triggering Join/Prune message.
            if (simTime() + holdTime > downstream->expiryTimer->getArrivalTime())
                restartTimer(downstream->expiryTimer, holdTime);
        }
        else if (downstream->joinPruneState == DownstreamInterface::PRUNE_PENDING)
        {
            // The (S,G) downstream state machine on interface I transitions
            // to the Join state.  The Prune-Pending Timer is canceled
            // (without triggering an expiry event).  The Expiry Timer is
            // restarted, set to maximum of its current value and the
            // HoldTime from the triggering Join/Prune message.
            cancelAndDeleteTimer(downstream->prunePendingTimer);
            if (simTime() + holdTime > downstream->expiryTimer->getArrivalTime())
                restartTimer(downstream->expiryTimer, holdTime);
        }
    }

    // check upstream state transition
    routeSG->updateJoinDesired();
}


void PIMSM::processJoinSGrpt(IPv4Address source, IPv4Address group, IPv4Address upstreamNeighborField, int holdTime, InterfaceEntry *inInterface)
{
    // TODO
}

void PIMSM::processPruneG(IPv4Address group, IPv4Address upstreamNeighborField, InterfaceEntry *inInterface)
{
    EV_DETAIL << "Processing Prune(*," << group << ") received on interface '" << inInterface->getName() << "'.\n";

    //
    // Downstream per-interface (*,G) state machine; event = Receive Prune(*,G)
    //

    // TODO check RP

    // check UpstreamNeighbor field
    if (upstreamNeighborField != inInterface->ipv4Data()->getIPAddress())
        return;

    Route *routeG = findRouteG(group);
    if (routeG)
    {
        DownstreamInterface *downstream = routeG->findDownstreamInterfaceByInterfaceId(inInterface->getInterfaceId());
        if (downstream && downstream->joinPruneState == DownstreamInterface::JOIN)
        {
            downstream->joinPruneState = DownstreamInterface::PRUNE_PENDING;
            downstream->startPrunePendingTimer();
        }
    }

    // check upstream state transition
    if (routeG)
        routeG->updateJoinDesired();

//    for (SGStateMap::iterator it = routes.begin(); it != routes.end(); ++it)
//    {
//        Route *route = it->second;
//        if (route->group == group)
//        {
//            int k = route->findDownstreamInterface(inInterface);
//            if (k >= 0)
//            {
//                EV << "Interface is present, removing it from the list of outgoing interfaces." << endl;
//                route->removeDownstreamInterface(k);
//            }
//
//            if (route->isOilistNull() && !route->isFlagSet(Route::P))
//            {
//                route->clearFlag(Route::C);
//                route->setFlags(Route::P);
//                cancelAndDeleteTimer(route->joinTimer);
//                bool iAmRP = IamRP(route->rpAddr);
//                if ((route->type == G && !iAmRP) || (route->type == SG && iAmRP))
//                {
//#if CISCO_SPEC_SIM == 1
//                    PIMNeighbor *RPFnbr = pimNbt->getFirstNeighborOnInterface(route->upstreamInterface->getInterfaceId());
//                    sendPIMPrune(group, route->type == G ? route->rpAddr : route->origin, RPFnbr->getAddress(), route->type);
//#else
//                    // XXX route->startPrunePendingTimer();
//#endif
//                }
//            }
//        }
//    }
}

void PIMSM::processPruneSG(IPv4Address source, IPv4Address group, IPv4Address upstreamNeighborField, InterfaceEntry *inInterface)
{
    EV_DETAIL << "Processing Prune(" << source << ", " << group <<") received on interface '" << inInterface->getName() << "'.'n";


    //
    // Downstream per-interface (S,G) state machine; event = Receive Prune(S,G)
    //

    // TODO check RP

    // check UpstreamNeighbor field
    if (upstreamNeighborField != inInterface->ipv4Data()->getIPAddress())
        return;

    Route *routeSG = findRouteSG(source, group);
    if (routeSG)
    {
        DownstreamInterface *downstream = routeSG->findDownstreamInterfaceByInterfaceId(inInterface->getInterfaceId());
        if (downstream && downstream->joinPruneState == DownstreamInterface::JOIN)
        {
            downstream->joinPruneState = DownstreamInterface::PRUNE_PENDING;
            downstream->startPrunePendingTimer();
        }
    }

    //
    // Upstream (S,G) State Machine
    //

    // check upstream state transition
    if (routeSG)
        routeSG->updateJoinDesired();

//    // upstream state machine
//    if (routeSG && routeSG->isOilistNull() && !routeSG->isFlagSet(Route::P))
//    {
//        routeSG->setFlags(Route::P);
//        cancelAndDeleteTimer(routeSG->joinTimer);
//        if (!IamDR(source))
//        {
//#if CISCO_SPEC_SIM == 1
//            PIMNeighbor *RPFnbr = pimNbt->getFirstNeighborOnInterface(routeSG->upstreamInterface->getInterfaceId());
//            sendPIMPrune(group, source, RPFnbr->getAddress(), SG);
//#else
//            // XXX routeSG->startPrunePendingTimer();
//#endif
//        }
//    }
}

void PIMSM::processPruneSGrpt(IPv4Address source, IPv4Address group, IPv4Address upstreamNeighborField, InterfaceEntry *inInterface)
{
    // TODO
}

/**
 * The method is used for processing PIM Register message sent from source DR.
 * If PIM Register isn't Null and route doesn't exist, it is created and PIM Register-Stop
 * is sent. If PIM Register is Null, Register-Stop is sent.
 */
void PIMSM::processRegisterPacket(PIMRegister *pkt)
{
    EV_INFO << "Received Register packet.\n";

    IPv4Datagram *encapData = check_and_cast<IPv4Datagram*>(pkt->decapsulate());
    IPv4Address source = encapData->getSrcAddress();
    IPv4Address group = encapData->getDestAddress();
    Route *routeG = findRouteG(group);
    Route *routeSG = findRouteSG(source, group);

    if (!pkt->getN()) // It is Null Register ?
    {
        if (!routeG)
        {
            routeG = createRouteG(group, Route::PRUNED);
            addRouteG(routeG);
        }

        if (!routeSG)
        {
            routeSG = createRouteSG(source, group, Route::PRUNED);
            routeSG->startKeepAliveTimer();
            addRouteSG(routeSG);
        }
        else if (routeSG->keepAliveTimer)
        {
            EV << " (S,G) KAT timer refresh" << endl;
            restartTimer(routeSG->keepAliveTimer, KAT);
        }

        if (!routeG->isOilistNull()) // we have some active receivers
        {
            // copy out interfaces from newRouteG
            IPv4MulticastRoute *ipv4Route = findIPv4Route(routeSG->source, routeSG->group);
            routeSG->clearDownstreamInterfaces();
            ipv4Route->clearOutInterfaces();
            for (unsigned int i = 0; i < routeG->downstreamInterfaces.size(); i++)
            {
                DownstreamInterface *downstream = new DownstreamInterface(*(routeG->downstreamInterfaces[i])); // XXX
                downstream->owner = routeSG;
                downstream->expiryTimer = NULL;
                downstream->startExpiryTimer(joinPruneHoldTime());
                routeSG->addDownstreamInterface(downstream);
                ipv4Route->addOutInterface(new PIMSMOutInterface(downstream));
            }

            routeSG->clearFlag(Route::PRUNED);

            if (!routeSG->isFlagSet(Route::SPT_BIT)) // only if isn't build SPT between RP and registering DR
            {
                for (unsigned i=0; i < routeG->downstreamInterfaces.size(); i++)
                {
                    DownstreamInterface *downstream = routeG->downstreamInterfaces[i];
                    if (downstream->isInOlist())
                        forwardMulticastData(encapData->dup(), downstream->getInterfaceId());
                }

                // send Join(S,G) toward source to establish SPT between RP and registering DR
                sendPIMJoin(group, source, routeSG->upstreamInterface->rpfNeighbor(), SG);

                // send register-stop packet
                IPv4ControlInfo *ctrlInfo = check_and_cast<IPv4ControlInfo*>(pkt->getControlInfo());
                sendPIMRegisterStop(ctrlInfo->getDestAddr(), ctrlInfo->getSrcAddr(), group, source);
            }
        }
    }

    if (routeG)
    {
        if (routeG->isFlagSet(Route::PRUNED) || pkt->getN())
        {
            IPv4ControlInfo *ctrlInfo = check_and_cast<IPv4ControlInfo*>(pkt->getControlInfo());
            sendPIMRegisterStop(ctrlInfo->getDestAddr(), ctrlInfo->getSrcAddr(), group, source);
        }
    }

    delete encapData;
    delete pkt;
}

/**
 * The method is used for processing PIM Register-Stop message sent from RP.
 * If the message is received Register Tunnel between RP and source DR is set
 * from Join status revert to Prune status. Also Register Stop Timer is created
 * for periodic sending PIM Register Null messages.
 */
void PIMSM::processRegisterStopPacket(PIMRegisterStop *pkt)
{
    EV_INFO << "Received RegisterStop packet.\n";

    // TODO support wildcard source address
    Route *routeSG = findRouteSG(pkt->getSourceAddress(), pkt->getGroupAddress());
    if (routeSG)
    {
        if (routeSG->registerState == Route::RS_JOIN || routeSG->registerState == Route::RS_JOIN_PENDING)
        {
            routeSG->registerState = Route::RS_PRUNE;
            cancelAndDeleteTimer(routeSG->registerStopTimer);

            // The Register-Stop Timer is set to a random value chosen
            // uniformly from the interval ( 0.5 * Register_Suppression_Time,
            // 1.5 * Register_Suppression_Time) minus Register_Probe_Time.
            // Subtracting off Register_Probe_Time is a bit unnecessary because
            // it is really small compared to Register_Suppression_Time, but
            // this was in the old spec and is kept for compatibility.
#if CISCO_SPEC_SIM == 1
            routeSG->startRegisterStopTimer(registerSuppressionTime);
#else
            routeSG->startRegisterStopTimer(uniform(0.5*registerSuppressionTime, 1.5*registerSuppressionTime) - registerProbeTime);
#endif
        }
    }
}

void PIMSM::processAssertPacket(PIMAssert *pkt)
{
    IPv4ControlInfo *ctrlInfo = check_and_cast<IPv4ControlInfo*>(pkt->getControlInfo());
    int incomingInterfaceId = ctrlInfo->getInterfaceId();
    IPv4Address source = pkt->getSourceAddress();
    IPv4Address group = pkt->getGroupAddress();
    AssertMetric receivedMetric = AssertMetric(pkt->getR(), pkt->getMetricPreference(), pkt->getMetric(), ctrlInfo->getSrcAddr());

    EV_INFO << "Received Assert(" << (source.isUnspecified() ? "*" : source.str()) << ", " << group << ")"
            << " packet on interface '" << ift->getInterfaceById(incomingInterfaceId)->getName() <<"'.\n";

    if (!source.isUnspecified() && !receivedMetric.rptBit)
    {
        Route *routeSG = findRouteSG(source, group);
        if (routeSG)
        {
            PimsmInterface *incomingInterface = routeSG->upstreamInterface->getInterfaceId() == incomingInterfaceId ?
                                               static_cast<PimsmInterface*>(routeSG->upstreamInterface) :
                                               static_cast<PimsmInterface*>(routeSG->findDownstreamInterfaceByInterfaceId(incomingInterfaceId));

            Interface::AssertState stateBefore = incomingInterface->assertState;
            processAssertSG(incomingInterface, receivedMetric);

            if (stateBefore != Interface::NO_ASSERT_INFO || incomingInterface->assertState != Interface::NO_ASSERT_INFO)
            {
                // processed by SG
                delete pkt;
                return;
            }
        }

    }

    // process (*,G) asserts and (S,G) asserts for which there is no assert state in (S,G) routes
    Route *routeG = findRouteG(group);
    PimsmInterface *incomingInterface = routeG->upstreamInterface->getInterfaceId() == incomingInterfaceId ?
                                       static_cast<PimsmInterface*>(routeG->upstreamInterface) :
                                       static_cast<PimsmInterface*>(routeG->findDownstreamInterfaceByInterfaceId(incomingInterfaceId));
    processAssertG(incomingInterface, receivedMetric);

    delete pkt;
}

void PIMSM::processAssertSG(PimsmInterface *interface, const AssertMetric &receivedMetric)
{
    Route *routeSG = interface->route();
    AssertMetric myMetric = interface->couldAssert ? // XXX check routeG metric too
                                routeSG->metric.setAddress(interface->ie->ipv4Data()->getIPAddress()) :
                                AssertMetric::INFINITE;

    // A "preferred assert" is one with a better metric than the current winner.
    bool isPreferredAssert = receivedMetric < interface->winnerMetric;

    // An "acceptable assert" is one that has a better metric than my_assert_metric(S,G,I).
    // An assert is never considered acceptable if its metric is infinite.
    bool isAcceptableAssert = receivedMetric < myMetric;

    // An "inferior assert" is one with a worse metric than my_assert_metric(S,G,I).
    // An assert is never considered inferior if my_assert_metric(S,G,I) is infinite.
    bool isInferiorAssert = myMetric < receivedMetric;

    if (interface->assertState == Interface::NO_ASSERT_INFO)
    {
        if (isInferiorAssert && !receivedMetric.rptBit && interface->couldAssert)
        {
            // An assert is received for (S,G) with the RPT bit cleared that
            // is inferior to our own assert metric.  The RPT bit cleared
            // indicates that the sender of the assert had (S,G) forwarding
            // state on this interface.  If the assert is inferior to our
            // metric, then we must also have (S,G) forwarding state (i.e.,
            // CouldAssert(S,G,I)==TRUE) as (S,G) asserts beat (*,G) asserts,
            // and so we should be the assert winner.  We transition to the
            // "I am Assert Winner" state and perform Actions A1 (below).
            interface->assertState = Interface::I_WON_ASSERT;
            interface->winnerMetric = myMetric;
            sendPIMAssert(routeSG->source, routeSG->group, myMetric, interface->ie, false);
            interface->startAssertTimer(assertTime - assertOverrideInterval);
        }
        else if (receivedMetric.rptBit && interface->couldAssert)
        {
            // An assert is received for (S,G) on I with the RPT bit set
            // (it's a (*,G) assert).  CouldAssert(S,G,I) is TRUE only if we
            // have (S,G) forwarding state on this interface, so we should be
            // the assert winner.  We transition to the "I am Assert Winner"
            // state and perform Actions A1 (below).
            interface->assertState = Interface::I_WON_ASSERT;
            interface->winnerMetric = myMetric;
            sendPIMAssert(routeSG->source, routeSG->group, myMetric, interface->ie, false);
            interface->startAssertTimer(assertTime - assertOverrideInterval);
        }
        else if (isAcceptableAssert && !receivedMetric.rptBit && interface->assertTrackingDesired)
        {
            // We're interested in (S,G) Asserts, either because I is a
            // downstream interface for which we have (S,G) or (*,G)
            // forwarding state, or because I is the upstream interface for S
            // and we have (S,G) forwarding state.  The received assert has a
            // better metric than our own, so we do not win the Assert.  We
            // transition to "I am Assert Loser" and perform Actions A6
            // (below).
            interface->assertState = Interface::I_LOST_ASSERT;
            interface->winnerMetric = receivedMetric;
            interface->startAssertTimer(assertTime);
            // TODO If (I is RPF_interface(S)) AND (UpstreamJPState(S,G) == true)
            //      set SPTbit(S,G) to TRUE.
        }
    }
    else if (interface->assertState == Interface::I_WON_ASSERT)
    {
        if (isInferiorAssert)
        {
            // We receive an (S,G) assert or (*,G) assert mentioning S that
            // has a worse metric than our own.  Whoever sent the assert is
            // in error, and so we resend an (S,G) Assert and restart the
            // Assert Timer (Actions A3 below).
            sendPIMAssert(routeSG->source, routeSG->group, myMetric, interface->ie, false);
            interface->startAssertTimer(assertTime - assertOverrideInterval);
        }
        else if (isPreferredAssert)
        {
            // We receive an (S,G) assert that has a better metric than our
            // own.  We transition to "I am Assert Loser" state and perform
            // Actions A2 (below).  Note that this may affect the value of
            // JoinDesired(S,G) and PruneDesired(S,G,rpt), which could cause
            // transitions in the upstream (S,G) or (S,G,rpt) state machines.
            interface->assertState = Interface::I_LOST_ASSERT;
            interface->winnerMetric = receivedMetric;
            interface->startAssertTimer(assertTime);
        }
    }
    else if (interface->assertState == Interface::I_LOST_ASSERT)
    {
        if (isPreferredAssert)
        {
            // We receive an assert that is better than that of the current
            // assert winner.  We stay in Loser state and perform Actions A2
            // below.
            interface->winnerMetric = receivedMetric;
            interface->startAssertTimer(assertTime);
        }
        else if (isAcceptableAssert && !receivedMetric.rptBit && receivedMetric.address == interface->winnerMetric.address)
        {
            // We receive an assert from the current assert winner that is
            // better than our own metric for this (S,G) (although the metric
            // may be worse than the winner's previous metric).  We stay in
            // Loser state and perform Actions A2 below.
            interface->winnerMetric = receivedMetric;
            interface->startAssertTimer(assertTime);
        }
        else if (isInferiorAssert /* or AssertCancel */ && receivedMetric.address == interface->winnerMetric.address)
        {
            // We receive an assert from the current assert winner that is
            // worse than our own metric for this group (typically, because
            // the winner's metric became worse or because it is an assert
            // cancel).  We transition to NoInfo state, deleting the (S,G)
            // assert information and allowing the normal PIM Join/Prune
            // mechanisms to operate.  Usually, we will eventually re-assert
            // and win when data packets from S have started flowing again.
            interface->deleteAssertInfo();
        }
    }
}

void PIMSM::processAssertG(PimsmInterface *interface, const AssertMetric &receivedMetric)
{
    Route *routeG = interface->route();
    AssertMetric myMetric = interface->couldAssert ?
                                routeG->metric.setAddress(interface->ie->ipv4Data()->getIPAddress()) :
                                AssertMetric::INFINITE;

    // A "preferred assert" is one with a better metric than the current winner.
    bool isPreferredAssert = receivedMetric < interface->winnerMetric;

    // An "acceptable assert" is one that has a better metric than my_assert_metric(S,G,I).
    // An assert is never considered acceptable if its metric is infinite.
    bool isAcceptableAssert = receivedMetric < myMetric;

    // An "inferior assert" is one with a worse metric than my_assert_metric(S,G,I).
    // An assert is never considered inferior if my_assert_metric(S,G,I) is infinite.
    bool isInferiorAssert = myMetric < receivedMetric;

    if (interface->assertState == Interface::NO_ASSERT_INFO)
    {
        if (isInferiorAssert && receivedMetric.rptBit && interface->couldAssert)
        {
            // An Inferior (*,G) assert is received for G on Interface I.  If
            // CouldAssert(*,G,I) is TRUE, then I is our downstream
            // interface, and we have (*,G) forwarding state on this
            // interface, so we should be the assert winner.  We transition
            // to the "I am Assert Winner" state and perform Actions:
            interface->assertState = Interface::I_WON_ASSERT;
            sendPIMAssert(IPv4Address::UNSPECIFIED_ADDRESS, routeG->group, myMetric, interface->ie, true);
            interface->startAssertTimer(assertTime - assertOverrideInterval);
            interface->winnerMetric = myMetric;
        }
        else if (isAcceptableAssert && receivedMetric.rptBit && interface->assertTrackingDesired)
        {
            // We're interested in (*,G) Asserts, either because I is a
            // downstream interface for which we have (*,G) forwarding state,
            // or because I is the upstream interface for RP(G) and we have
            // (*,G) forwarding state.  We get a (*,G) Assert that has a
            // better metric than our own, so we do not win the Assert.  We
            // transition to "I am Assert Loser" and perform Actions:
            interface->assertState = Interface::I_LOST_ASSERT;
            interface->winnerMetric = receivedMetric;
            interface->startAssertTimer(assertTime);
        }
    }
    else if (interface->assertState == Interface::I_WON_ASSERT)
    {
        if (isInferiorAssert)
        {
            // We receive a (*,G) assert that has a worse metric than our
            // own.  Whoever sent the assert has lost, and so we resend a
            // (*,G) Assert and restart the Assert Timer (Actions A3 below).
            sendPIMAssert(IPv4Address::UNSPECIFIED_ADDRESS, routeG->group, myMetric, interface->ie, true);
            interface->startAssertTimer(assertTime - assertOverrideInterval);
        }
        else if (isPreferredAssert)
        {
            // We receive a (*,G) assert that has a better metric than our
            // own.  We transition to "I am Assert Loser" state and perform
            // Actions A2 (below).
            interface->assertState = Interface::I_LOST_ASSERT;
            interface->winnerMetric = receivedMetric;
            interface->startAssertTimer(assertTime);
        }
    }
    else if (interface->assertState == Interface::I_LOST_ASSERT)
    {
        if (isPreferredAssert && receivedMetric.rptBit)
        {
            // We receive a (*,G) assert that is better than that of the
            // current assert winner.  We stay in Loser state and perform
            // Actions A2 below.
            interface->winnerMetric = receivedMetric;
            interface->startAssertTimer(assertTime);
        }
        else if (isAcceptableAssert && receivedMetric.address == interface->winnerMetric.address && receivedMetric.rptBit)
        {
            // We receive a (*,G) assert from the current assert winner that
            // is better than our own metric for this group (although the
            // metric may be worse than the winner's previous metric).  We
            // stay in Loser state and perform Actions A2 below.
            //interface->winnerAddress = ...;
            interface->winnerMetric = receivedMetric;
            interface->startAssertTimer(assertTime);
        }
        else if (isInferiorAssert /*or AssertCancel*/ && receivedMetric.address == interface->winnerMetric.address)
        {
            // We receive an assert from the current assert winner that is
            // worse than our own metric for this group (typically because
            // the winner's metric became worse or is now an assert cancel).
            // We transition to NoInfo state, delete this (*,G) assert state
            // (Actions A5), and allow the normal PIM Join/Prune mechanisms
            // to operate.  Usually, we will eventually re-assert and win
            // when data packets for G have started flowing again.
            interface->deleteAssertInfo();
        }
    }
}


void PIMSM::sendPIMJoin(IPv4Address group, IPv4Address source, IPv4Address upstreamNeighbor, RouteType routeType)
{
    EV_INFO << "Sending Join(S=" << (routeType==G?"*":source.str()) << ", G=" << group << ") to neighbor " << upstreamNeighbor << ".\n";

    PIMJoinPrune *msg = new PIMJoinPrune();
    msg->setType(JoinPrune);
    msg->setName("PIMJoin");
    msg->setUpstreamNeighborAddress(upstreamNeighbor);
    msg->setHoldTime(joinPruneHoldTime());

    msg->setMulticastGroupsArraySize(1);
    MulticastGroup &multGroup = msg->getMulticastGroups(0);
    multGroup.setGroupAddress(group);
    multGroup.setJoinedSourceAddressArraySize(1);
    EncodedAddress &encodedAddr = multGroup.getJoinedSourceAddress(0);
    encodedAddr.IPaddress = source;
    encodedAddr.S = true;
    encodedAddr.W = (routeType == G);
    encodedAddr.R = (routeType == G);

    InterfaceEntry *interfaceToRP = rt->getInterfaceForDestAddr(source);
    sendToIP(msg, IPv4Address::UNSPECIFIED_ADDRESS, ALL_PIM_ROUTERS_MCAST, interfaceToRP->getInterfaceId(), 1);
}

void PIMSM::sendPIMPrune(IPv4Address group, IPv4Address source, IPv4Address upstreamNeighbor, RouteType routeType)
{
    EV_INFO << "Sending Prune(S=" << (routeType==G?"*":source.str()) << ", G=" << group << ") to neighbor " << upstreamNeighbor << ".\n";

    PIMJoinPrune *msg = new PIMJoinPrune();
    msg->setType(JoinPrune);
    msg->setName("PIMPrune");
    msg->setUpstreamNeighborAddress(upstreamNeighbor);
    msg->setHoldTime(joinPruneHoldTime());

    msg->setMulticastGroupsArraySize(1);
    MulticastGroup &multGroup = msg->getMulticastGroups(0);
    multGroup.setGroupAddress(group);
    multGroup.setPrunedSourceAddressArraySize(1);
    EncodedAddress &encodedAddr = multGroup.getPrunedSourceAddress(0);
    encodedAddr.IPaddress = source;
    encodedAddr.S = true;
    encodedAddr.W = (routeType == G);
    encodedAddr.R = (routeType == G);

    InterfaceEntry *interfaceToRP = rt->getInterfaceForDestAddr(source);
    sendToIP(msg, IPv4Address::UNSPECIFIED_ADDRESS, ALL_PIM_ROUTERS_MCAST, interfaceToRP->getInterfaceId(), 1);
}

void PIMSM::sendPIMRegisterNull(IPv4Address multOrigin, IPv4Address multGroup)
{
    EV << "pimSM::sendPIMRegisterNull" << endl;

    // only if (S,G exist)
    //if (getRouteFor(multDest,multSource))
    if (findRouteG(multGroup))
    {
        PIMRegister *msg = new PIMRegister();
        msg->setName("PIMRegister(Null)");
        msg->setType(Register);
        msg->setN(true);
        msg->setB(false);

        // set encapsulated packet (IPv4 header only)
        IPv4Datagram *datagram = new IPv4Datagram();
        datagram->setDestAddress(multGroup);
        datagram->setSrcAddress(multOrigin);
        datagram->setTransportProtocol(IP_PROT_PIM);
        msg->encapsulate(datagram);

        InterfaceEntry *interfaceToRP = rt->getInterfaceForDestAddr(rpAddr);
        sendToIP(msg, IPv4Address::UNSPECIFIED_ADDRESS, rpAddr, interfaceToRP->getInterfaceId(), MAX_TTL);
    }
}

void PIMSM::sendPIMRegister(IPv4Datagram *datagram, IPv4Address dest, int outInterfaceId)
{
    EV << "pimSM::sendPIMRegister - encapsulating data packet into Register packet and sending to RP" << endl;

    PIMRegister *msg = new PIMRegister("PIMRegister");
    msg->setType(Register);
    msg->setN(false);
    msg->setB(false);

    IPv4Datagram *datagramCopy = datagram->dup();
    delete datagramCopy->removeControlInfo();
    msg->encapsulate(datagramCopy);

    sendToIP(msg, IPv4Address::UNSPECIFIED_ADDRESS, dest, outInterfaceId, MAX_TTL);
}

void PIMSM::sendPIMRegisterStop(IPv4Address source, IPv4Address dest, IPv4Address multGroup, IPv4Address multSource)
{
    EV << "pimSM::sendPIMRegisterStop" << endl;

    // create PIM Register datagram
    PIMRegisterStop *msg = new PIMRegisterStop();

    // set PIM packet
    msg->setName("PIMRegisterStop");
    msg->setType(RegisterStop);
    msg->setSourceAddress(multSource);
    msg->setGroupAddress(multGroup);

    // set IP packet
    InterfaceEntry *interfaceToDR = rt->getInterfaceForDestAddr(dest);
    sendToIP(msg, source, dest, interfaceToDR->getInterfaceId(), MAX_TTL);
}

void PIMSM::sendPIMAssert(IPv4Address source, IPv4Address group, AssertMetric metric, InterfaceEntry *ie, bool rptBit)
{
    EV_INFO << "Sending Assert(S= " << source << ", G= " << group << ") message on interface '" << ie->getName() << "'\n";

    PIMAssert *pkt = new PIMAssert("PIMAssert");
    pkt->setGroupAddress(group);
    pkt->setSourceAddress(source);
    pkt->setR(rptBit);
    pkt->setMetricPreference(metric.preference);
    pkt->setMetric(metric.metric);

    sendToIP(pkt, IPv4Address::UNSPECIFIED_ADDRESS, ALL_PIM_ROUTERS_MCAST, ie->getInterfaceId(), 1);
}


void PIMSM::sendToIP(PIMPacket *packet, IPv4Address srcAddr, IPv4Address destAddr, int outInterfaceId, short ttl)
{
    IPv4ControlInfo *ctrl = new IPv4ControlInfo();
    ctrl->setSrcAddr(srcAddr);
    ctrl->setDestAddr(destAddr);
    ctrl->setProtocol(IP_PROT_PIM);
    ctrl->setTimeToLive(ttl);
    ctrl->setInterfaceId(outInterfaceId);
    packet->setControlInfo(ctrl);
    send(packet, "ipOut");
}


/**
 * FORWARD MULTICAST DATA
 *
 * The method is used as abstraction for encapsulation multicast data to Register packet.
 * The method create message MultData with multicast source address and multicast group address
 * and send the message from RP to RPT.
 *
 * @param info Pointer to structure, which keep all information for creating and sending message.
 * @see MultData()
 */
void PIMSM::forwardMulticastData(IPv4Datagram *datagram, int outInterfaceId)
{
    EV << "pimSM::forwardMulticastData" << endl;

    //
    // Note: we should inject the datagram somehow into the normal IPv4 forwarding path.
    //
    cPacket *data = datagram->decapsulate();

    // set control info
    IPv4ControlInfo *ctrl = new IPv4ControlInfo();
    ctrl->setDestAddr(datagram->getDestAddress());
    // XXX ctrl->setSrcAddr(datagram->getSrcAddress()); // FIXME IP won't accept if the source is non-local
    ctrl->setInterfaceId(outInterfaceId);
    ctrl->setTimeToLive(MAX_TTL-2);                     //one minus for source DR router and one for RP router // XXX specification???
    ctrl->setProtocol(datagram->getTransportProtocol());
    data->setControlInfo(ctrl);
    send(data, "ipOut");
}

void PIMSM::unroutableMulticastPacketArrived(IPv4Address source, IPv4Address group)
{
    InterfaceEntry *interfaceTowardSource = rt->getInterfaceForDestAddr(source);
    if (!interfaceTowardSource)
        return;

    PIMInterface *rpfInterface = pimIft->getInterfaceById(interfaceTowardSource->getInterfaceId());
    if (!rpfInterface || rpfInterface->getMode() != PIMInterface::SparseMode)
        return;

    InterfaceEntry *interfaceTowardRP = rt->getInterfaceForDestAddr(this->getRPAddress());

    // RPF check and check if I am DR of the source
    if ((interfaceTowardRP != interfaceTowardSource) && IamDR(source))
    {
        EV_DETAIL << "New multicast source observed: source=" << source << ", group=" << group << ".\n";

        // create new (S,G) route
        Route *newRouteSG = createRouteSG(source, group, Route::PRUNED | Route::REGISTER | Route::SPT_BIT);
        newRouteSG->startKeepAliveTimer();
        newRouteSG->registerState = Route::RS_JOIN;
        newRouteSG->addDownstreamInterface(new DownstreamInterface(newRouteSG, interfaceTowardRP, DownstreamInterface::NO_INFO, false));      // create new outgoing interface to RP

        addRouteSG(newRouteSG);

        // create new (*,G) route
        Route *newRouteG = createRouteG(newRouteSG->group, Route::PRUNED | Route::REGISTER);
        addRouteG(newRouteG);
    }
}

void PIMSM::multicastReceiverRemoved(InterfaceEntry *ie, IPv4Address group)
{
    EV_DETAIL << "No more receiver for group " << group << " on interface '" << ie->getName() << "'.\n";

    for (RoutingTable::iterator it = routes.begin(); it != routes.end(); ++it)
    {
        Route *route = it->second;
        if (route->group != group)
            continue;

        // is interface in list of outgoing interfaces?
        int k = route->findDownstreamInterface(ie);
        if (k >= 0)
        {
            EV << "Interface is present, removing it from the list of outgoing interfaces." << endl;
            route->removeDownstreamInterface(k);
        }

        route->clearFlag(Route::CONNECTED);

        // there is no receiver of multicast, prune the router from the multicast tree
        if (route->isOilistNull())
        {
            route->setFlags(Route::PRUNED);
            PIMNeighbor *neighborToRP = pimNbt->getFirstNeighborOnInterface(route->upstreamInterface->getInterfaceId());
            sendPIMPrune(route->group,this->getRPAddress(),neighborToRP->getAddress(),G);
            cancelAndDeleteTimer(route->joinTimer);
        }
    }
}

void PIMSM::multicastReceiverAdded(InterfaceEntry *ie, IPv4Address group)
{
    EV_DETAIL << "Multicast receiver added for group " << group << " on interface '" << ie->getName() << "'.\n";

    Route *routeG = findRouteG(group);
    if (!routeG)
    {
        // create new (*,G) route
        Route *newRouteG = createRouteG(group, Route::CONNECTED);
        newRouteG->startJoinTimer();
        if (newRouteG->upstreamInterface)
            newRouteG->upstreamInterface->startExpiryTimer(joinPruneHoldTime());

        // add downstream interface
        DownstreamInterface *downstream = new DownstreamInterface(newRouteG, ie, DownstreamInterface::JOIN);
        downstream->startExpiryTimer(HOLDTIME_HOST);
        newRouteG->addDownstreamInterface(downstream);

        // add route to tables
        addRouteG(newRouteG);

        // oilist != NULL -> send Join(*,G) to 224.0.0.13
        if (newRouteG->upstreamInterface && !newRouteG->isOilistNull())
            sendPIMJoin(group, newRouteG->rpAddr, newRouteG->upstreamInterface->rpfNeighbor(), G);
    }
    else                                                                                          // add new outgoing interface to existing (*,G) route
    {
        DownstreamInterface *downstream = new DownstreamInterface(routeG, ie, DownstreamInterface::JOIN);
        // downstream->startExpiryTimer(joinPruneHoldTime());
        routeG->addDownstreamInterface(downstream);
        routeG->setFlags(Route::CONNECTED);

        IPv4MulticastRoute *ipv4Route = findIPv4Route(IPv4Address::UNSPECIFIED_ADDRESS, group);
        ipv4Route->addOutInterface(new PIMSMOutInterface(downstream));
    }
}

void PIMSM::multicastPacketArrivedOnNonRpfInterface(Route *route, int interfaceId)
{
    if (route->type == G || route->type == SG)
    {
        //
        // (S,G) Assert State Machine; event: An (S,G) data packet arrives on interface I
        // OR
        // (*,G) Assert State Machine; event: A data packet destined for G arrives on interface I
        //
        DownstreamInterface *downstream = route->findDownstreamInterfaceByInterfaceId(interfaceId);
        if (downstream && downstream->couldAssert && downstream->assertState == Interface::NO_ASSERT_INFO)
        {
            // An (S,G) data packet arrived on an downstream interface that
            // is in our (S,G) or (*,G) outgoing interface list.  We optimistically
            // assume that we will be the assert winner for this (S,G) or (*,G), and
            // so we transition to the "I am Assert Winner" state and perform
            // Actions A1 (below), which will initiate the assert negotiation
            // for (S,G) or (*,G).
            downstream->assertState = Interface::I_WON_ASSERT;
            downstream->winnerMetric = route->metric.setAddress(downstream->ie->ipv4Data()->getIPAddress());
            sendPIMAssert(route->source, route->group, downstream->winnerMetric, downstream->ie, route->type == G);
            downstream->startAssertTimer(assertTime - assertOverrideInterval);
        }
        else if (route->type == SG && (!downstream || downstream->assertState == Interface::NO_ASSERT_INFO))
        {
            // When in NO_ASSERT_INFO state before and after consideration of the received message,
            // then call (*,G) assert processing.
            Route *routeG = findRouteG(route->group);
            if (routeG)
                multicastPacketArrivedOnNonRpfInterface(routeG, interfaceId);
        }
    }
}

void PIMSM::multicastPacketForwarded(IPv4Datagram *datagram)
{
    IPv4Address source = datagram->getSrcAddress();
    IPv4Address group = datagram->getDestAddress();

    Route *routeSG = findRouteSG(source, group);
    if (!routeSG || !routeSG->isFlagSet(Route::REGISTER) || !routeSG->isFlagSet(Route::PRUNED))
        return;

    // send Register message to RP

    InterfaceEntry *interfaceTowardRP = rt->getInterfaceForDestAddr(routeSG->rpAddr);

    if (routeSG->registerState == Route::RS_JOIN)
    {
        // refresh KAT timer
        if (routeSG->keepAliveTimer)
        {
            EV << " (S,G) KAT timer refresh" << endl;
            restartTimer(routeSG->keepAliveTimer, KAT);
        }

        sendPIMRegister(datagram, routeSG->rpAddr, interfaceTowardRP->getInterfaceId());
    }
}


/*
 * JoinDesired(*,G) -> FALSE/TRUE
 * JoinDesired(S,G) -> FALSE/TRUE
 */
void PIMSM::joinDesiredChanged(Route *route)
{
    if (route->type == G)
    {
        Route *routeG = route;

        if (routeG->isFlagSet(Route::PRUNED) && routeG->joinDesired())
        {
            //
            // Upstream (*,G) State Machine; event: JoinDesired(S,G) -> TRUE
            //
            routeG->clearFlag(Route::PRUNED);
            if (routeG->upstreamInterface)
            {
                sendPIMJoin(routeG->group, routeG->rpAddr, routeG->upstreamInterface->rpfNeighbor(), G);
                routeG->startJoinTimer();
            }
        }
        else if (!routeG->isFlagSet(Route::PRUNED) && !routeG->joinDesired())
        {
            //
            // Upstream (*,G) State Machine; event: JoinDesired(S,G) -> FALSE
            //
            routeG->setFlags(Route::PRUNED);
            cancelAndDeleteTimer(routeG->joinTimer);
            if (routeG->upstreamInterface)
                sendPIMPrune(routeG->group, routeG->rpAddr, routeG->upstreamInterface->rpfNeighbor(), G);
        }
    }
    else if (route->type == SG)
    {
        Route *routeSG = route;

        if (routeSG->isFlagSet(Route::PRUNED) && routeSG->joinDesired())
        {
            //
            // Upstream (S,G) State Machine; event: JoinDesired(S,G) -> TRUE
            //
            routeSG->clearFlag(Route::PRUNED);
            if (!IamDR(routeSG->source))
            {
                sendPIMJoin(routeSG->group, routeSG->source, routeSG->upstreamInterface->rpfNeighbor(), SG);
                routeSG->startJoinTimer();
            }
        }
        else if (!routeSG->isFlagSet(Route::PRUNED) && !route->joinDesired())
        {
            //
            // Upstream (S,G) State Machine; event: JoinDesired(S,G) -> FALSE
            //

            // The upstream (S,G) state machine transitions to NotJoined
            // state.  Send Prune(S,G) to the appropriate upstream neighbor,
            // which is RPF'(S,G).  Cancel the Join Timer (JT), and set
            // SPTbit(S,G) to FALSE.
            routeSG->setFlags(Route::PRUNED);
            routeSG->clearFlag(Route::SPT_BIT);
            cancelAndDeleteTimer(routeSG->joinTimer);
            if (!IamDR(routeSG->source))
                sendPIMPrune(routeSG->group, routeSG->source, routeSG->upstreamInterface->rpfNeighbor(), SG);
        }
    }

}

/**
 * PROCESS PIM TIMER
 *
 * The method is used to process PIM timers. According to type of PIM timer, the timer is sent to
 * appropriate method.
 *
 * @param timer Pointer to PIM timer.
 * @see PIMTimer()
 * @see processPruneTimer()
 * @see processGraftRetryTimer()
 */
void PIMSM::processPIMTimer(cMessage *timer)
{
    EV << "pimSM::processPIMTimer: ";

    switch(timer->getKind())
    {
        case HelloTimer:
            processHelloTimer(timer);
            break;
        case JoinTimer:
            EV << "JoinTimer" << endl;
            processJoinTimer(timer);
            break;
        case PrunePendingTimer:
            EV << "PrunePendingTimer" << endl;
            processPrunePendingTimer(timer);
            break;
        case ExpiryTimer:
            EV << "ExpiryTimer" << endl;
            processExpiryTimer(timer);
            break;
        case KeepAliveTimer:
            EV << "KeepAliveTimer" << endl;
            processKeepAliveTimer(timer);
            break;
        case RegisterStopTimer:
            EV << "RegisterStopTimer" << endl;
            processRegisterStopTimer(timer);
            break;
        case AssertTimer:
            processAssertTimer(timer);
            break;
        default:
            EV << "BAD TYPE, DROPPED" << endl;
            delete timer;
            break;
    }
}

/*
    Bootstrap = 4,
    CandidateRPAdvertisement = 8,
 */
void PIMSM::processPIMPacket(PIMPacket *pkt)
{
    switch(pkt->getType())
    {
        case Hello:
            processHelloPacket(check_and_cast<PIMHello*>(pkt));
            break;
        case JoinPrune:
            processJoinPrunePacket(check_and_cast<PIMJoinPrune *> (pkt));
            break;
        case Register:
            processRegisterPacket(check_and_cast<PIMRegister *> (pkt));
            break;
        case RegisterStop:
            processRegisterStopPacket(check_and_cast<PIMRegisterStop *> (pkt));
            break;
        case Assert:
            processAssertPacket(check_and_cast<PIMAssert*>(pkt));
            break;
        case Graft:
            EV_WARN << "Ignoring PIM-DM Graft packet.\n";
            delete pkt;
            break;
        case GraftAck:
            EV_WARN << "Ignoring PIM-DM GraftAck packet.\n";
            delete pkt;
            break;
        case StateRefresh:
            EV_WARN << "Ignoring PIM-DM StateRefresh packet.\n";
            delete pkt;
            break;
        default:
            throw cRuntimeError("PIMSM: received unknown PIM packet: %s (%s)", pkt->getName(), pkt->getClassName());
    }
}

/**
 * RECEIVE CHANGE NOTIFICATION
 *
 * The method from class Notification Board is used to catch its events.
 *
 * @param category Category of notification.
 * @param details Additional information for notification.
 * @see newMulticast()
 * @see newMulticastAddr()
 */
void PIMSM::receiveSignal(cComponent *source, simsignal_t signalID, cObject *details)
{
    // ignore notifications during initialize
    if (simulation.getContextType()==CTX_INITIALIZE)
        return;

    Enter_Method_Silent();
    printNotificationBanner(signalID, details);
    Route *route;
    IPv4Datagram *datagram;
    PIMInterface *pimInterface;

    if (signalID == NF_IPv4_MCAST_REGISTERED)
    {
        EV <<  "pimSM::receiveChangeNotification - NEW IGMP ADDED" << endl;
        IPv4MulticastGroupInfo *info = check_and_cast<IPv4MulticastGroupInfo*>(details);
        pimInterface = pimIft->getInterfaceById(info->ie->getInterfaceId());
        if (pimInterface && pimInterface->getMode() == PIMInterface::SparseMode)
            multicastReceiverAdded(info->ie, info->groupAddress);
    }
    else if (signalID == NF_IPv4_MCAST_UNREGISTERED)
    {
        EV <<  "pimSM::receiveChangeNotification - IGMP REMOVED" << endl;
        IPv4MulticastGroupInfo *info = check_and_cast<IPv4MulticastGroupInfo*>(details);
        pimInterface = pimIft->getInterfaceById(info->ie->getInterfaceId());
        if (pimInterface && pimInterface->getMode() == PIMInterface::SparseMode)
            multicastReceiverRemoved(info->ie, info->groupAddress);
    }
    else if (signalID == NF_IPv4_NEW_MULTICAST)
    {
        EV <<  "PimSM::receiveChangeNotification - NEW MULTICAST" << endl;
        datagram = check_and_cast<IPv4Datagram*>(details);
        IPv4Address srcAddr = datagram->getSrcAddress();
        IPv4Address destAddr = datagram->getDestAddress();
        unroutableMulticastPacketArrived(srcAddr, destAddr);
    }
    else if (signalID == NF_IPv4_DATA_ON_RPF)
    {
        EV <<  "pimSM::receiveChangeNotification - DATA ON RPF" << endl;
        datagram = check_and_cast<IPv4Datagram*>(details);
        PIMInterface *incomingInterface = getIncomingInterface(datagram);
        if (incomingInterface && incomingInterface->getMode() == PIMInterface::SparseMode)
        {
            route = findRouteG(datagram->getDestAddress());
            if (route)
                multicastPacketArrivedOnRpfInterface(route);
            route = findRouteSG(datagram->getSrcAddress(), datagram->getDestAddress());
            if (route)
                multicastPacketArrivedOnRpfInterface(route);
        }
    }
    else if (signalID == NF_IPv4_DATA_ON_NONRPF)
    {
        datagram = check_and_cast<IPv4Datagram*>(details);
        PIMInterface *incomingInterface = getIncomingInterface(datagram);
        if (incomingInterface && incomingInterface->getMode() == PIMInterface::SparseMode)
        {
            IPv4Address srcAddr = datagram->getSrcAddress();
            IPv4Address destAddr = datagram->getDestAddress();
            if ((route = findRouteSG(srcAddr, destAddr)) != NULL)
                multicastPacketArrivedOnNonRpfInterface(route, incomingInterface->getInterfaceId());
            else if ((route = findRouteG(destAddr)) != NULL)
                multicastPacketArrivedOnNonRpfInterface(route, incomingInterface->getInterfaceId());
        }
    }
    else if (signalID == NF_IPv4_MDATA_REGISTER)
    {
        EV <<  "pimSM::receiveChangeNotification - REGISTER DATA" << endl;
        datagram = check_and_cast<IPv4Datagram*>(details);
        PIMInterface *incomingInterface = getIncomingInterface(datagram);
        route = findRouteSG(datagram->getSrcAddress(), datagram->getDestAddress());
        if (incomingInterface && incomingInterface->getMode() == PIMInterface::SparseMode)
            multicastPacketForwarded(datagram);
    }
}

PIMInterface *PIMSM::getIncomingInterface(IPv4Datagram *datagram)
{
    cGate *g = datagram->getArrivalGate();
    if (g)
    {
        InterfaceEntry *ie = g ? ift->getInterfaceByNetworkLayerGateIndex(g->getIndex()) : NULL;
        if (ie)
            return pimIft->getInterfaceById(ie->getInterfaceId());
    }
    return NULL;
}

bool PIMSM::deleteMulticastRoute(Route *route)
{
    if (removeRoute(route))
    {
        // remove route from the routing table
        IPv4MulticastRoute *ipv4Route = findIPv4Route(route->source, route->group);
        if (ipv4Route)
            rt->deleteMulticastRoute(ipv4Route);

        delete route;
        return true;
    }
    return false;
}

void PIMSM::addRouteG(Route *route)
{
    SourceAndGroup sg(IPv4Address::UNSPECIFIED_ADDRESS, route->group);
    routes[sg] = route;

    rt->addMulticastRoute(createIPv4Route(route));
}

void PIMSM::addRouteSG(Route *route)
{
    SourceAndGroup sg(route->source, route->group);
    routes[sg] = route;

    rt->addMulticastRoute(createIPv4Route(route));
}

PIMSM::Route *PIMSM::createRouteG(IPv4Address group, int flags)
{
    Route *newRouteG = new Route(this, G, IPv4Address::UNSPECIFIED_ADDRESS,group);
    newRouteG->setFlags(flags);
    newRouteG->rpAddr = rpAddr;

    // set upstream interface toward RP and set metric
    if (!IamRP(rpAddr))
    {
        IPv4Route *routeToRP = rt->findBestMatchingRoute(rpAddr);
        if (routeToRP)
        {
            InterfaceEntry *ieTowardRP = routeToRP->getInterface();
            IPv4Address rpfNeighbor = routeToRP->getGateway();
            if (!pimNbt->findNeighbor(ieTowardRP->getInterfaceId(), rpfNeighbor))
            {
                PIMNeighbor *neighbor = pimNbt->getFirstNeighborOnInterface(ieTowardRP->getInterfaceId());
                if (neighbor)
                    rpfNeighbor = neighbor->getAddress();
            }
            newRouteG->upstreamInterface = new UpstreamInterface(newRouteG, ieTowardRP, rpfNeighbor);
            newRouteG->metric = AssertMetric(true, routeToRP->getAdminDist(), routeToRP->getMetric());
        }
    }

    return newRouteG;
}

PIMSM::Route *PIMSM::createRouteSG(IPv4Address source, IPv4Address group, int flags)
{
    Route *newRouteSG = new Route(this, SG, source, group);
    newRouteSG->setFlags(flags);
    newRouteSG->rpAddr = rpAddr;

    // set upstream interface toward source and set metric
    IPv4Route *routeToSource = rt->findBestMatchingRoute(source);
    if (routeToSource)
    {
        InterfaceEntry *ieTowardSource = routeToSource->getInterface();
        IPv4Address rpfNeighbor;
        if (!IamDR(source))
        {
            rpfNeighbor = routeToSource->getGateway();
            if (!pimNbt->findNeighbor(ieTowardSource->getInterfaceId(), rpfNeighbor))
            {
                PIMNeighbor *neighbor = pimNbt->getFirstNeighborOnInterface(ieTowardSource->getInterfaceId());
                if (neighbor)
                    rpfNeighbor = neighbor->getAddress();
            }
        }
        newRouteSG->upstreamInterface = new UpstreamInterface(newRouteSG, ieTowardSource, rpfNeighbor);
        newRouteSG->metric = AssertMetric(false, routeToSource->getAdminDist(), routeToSource->getMetric());
    }

    return newRouteSG;
}

IPv4MulticastRoute *PIMSM::createIPv4Route(Route *route)
{
    IPv4MulticastRoute *newRoute = new IPv4MulticastRoute();
    newRoute->setOrigin(route->source);
    newRoute->setOriginNetmask(route->source.isUnspecified() ? IPv4Address::UNSPECIFIED_ADDRESS : IPv4Address::ALLONES_ADDRESS);
    newRoute->setMulticastGroup(route->group);
    newRoute->setSourceType(IMulticastRoute::PIM_SM);
    newRoute->setSource(this);
    if (route->upstreamInterface)
        newRoute->setInInterface(new IMulticastRoute::InInterface(route->upstreamInterface->ie));
    unsigned int numOutInterfaces = route->downstreamInterfaces.size();
    for (unsigned int i = 0; i < numOutInterfaces; ++i)
    {
        DownstreamInterface *downstream = route->downstreamInterfaces[i];
        newRoute->addOutInterface(new PIMSMOutInterface(downstream));
    }
    return newRoute;
}

bool PIMSM::removeRoute(Route *route)
{
    SourceAndGroup sg(route->source, route->group);
    return routes.erase(sg);
}

PIMSM::Route *PIMSM::findRouteG(IPv4Address group)
{
    SourceAndGroup sg(IPv4Address::UNSPECIFIED_ADDRESS, group);
    RoutingTable::iterator it = routes.find(sg);
    return it != routes.end() ? it->second : NULL;
}

PIMSM::Route *PIMSM::findRouteSG(IPv4Address source, IPv4Address group)
{
    ASSERT(!source.isUnspecified());
    SourceAndGroup sg(source, group);
    RoutingTable::iterator it = routes.find(sg);
    return it != routes.end() ? it->second : NULL;
}

IPv4MulticastRoute *PIMSM::findIPv4Route(IPv4Address source, IPv4Address group)
{
    unsigned int numMulticastRoutes = rt->getNumMulticastRoutes();
    for (unsigned int i = 0; i < numMulticastRoutes; ++i)
    {
        IPv4MulticastRoute *ipv4Route = rt->getMulticastRoute(i);
        if (ipv4Route->getSource() == this && ipv4Route->getOrigin() == source && ipv4Route->getMulticastGroup() == group)
            return ipv4Route;
    }
    return NULL;
}

void PIMSM::cancelAndDeleteTimer(cMessage *&timer)
{
    cancelAndDelete(timer);
    timer = NULL;
}

void PIMSM::restartTimer(cMessage *timer, double interval)
{
    cancelEvent(timer);
    scheduleAt(simTime() + interval, timer);
}

void PIMSM::Route::clearDownstreamInterfaces()
{
    if (!downstreamInterfaces.empty())
    {
        for (DownstreamInterfaceVector::iterator it = downstreamInterfaces.begin(); it != downstreamInterfaces.end(); ++it)
            delete *it;
        downstreamInterfaces.clear();
    }
}

void PIMSM::Route::addDownstreamInterface(DownstreamInterface *outInterface)
{
    ASSERT(outInterface);

    DownstreamInterfaceVector::iterator it;
    for (it = downstreamInterfaces.begin(); it != downstreamInterfaces.end(); ++it)
    {
        if ((*it)->ie == outInterface->ie)
            break;
    }

    if (it != downstreamInterfaces.end())
    {
        delete *it;
        *it = outInterface;
    }
    else
    {
        downstreamInterfaces.push_back(outInterface);
    }
}

void PIMSM::Route::removeDownstreamInterface(unsigned int i)
{
    // remove corresponding out interface from the IPv4 route,
    // because it refers to the downstream interface to be deleted
    IPv4MulticastRoute *ipv4Route = pimsm()->findIPv4Route(source, group);
    ipv4Route->removeOutInterface(i);

    DownstreamInterface *outInterface = downstreamInterfaces[i];
    downstreamInterfaces.erase(downstreamInterfaces.begin()+i);
    delete outInterface;
}

PIMSM::PimsmInterface::PimsmInterface(Route *owner, InterfaceEntry *ie)
    : Interface(owner, ie), expiryTimer(NULL), couldAssert(false), assertTrackingDesired(false)
{
}

PIMSM::PimsmInterface::~PimsmInterface()
{
    owner->owner->cancelAndDelete(expiryTimer);
    owner->owner->cancelAndDelete(assertTimer);
}

void PIMSM::PimsmInterface::startExpiryTimer(double holdTime)
{
    expiryTimer = new cMessage("PIMExpiryTimer", ExpiryTimer);
    expiryTimer->setContextPointer(this);
    owner->owner->scheduleAt(simTime() + holdTime, expiryTimer);
}

PIMSM::DownstreamInterface::~DownstreamInterface()
{
    owner->owner->cancelAndDelete(prunePendingTimer);
}

bool PIMSM::DownstreamInterface::isInInheritedOlist() const
{
    return isInImmediateOlist(); /* XXX */
}

void PIMSM::DownstreamInterface::startPrunePendingTimer()
{
    ASSERT(!prunePendingTimer);
    prunePendingTimer = new cMessage("PIMPrunePendingTimer", PrunePendingTimer);
    prunePendingTimer->setContextPointer(this);
    pimsm()->scheduleAt(simTime() + pimsm()->joinPruneOverrideInterval(), prunePendingTimer);
}

void PIMSM::Route::startKeepAliveTimer()
{
    ASSERT(this->type == SG);
    keepAliveTimer = new cMessage("PIMKeepAliveTimer", KeepAliveTimer);
    keepAliveTimer->setContextPointer(this);
    pimsm()->scheduleAt(simTime() + pimsm()->keepAlivePeriod, keepAliveTimer);
}

void PIMSM::Route::startRegisterStopTimer(double interval)
{
    registerStopTimer = new cMessage("PIMRegisterStopTimer", RegisterStopTimer);
    registerStopTimer->setContextPointer(this);
    owner->scheduleAt(simTime() + interval, registerStopTimer);
}

void PIMSM::Route::startJoinTimer()
{
    joinTimer = new cMessage("PIMJoinTimer", JoinTimer);
    joinTimer->setContextPointer(this);
    pimsm()->scheduleAt(simTime() + pimsm()->joinPrunePeriod, joinTimer);
}

