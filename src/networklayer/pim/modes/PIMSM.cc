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
    : owner(owner), type(type), origin(origin), group(group), rpAddr(IPv4Address::UNSPECIFIED_ADDRESS), flags(0), sequencenumber(0),
      keepAliveTimer(NULL), registerStopTimer(NULL), joinTimer(NULL), prunePendingTimer(NULL),
      upstreamInterface(NULL)
{
}

PIMSM::Route::~Route()
{
    owner->cancelAndDelete(keepAliveTimer);
    owner->cancelAndDelete(registerStopTimer);
    owner->cancelAndDelete(joinTimer);
    owner->cancelAndDelete(prunePendingTimer);
    delete upstreamInterface;
    for (DownstreamInterfaceVector::iterator it = downstreamInterfaces.begin(); it != downstreamInterfaces.end(); ++it)
        delete *it;
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
    {
        DownstreamInterface *outInterface = downstreamInterfaces[i];
        if (outInterface->forwarding == Forward)
            return false;
    }
    return true;
}

PIMSM::~PIMSM()
{
    for (SGStateMap::iterator it = routes.begin(); it != routes.end(); ++it)
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
	   processPIMPkt(pkt);
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
    if (route->keepAliveTimer)
    {
        if (route->origin == IPv4Address::UNSPECIFIED_ADDRESS)     // (*,G) route
        {
            restartTimer(route->keepAliveTimer, keepAlivePeriod + KAT); // XXX ??? shouldn't be any KAT(*,G) timer
            EV << "PIMSM::dataOnRpf: restart (*,G) KAT" << endl;
        }
        else                                                            // (S,G) route
        {
            restartTimer(route->keepAliveTimer, keepAlivePeriod);
            EV << "PIMSM::dataOnRpf: restart (S,G) KAT" << endl;
            if (!route->isFlagSet(Route::T))
                route->setFlags(Route::T);
        }
    }

    //TODO SPT threshold at last hop router
    if (route->isFlagSet(Route::C))
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
 * I AM RP
 *
 * The method is used to determine if router is Rendezvous Point.
 *
 * @param RPaddress IP address to test.
 */
bool PIMSM::IamRP (IPv4Address RPaddress)
{
    InterfaceEntry *intf;

    for (int i=0; i < ift->getNumInterfaces(); i++)
    {
        intf = ift->getInterface(i);
        if (intf->ipv4Data()->getIPAddress() == RPaddress)
            return true;
    }
    return false;
}

/**
 * I AM RP
 *
 * The method is used to determine if router is Designed Router for given IP address.
 *
 * @param sourceAddr IP address to test.
 */
bool PIMSM::IamDR (IPv4Address sourceAddr)
{
    InterfaceEntry *intf;
    IPv4Address intfAddr;
    IPv4Address intfMask;

    for (int i=0; i < ift->getNumInterfaces(); i++)
    {
        intf = ift->getInterface(i);
        intfAddr = intf->ipv4Data()->getIPAddress();
        intfMask = intf->ipv4Data()->getNetmask();

        if (IPv4Address::maskedAddrAreEqual(intfAddr,sourceAddr,intfMask))
        {
            EV << "I AM DR for: " << intfAddr << endl;
            return true;
        }
    }
    EV << "I AM NOT DR " << endl;
    return false;
}

/**
 * PROCESS KEEP ALIVE TIMER
 *
 * The method is used to process PIM Keep Alive Timer. It is (S,G) timer. When Keep Alive Timer expires,
 * route is removed from multicast routing table.
 *
 * @param timer Pointer to Keep Alive Timer.
 * @see PIMkat()
 */
void PIMSM::processKeepAliveTimer(cMessage *timer)
{
    EV << "pimSM::processKeepAliveTimer: route will be deleted" << endl;
    Route *route = static_cast<Route*>(timer->getContextPointer());
    ASSERT(timer == route->keepAliveTimer);

    for (unsigned i=0; i < route->downstreamInterfaces.size(); i++)
    {
        DownstreamInterface *downstream = route->downstreamInterfaces[i];
        cancelAndDeleteTimer(downstream->expiryTimer);
    }

    // only for RP, when KAT for (S,G) expire, set KAT for (*,G)
    if (IamRP(this->getRPAddress()) && route->origin != IPv4Address::UNSPECIFIED_ADDRESS)
    {
        Route *routeG = getRouteFor(route->group, IPv4Address::UNSPECIFIED_ADDRESS);
        if (routeG && !routeG->keepAliveTimer)
            routeG->startKeepAliveTimer();
    }

    delete timer;
    route->keepAliveTimer = NULL;
    deleteMulticastRoute(route);
}

/**
 * PROCESS REGISTER STOP TIMER
 *
 * The method is used to process PIM Register Stop Timer. RST is used to send
 * periodic Register-Null messages
 *
 * @param timer Pointer to Register Stop Timer.
 * @see PIMrst()
 */
void PIMSM::processRegisterStopTimer(cMessage *timer)
{
    EV << "pimSM::processRegisterStopTimer: " << endl;
    Route *route = static_cast<Route*>(timer->getContextPointer());
    ASSERT(timer == route->registerStopTimer);

    sendPIMRegisterNull(route->origin, route->group);
    //TODO set RST to probe time, if RST expires, add encapsulation tunnel

    delete timer;
    route->registerStopTimer = NULL;
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

    Interface *interface = static_cast<Interface*>(timer->getContextPointer());
    Route *route = interface->owner;

    if (interface != route->upstreamInterface)
    {
        int i = route->findDownstreamInterface(interface->ie);
        if (i >= 0 && route->downstreamInterfaces[i] == interface)
            route->removeDownstreamInterface(i);

        if (route->isOilistNull())
        {
            route->clearFlag(Route::C);
            route->setFlags(Route::P);
            PIMNeighbor *RPFneighbor = pimNbt->getFirstNeighborOnInterface(route->upstreamInterface->getInterfaceId());
            if (route->type == G && !IamRP(route->rpAddr))
                sendPIMPrune(route->group, route->rpAddr, RPFneighbor->getAddress(), G);
            else if (route->type == SG)
                sendPIMPrune(route->group, route->origin, RPFneighbor->getAddress(), SG);

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
    IPv4Address joinAddr = route->type == G ? route->rpAddr : route->origin;

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
    Route *route = static_cast<Route*>(timer->getContextPointer());
    ASSERT(timer == route->prunePendingTimer);

    IPv4Address pruneAddr = route->type == G ? route->rpAddr : route->origin;
    PIMNeighbor *neighbor = pimNbt->getFirstNeighborOnInterface(route->upstreamInterface->getInterfaceId()); // XXX why not nextHop?

    if ((route->type == G && !IamRP(this->getRPAddress())) || route->type == SG)
        sendPIMPrune(route->group, pruneAddr, neighbor->getAddress(), route->type);

    route->prunePendingTimer = NULL;
    delete timer;
}

/**
 * RESTART EXPIRY TIMER
 *
 * The method is used to restart ET. ET is used for outgoing interfaces
 * and whole route in router. After ET expires, outgoing interface is
 * removed or if there aren't any outgoing interface, route is removed
 * after ET expires.
 *
 * @param route Pointer to multicast route.
 * @param originIntf Pointer to origin interface to packet
 * @param holdTime time for ET
 * @see getEt
 * @see setEt
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
                EV << /*timer->getStateType() << " , " <<*/ route->group << " , " << route->origin << ", int: " << downstream->ie->getName() << endl;
                restartTimer(downstream->expiryTimer, holdTime);
                break;
            }
        }
    }
}

