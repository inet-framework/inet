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

PIMSM::DownstreamInterface *PIMSM::Route::addNewDownstreamInterface(InterfaceEntry *ie, int holdTime)
{
    DownstreamInterface *downstream = new DownstreamInterface(this, ie, DownstreamInterface::JOIN);
    downstream->startExpiryTimer(holdTime);
    addDownstreamInterface(downstream);

    IPv4MulticastRoute *ipv4Route = owner->findIPv4Route(origin, group);
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
        Route *routeG = findGRoute(route->group);
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
            Route *routeG = findGRoute(groupAddr);
            if (routeG)
                cancelAndDeleteTimer(routeG->prunePendingTimer);
            EncodedAddress &source = group.getJoinedSourceAddress(j);
            if (source.S)
            {
                if (source.W)      // (*,G) Join
                    processJoinG(groupAddr, source.IPaddress, upstreamNeighbor, holdTime, inInterface);
                else if (source.R) // (S,G,rpt) Join
                    processJoinSGrpt(source.IPaddress, groupAddr, holdTime, inInterface);
                else               // (S,G) Join
                    processJoinSG(source.IPaddress, groupAddr, holdTime, inInterface);
            }
        }

        // go through list of pruned sources
        for(unsigned int j = 0; j < group.getPrunedSourceAddressArraySize(); j++)
        {
            EncodedAddress &source = group.getPrunedSourceAddress(j);
            if (source.S)
            {
                if (source.W)      // (*,G) Prune
                    processPruneG(groupAddr, inInterface);
                else if (source.R) // (S,G,rpt) Prune
                    processPruneSGrpt(source.IPaddress, groupAddr, inInterface);
                else               // (S,G) Prune
                    processPruneSG(source.IPaddress, groupAddr, inInterface);
            }
        }
    }

    delete pkt;
}

void PIMSM::processJoinG(IPv4Address group, IPv4Address rp, IPv4Address target, int holdTime, InterfaceEntry *inInterface)
{
    // TODO RP check

    //
    // Downstream per-interface (*,G) state machine; event = Receive Join(*,G)
    //

    // check UpstreamNeighbor field
    if (target != inInterface->ipv4Data()->getIPAddress())
        return;

    Route *routeG = findGRoute(group);
    if (!routeG)                                // check if (*,G) exist
    {
        InterfaceEntry *rpfInterface = rt->getInterfaceForDestAddr(this->rpAddr);
        PIMNeighbor *neighborToRP = pimNbt->getFirstNeighborOnInterface(rpfInterface->getInterfaceId());

        if (inInterface != rpfInterface)
        {
            Route *newRouteG = createRouteG(group, Route::NO_FLAG);

            if (!IamRP(rp))
            {
                newRouteG->upstreamInterface = new UpstreamInterface(newRouteG, rpfInterface, neighborToRP->getAddress()); //  (*,G) route hasn't incoming interface at RP
                newRouteG->startJoinTimer();              // periodic Join (*,G)
            }

            DownstreamInterface *downstream = new DownstreamInterface(newRouteG, inInterface, DownstreamInterface::JOIN);
            downstream->startExpiryTimer(holdTime);
            newRouteG->addDownstreamInterface(downstream);

            if (newRouteG->upstreamInterface) // XXX should always have expiryTimer
                newRouteG->upstreamInterface->startExpiryTimer(holdTime);

            addGRoute(newRouteG);

            if (!IamRP(this->getRPAddress()))
                sendPIMJoin(group,this->getRPAddress(),neighborToRP->getAddress(),G);                         // triggered Join (*,G)
        }
    }
    else            // (*,G) route exist
    {
        //if (!routeG->isRpf(inInterface->getInterfaceId()))
        if (!routeG->upstreamInterface || routeG->upstreamInterface->ie != inInterface)
        {
            if (IamRP(rp)) // (*,G) route exists at RP
            {
                for (SGStateMap::iterator it = routes.begin(); it != routes.end(); ++it)
                {
                    Route *route = it->second;
                    if (route->group == group && route->sequencenumber == 0)   // only check if route was installed
                    {
                        if (route->type == SG)
                        {
                            // update flags
                            route->clearFlag(Route::P);
                            route->setFlags(Route::T);
                            route->startJoinTimer();

                            if (route->downstreamInterfaces.empty())          // Has route any outgoing interface?
                            {
                                route->addNewDownstreamInterface(inInterface, holdTime);
                                sendPIMJoin(group, route->origin, route->upstreamInterface->nextHop, SG);
                            }
                        }
                        else if (route->type == G)
                        {
                            route->clearFlag(Route::P);
                            route->clearFlag(Route::F);
                            cancelAndDeleteTimer(route->keepAliveTimer);

                            if (route->findDownstreamInterface(inInterface) < 0)
                            {
                                route->addNewDownstreamInterface(inInterface, holdTime);
                                if (route->upstreamInterface) // XXX should always have expiryTimer
                                    route->upstreamInterface->startExpiryTimer(holdTime);
                            }
                        }

                        route->sequencenumber = 1;
                    }
                }
            }
            else        // (*,G) route exist somewhere in RPT
            {
                if (routeG->findDownstreamInterface(inInterface) < 0)
                    routeG->addNewDownstreamInterface(inInterface, holdTime);
            }

            // restart ET for given interface - for (*,G) and also (S,G)
            restartExpiryTimer(routeG, inInterface, holdTime);
            for (SGStateMap::iterator it = routes.begin(); it != routes.end(); ++it)
            {
                Route *routeSG = it->second;
                if (routeSG->group == group && !routeSG->origin.isUnspecified())
                {
                    //restart ET for (S,G)
                    restartExpiryTimer(routeSG, inInterface, holdTime);
                }
            }
        }
    }
}