/**
 * PROCESS JOIN/PRUNE PACKET
 *
 * The method is used to process Join or Prune packet. Joining and Pruning
 * addresses are carried in one PIM packet. This method determine which
 * type (Join/Prune) will be processed.
 *
 * @param pkt Pointer to PIM Join/Prune paket.
 * @see getJoinedSourceAddressArraySize()
 * @see getPrunedSourceAddressArraySize()
 */
void PIMSM::processJoinPrunePacket(PIMJoinPrune *pkt)
{
    EV <<  "pimSM::processJoinPrunePacket" << endl;
    EncodedAddress encodedAddr;

    for (unsigned int i = 0; i < pkt->getMulticastGroupsArraySize(); i++)
    {
        MulticastGroup group = pkt->getMulticastGroups(i);
        IPv4Address multGroup = group.getGroupAddress();

        if (group.getJoinedSourceAddressArraySize() > 0)
        {
            Route *routeG = getRouteFor(multGroup,IPv4Address::UNSPECIFIED_ADDRESS);
            if (routeG)
                cancelAndDeleteTimer(routeG->prunePendingTimer);
            encodedAddr = group.getJoinedSourceAddress(0);
            processJoinPacket(pkt,multGroup,encodedAddr);
        }
        if (group.getPrunedSourceAddressArraySize())
        {
            encodedAddr = group.getPrunedSourceAddress(0);
            processPrunePacket(pkt,multGroup,encodedAddr);
        }
    }
}

/**
 * PROCESS PRUNE PACKET
 *
 * The method is used to process Prune packet from downstream router.
 * If Prune is received, outgoing interface is removed from multicast route.
 * If multicast route doesn't have any  outgoing interface, Prune message is
 * send to upstream neighbor.
 *
 * @param pkt Pointer to PIM Join/Prune paket.
 * @param multgroup Address for multicast group
 * @param encodedAddr Encoded address format
 * @see sendPIMJoinPrune()
 * @see encodedAddr
 */
void PIMSM::processPrunePacket(PIMJoinPrune *pkt, IPv4Address multGroup, EncodedAddress encodedAddr)
{
    EV <<  "pimSM::processPrunePacket: ";

    IPv4ControlInfo *ctrl = check_and_cast<IPv4ControlInfo*>(pkt->getControlInfo());
    InterfaceEntry *outIntToDel = ift->getInterfaceById(ctrl->getInterfaceId());

    if (!encodedAddr.R && !encodedAddr.W && encodedAddr.S)                                              // (S,G) Prune
    {
        EV << "(S,G) Prune processing" << endl;

        Route *routeSG = getRouteFor(multGroup,encodedAddr.IPaddress);
        int i = routeSG->findDownstreamInterface(outIntToDel);
        if (i >= 0)
        {
            EV << "Interface is present, removing it from the list of outgoing interfaces." << endl;
            routeSG->removeDownstreamInterface(i);
//                InterfaceEntry *newInIntG = rt->getInterfaceForDestAddr(this->getRPAddress());
//                routeSG->addOutIntFull(newInIntG,newInIntG->getInterfaceId(),
//                                        Pruned,
//                                        PIMMulticastRoute::Sparsemode,
//                                        NULL,
//                                        NULL,
//                                        NoInfo,
//                                        Join,false);      // create "virtual" outgoing interface to RP
            //routeSG->setRegisterTunnel(false);                   //we need to set register state to output interface, but output interface has to be null for now
        }

        if (routeSG->isOilistNull())
        {
            routeSG->setFlags(Route::P);
            cancelAndDeleteTimer(routeSG->joinTimer);
            if (!IamDR(routeSG->origin))
            {
#if CISCO_SPEC_SIM == 1
                PIMNeighbor *RPFnbr = pimNbt->getFirstNeighborOnInterface(routeSG->upstreamInterface->getInterfaceId());
                sendPIMPrune(multGroup,encodedAddr.IPaddress,RPFnbr->getAddress(),SG);
#else
                routeSG->startPrunePendingTimer();
#endif
            }
        }

    }
    if (encodedAddr.R && encodedAddr.W && encodedAddr.S)    // (*,G) Prune
    {
        EV << "(*,G) Prune processing" << endl;
        std::vector<Route*> routes = getRouteFor(multGroup);
        for (unsigned int j = 0; j < routes.size(); j++)
        {
            Route *route = routes[j];
            IPv4Address multOrigin = route->origin;

            int k = route->findDownstreamInterface(outIntToDel);
            if (k >= 0)
            {
                EV << "Interface is present, removing it from the list of outgoing interfaces." << endl;
                route->removeDownstreamInterface(k);
            }

            // there is no receiver of multicast, prune the router from the multicast tree
            if (route->isOilistNull())
            {
                route->clearFlag(Route::C);
                route->setFlags(Route::P);
                cancelAndDeleteTimer(route->joinTimer);
                // send Prune message
                if (!IamRP(this->getRPAddress()) && (multOrigin == IPv4Address::UNSPECIFIED_ADDRESS))
                {
#if CISCO_SPEC_SIM == 1
                    PIMNeighbor *RPFnbr = pimNbt->getFirstNeighborOnInterface(route->upstreamInterface->getInterfaceId());
                    sendPIMPrune(multGroup,this->getRPAddress(),RPFnbr->getAddress(),G);      // only for non-RP routers in RPT
#else
                    route->startPrunePendingTimer();
#endif
                }
                else if (IamRP(this->getRPAddress()) && (multOrigin != IPv4Address::UNSPECIFIED_ADDRESS))
                {
#if CISCO_SPEC_SIM == 1
                    PIMNeighbor *RPFnbr = pimNbt->getFirstNeighborOnInterface(route->upstreamInterface->getInterfaceId());
                    sendPIMPrune(multGroup,multOrigin,RPFnbr->getAddress(),SG);               // send from RP only if (S,G) is available
#else
                    route->startPrunePendingTimer();
#endif
                }
            }
        }
    }
}

/**
 * PROCESS SG JOIN
 *
 * The method is used to process (S,G) Join PIM message. SG Join is process in
 * source tree between RP and source DR. If (S,G) route doesn't exist is created
 * along with (*,G) route. Otherwise outgoing interface and JT are created.
 *
 * @param pkt Pointer to PIM Join/Prune packet.
 * @param multgroup Address for multicast group
 * @param multOrigin Address for multicast source
 * @see sendPIMJoinPrune()
 * @see getRouteFor()
 */
void PIMSM::processSGJoin(PIMJoinPrune *pkt, IPv4Address multOrigin, IPv4Address multGroup)
{
    Route *newRouteSG = new Route(this, SG, multOrigin, multGroup);
    IPv4ControlInfo *ctrl = check_and_cast<IPv4ControlInfo*>(pkt->getControlInfo());
    InterfaceEntry *newInIntSG = rt->getInterfaceForDestAddr(multOrigin);
    PIMNeighbor *neighborToSrcDR = pimNbt->getFirstNeighborOnInterface(newInIntSG->getInterfaceId());
    Route *routePointer;
    IPv4Address pktSource = ctrl->getSrcAddr();
    int holdTime = pkt->getHoldTime();

    if (!IamDR(multOrigin))
    {
        Route *newRouteG = new Route(this, G, IPv4Address::UNSPECIFIED_ADDRESS, multGroup);
        routePointer = newRouteG;
        if (!(newRouteG = getRouteFor(multGroup, IPv4Address::UNSPECIFIED_ADDRESS)))        // create (*,G) route between RP and source DR
        {
            InterfaceEntry *newInIntG = rt->getInterfaceForDestAddr(this->rpAddr);
            PIMNeighbor *neighborToRP = pimNbt->getFirstNeighborOnInterface(newInIntG->getInterfaceId());

            newRouteG = routePointer;
            newRouteG->rpAddr = this->getRPAddress();
            newRouteG->startKeepAliveTimer();
            newRouteG->setFlags(Route::P);
            newRouteG->upstreamInterface = new UpstreamInterface(newRouteG, newInIntG, neighborToRP->getAddress());
            addGRoute(newRouteG);
            rt->addMulticastRoute(createMulticastRoute(newRouteG));
        }
    }

    routePointer = newRouteSG;
    if (!(newRouteSG = getRouteFor(multGroup, multOrigin)))         // create (S,G) route between RP and source DR
    {
        newRouteSG = routePointer;
        // set source, mult. group, etc...
        newRouteSG->rpAddr = this->getRPAddress();
        newRouteSG->startKeepAliveTimer();

        // set outgoing and incoming interface and ET
        InterfaceEntry *outInt = rt->getInterfaceForDestAddr(this->getRPAddress());

        // RPF check
        if (newInIntSG->getInterfaceId() != outInt->getInterfaceId())
        {
            newRouteSG->upstreamInterface = new UpstreamInterface(newRouteSG, newInIntSG, neighborToSrcDR->getAddress());
            DownstreamInterface *downstream = new DownstreamInterface(newRouteSG, outInt, Forward);
            downstream->startExpiryTimer(holdTime);
            newRouteSG->addDownstreamInterface(downstream);

            if (!IamDR(multOrigin))
                newRouteSG->startJoinTimer();

            addSGRoute(newRouteSG);
            rt->addMulticastRoute(createMulticastRoute(newRouteSG));

            if (!IamDR(multOrigin))
                sendPIMJoin(multGroup,multOrigin,neighborToSrcDR->getAddress(),SG);       // triggered join except DR
        }
    }
    else
    {
        // on source DR isn't RPF check - DR doesn't have incoming interface
        if (IamDR(multOrigin) && (newRouteSG->sequencenumber == 0))
        {
            //InterfaceEntry *outIntf = rt->getInterfaceForDestAddr(pktSource);
            //PIMet *timerEt = createExpiryTimer(outIntf->getInterfaceId(), holdTime, multGroup,multOrigin,SG);

            newRouteSG->clearFlag(Route::P);
            // update interfaces to forwarding state
            for (unsigned j=0; j < newRouteSG->downstreamInterfaces.size(); j++)
            {
                DownstreamInterface *downstream = newRouteSG->downstreamInterfaces[j];
                downstream->startExpiryTimer(holdTime);
                downstream->forwarding = Forward;
                //downstream->expiryTimer = timerEt;
                downstream->shRegTun = true;
            }
            //newRouteSG->setRegisterTunnel(true);
            newRouteSG->sequencenumber = 1;
        }
    }

    // restart ET for given interface - for (*,G) and also (S,G)
    restartExpiryTimer(newRouteSG,rt->getInterfaceForDestAddr(pktSource), holdTime);
}