/**
 * The method is used to process (S,G) Join PIM message. SG Join is process in
 * source tree between RP and source DR. If (S,G) route doesn't exist is created
 * along with (*,G) route. Otherwise outgoing interface and JT are created.
 */
void PIMSM::processJoinSG(IPv4Address source, IPv4Address group, int holdTime, InterfaceEntry *inInterface)
{
    if (!IamDR(source))
    {
        Route *routeG = findGRoute(group);
        if (!routeG)        // create (*,G) route between RP and source DR
        {
            Route *newRouteG = createRouteG(group, Route::P);
            InterfaceEntry *newInIntG = rt->getInterfaceForDestAddr(this->rpAddr);
            PIMNeighbor *neighborToRP = pimNbt->getFirstNeighborOnInterface(newInIntG->getInterfaceId());

            newRouteG->startKeepAliveTimer();
            newRouteG->upstreamInterface = new UpstreamInterface(newRouteG, newInIntG, neighborToRP->getAddress());
            addGRoute(newRouteG);
        }
    }

    Route *routeSG = findSGRoute(source, group);
    if (!routeSG)         // create (S,G) route between RP and source DR
    {
        InterfaceEntry *newInIntSG = rt->getInterfaceForDestAddr(source);
        PIMNeighbor *neighborToSrcDR = pimNbt->getFirstNeighborOnInterface(newInIntSG->getInterfaceId());

        routeSG = createRouteSG(source, group, Route::NO_FLAG);
        routeSG->startKeepAliveTimer();

        // set outgoing and incoming interface and ET
        InterfaceEntry *outInt = rt->getInterfaceForDestAddr(this->getRPAddress());

        // RPF check
        if (newInIntSG->getInterfaceId() != outInt->getInterfaceId())
        {
            routeSG->upstreamInterface = new UpstreamInterface(routeSG, newInIntSG, neighborToSrcDR->getAddress());
            DownstreamInterface *downstream = new DownstreamInterface(routeSG, outInt, DownstreamInterface::JOIN);
            downstream->startExpiryTimer(holdTime);
            routeSG->addDownstreamInterface(downstream);

            if (!IamDR(source))
                routeSG->startJoinTimer();

            addSGRoute(routeSG);

            if (!IamDR(source))
                sendPIMJoin(group, source, neighborToSrcDR->getAddress(), SG);       // triggered join except DR
        }
    }
    else
    {
        // on source DR isn't RPF check - DR doesn't have incoming interface
        if (IamDR(source) && (routeSG->sequencenumber == 0))
        {
            //InterfaceEntry *outIntf = rt->getInterfaceForDestAddr(pktSource);
            //PIMet *timerEt = createExpiryTimer(outIntf->getInterfaceId(), holdTime, multGroup,multOrigin,SG);

            routeSG->clearFlag(Route::P);
            // update interfaces to forwarding state
            for (unsigned j=0; j < routeSG->downstreamInterfaces.size(); j++)
            {
                DownstreamInterface *downstream = routeSG->downstreamInterfaces[j];
                downstream->startExpiryTimer(holdTime);
                downstream->joinPruneState = DownstreamInterface::JOIN;
                //downstream->expiryTimer = timerEt;
                downstream->shRegTun = true;
            }
            routeSG->sequencenumber = 1;
        }
    }

    // restart ET for given interface - for (*,G) and also (S,G)
    restartExpiryTimer(routeSG, inInterface, holdTime);
}