/**
 * PROCESS JOIN ROUTE G ON RP
 *
 * The method is used to process PIM Join message if (*,G) and (S,G) route exist at RP.
 * Routes for multicast group on RP are find at first and after this is created new
 * outgoing interface and updated or created timers for route - ET or KAT, JT.
 *
 * @param multgroup Address for multicast group
 * @param packetOrigin Source address of packet
 * @param holdtime Time for ET
 * @see sendPIMJoinPrune()
 * @see getRouteFor()
 */
void PIMSM::processJoinRouteGexistOnRP(IPv4Address multGroup, IPv4Address packetOrigin, int msgHoldtime)
{
    Route *routePointer;
    IPv4Address multOrigin;

    std::vector<Route*> routes = getRouteFor(multGroup);       // get all mult. routes at RP
    for (unsigned i=0; i<routes.size();i++)
    {
        routePointer = routes[i];
        if (routePointer->sequencenumber == 0)                                      // only check if route was installed
        {
            multOrigin = routePointer->origin;
            if (multOrigin != IPv4Address::UNSPECIFIED_ADDRESS)                     // for (S,G) route
            {
                UpstreamInterface *inInterface = routePointer->upstreamInterface;

                // update flags
                routePointer->clearFlag(Route::P);
                routePointer->setFlags(Route::T);
                routePointer->startJoinTimer();

                if (routePointer->downstreamInterfaces.size() == 0)                                         // Has route any outgoing interface?
                {
                    InterfaceEntry *interface = rt->getInterfaceForDestAddr(packetOrigin);
                    DownstreamInterface *downstream = new DownstreamInterface(routePointer, interface, Forward);
                    downstream->startExpiryTimer(msgHoldtime);
                    routePointer->addDownstreamInterface(downstream);
                    IPv4MulticastRoute *ipv4Route = findIPv4Route(routePointer->origin, routePointer->group);
                    ipv4Route->addOutInterface(new PIMSMOutInterface(downstream));
                    sendPIMJoin(multGroup, multOrigin, inInterface->nextHop, SG);
                }
                routePointer->sequencenumber = 1;
            }
            else                                                                    // for (*,G) route
            {
                if (routePointer->isFlagSet(Route::P) || routePointer->isFlagSet(Route::F))
                {
                    routePointer->clearFlag(Route::P);
                    routePointer->clearFlag(Route::F);
                }

                cancelAndDeleteTimer(routePointer->keepAliveTimer);

                InterfaceEntry *interface = rt->getInterfaceForDestAddr(packetOrigin);
                if (!routePointer->findDownstreamInterfaceByInterfaceId(interface->getInterfaceId()))
                {
                    DownstreamInterface *downstream = new DownstreamInterface(routePointer, interface, Forward);
                    downstream->startExpiryTimer(msgHoldtime);
                    routePointer->addDownstreamInterface(downstream);
                    IPv4MulticastRoute *ipv4Route = findIPv4Route(routePointer->origin, routePointer->group);
                    ipv4Route->addOutInterface(new PIMSMOutInterface(downstream));
                    if (routePointer->upstreamInterface) // XXX should always have expiryTimer
                        routePointer->upstreamInterface->startExpiryTimer(msgHoldtime);
                }
                routePointer->sequencenumber = 1;
            }
        }
    }
}

/**
 * PROCESS JOIN PACKET
 *
 * The method is used as main method for processing PIM Join messages. Processing is
 * divided into part for (*,G) and (S,G) Join. If Router receive Join message from
 * downstream neighbor and any route doesn't exist, route is created. Otherwise outgoing
 * interface and timers are created or updated.
 *
 * @param pkt Pointer to PIM Join/Prune packet.
 * @param multgroup Address for multicast group
 * @param encodedAddr Encoded address format
 * @see sendPIMJoinPrune()
 * @see restartExpiryTimer()
 * @see getRouteFor()
 */
void PIMSM::processJoinPacket(PIMJoinPrune *pkt, IPv4Address multGroup, EncodedAddress encodedAddr)
{
    IPv4ControlInfo *ctrl = check_and_cast<IPv4ControlInfo*>(pkt->getControlInfo());
    Route *newRouteG = new Route(this, G, IPv4Address::UNSPECIFIED_ADDRESS, multGroup);
    Route *routePointer;
    int msgHoldTime = pkt->getHoldTime();
    IPv4Address pktSource = ctrl->getSrcAddr();
    InterfaceEntry *JoinIncomingInt = rt->getInterfaceForDestAddr(pktSource);
    IPv4Address multOrigin;

    if (!encodedAddr.R && !encodedAddr.W && encodedAddr.S)                                              // (S,G) Join
    {
        multOrigin = encodedAddr.IPaddress;
        processSGJoin(pkt,multOrigin,multGroup);
    }
    if (encodedAddr.R && encodedAddr.W && encodedAddr.S)                                                // (*,G) Join
    {
        routePointer = newRouteG;
        if (!(newRouteG = getRouteFor(multGroup, IPv4Address::UNSPECIFIED_ADDRESS)))                                // check if (*,G) exist
        {
            newRouteG = routePointer;
            InterfaceEntry *newInIntG = rt->getInterfaceForDestAddr(this->rpAddr);                                       // incoming interface
            PIMNeighbor *neighborToRP = pimNbt->getFirstNeighborOnInterface(newInIntG->getInterfaceId());                            // RPF neighbor
            InterfaceEntry *outIntf = JoinIncomingInt;                                      // outgoing interface

            if (JoinIncomingInt->getInterfaceId() != newInIntG->getInterfaceId())
            {
                newRouteG->rpAddr = this->getRPAddress();

                if (!IamRP(this->getRPAddress()))
                {
                    newRouteG->upstreamInterface = new UpstreamInterface(newRouteG, newInIntG, neighborToRP->getAddress()); //  (*,G) route hasn't incoming interface at RP
                    newRouteG->startJoinTimer();              // periodic Join (*,G)
                }

                DownstreamInterface *downstream = new DownstreamInterface(newRouteG, outIntf, Forward);
                downstream->startExpiryTimer(msgHoldTime);
                newRouteG->addDownstreamInterface(downstream);

                if (newRouteG->upstreamInterface) // XXX should always have expiryTimer
                    newRouteG->upstreamInterface->startExpiryTimer(msgHoldTime);

                addGRoute(newRouteG);
                rt->addMulticastRoute(createMulticastRoute(newRouteG));

                if (!IamRP(this->getRPAddress()))
                    sendPIMJoin(multGroup,this->getRPAddress(),neighborToRP->getAddress(),G);                         // triggered Join (*,G)
            }
        }
        else            // (*,G) route exist
        {
            //if (!newRouteG->isRpf(JoinIncomingInt->getInterfaceId()))
            if (!newRouteG->upstreamInterface || newRouteG->upstreamInterface->ie != JoinIncomingInt)
            {
                if (IamRP(this->getRPAddress()))
                    processJoinRouteGexistOnRP(multGroup, pktSource,msgHoldTime);
                else        // (*,G) route exist somewhere in RPT
                {
                    InterfaceEntry *interface = JoinIncomingInt;
                    if (!newRouteG->findDownstreamInterfaceByInterfaceId(interface->getInterfaceId()))
                    {
                        DownstreamInterface *downstream = new DownstreamInterface(newRouteG, interface, Forward);
                        downstream->startExpiryTimer(msgHoldTime);                        newRouteG->addDownstreamInterface(downstream);
                        IPv4MulticastRoute *ipv4Route = findIPv4Route(newRouteG->origin, newRouteG->group);
                        ipv4Route->addOutInterface(new PIMSMOutInterface(downstream));
                    }
                }
                // restart ET for given interface - for (*,G) and also (S,G)
                restartExpiryTimer(newRouteG,JoinIncomingInt, msgHoldTime);
                std::vector<Route*> routes = getRouteFor(multGroup);
                for (unsigned i=0; i<routes.size();i++)
                {
                    routePointer = routes[i];
                    //restart ET for (S,G)
                    if (routePointer->origin != IPv4Address::UNSPECIFIED_ADDRESS)
                        restartExpiryTimer(routePointer,JoinIncomingInt, msgHoldTime);
                }
            }
        }
    }
}

/**
 * PROCESS REGISTER PACKET
 *
 * The method is used for processing PIM Register message sended from source DR.
 * If PIM Register isn't Null and route doesn't exist, it is created and PIM Register-Stop
 * sended. If PIM Register is Null, Register-Stop is send.
 *
 * @param pkt Pointer to PIM Join/Prune packet.
 * @see sendPIMRegisterStop()
 * @see setKat()
 * @see getRouteFor()
 */