void PIMSM::processJoinSGrpt(IPv4Address multOrigin, IPv4Address multGroup, int holdTime, InterfaceEntry *inInterface)
{
    // TODO
}

void PIMSM::processPruneG(IPv4Address group, InterfaceEntry *inInterface)
{
    EV_DETAIL << "Processing Prune(*," << group << ") received on interface '" << inInterface->getName() << "'.\n";

    for (SGStateMap::iterator it = routes.begin(); it != routes.end(); ++it)
    {
        Route *route = it->second;
        if (route->group == group)
        {
            int k = route->findDownstreamInterface(inInterface);
            if (k >= 0)
            {
                EV << "Interface is present, removing it from the list of outgoing interfaces." << endl;
                route->removeDownstreamInterface(k);
            }

            if (route->isOilistNull())
            {
                route->clearFlag(Route::C);
                route->setFlags(Route::P);
                cancelAndDeleteTimer(route->joinTimer);
                bool iAmRP = IamRP(route->rpAddr);
                if ((route->type == G && !iAmRP) || (route->type == SG && iAmRP))
                {
#if CISCO_SPEC_SIM == 1
                    PIMNeighbor *RPFnbr = pimNbt->getFirstNeighborOnInterface(route->upstreamInterface->getInterfaceId());
                    sendPIMPrune(group, route->type == G ? route->rpAddr : route->origin, RPFnbr->getAddress(), route->type);
#else
                    route->startPrunePendingTimer();
#endif
                }
            }
        }
    }
}

void PIMSM::processPruneSG(IPv4Address source, IPv4Address group, InterfaceEntry *inInterface)
{
    EV_DETAIL << "Processing Prune(" << source << ", " << group <<") received on interface '" << inInterface->getName() << "'.'n";

    Route *routeSG = findSGRoute(source, group);
    int i = routeSG->findDownstreamInterface(inInterface);
    if (i >= 0)
    {
        EV << "Interface is present, removing it from the list of outgoing interfaces." << endl;
        routeSG->removeDownstreamInterface(i);
    }

    if (routeSG->isOilistNull())
    {
        routeSG->setFlags(Route::P);
        cancelAndDeleteTimer(routeSG->joinTimer);
        if (!IamDR(source))
        {
#if CISCO_SPEC_SIM == 1
            PIMNeighbor *RPFnbr = pimNbt->getFirstNeighborOnInterface(routeSG->upstreamInterface->getInterfaceId());
            sendPIMPrune(group, source, RPFnbr->getAddress(), SG);
#else
            routeSG->startPrunePendingTimer();
#endif
        }
    }
}