void PIMSM::processRegisterPacket(PIMRegister *pkt)
{
    EV << "pimSM:processRegisterPacket" << endl;

    Route *routePointer;
    IPv4Datagram *encapData = check_and_cast<IPv4Datagram*>(pkt->decapsulate());
    IPv4Address multOrigin = encapData->getSrcAddress();
    IPv4Address multGroup = encapData->getDestAddress();
    Route *newRouteG = new Route(this, G, IPv4Address::UNSPECIFIED_ADDRESS,multGroup);
    Route *newRoute = new Route(this, SG, multOrigin,multGroup);

    if (!pkt->getN())                                                                                       //It is Null Register ?
    {
        routePointer = newRouteG;
        if (!(newRouteG = getRouteFor(multGroup, IPv4Address::UNSPECIFIED_ADDRESS)))                    // check if exist (*,G)
        {
            newRouteG = routePointer;
            newRouteG->rpAddr = this->getRPAddress();
            newRouteG->setFlags(Route::P);                                                // create and set (*,G) KAT timer, add to routing table
            newRouteG->startKeepAliveTimer();
            addGRoute(newRouteG);
            rt->addMulticastRoute(createMulticastRoute(newRouteG));
        }
        routePointer = newRoute;                                                                            // check if exist (S,G)
        if (!(newRoute = getRouteFor(multGroup,multOrigin)))
        {
            InterfaceEntry *newInIntG = rt->getInterfaceForDestAddr(multOrigin);
            PIMNeighbor *pimIntfToDR = pimNbt->getFirstNeighborOnInterface(newInIntG->getInterfaceId());
            newRoute = routePointer;
            newRoute->upstreamInterface = new UpstreamInterface(newRoute, newInIntG, pimIntfToDR->getAddress());
            newRoute->rpAddr = this->getRPAddress();
            newRoute->setFlags(Route::P);
            newRoute->startKeepAliveTimer();   // create and set (S,G) KAT timer, add to routing table
            addSGRoute(newRoute);
            rt->addMulticastRoute(createMulticastRoute(newRoute));
        }
                                                                                                            // we have some active receivers
        if (!newRouteG->isOilistNull())
        {
            // copy out interfaces from newRouteG
            IPv4MulticastRoute *ipv4Route = findIPv4Route(newRoute->origin, newRoute->group);
            newRoute->clearDownstreamInterfaces();
            ipv4Route->clearOutInterfaces();
            for (unsigned int i = 0; i < newRouteG->downstreamInterfaces.size(); i++)
            {
                DownstreamInterface *downstream = new DownstreamInterface(*(newRouteG->downstreamInterfaces[i])); // XXX
                downstream->owner = newRoute;
                downstream->expiryTimer = NULL;
                downstream->startExpiryTimer(joinPruneHoldTime());
                newRoute->addDownstreamInterface(downstream);
                ipv4Route->addOutInterface(new PIMSMOutInterface(downstream));
            }

            newRoute->clearFlag(Route::P);                                                                        // update flags for SG route

            if (!newRoute->isFlagSet(Route::T))                                                                    // only if isn't build SPT between RP and registering DR
            {
                for (unsigned i=0; i < newRouteG->downstreamInterfaces.size(); i++)
                {
                    DownstreamInterface *downstream = newRouteG->downstreamInterfaces[i];
                    if (downstream->forwarding == Forward)                           // for active outgoing interface forward encapsulated data
                        forwardMulticastData(encapData->dup(), downstream->getInterfaceId());
                }
                // send Join(S,G) toward source to establish SPT between RP and registering DR
                Route *routeSG = getRouteFor(multGroup, multOrigin);
                UpstreamInterface *rpfInterface = routeSG->upstreamInterface;
                sendPIMJoin(multGroup,multOrigin, rpfInterface->nextHop, SG);

                // send register-stop packet
                IPv4ControlInfo *PIMctrl = check_and_cast<IPv4ControlInfo*>(pkt->getControlInfo());
                sendPIMRegisterStop(PIMctrl->getDestAddr(),PIMctrl->getSrcAddr(),multGroup,multOrigin);
            }
        }
        if (newRoute->keepAliveTimer)                                                                             // refresh KAT timers
        {
            EV << " (S,G) KAT timer refresh" << endl;
            restartTimer(newRoute->keepAliveTimer, KAT);
        }
        if (newRouteG->keepAliveTimer)
        {
            EV << " (*,G) KAT timer refresh" << endl;
            restartTimer(newRouteG->keepAliveTimer, 2*KAT);
        }
    }
    else
    {                                                                                                       //get routes for next step if register is register-null
        newRoute =  getRouteFor(multGroup,multOrigin);
        newRouteG = getRouteFor(multGroup, IPv4Address::UNSPECIFIED_ADDRESS);
    }

    //if (newRoute)
    if (newRouteG)
    {
        //if ((newRoute->isFlagSet(P) && newRouteG->isFlagSet(P)) || pkt->getN())
        if (newRouteG->isFlagSet(Route::P) || pkt->getN())
        {                                                                                                       // send register-stop packet
            IPv4ControlInfo *PIMctrl = check_and_cast<IPv4ControlInfo*>(pkt->getControlInfo());
            sendPIMRegisterStop(PIMctrl->getDestAddr(),PIMctrl->getSrcAddr(),multGroup,multOrigin);
        }
    }

    delete encapData;
    delete pkt;
}

/**
 * PROCESS REGISTER STOP PACKET
 *
 * The method is used for processing PIM Register-Stop message sended from RP.
 * If the message is received Register Tunnel between RP and source DR is set
 * from Join status revert to Prune status. Also Register Stop Timer is created
 * for periodic sending PIM Register Null messages.
 *
 * @param pkt Pointer to PIM Join/Prune packet.
 * @see sendPIMRegisterStop()
 * @see setKat()
 * @see getRouteFor()
 */
void PIMSM::processRegisterStopPacket(PIMRegisterStop *pkt)
{
    EV << "pimSM:processRegisterStopPacket" << endl;

    InterfaceEntry *intToRP = rt->getInterfaceForDestAddr(this->getRPAddress());
    Route *routeSG = getRouteFor(pkt->getGroupAddress(),pkt->getSourceAddress());
    if (routeSG)
    {
        // Set RST timer for S,G route
        routeSG->startRegisterStopTimer();

        DownstreamInterface *outInterface = routeSG->findDownstreamInterfaceByInterfaceId(intToRP->getInterfaceId());
        if (outInterface && outInterface->regState == RS_JOIN)
        {
            EV << "Register tunnel is connect - has to be disconnect" << endl;
            outInterface->regState = RS_PRUNE;
        }
        else
            EV << "Register tunnel is disconnect" << endl;
    }
}

void PIMSM::processAssertPacket(PIMAssert *pkt)
{
    // TODO
}


void PIMSM::sendPIMJoin(IPv4Address group, IPv4Address source, IPv4Address upstreamNeighbor, RouteType routeType)
{
    EV_INFO << "Sending Join(S=" << source << ", G=" << group << ") to neighbor " << upstreamNeighbor << ".\n";

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
    EV_INFO << "Sending Prune(S=" << source << ", G=" << group << ") to neighbor " << upstreamNeighbor << ".\n";

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
    if (getRouteFor(multGroup,IPv4Address::UNSPECIFIED_ADDRESS))
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
        Route *newRouteSG = new Route(this, SG, source, group);
        newRouteSG->setFlags(Route::P | Route::F | Route::T);
        newRouteSG->rpAddr = this->getRPAddress();
        newRouteSG->startKeepAliveTimer();
        newRouteSG->upstreamInterface = new UpstreamInterface(newRouteSG, interfaceTowardSource, IPv4Address("0.0.0.0"));
        newRouteSG->addDownstreamInterface(new DownstreamInterface(newRouteSG, interfaceTowardRP, Pruned, RS_JOIN,false));      // create new outgoing interface to RP

        addSGRoute(newRouteSG);
        rt->addMulticastRoute(createMulticastRoute(newRouteSG));

        // create new (*,G) route
        Route *newRouteG = new Route(this, G, IPv4Address::UNSPECIFIED_ADDRESS,newRouteSG->group);
        newRouteG->setFlags(Route::P | Route::F);
        newRouteG->rpAddr = this->getRPAddress();
        newRouteG->startKeepAliveTimer();
        PIMNeighbor *rpfNeighbor = pimNbt->getFirstNeighborOnInterface(interfaceTowardRP->getInterfaceId());
        newRouteG->upstreamInterface = new UpstreamInterface(newRouteG, interfaceTowardRP, rpfNeighbor->getAddress());

        addGRoute(newRouteG);
        rt->addMulticastRoute(createMulticastRoute(newRouteG));
    }
}

void PIMSM::multicastReceiverRemoved(InterfaceEntry *ie, IPv4Address group)
{
    EV_DETAIL << "No more receiver for group " << group << " on interface '" << ie->getName() << "'.\n";

    for (SGStateMap::iterator it = routes.begin(); it != routes.end(); ++it)
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

        route->clearFlag(Route::C);

        // there is no receiver of multicast, prune the router from the multicast tree
        if (route->isOilistNull())
        {
            route->setFlags(Route::P);
            PIMNeighbor *neighborToRP = pimNbt->getFirstNeighborOnInterface(route->upstreamInterface->getInterfaceId());
            sendPIMPrune(route->group,this->getRPAddress(),neighborToRP->getAddress(),G);
            cancelAndDeleteTimer(route->joinTimer);
        }
    }
}

void PIMSM::multicastReceiverAdded(InterfaceEntry *ie, IPv4Address group)
{
    EV_DETAIL << "Multicast receiver added for group " << group << " on interface '" << ie->getName() << "'.\n";

    Route *routeG = findGRoute(group);
    if (!routeG)
    {
        // create new (*,G) route
        Route *newRouteG = new Route(this, G, IPv4Address::UNSPECIFIED_ADDRESS,group);
        newRouteG->rpAddr = this->getRPAddress();
        newRouteG->setFlags(Route::C);
        newRouteG->startJoinTimer();

        // set upstream interface
        InterfaceEntry *ieTowardRP = rt->getInterfaceForDestAddr(this->getRPAddress());
        PIMNeighbor *neighborTowardRP = pimNbt->getFirstNeighborOnInterface(ieTowardRP->getInterfaceId()); // XXX neighborTowardRP can be NULL!
        newRouteG->upstreamInterface = new UpstreamInterface(newRouteG, ieTowardRP, neighborTowardRP->getAddress());
        newRouteG->upstreamInterface->startExpiryTimer(joinPruneHoldTime());

        // add downstream interface
        DownstreamInterface *downstream = new DownstreamInterface(newRouteG, ie, Forward);
        downstream->startExpiryTimer(HOLDTIME_HOST);
        newRouteG->addDownstreamInterface(downstream);

        // add route to tables
        addGRoute(newRouteG);
        rt->addMulticastRoute(createMulticastRoute(newRouteG));

        // oilist != NULL -> send Join(*,G) to 224.0.0.13
        if (!newRouteG->isOilistNull())
            sendPIMJoin(group, newRouteG->rpAddr, neighborTowardRP->getAddress(), G);
    }
    else                                                                                          // add new outgoing interface to existing (*,G) route
    {
        DownstreamInterface *downstream = new DownstreamInterface(routeG, ie, Forward);
        // downstream->startExpiryTimer(joinPruneHoldTime());
        routeG->addDownstreamInterface(downstream);
        routeG->setFlags(Route::C);

        IPv4MulticastRoute *ipv4Route = findIPv4Route(IPv4Address::UNSPECIFIED_ADDRESS, group);
        ipv4Route->addOutInterface(new PIMSMOutInterface(downstream));
    }
}

void PIMSM::multicastPacketForwarded(IPv4Datagram *datagram)
{
    IPv4Address source = datagram->getSrcAddress();
    IPv4Address group = datagram->getDestAddress();

    Route *routeSG = findSGRoute(source, group);
    if (!routeSG || !routeSG->isFlagSet(Route::F) || !routeSG->isFlagSet(Route::P))
        return;

    // send Register message to RP

    InterfaceEntry *interfaceTowardRP = rt->getInterfaceForDestAddr(routeSG->rpAddr);
    DownstreamInterface *downstream = routeSG->findDownstreamInterfaceByInterfaceId(interfaceTowardRP->getInterfaceId());

    if (downstream && downstream->regState == RS_JOIN)
    {
        Route *routeG = findGRoute(group);

        // refresh KAT timers
        if (routeSG->keepAliveTimer)
        {
            EV << " (S,G) KAT timer refresh" << endl;
            restartTimer(routeSG->keepAliveTimer, KAT);
        }
        if (routeG->keepAliveTimer)
        {
            EV << " (*,G) KAT timer refresh" << endl;
            restartTimer(routeG->keepAliveTimer, 2*KAT);
        }

        sendPIMRegister(datagram, routeSG->rpAddr, interfaceTowardRP->getInterfaceId());
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
        default:
            EV << "BAD TYPE, DROPPED" << endl;
            delete timer;
            break;
    }
}

/**
 * PROCESS PIM PACKET
 *
 * The method is used to process PIM packets. According to type of PIM packet, the packet is sent to
 * appropriate method.
 *
 * @param pkt Pointer to PIM packet.
 * @see PIMPacket()
 */