void PIMSM::processPruneSGrpt(IPv4Address source, IPv4Address group, InterfaceEntry *inInterface)
{
    // TODO
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
    Route *newRouteG = createRouteG(multGroup, Route::P);
    Route *newRoute = createRouteSG(multOrigin, multGroup, Route::P);

    if (!pkt->getN())                                                                                       //It is Null Register ?
    {
        routePointer = newRouteG;
        if (!(newRouteG = findGRoute(multGroup)))                    // check if exist (*,G)
        {
            newRouteG = routePointer;
            newRouteG->startKeepAliveTimer();
            addGRoute(newRouteG);
        }
        routePointer = newRoute;                                                                            // check if exist (S,G)
        if (!(newRoute = findSGRoute(multOrigin, multGroup)))
        {
            InterfaceEntry *newInIntG = rt->getInterfaceForDestAddr(multOrigin);
            PIMNeighbor *pimIntfToDR = pimNbt->getFirstNeighborOnInterface(newInIntG->getInterfaceId());
            newRoute = routePointer;
            newRoute->upstreamInterface = new UpstreamInterface(newRoute, newInIntG, pimIntfToDR->getAddress());
            newRoute->startKeepAliveTimer();   // create and set (S,G) KAT timer, add to routing table
            addSGRoute(newRoute);
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
                    if (downstream->isInOlist())                           // for active outgoing interface forward encapsulated data
                        forwardMulticastData(encapData->dup(), downstream->getInterfaceId());
                }
                // send Join(S,G) toward source to establish SPT between RP and registering DR
                Route *routeSG = findSGRoute(multOrigin, multGroup);
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
        newRoute =  findSGRoute(multOrigin, multGroup);
        newRouteG = findGRoute(multGroup);
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
    Route *routeSG = findSGRoute(pkt->getSourceAddress(), pkt->getGroupAddress());
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
    if (findGRoute(multGroup))
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
        Route *newRouteSG = createRouteSG(source, group, Route::P | Route::F | Route::T);
        newRouteSG->startKeepAliveTimer();
        newRouteSG->upstreamInterface = new UpstreamInterface(newRouteSG, interfaceTowardSource, IPv4Address("0.0.0.0"));
        newRouteSG->addDownstreamInterface(new DownstreamInterface(newRouteSG, interfaceTowardRP, DownstreamInterface::NO_INFO, RS_JOIN,false));      // create new outgoing interface to RP

        addSGRoute(newRouteSG);

        // create new (*,G) route
        Route *newRouteG = createRouteG(newRouteSG->group, Route::P | Route::F);
        newRouteG->startKeepAliveTimer();
        PIMNeighbor *rpfNeighbor = pimNbt->getFirstNeighborOnInterface(interfaceTowardRP->getInterfaceId());
        newRouteG->upstreamInterface = new UpstreamInterface(newRouteG, interfaceTowardRP, rpfNeighbor->getAddress());

        addGRoute(newRouteG);
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
        Route *newRouteG = createRouteG(group, Route::C);
        newRouteG->startJoinTimer();

        // set upstream interface
        InterfaceEntry *ieTowardRP = rt->getInterfaceForDestAddr(this->getRPAddress());
        PIMNeighbor *neighborTowardRP = pimNbt->getFirstNeighborOnInterface(ieTowardRP->getInterfaceId()); // XXX neighborTowardRP can be NULL!
        newRouteG->upstreamInterface = new UpstreamInterface(newRouteG, ieTowardRP, neighborTowardRP->getAddress());
        newRouteG->upstreamInterface->startExpiryTimer(joinPruneHoldTime());

        // add downstream interface
        DownstreamInterface *downstream = new DownstreamInterface(newRouteG, ie, DownstreamInterface::JOIN);
        downstream->startExpiryTimer(HOLDTIME_HOST);
        newRouteG->addDownstreamInterface(downstream);

        // add route to tables
        addGRoute(newRouteG);

        // oilist != NULL -> send Join(*,G) to 224.0.0.13
        if (!newRouteG->isOilistNull())
            sendPIMJoin(group, newRouteG->rpAddr, neighborTowardRP->getAddress(), G);
    }
    else                                                                                          // add new outgoing interface to existing (*,G) route
    {
        DownstreamInterface *downstream = new DownstreamInterface(routeG, ie, DownstreamInterface::JOIN);
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
            route = findGRoute(datagram->getDestAddress());
            if (route)
                multicastPacketArrivedOnRpfInterface(route);
            route = findSGRoute(datagram->getSrcAddress(), datagram->getDestAddress());
            if (route)
                multicastPacketArrivedOnRpfInterface(route);
        }
    }
    else if (signalID == NF_IPv4_MDATA_REGISTER)
    {
        EV <<  "pimSM::receiveChangeNotification - REGISTER DATA" << endl;
        datagram = check_and_cast<IPv4Datagram*>(details);
        PIMInterface *incomingInterface = getIncomingInterface(datagram);
        route = findSGRoute(datagram->getSrcAddress(), datagram->getDestAddress());
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

void PIMSM::addGRoute(Route *route)
{
    SourceAndGroup sg(IPv4Address::UNSPECIFIED_ADDRESS, route->group);
    routes[sg] = route;

    rt->addMulticastRoute(createIPv4Route(route));
}

void PIMSM::addSGRoute(Route *route)
{
    SourceAndGroup sg(route->origin, route->group);
    routes[sg] = route;

    rt->addMulticastRoute(createIPv4Route(route));
}

PIMSM::Route *PIMSM::createRouteG(IPv4Address group, int flags)
{
    Route *newRouteG = new Route(this, G, IPv4Address::UNSPECIFIED_ADDRESS,group);
    newRouteG->setFlags(flags);
    newRouteG->rpAddr = rpAddr;
    return newRouteG;
}

PIMSM::Route *PIMSM::createRouteSG(IPv4Address source, IPv4Address group, int flags)
{
    Route *newRouteG = new Route(this, SG, source, group);
    newRouteG->setFlags(flags);
    newRouteG->rpAddr = rpAddr;
    return newRouteG;
}

IPv4MulticastRoute *PIMSM::createIPv4Route(Route *route)
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
    ASSERT(!source.isUnspecified());
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