void PIMSM::processPIMPkt(PIMPacket *pkt)
{
    EV << "pimSM::processPIMPkt: ";

    switch(pkt->getType())
    {
        case Hello:
            EV << "Hello" << endl;
            processHelloPacket(check_and_cast<PIMHello*>(pkt));
            break;
        case JoinPrune:
            EV << "JoinPrune" << endl;
            processJoinPrunePacket(check_and_cast<PIMJoinPrune *> (pkt));
            break;
        case Register:
            EV << "Register" << endl;
            processRegisterPacket(check_and_cast<PIMRegister *> (pkt));
            break;
        case RegisterStop:
            EV << "Register-stop" << endl;
            processRegisterStopPacket(check_and_cast<PIMRegisterStop *> (pkt));
            break;
        case Assert:
            EV << "Assert" << endl;
            processAssertPacket(check_and_cast<PIMAssert*>(pkt));
            break;
        default:
            EV << "BAD TYPE, DROPPED" << endl;
            delete pkt;
            break;
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
            route = getRouteFor(datagram->getDestAddress(), IPv4Address::UNSPECIFIED_ADDRESS);
            if (route)
                multicastPacketArrivedOnRpfInterface(route);
            route = getRouteFor(datagram->getDestAddress(), datagram->getSrcAddress());
            if (route)
                multicastPacketArrivedOnRpfInterface(route);
        }
    }
    else if (signalID == NF_IPv4_MDATA_REGISTER)
    {
        EV <<  "pimSM::receiveChangeNotification - REGISTER DATA" << endl;
        datagram = check_and_cast<IPv4Datagram*>(details);
        PIMInterface *incomingInterface = getIncomingInterface(datagram);
        route = getRouteFor(datagram->getDestAddress(), datagram->getSrcAddress());
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
        IPv4MulticastRoute *ipv4Route = findIPv4Route(route->origin, route->group);
        if (ipv4Route)
            rt->deleteMulticastRoute(ipv4Route);

        delete route;
        return true;
    }
    return false;
}

PIMSM::Route *PIMSM::getRouteFor(IPv4Address group, IPv4Address source)
{
    return source.isUnspecified() ? findGRoute(group) : findSGRoute(source, group);
}

std::vector<PIMSM::Route*> PIMSM::getRouteFor(IPv4Address group)
{
    std::vector<Route*> result;
    for (SGStateMap::iterator it = routes.begin(); it != routes.end(); ++it)
    {
        Route *route = it->second;
        if (route->group == group)
            result.push_back(route);
    }
    return result;
}

void PIMSM::addGRoute(Route *route)
{
    SourceAndGroup sg(IPv4Address::UNSPECIFIED_ADDRESS, route->group);
    routes[sg] = route;
}

void PIMSM::addSGRoute(Route *route)
{
    SourceAndGroup sg(route->origin, route->group);
    routes[sg] = route;
}

IPv4MulticastRoute *PIMSM::createMulticastRoute(Route *route)
{
    IPv4MulticastRoute *newRoute = new IPv4MulticastRoute();
    newRoute->setOrigin(route->origin);
    newRoute->setOriginNetmask(route->origin.isUnspecified() ? IPv4Address::UNSPECIFIED_ADDRESS : IPv4Address::ALLONES_ADDRESS);
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
    SourceAndGroup sg(route->origin, route->group);
    return routes.erase(sg);
}

PIMSM::Route *PIMSM::findGRoute(IPv4Address group)
{
    SourceAndGroup sg(IPv4Address::UNSPECIFIED_ADDRESS, group);
    SGStateMap::iterator it = routes.find(sg);
    return it != routes.end() ? it->second : NULL;
}

PIMSM::Route *PIMSM::findSGRoute(IPv4Address source, IPv4Address group)
{
    SourceAndGroup sg(source, group);
    SGStateMap::iterator it = routes.find(sg);
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
    IPv4MulticastRoute *ipv4Route = owner->findIPv4Route(origin, group);
    ipv4Route->removeOutInterface(i);

    DownstreamInterface *outInterface = downstreamInterfaces[i];
    downstreamInterfaces.erase(downstreamInterfaces.begin()+i);
    delete outInterface;
}

void PIMSM::Interface::startExpiryTimer(double holdTime)
{
    expiryTimer = new cMessage("PIMExpiryTimer", ExpiryTimer);
    expiryTimer->setContextPointer(this);
    owner->owner->scheduleAt(simTime() + holdTime, expiryTimer);
}

void PIMSM::Route::startKeepAliveTimer()
{
    keepAliveTimer = new cMessage("PIMKeepAliveTimer", KeepAliveTimer);
    keepAliveTimer->setContextPointer(this);
    double interval = group.isUnspecified() ?
                        2 * KAT : // for (*,G)
                        KAT;      // for (S,G)
    owner->scheduleAt(simTime() + interval, keepAliveTimer);
}

void PIMSM::Route::startRegisterStopTimer()
{
    registerStopTimer = new cMessage("PIMRegisterStopTimer", RegisterStopTimer);
    registerStopTimer->setContextPointer(this);

    // The Register-Stop Timer is set to a random value chosen
    // uniformly from the interval ( 0.5 * Register_Suppression_Time,
    // 1.5 * Register_Suppression_Time) minus Register_Probe_Time.
    // Subtracting off Register_Probe_Time is a bit unnecessary because
    // it is really small compared to Register_Suppression_Time, but
    // this was in the old spec and is kept for compatibility.
    // XXX randomize
#if CISCO_SPEC_SIM == 1
    owner->scheduleAt(simTime() + owner->registerSuppressionTime, registerStopTimer);
#else
    owner->scheduleAt(simTime() + owner->registerSuppressionTime - owner->registerProbeTime, registerStopTimer);
#endif
}

void PIMSM::Route::startJoinTimer()
{
    joinTimer = new cMessage("PIMJoinTimer", JoinTimer);
    joinTimer->setContextPointer(this);
    owner->scheduleAt(simTime() + owner->joinPrunePeriod, joinTimer);
}

void PIMSM::Route::startPrunePendingTimer()
{
    prunePendingTimer = new cMessage("PIMPrunePendingTimer", PrunePendingTimer);
    prunePendingTimer->setContextPointer(this);
    owner->scheduleAt(simTime() + owner->joinPruneOverrideInterval(), prunePendingTimer);
}
