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

#include "PIMSM.h"
#include "IPv4Datagram.h"

Define_Module(PIMSM);

typedef PIMMulticastRoute::PIMInInterface PIMInInterface;
typedef IPv4MulticastRoute::OutInterface OutInterface;
typedef PIMMulticastRoute::PIMOutInterface PIMOutInterface;

/**
 * HANDLE MESSAGE
 *
 * The method is used to handle new messages. Self messages are timer and they are sent to
 * method which processes PIM timers. Other messages should be PIM packets, so they are sent
 * to method which processes PIM packets.
 *
 * @param msg Pointer to new message.
 * @see PIMPacket()
 * @see PIMTimer()
 * @see processPIMTimer()
 * @see processPIMPkt()
 */
void PIMSM::handleMessage(cMessage *msg)
{
	EV << "PIMSM::handleMessage" << endl;

	// self message (timer)
	if (msg->isSelfMessage())
	{
	   EV << "PIMSM::handleMessage:Timer" << endl;
	   processPIMTimer(msg);
	}
	else if (dynamic_cast<PIMPacket *>(msg))
	{
	   EV << "PIMSM::handleMessage: PIM-SM packet" << endl;
	   PIMPacket *pkt = check_and_cast<PIMPacket *>(msg);
	   EV << "Version: " << pkt->getVersion() << ", type: " << pkt->getType() << endl;
	   processPIMPkt(pkt);
	}
	else
	   EV << "PIMSM::handleMessage: Wrong message" << endl;
}

/**
 * INITIALIZE
 *
 * The method initializes PIM-SM module. It get access to all needed tables and other objects.
 * It subscribes to important notifications. If there is no PIM interface, all module can be
 * disabled. The method also read global pim-sm configuration as RP address and SPT threshold.
 *
 * @param stage Stage of initialization.
 */
void PIMSM::initialize(int stage)
{
    PIMBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        setRPAddress(par("RP").stdstringValue());
        setSPTthreshold(par("sptThreshold").stdstringValue());
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
 * CREATE KEEP ALIVE TIMER
 *
 * The method is used to create PIMKeepAliveTimer timer. The timer is set when source of multicast is
 * connected directly to the router.  If timer expires, router will remove the route from multicast
 * routing table. It is set to (S,G).
 *
 * @param source IP address of multicast source.
 * @param group IP address of multicast group.
 * @return Pointer to new Keep Alive Timer
 * @see PIMkat()
 */
PIMkat* PIMSM::createKeepAliveTimer(IPv4Address source, IPv4Address group)
{
    PIMkat *timer = new PIMkat("PIMKeepAliveTimer", KeepAliveTimer);
    timer->setSource(source);
    timer->setGroup(group);
    if (group == IPv4Address::UNSPECIFIED_ADDRESS)      //for (*,G)
        scheduleAt(simTime() + 2*KAT, timer);
    else
        scheduleAt(simTime() + KAT, timer);            //for (S,G)
    return timer;
}

/**
 * CREATE REGISTER-STOP TIMER
 *
 * The method is used to create PIMRegisterStopTimer timer. The timer is used to violate Register-null
 * message to keep Register tunnel disconnected. If timer expires, DR router of source is going to send
 * Register-null message.
 *
 * @param source IP address of multicast source.
 * @param group IP address of multicast group.
 * @return Pointer to new Register Stop Timer
 * @see PIMrst()
 */
PIMrst* PIMSM::createRegisterStopTimer(IPv4Address source, IPv4Address group)
{
    PIMrst *timer = new PIMrst("PIMRegisterStopTimer", RegisterStopTimer);
    timer->setSource(source);
    timer->setGroup(group);

#if CISCO_SPEC_SIM == 1
    scheduleAt(simTime() + RST, timer);
#else
    scheduleAt(simTime() + RST - REGISTER_PROBE_TIME, timer);
#endif

    return timer;
}

/**
 * CREATE EXPIRY TIMER
 *
 * The method is used to create PIMExpiryTimer.
 *
 * @param holdtime time to keep route in routing table
 * @param group IP address of multicast group.
 * @return Pointer to new Expiry Timer
 * @see PIMet()
 */
PIMet* PIMSM::createExpiryTimer(int intID, int holdtime, IPv4Address group, IPv4Address source, int StateType)
{
    EV << "Creating ET timer " << group << " , " << source << " : " << intID << " : " << StateType << endl;

    PIMet *timer = new PIMet("PIMExpiryTimer", ExpiryTimer);
    timer->setName("PIMExpiryTimer");
    timer->setIntId(intID);
    timer->setGroup(group);
    timer->setSource(source);
    timer->setStateType(StateType);

    scheduleAt(simTime() + holdtime, timer);
    return timer;
}

/**
 * CREATE JOIN TIMER
 *
 * The method is used to create PIMJoinTimer.
 *
 * @param group IP address of multicast group.
 * @param JPaddr joining IP address.
 * @param upstreamNbr IP address of upstream neighbor.
 * @return Pointer to new Join Timer
 * @see PIMjt()
 */
PIMjt* PIMSM::createJoinTimer(IPv4Address group, IPv4Address JPaddr, IPv4Address upstreamNbr, int JoinType)
{
    PIMjt *timer = new PIMjt("PIMJoinTimer", JoinTimer);
    timer->setGroup(group);
    timer->setJoinPruneAddr(JPaddr);
    timer->setUpstreamNbr(upstreamNbr);
    timer->setJoinType(JoinType);

    scheduleAt(simTime() + JT, timer);
    return timer;
}

/**
 * CREATE PRUNE PENDING TIMER
 *
 * The method is used to create PIMPrunePendingTimer.
 *
 * @param group IP address of multicast group.
 * @param JPaddr joining IP address.
 * @param upstreamNbr IP address of upstream neighbor.
 * @return Pointer to new PrunePending Timer
 * @see PIMppt()
 */
PIMppt* PIMSM::createPrunePendingTimer(IPv4Address group, IPv4Address JPaddr, IPv4Address upstreamNbr, JPMsgType JPtype)
{
    PIMppt *timer = new PIMppt("PIMPrunePendingTimer", PrunePendingTimer);

    timer->setGroup(group);
    timer->setJoinPruneAddr(JPaddr);
    timer->setUpstreamNbr(upstreamNbr);
    timer->setJoinPruneType(JPtype);

    scheduleAt(simTime() + PPT, timer);
    return timer;
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
void PIMSM::dataOnRpf(PIMMulticastRoute *route)
{
    if (route->getKeepAliveTimer())
    {
        cancelEvent(route->getKeepAliveTimer());
        if (route->getOrigin() == IPv4Address::UNSPECIFIED_ADDRESS)     // (*,G) route
        {
            scheduleAt(simTime() + KAT2+KAT, route->getKeepAliveTimer());
            EV << "PIMSM::dataOnRpf: restart (*,G) KAT" << endl;
        }
        else                                                            // (S,G) route
        {
            scheduleAt(simTime() + KAT2, route->getKeepAliveTimer());
            EV << "PIMSM::dataOnRpf: restart (S,G) KAT" << endl;
            if (!route->isFlagSet(PIMMulticastRoute::T))
                route->setFlags(PIMMulticastRoute::T);
        }
    }

    //TODO SPT threshold at last hop router
    if (route->isFlagSet(PIMMulticastRoute::C))
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
        RPAddress = IPv4Address(RP.c_str());
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
        SPTthreshold.append(threshold);
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
 * SET CTRL FOR MESSAGE
 *
 * The method is used to set up ctrl attributes.
 *
 * @param destAddr destination IP address.
 * @param source destination IP address.
 * @param protocol number in integer.
 * @param interfaceId in integer.
 * @return TTL time to live.
 */
IPv4ControlInfo *PIMSM::setCtrlForMessage (IPv4Address destAddr,IPv4Address srcAddr,int protocol, int interfaceId, int TTL)
{

    IPv4ControlInfo *ctrl = new IPv4ControlInfo();
    ctrl->setDestAddr(destAddr);
    ctrl->setSrcAddr(srcAddr);
    ctrl->setProtocol(protocol);
    ctrl->setInterfaceId(interfaceId);
    ctrl->setTimeToLive(TTL);

    return ctrl;
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
void PIMSM::processKeepAliveTimer(PIMkat *timer)
{
    EV << "pimSM::processKeepAliveTimer: route will be deleted" << endl;
    PIMMulticastRoute *route = getRouteFor(timer->getGroup(), timer->getSource());

    for (unsigned i=0; i < route->getNumOutInterfaces(); i++)
    {
        PIMOutInterface *outInterface = route->getPIMOutInterface(i);
        if (outInterface->expiryTimer)
        {
            cancelEvent(outInterface->expiryTimer);
            delete outInterface->expiryTimer;
        }
    }

    // only for RP, when KAT for (S,G) expire, set KAT for (*,G)
    if (IamRP(this->getRPAddress()) && route->getOrigin() != IPv4Address::UNSPECIFIED_ADDRESS)
    {
        PIMMulticastRoute *routeG = getRouteFor(timer->getGroup(), IPv4Address::UNSPECIFIED_ADDRESS);
        if (routeG && !routeG->getKeepAliveTimer())
            routeG->setKeepAliveTimer(createKeepAliveTimer(IPv4Address::UNSPECIFIED_ADDRESS, timer->getGroup()));
    }

    delete timer;
    route->setKeepAliveTimer(NULL);
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
void PIMSM::processRegisterStopTimer(PIMrst *timer)
{
    EV << "pimSM::processRegisterStopTimer: " << endl;

    sendPIMRegisterNull(timer->getSource(), timer->getGroup());
    //TODO set RST to probe time, if RST expires, add encapsulation tunnel
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
void PIMSM::processExpiryTimer(PIMet *timer)
{
    EV << "pimSM::processExpiryTimer: " << endl;

    PIMMulticastRoute *route = getRouteFor(timer->getGroup(), timer->getSource());
    PIMInInterface *rpfInterface = route->getPIMInInterface();
    PIMNeighbor *RPFneighbor = pimNbt->getFirstNeighborOnInterface(rpfInterface->getInterfaceId());
    int msgType = timer->getStateType();
    int timerIntID = timer->getIntId();
    IPv4Address multGroup = route->getMulticastGroup();
    IPv4Address JPaddr = timer->getSource();

    if (timerIntID != NO_INT_TIMER)
    {
        for(unsigned i=0; i<route->getNumOutInterfaces();i++)
        {
            PIMOutInterface *outInterface = route->getPIMOutInterface(i);
            if (outInterface->getInterfaceId() == timerIntID)
            {
                if (outInterface->expiryTimer)
                {
                    cancelEvent (outInterface->expiryTimer);
                    delete outInterface->expiryTimer;
                }
                route->removeOutInterface(i); // FIXME missing i-- or break
            }
        }

        //if (route->getNumOutInterfaces() == 0)
        if (route->isOilistNull())
        {
            route->clearFlag(PIMMulticastRoute::C);
            route->setFlags(PIMMulticastRoute::P);
            if (msgType == G && !IamRP(this->getRPAddress()))
                sendPIMJoinPrune(multGroup, this->getRPAddress(), RPFneighbor->getAddress(), PruneMsg, G);
            else if (msgType == SG)
                sendPIMJoinPrune(multGroup, JPaddr ,RPFneighbor->getAddress(), PruneMsg, SG);
            if (route->getJoinTimer())             // any interface in olist -> cancel and delete JT
            {
                cancelEvent(route->getJoinTimer());
                delete(route->getJoinTimer());
                route->setJoinTimer(NULL);
            }
        }
    }
    if (route->getExpiryTimer() && timerIntID == NO_INT_TIMER)
    {
        for(unsigned i=0; i<route->getNumOutInterfaces();i++)
        {
            PIMOutInterface *outInterface = route->getPIMOutInterface(i);
            if (outInterface->expiryTimer)
            {
                if (outInterface->expiryTimer)
                {
                    cancelEvent (outInterface->expiryTimer);
                    delete outInterface->expiryTimer;
                }
                route->removeOutInterface(i); // FIXME missing i--
            }
        }
        if (IamRP(this->getRPAddress()) && msgType == G)
        {
            EV << "ET for (*,G) route on RP expires - go to stopped" << endl;
            cancelEvent (route->getExpiryTimer());
            delete route->getExpiryTimer();
            route->setExpiryTimer(NULL);
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
void PIMSM::processJoinTimer(PIMjt *timer)
{
    EV << "pimSM::processJoinTimer:" << endl;

    PIMMulticastRoute *route;

    if (timer->getJoinType() == SG)
    {
        route = getRouteFor(timer->getGroup(), timer->getJoinPruneAddr());
        if (route && !route->isOilistNull())
            sendPIMJoinPrune(timer->getGroup(),timer->getJoinPruneAddr(),timer->getUpstreamNbr(),JoinMsg,SG);
    }
    else if (timer->getJoinType() == G)
    {
        route = getRouteFor(timer->getGroup(), IPv4Address::UNSPECIFIED_ADDRESS);
        if (route && !route->isOilistNull())
            sendPIMJoinPrune(timer->getGroup(),timer->getJoinPruneAddr(),timer->getUpstreamNbr(),JoinMsg,G);
    }
    else
        throw cRuntimeError("pimSM::processJoinTimer - Bad Join type!");

    // restart JT timer
    if (route && !route->isOilistNull())
    {
        if (route->getJoinTimer())
        {
            cancelEvent(route->getJoinTimer());
            scheduleAt(simTime() + JT, route->getJoinTimer());
        }
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
void PIMSM::processPrunePendingTimer(PIMppt *timer)
{
    EV << "pimSM::processPrunePendingTimer:" << endl;

    if (!IamRP(this->getRPAddress()) && timer->getJoinPruneType() == G)
        sendPIMJoinPrune(timer->getGroup(),timer->getJoinPruneAddr(),timer->getUpstreamNbr(),PruneMsg,G);
    if (timer->getJoinPruneType() == SG)
        sendPIMJoinPrune(timer->getGroup(),timer->getJoinPruneAddr(),timer->getUpstreamNbr(),PruneMsg,SG);

    PIMMulticastRoute *route = getRouteFor(timer->getGroup(), IPv4Address::UNSPECIFIED_ADDRESS);
    if (route->getPrunePendingTimer())
        route->setPrunePendingTimer(NULL);
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
void PIMSM::restartExpiryTimer(PIMMulticastRoute *route, InterfaceEntry *originIntf, int holdTime)
{
    EV << "pimSM::restartExpiryTimer: next ET @ " << simTime() + holdTime << " for type: ";

    if (route)
    {
        // ET for route
        if (route->getExpiryTimer())
        {
            cancelEvent(route->getExpiryTimer());
            scheduleAt(simTime() + holdTime, route->getExpiryTimer());
        }

        // ET for outgoing interfaces
        for (unsigned i=0; i< route->getNumOutInterfaces(); i++)
        {   // if exist ET and for given interface
            PIMOutInterface *outInterface = route->getPIMOutInterface(i);
            if (outInterface->expiryTimer && (outInterface->getInterfaceId() == originIntf->getInterfaceId()))
            {
                PIMet *timer = outInterface->expiryTimer;
                EV << timer->getStateType() << " , " << timer->getGroup() << " , " << timer->getSource() << ", int: " << timer->getIntId() << endl;
                cancelEvent(timer);
                scheduleAt(simTime() + holdTime, timer);
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
            PIMMulticastRoute *routeG = getRouteFor(multGroup,IPv4Address::UNSPECIFIED_ADDRESS);
            if (routeG)
            {
                if (routeG->getPrunePendingTimer())
                {
                    cancelEvent(routeG->getPrunePendingTimer());
                    delete routeG->getPrunePendingTimer();
                }
            }
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
    int outIntToDel = ctrl->getInterfaceId();

    if (!encodedAddr.R && !encodedAddr.W && encodedAddr.S)                                              // (S,G) Prune
    {
        EV << "(S,G) Prune processing" << endl;

        PIMMulticastRoute *routeSG = getRouteFor(multGroup,encodedAddr.IPaddress);
        PIMInInterface *rpfInterface = routeSG->getPIMInInterface();
        PIMNeighbor *RPFnbr = pimNbt->getFirstNeighborOnInterface(rpfInterface->getInterfaceId());

// NOTES from btomi:
//
// The following loop was like this before:
//
//        PIMMulticastRoute::InterfaceVector outIntSG = routeSG->getOutInt();
//
//        for (unsigned k = 0; k < outIntSG.size(); k++)
//        {
//            if (outIntSG[k].intId == outIntToDel)
//            {
//                EV << "Interface is present, removing it from the list of outgoing interfaces." << endl;
//                if (outIntSG[k].expiryTimer)
//                {
//                    cancelEvent(outIntSG[k].expiryTimer);
//                    delete (outIntSG[k].expiryTimer);
//                }
//                outIntSG.erase(outIntSG.begin() + k);
//
//                InterfaceEntry *newInIntG = rt->getInterfaceForDestAddr(this->getRPAddress());
//                routeSG->addOutIntFull(newInIntG,newInIntG->getInterfaceId(),
//                                        PIMMulticastRoute::Pruned,
//                                        PIMMulticastRoute::Sparsemode,
//                                        NULL,
//                                        NULL,
//                                        PIMMulticastRoute::NoInfo,
//                                        PIMMulticastRoute::Join,false); // create "virtual" outgoing interface to RP
//                //routeSG->setRegisterTunnel(false); //we need to set register state to output interface, but output interface has to be null for now
//                break;
//            }
//        }
//        routeSG->setOutInt(outIntSG);
//
// The route->addOutIntFull(...) had no effect, because the list of outgoing interfaces is overwritten by the routeSG->setOutInt(...) statement after the loop.
// To keep the original behaviour (and fingerprints), I temporarily commented out the routeSG->addOutIntFull(...) call below.

        for (unsigned k = 0; k < routeSG->getNumOutInterfaces(); k++)
        {
            PIMOutInterface *outIntSG = routeSG->getPIMOutInterface(k);
            if (outIntSG->getInterfaceId() == outIntToDel)
            {
                EV << "Interface is present, removing it from the list of outgoing interfaces." << endl;
                if (outIntSG->expiryTimer)
                {
                    cancelEvent(outIntSG->expiryTimer);
                    delete (outIntSG->expiryTimer);
                }
                routeSG->removeOutInterface(k);

//                InterfaceEntry *newInIntG = rt->getInterfaceForDestAddr(this->getRPAddress());
//                routeSG->addOutIntFull(newInIntG,newInIntG->getInterfaceId(),
//                                        PIMMulticastRoute::Pruned,
//                                        PIMMulticastRoute::Sparsemode,
//                                        NULL,
//                                        NULL,
//                                        PIMMulticastRoute::NoInfo,
//                                        PIMMulticastRoute::Join,false);      // create "virtual" outgoing interface to RP
                //routeSG->setRegisterTunnel(false);                   //we need to set register state to output interface, but output interface has to be null for now
                break;
            }
        }

        if (routeSG->isOilistNull())
        {
            routeSG->setFlags(PIMMulticastRoute::P);
            if (routeSG->getJoinTimer())                           // if is JT set, delete it
            {
                cancelEvent(routeSG->getJoinTimer());
                delete routeSG->getJoinTimer();
                routeSG->setJoinTimer(NULL);
            }
            if (!IamDR(routeSG->getOrigin()))
            {
#if CISCO_SPEC_SIM == 1
                sendPIMJoinPrune(multGroup,encodedAddr.IPaddress,RPFnbr->getAddress(),PruneMsg,SG);
#else
                PIMppt* timerPpt = createPrunePendingTimer(multGroup, encodedAddr.IPaddress, RPFnbr->getAddress(), SG);
                routeSG->setPrunePendingTimer(timerPpt);
#endif
            }
        }

    }
    if (encodedAddr.R && encodedAddr.W && encodedAddr.S)    // (*,G) Prune
    {
        EV << "(*,G) Prune processing" << endl;
        std::vector<PIMMulticastRoute*> routes = getRouteFor(multGroup);
        for (unsigned int j = 0; j < routes.size(); j++)
        {
            PIMMulticastRoute *route = routes[j];
            IPv4Address multOrigin = route->getOrigin();

            for (unsigned k = 0; k < route->getNumOutInterfaces(); k++)
            {
                PIMOutInterface *outInt = route->getPIMOutInterface(k);
                if (outInt->getInterfaceId() == outIntToDel)
                {
                    EV << "Interface is present, removing it from the list of outgoing interfaces." << endl;
                    if (outInt->expiryTimer)
                    {
                        cancelEvent(outInt->expiryTimer);
                        delete (outInt->expiryTimer);
                    }
                    route->removeOutInterface(k); // FIXME missing k-- or break
                }
            }

            // there is no receiver of multicast, prune the router from the multicast tree
            if (route->isOilistNull())
            {
                route->clearFlag(PIMMulticastRoute::C);
                route->setFlags(PIMMulticastRoute::P);
                if (route->getJoinTimer())                     // if is JT set, delete it
                {
                    cancelEvent(route->getJoinTimer());
                    delete route->getJoinTimer();
                    route->setJoinTimer(NULL);
                }
                // send Prune message
                PIMInInterface *rpfInterface = route->getPIMInInterface();
                PIMNeighbor *RPFnbr = rpfInterface ? pimNbt->getFirstNeighborOnInterface(rpfInterface->getInterfaceId()) : NULL;
                if (!IamRP(this->getRPAddress()) && (multOrigin == IPv4Address::UNSPECIFIED_ADDRESS))
                {
#if CISCO_SPEC_SIM == 1
                    sendPIMJoinPrune(multGroup,this->getRPAddress(),RPFnbr->getAddress(),PruneMsg,G);      // only for non-RP routers in RPT
#else
                    PIMppt* timerPpt = createPrunePendingTimer(multGroup, this->getRPAddress(), RPFnbr->getAddress(),G);
                    route->setPrunePendingTimer(timerPpt);
#endif
                }
                else if (IamRP(this->getRPAddress()) && (multOrigin != IPv4Address::UNSPECIFIED_ADDRESS))
                {
#if CISCO_SPEC_SIM == 1
                    sendPIMJoinPrune(multGroup,multOrigin,RPFnbr->getAddress(),PruneMsg,SG);               // send from RP only if (S,G) is available
#else
                    PIMppt* timerPpt = createPrunePendingTimer(multGroup, multOrigin, RPFnbr->getAddress(), SG);
                    route->setPrunePendingTimer(timerPpt);
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
    PIMMulticastRoute *newRouteSG = new PIMMulticastRoute(multOrigin, multGroup);
    IPv4ControlInfo *ctrl = check_and_cast<IPv4ControlInfo*>(pkt->getControlInfo());
    InterfaceEntry *newInIntSG = rt->getInterfaceForDestAddr(multOrigin);
    PIMNeighbor *neighborToSrcDR = pimNbt->getFirstNeighborOnInterface(newInIntSG->getInterfaceId());
    PIMMulticastRoute *routePointer;
    IPv4Address pktSource = ctrl->getSrcAddr();
    int holdTime = pkt->getHoldTime();

    if (!IamDR(multOrigin))
    {
        PIMMulticastRoute *newRouteG = new PIMMulticastRoute(IPv4Address::UNSPECIFIED_ADDRESS, multGroup);
        routePointer = newRouteG;
        if (!(newRouteG = getRouteFor(multGroup, IPv4Address::UNSPECIFIED_ADDRESS)))        // create (*,G) route between RP and source DR
        {
            InterfaceEntry *newInIntG = rt->getInterfaceForDestAddr(this->RPAddress);
            PIMNeighbor *neighborToRP = pimNbt->getFirstNeighborOnInterface(newInIntG->getInterfaceId());

            newRouteG = routePointer;
            newRouteG->setRP(this->getRPAddress());
            newRouteG->setKeepAliveTimer(createKeepAliveTimer(IPv4Address::UNSPECIFIED_ADDRESS, multGroup));
            newRouteG->setFlags(PIMMulticastRoute::S | PIMMulticastRoute::P);
            newRouteG->setInInterface(new PIMMulticastRoute::PIMInInterface(newInIntG, neighborToRP->getAddress()));
            rt->addMulticastRoute(newRouteG);
        }
    }

    routePointer = newRouteSG;
    if (!(newRouteSG = getRouteFor(multGroup, multOrigin)))         // create (S,G) route between RP and source DR
    {
        newRouteSG = routePointer;
        // set source, mult. group, etc...
        newRouteSG->setRP(this->getRPAddress());
        newRouteSG->setKeepAliveTimer(createKeepAliveTimer(newRouteSG->getOrigin(), newRouteSG->getMulticastGroup()));

        // set outgoing and incoming interface and ET
        InterfaceEntry *outInt = rt->getInterfaceForDestAddr(this->getRPAddress());

        // RPF check
        if (newInIntSG->getInterfaceId() != outInt->getInterfaceId())
        {
            newRouteSG->setInInterface(new PIMMulticastRoute::PIMInInterface(newInIntSG, neighborToSrcDR->getAddress()));
            PIMet *timerEt = createExpiryTimer(outInt->getInterfaceId(), holdTime, multGroup,multOrigin,SG);
            newRouteSG->addOutInterface(new PIMMulticastRoute::PIMOutInterface(outInt,
                                        PIMMulticastRoute::Forward,
                                        PIMInterface::SparseMode,
                                        NULL,
                                        timerEt,
                                        PIMMulticastRoute::NoInfo,
                                        PIMMulticastRoute::NoInfoRS,true));

            if (!IamDR(multOrigin))
                newRouteSG->setJoinTimer(createJoinTimer(multGroup, multOrigin, neighborToSrcDR->getAddress(),SG));

            rt->addMulticastRoute(newRouteSG);

            if (!IamDR(multOrigin))
                sendPIMJoinPrune(multGroup,multOrigin,neighborToSrcDR->getAddress(),JoinMsg,SG);       // triggered join except DR
        }
    }
    else
    {
        // on source DR isn't RPF check - DR doesn't have incoming interface
        if (IamDR(multOrigin) && (newRouteSG->getSequencenumber() == 0))
        {
            InterfaceEntry *outIntf = rt->getInterfaceForDestAddr(pktSource);
            PIMet *timerEt = createExpiryTimer(outIntf->getInterfaceId(), holdTime, multGroup,multOrigin,SG);

            newRouteSG->clearFlag(PIMMulticastRoute::P);
            // update interfaces to forwarding state
            for (unsigned j=0; j < newRouteSG->getNumOutInterfaces(); j++)
            {
                PIMOutInterface *outInterface = newRouteSG->getPIMOutInterface(j);
                outInterface->forwarding = PIMMulticastRoute::Forward;
                outInterface->expiryTimer = timerEt;
                outInterface->shRegTun = true;
            }
            //newRouteSG->setRegisterTunnel(true);
            newRouteSG->setSequencenumber(1);
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
    PIMMulticastRoute *routePointer;
    IPv4Address multOrigin;

    std::vector<PIMMulticastRoute*> routes = getRouteFor(multGroup);       // get all mult. routes at RP
    for (unsigned i=0; i<routes.size();i++)
    {
        routePointer = routes[i];
        if (routePointer->getSequencenumber() == 0)                                 // only check if route was installed
        {
            multOrigin = routePointer->getOrigin();
            if (multOrigin != IPv4Address::UNSPECIFIED_ADDRESS)                     // for (S,G) route
            {
                PIMInInterface *inInterface = routePointer->getPIMInInterface();

                // update flags
                routePointer->clearFlag(PIMMulticastRoute::P);
                routePointer->setFlags(PIMMulticastRoute::T);
                routePointer->setJoinTimer(createJoinTimer(multGroup, multOrigin, inInterface->nextHop, SG));

                if (routePointer->getNumOutInterfaces() == 0)                                         // Has route any outgoing interface?
                {
                    InterfaceEntry *interface = rt->getInterfaceForDestAddr(packetOrigin);
                    PIMet *timerEt = createExpiryTimer(interface->getInterfaceId(),msgHoldtime, multGroup,multOrigin,SG);
                    routePointer->addOutInterface(new PIMMulticastRoute::PIMOutInterface(interface,
                                                PIMMulticastRoute::Forward,
                                                PIMInterface::SparseMode,
                                                NULL,
                                                timerEt,
                                                PIMMulticastRoute::NoInfo,
                                                PIMMulticastRoute::NoInfoRS,true));
                    sendPIMJoinPrune(multGroup, multOrigin, inInterface->nextHop, JoinMsg,SG);
                }
                routePointer->setSequencenumber(1);
            }
            else                                                                    // for (*,G) route
            {
                if (routePointer->isFlagSet(PIMMulticastRoute::P) || routePointer->isFlagSet(PIMMulticastRoute::F))
                {
                    routePointer->clearFlag(PIMMulticastRoute::P);
                    routePointer->clearFlag(PIMMulticastRoute::F);
                }
                if (routePointer->getKeepAliveTimer())                                         // remove KAT and set ET to (*,G)
                {
                    cancelEvent(routePointer->getKeepAliveTimer());
                    delete routePointer->getKeepAliveTimer();
                    routePointer->setKeepAliveTimer(NULL);
                }
                InterfaceEntry *interface = rt->getInterfaceForDestAddr(packetOrigin);
                if (!routePointer->findOutInterfaceByInterfaceId(interface->getInterfaceId()))
                {
                    PIMet *timerEt = createExpiryTimer(interface->getInterfaceId(),msgHoldtime, multGroup,IPv4Address::UNSPECIFIED_ADDRESS,G);
                    PIMet *timerEtNI = createExpiryTimer(NO_INT_TIMER,msgHoldtime, multGroup,IPv4Address::UNSPECIFIED_ADDRESS,G);
                    routePointer->addOutInterface(new PIMMulticastRoute::PIMOutInterface(interface,
                                                PIMMulticastRoute::Forward,
                                                PIMInterface::SparseMode,
                                                NULL,
                                                timerEt,
                                                PIMMulticastRoute::NoInfo,
                                                PIMMulticastRoute::NoInfoRS,true));
                    routePointer->setExpiryTimer(timerEtNI);
                }
                routePointer->setSequencenumber(1);
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
    PIMMulticastRoute *newRouteG = new PIMMulticastRoute(IPv4Address::UNSPECIFIED_ADDRESS, multGroup);
    PIMMulticastRoute *routePointer;
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
            InterfaceEntry *newInIntG = rt->getInterfaceForDestAddr(this->RPAddress);                                       // incoming interface
            PIMNeighbor *neighborToRP = pimNbt->getFirstNeighborOnInterface(newInIntG->getInterfaceId());                            // RPF neighbor
            InterfaceEntry *outIntf = JoinIncomingInt;                                      // outgoing interface

            if (JoinIncomingInt->getInterfaceId() != newInIntG->getInterfaceId())
            {
                newRouteG->setRP(this->getRPAddress());
                newRouteG->setFlags(PIMMulticastRoute::S);

                if (!IamRP(this->getRPAddress()))
                {
                    newRouteG->setInInterface(new PIMMulticastRoute::PIMInInterface(newInIntG, neighborToRP->getAddress())); //  (*,G) route hasn't incoming interface at RP
                    newRouteG->setJoinTimer(createJoinTimer(multGroup, this->getRPAddress(), neighborToRP->getAddress(),G));              // periodic Join (*,G)
                }

                PIMet *timerEt = createExpiryTimer(outIntf->getInterfaceId(),msgHoldTime, multGroup,IPv4Address::UNSPECIFIED_ADDRESS,G);
                PIMet *timerEtNI = createExpiryTimer(NO_INT_TIMER,msgHoldTime, multGroup,IPv4Address::UNSPECIFIED_ADDRESS,G);
                newRouteG->addOutInterface(new PIMMulticastRoute::PIMOutInterface(outIntf,
                                        PIMMulticastRoute::Forward,
                                        PIMInterface::SparseMode,
                                        NULL,
                                        timerEt,
                                        PIMMulticastRoute::NoInfo,
                                        PIMMulticastRoute::NoInfoRS,true));
                newRouteG->setExpiryTimer(timerEtNI);
                rt->addMulticastRoute(newRouteG);

                if (!IamRP(this->getRPAddress()))
                    sendPIMJoinPrune(multGroup,this->getRPAddress(),neighborToRP->getAddress(),JoinMsg,G);                         // triggered Join (*,G)
            }
        }
        else            // (*,G) route exist
        {
            //if (!newRouteG->isRpf(JoinIncomingInt->getInterfaceId()))
            if (!newRouteG->getInInterface() || newRouteG->getPIMInInterface()->getInterface() != JoinIncomingInt)
            {
                if (IamRP(this->getRPAddress()))
                    processJoinRouteGexistOnRP(multGroup, pktSource,msgHoldTime);
                else        // (*,G) route exist somewhere in RPT
                {
                    InterfaceEntry *interface = JoinIncomingInt;
                    if (!newRouteG->findOutInterfaceByInterfaceId(interface->getInterfaceId()))
                    {
                        PIMet *timerEt = createExpiryTimer(interface->getInterfaceId(),msgHoldTime, multGroup,IPv4Address::UNSPECIFIED_ADDRESS,G);
                        newRouteG->addOutInterface(new PIMMulticastRoute::PIMOutInterface(interface,
                                                    PIMMulticastRoute::Forward,
                                                    PIMInterface::SparseMode,
                                                    NULL,
                                                    timerEt,
                                                    PIMMulticastRoute::NoInfo,
                                                    PIMMulticastRoute::NoInfoRS,true));
                    }
                }
                // restart ET for given interface - for (*,G) and also (S,G)
                restartExpiryTimer(newRouteG,JoinIncomingInt, msgHoldTime);
                std::vector<PIMMulticastRoute*> routes = getRouteFor(multGroup);
                for (unsigned i=0; i<routes.size();i++)
                {
                    routePointer = routes[i];
                    //restart ET for (S,G)
                    if (routePointer->getOrigin() != IPv4Address::UNSPECIFIED_ADDRESS)
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

    PIMMulticastRoute *routePointer;
    IPv4Datagram *encapData = check_and_cast<IPv4Datagram*>(pkt->decapsulate());
    IPv4Address multOrigin = encapData->getSrcAddress();
    IPv4Address multGroup = encapData->getDestAddress();
    PIMMulticastRoute *newRouteG = new PIMMulticastRoute(IPv4Address::UNSPECIFIED_ADDRESS,multGroup);
    PIMMulticastRoute *newRoute = new PIMMulticastRoute(multOrigin,multGroup);
    multDataInfo *info = new multDataInfo;

    if (!pkt->getN())                                                                                       //It is Null Register ?
    {
        routePointer = newRouteG;
        if (!(newRouteG = getRouteFor(multGroup, IPv4Address::UNSPECIFIED_ADDRESS)))                    // check if exist (*,G)
        {
            newRouteG = routePointer;
            newRouteG->setRP(this->getRPAddress());
            newRouteG->setFlags(PIMMulticastRoute::S | PIMMulticastRoute::P);                           // create and set (*,G) KAT timer, add to routing table
            newRouteG->setKeepAliveTimer(createKeepAliveTimer(IPv4Address::UNSPECIFIED_ADDRESS, newRouteG->getMulticastGroup()));
            rt->addMulticastRoute(newRouteG);
        }
        routePointer = newRoute;                                                                            // check if exist (S,G)
        if (!(newRoute = getRouteFor(multGroup,multOrigin)))
        {
            InterfaceEntry *newInIntG = rt->getInterfaceForDestAddr(multOrigin);
            PIMNeighbor *pimIntfToDR = pimNbt->getFirstNeighborOnInterface(newInIntG->getInterfaceId());
            newRoute = routePointer;
            newRoute->setInInterface(new PIMMulticastRoute::PIMInInterface(newInIntG, pimIntfToDR->getAddress()));
            newRoute->setRP(this->getRPAddress());
            newRoute->setFlags(PIMMulticastRoute::P);
            newRoute->setKeepAliveTimer(createKeepAliveTimer(newRoute->getOrigin(), newRoute->getMulticastGroup()));   // create and set (S,G) KAT timer, add to routing table
            rt->addMulticastRoute(newRoute);
        }
                                                                                                            // we have some active receivers
        if (!newRouteG->isOilistNull())
        {
            // copy out interfaces from newRouteG
            newRoute->clearOutInterfaces();
            for (unsigned int i = 0; i < newRouteG->getNumOutInterfaces(); i++)
                newRoute->addOutInterface(new PIMOutInterface(*newRouteG->getPIMOutInterface(i)));

            newRoute->clearFlag(PIMMulticastRoute::P);                                                                        // update flags for SG route

            if (!newRoute->isFlagSet(PIMMulticastRoute::T))                                                                    // only if isn't build SPT between RP and registering DR
            {
                for (unsigned i=0; i < newRouteG->getNumOutInterfaces(); i++)
                {
                    PIMOutInterface *outInterface = newRouteG->getPIMOutInterface(i);
                    if (outInterface->forwarding == PIMMulticastRoute::Forward)                           // for active outgoing interface forward encapsulated data
                    {                                                                                       // simulate multicast data
                        info->group = multGroup;
                        info->origin = multOrigin;
                        info->interface_id = outInterface->getInterfaceId();
                        InterfaceEntry *entry = ift->getInterfaceById(outInterface->getInterfaceId());
                        info->srcAddr = entry->ipv4Data()->getIPAddress();
                        forwardMulticastData(encapData->dup(), info);
                    }
                }
                sendPIMJoinTowardSource(info);                                                              // send Join(S,G) to establish SPT between RP and registering DR
                IPv4ControlInfo *PIMctrl = check_and_cast<IPv4ControlInfo*>(pkt->getControlInfo());         // send register-stop packet
                sendPIMRegisterStop(PIMctrl->getDestAddr(),PIMctrl->getSrcAddr(),multGroup,multOrigin);
            }
        }
        if (newRoute->getKeepAliveTimer())                                                                             // refresh KAT timers
        {
            EV << " (S,G) KAT timer refresh" << endl;
            cancelEvent(newRoute->getKeepAliveTimer());
            scheduleAt(simTime() + KAT, newRoute->getKeepAliveTimer());
        }
        if (newRouteG->getKeepAliveTimer())
        {
            EV << " (*,G) KAT timer refresh" << endl;
            cancelEvent(newRouteG->getKeepAliveTimer());
            scheduleAt(simTime() + 2*KAT, newRouteG->getKeepAliveTimer());
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
        if (newRouteG->isFlagSet(PIMMulticastRoute::P) || pkt->getN())
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

    PIMMulticastRoute *routeSG, *routeG;
    InterfaceEntry *intToRP = rt->getInterfaceForDestAddr(this->getRPAddress());

    // Set RST timer for S,G route
    PIMrst* timerRST = createRegisterStopTimer(pkt->getSourceAddress(), pkt->getGroupAddress());
    routeSG = getRouteFor(pkt->getGroupAddress(),pkt->getSourceAddress());
    routeG = getRouteFor(pkt->getGroupAddress(), IPv4Address::UNSPECIFIED_ADDRESS);
    if (routeG)
        routeG->setRegisterStopTimer(timerRST);

    if (routeSG)
    {
        PIMOutInterface *outInterface = routeSG->findOutInterfaceByInterfaceId(intToRP->getInterfaceId());
        if (outInterface && outInterface->regState == PIMMulticastRoute::Join)
        {
            EV << "Register tunnel is connect - has to be disconnect" << endl;
            outInterface->regState = PIMMulticastRoute::Prune;
        }
        else
            EV << "Register tunnel is disconnect" << endl;
    }
}

/**
 * SEND PIM JOIN PRUNE
 *
 * The method is used for creating and sending PIM Join/Prune messages.
 *
 * @param multGroup Address of multicast group for which Join/Prune message is created.
 * @param joinPruneIPaddr Address for Join or prune.
 * @param upstreamNbr IP address of upstream neighbor.
 * @param msgType Type of message - Join or Prune message.
 * @param JPType Type of Join/Prune message - G, SG.
 * @see encodedAddr()
 * @see setCtrlForMessage()
 * @see getRouteFor()
 */
void PIMSM::sendPIMJoinPrune(IPv4Address multGroup, IPv4Address joinPruneIPaddr, IPv4Address upstreamNbr, joinPruneMsg msgType, JPMsgType JPtype)
{
    // create PIM Register datagram
    PIMJoinPrune *msg = new PIMJoinPrune();
    MulticastGroup *group = new MulticastGroup();
    EncodedAddress encodedAddr;

    // set PIM packet
    msg->setType(JoinPrune);
    msg->setUpstreamNeighborAddress(upstreamNbr);
    msg->setHoldTime(HOLDTIME);

    encodedAddr.IPaddress = joinPruneIPaddr;
    if (JPtype == G)
    {
        EV << "pimSM::sendPIMJoinPrune, assembling (*,G) ";
        encodedAddr.S = true;
        encodedAddr.W = true;
        encodedAddr.R = true;
    }
    if (JPtype == SG)
    {
        EV << "pimSM::sendPIMJoinPrune, assembling (S,G) ";
        encodedAddr.S = true;
        encodedAddr.W = false;
        encodedAddr.R = false;
    }

    msg->setMulticastGroupsArraySize(1);
    group->setGroupAddress(multGroup);
    if (msgType == JoinMsg)
    {
        EV << "Join" << endl;
        msg->setName("PIMJoin/Prune(Join)");
        group->setJoinedSourceAddressArraySize(1);
        group->setPrunedSourceAddressArraySize(0);
        group->setJoinedSourceAddress(0,encodedAddr);
    }
    if (msgType == PruneMsg)
    {
        EV << "Prune" << endl;
        msg->setName("PIMJoin/Prune(Prune)");
        group->setJoinedSourceAddressArraySize(0);
        group->setPrunedSourceAddressArraySize(1);
        EncodedAddress PruneAddress;
        group->setPrunedSourceAddress(0,encodedAddr);
    }
    msg->setMulticastGroups(0, *group);
    // set IP Control info
    InterfaceEntry *interfaceToRP = rt->getInterfaceForDestAddr(joinPruneIPaddr);
    IPv4ControlInfo *ctrl = setCtrlForMessage(ALL_PIM_ROUTERS_MCAST,interfaceToRP->ipv4Data()->getIPAddress(),
                                                        IP_PROT_PIM,interfaceToRP->getInterfaceId(),1);
    msg->setControlInfo(ctrl);
    send(msg, "ipOut");
}

/**
 * SEND PIM REGISTER NULL
 *
 * The method is used for creating and sending of PIM Register Null message.
 *
 * @param multGroup Address of multicast group for which Join/Prune message is created.
 * @param multOrigin Address of multicast source.
 * @param upstreamNbr Address of multicast group.
 * @see setCtrlForMessage()
 * @see setKat()
 * @see getRouteFor()
 */
void PIMSM::sendPIMRegisterNull(IPv4Address multOrigin, IPv4Address multGroup)
{
    EV << "pimSM::sendPIMRegisterNull" << endl;

    // only if (S,G exist)
    //if (getRouteFor(multDest,multSource))
    if (getRouteFor(multGroup,IPv4Address::UNSPECIFIED_ADDRESS))
    {
        // create PIM Register NULL datagram
        PIMRegister *msg = new PIMRegister();

        // set fields for PIM Register packet
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

        // set IP Control info
        InterfaceEntry *interfaceToRP = rt->getInterfaceForDestAddr(RPAddress);
        IPv4ControlInfo *ctrl = setCtrlForMessage(RPAddress,interfaceToRP->ipv4Data()->getIPAddress(),
                                                            IP_PROT_PIM,interfaceToRP->getInterfaceId(),MAX_TTL);
        msg->setControlInfo(ctrl);
        send(msg, "ipOut");
    }
}

/**
 * SEND PIM REGISTER
 *
 * The method is used for creating and sending of PIM Register message.
 *
 * @param datagram Pointer to IPv4 datagram.
 * @see setCtrlForMessage()
 * @see setKat()
 * @see getRouteFor()
 */
void PIMSM::sendPIMRegister(IPv4Datagram *datagram)
{
    EV << "pimSM::sendPIMRegister - encapsulating data packet into Register packet and sending to RP" << endl;

    IPv4Address multGroup = datagram->getDestAddress();
    IPv4Address multOrigin = datagram->getSrcAddress();
    InterfaceEntry *intToRP = rt->getInterfaceForDestAddr(this->getRPAddress());

    PIMMulticastRoute *routeSG = getRouteFor(multGroup,multOrigin);
    PIMMulticastRoute *routeG = getRouteFor(multGroup, IPv4Address::UNSPECIFIED_ADDRESS);
    if (routeSG == NULL)
        throw cRuntimeError("pimSM::sendPIMRegister - route for (S,G) not found!");

    // refresh KAT timers
    if (routeSG->getKeepAliveTimer())
    {
        EV << " (S,G) KAT timer refresh" << endl;
        cancelEvent(routeSG->getKeepAliveTimer());
        scheduleAt(simTime() + KAT, routeSG->getKeepAliveTimer());
    }
    if (routeG->getKeepAliveTimer())
    {
        EV << " (*,G) KAT timer refresh" << endl;
        cancelEvent(routeG->getKeepAliveTimer());
        scheduleAt(simTime() + 2*KAT, routeG->getKeepAliveTimer());
    }

    // Check if is register tunnel connected
    PIMOutInterface *outInterface = routeSG->findOutInterfaceByInterfaceId(intToRP->getInterfaceId());
    if (outInterface && outInterface->regState == PIMMulticastRoute::Join)
    {
        // create PIM Register datagram
        PIMRegister *msg = new PIMRegister();

        // set fields for PIM Register packet
        msg->setName("PIMRegister");
        msg->setType(Register);
        msg->setN(false);
        msg->setB(false);

        IPv4Datagram *datagramCopy = datagram->dup();
        delete datagramCopy->removeControlInfo();
        msg->encapsulate(datagramCopy);

        // set IP Control info
        //InterfaceEntry *interfaceToRP = rt->getInterfaceForDestAddr(RPAddress);
        //IPv4ControlInfo *ctrl = setCtrlForMessage(RPAddress,interfaceToRP->ipv4Data()->getIPAddress(),
        //                                            IP_PROT_PIM,interfaceToRP->getInterfaceId(),MAX_TTL);

        IPv4ControlInfo *ctrl = setCtrlForMessage(this->getRPAddress(),intToRP->ipv4Data()->getIPAddress(),
                                                    IP_PROT_PIM,intToRP->getInterfaceId(),MAX_TTL);
        msg->setControlInfo(ctrl);
        send(msg, "ipOut");
    }
    else if (outInterface && outInterface->regState == PIMMulticastRoute::Prune)
        EV << "PIM-SM:sendPIMRegister - register tunnel is disconnect." << endl;
}

/**
 * SEND PIM JOIN TOWARD SOURCE
 *
 * The method is used for send triggered Join toward source of multicast
 * in the processRegisterPacket() method.
 *
 * @param multGroup Pointer to controll info.
 * @see setCtrlForMessage()
 * @see setKat()
 * @see getRouteFor()
 */
void PIMSM::sendPIMJoinTowardSource(multDataInfo *info)
{
    EV << "pimSM::sendPIMJoinTowardSource" << endl;

    PIMMulticastRoute *routeSG = getRouteFor(info->group,info->origin);
    PIMInInterface *rpfInterface = routeSG->getPIMInInterface();
    sendPIMJoinPrune(info->group,info->origin, rpfInterface->nextHop,JoinMsg,SG);
}

/**
 * SEND PIM REGISTER STOP
 *
 * The method is used for sending PIM Register-Stop message, which is delivered
 * to DR of multicast source. Register-Stop message stop process of multicast
 * data encapsulation.
 *
 * @param source Address of RP.
 * @param dest Address of source DR.
 * @param multGroup Address of multicast group.
 * @param multSource Address of multicast source.
 * @see getInterfaceForDestAddr()
 */
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
    IPv4ControlInfo *ctrl = new IPv4ControlInfo();
    ctrl->setDestAddr(dest);
    ctrl->setSrcAddr(source);
    ctrl->setProtocol(IP_PROT_PIM);
    ctrl->setInterfaceId(interfaceToDR->getInterfaceId());
    ctrl->setTimeToLive(255);
    msg->setControlInfo(ctrl);

    send(msg, "ipOut");
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
void PIMSM::forwardMulticastData(IPv4Datagram *datagram, multDataInfo *info)
{
    EV << "pimSM::forwardMulticastData" << endl;

    //
    // Note: we should inject the datagram somehow into the normal IPv4 forwarding path.
    //
    cPacket *data = datagram->decapsulate();

    // set control info
    IPv4ControlInfo *ctrl = new IPv4ControlInfo();
    ctrl->setDestAddr(datagram->getDestAddress());
    // XXX ctrl->setSrcAddr(datagram->getSrcAddress());
    ctrl->setInterfaceId(info->interface_id);
    ctrl->setTimeToLive(MAX_TTL-2);                     //one minus for source DR router and one for RP router // XXX specification???
    ctrl->setProtocol(datagram->getTransportProtocol());
    data->setControlInfo(ctrl);
    send(data, "ipOut");
}

/**
 * NEW MULTICAST REGISTER DR
 *
 * The method process notification about new multicast data stream at DR.
 *
 * @param newRoute Pointer to new entry in the multicast routing table.
 * @see addOutIntFull()
 * @see setKat()
 */
void PIMSM::newMulticastRegisterDR(IPv4Address srcAddr, IPv4Address destAddr)
{
    EV << "pimSM::newMulticastRegisterDR" << endl;

    InterfaceEntry *inInt = rt->getInterfaceForDestAddr(srcAddr);
    if (!inInt)
    {
        EV << "ERROR: PimSplitter::newMulticast(): cannot find RPF interface, routing information is missing.";
        return;
    }

    PIMInterface *rpfInterface = pimIft->getInterfaceById(inInt->getInterfaceId());
    if (!rpfInterface || rpfInterface->getMode() != PIMInterface::SparseMode)
        return;

    InterfaceEntry *newInIntG = rt->getInterfaceForDestAddr(this->getRPAddress());

    // RPF check and check if I am DR for given address
    if ((newInIntG->getInterfaceId() != rpfInterface->getInterfaceId()) && IamDR(srcAddr))
    {
        EV << "PIMSM::newMulticast - group: " << destAddr << ", source: " << srcAddr << endl;

        // create new multicast route
        PIMMulticastRoute *newRoute = new PIMMulticastRoute(srcAddr, destAddr);
        newRoute->setInInterface(new PIMMulticastRoute::PIMInInterface(rpfInterface->getInterfacePtr(), IPv4Address("0.0.0.0")));


        PIMMulticastRoute *newRouteG = new PIMMulticastRoute(IPv4Address::UNSPECIFIED_ADDRESS,newRoute->getMulticastGroup());


        // Set Keep Alive timer for routes
        PIMkat* timerKat = createKeepAliveTimer(newRoute->getOrigin(), newRoute->getMulticastGroup());
        PIMkat* timerKatG = createKeepAliveTimer(IPv4Address::UNSPECIFIED_ADDRESS, newRoute->getMulticastGroup());
        newRoute->setKeepAliveTimer(timerKat);
        newRouteG->setKeepAliveTimer(timerKatG);

        //Create (*,G) state
        PIMNeighbor *RPFnbr = pimNbt->getFirstNeighborOnInterface(newInIntG->getInterfaceId());                            // RPF neighbor
        newRouteG->setInInterface(new PIMMulticastRoute::PIMInInterface(newInIntG, RPFnbr->getAddress()));
        newRouteG->setRP(this->getRPAddress());
        newRouteG->setFlags(PIMMulticastRoute::S | PIMMulticastRoute::P | PIMMulticastRoute::F);

        //Create (S,G) state - set flags and Register state, other is set by  PimSplitter
        newRoute->setFlags(PIMMulticastRoute::P | PIMMulticastRoute::F | PIMMulticastRoute::T);
        newRoute->addOutInterface(new PIMMulticastRoute::PIMOutInterface(newInIntG,
                                    PIMMulticastRoute::Pruned,
                                    PIMInterface::SparseMode,
                                    NULL,
                                    NULL,
                                    PIMMulticastRoute::NoInfo,
                                    PIMMulticastRoute::Join,false));      // create new outgoing interface to RP
        newRoute->setRP(this->getRPAddress());
        //newRoute->setRegisterTunnel(false);                   //we need to set register state to output interface, but output interface has to be null for now

        rt->addMulticastRoute(newRouteG);
        rt->addMulticastRoute(newRoute);

        EV << "pimSM::newMulticast: New routes was added to the multicast routing table." << endl;
    }
}

/**
 * REMOVE MULTICAST RECEIVER
 *
 * The method remove outgoing interface to multicast  listener.
 * If outgoing interface is become null after removing interface,
 * Prune message has to be sent.
 *
 * @param members Pointer to addRemoveAddr class.
 * @see addRemoveAddr
 * @see sendPIMJoinPrune()
 */
void PIMSM::removeMulticastReceiver(PIMInterface *pimInt, IPv4Address multicastGroup)
{
    EV << "pimSM::removeMulticastReciever" << endl;

    EV << "Removed multicast address: " << multicastGroup << endl;
    std::vector<PIMMulticastRoute*> routes = getRouteFor(multicastGroup);

    // go through all multicast routes
    for (unsigned int j = 0; j < routes.size(); j++)
    {
        PIMMulticastRoute *route = routes[j];
        PIMInInterface *rpfInterface = route->getPIMInInterface();
        PIMNeighbor *neighborToRP = pimNbt->getFirstNeighborOnInterface(rpfInterface->getInterfaceId());
        unsigned int k;

        // is interface in list of outgoing interfaces?
        for (k = 0; k < route->getNumOutInterfaces(); k++)
        {
            PIMOutInterface *outInterface = route->getPIMOutInterface(k);
            if (outInterface->getInterfaceId() == pimInt->getInterfaceId())
            {
                EV << "Interface is present, removing it from the list of outgoing interfaces." << endl;
                if (outInterface->expiryTimer)
                {
                    cancelEvent(outInterface->expiryTimer);
                    delete (outInterface->expiryTimer);
                }
                route->removeOutInterface(k); // FIXME missing k-- or break
            }
        }
        route->clearFlag(PIMMulticastRoute::C);
        // there is no receiver of multicast, prune the router from the multicast tree
        if (route->isOilistNull())
        {
            route->setFlags(PIMMulticastRoute::P);
            sendPIMJoinPrune(route->getMulticastGroup(),this->getRPAddress(),neighborToRP->getAddress(),PruneMsg,G);
            if (route->getJoinTimer())
            {
                cancelEvent(route->getJoinTimer());
                delete route->getJoinTimer();
                route->setJoinTimer(NULL);
            }
        }
    }
}

void PIMSM::newMulticastReceiver(PIMInterface *pimInterface, IPv4Address multicastGroup)
{
    EV << "pimSM::newMulticastReciever" << endl;

    PIMMulticastRoute *newRouteG = new PIMMulticastRoute(IPv4Address::UNSPECIFIED_ADDRESS,multicastGroup);
    PIMMulticastRoute *routePointer;

    int interfaceId = pimInterface->getInterfaceId();
    InterfaceEntry *newInIntG = rt->getInterfaceForDestAddr(this->RPAddress);
    PIMNeighbor *neighborToRP = pimNbt->getFirstNeighborOnInterface(newInIntG->getInterfaceId());

    // XXX neighborToRP can be NULL!

    routePointer = newRouteG;
    if (!(newRouteG = getRouteFor(multicastGroup,IPv4Address::UNSPECIFIED_ADDRESS)))                             // create new (*,G) route
    {
        InterfaceEntry *outInt = ift->getInterfaceById(interfaceId);
        newRouteG = routePointer;
        // set source, mult. group, etc...
        newRouteG->setRP(this->getRPAddress());
        newRouteG->setFlags(PIMMulticastRoute::S | PIMMulticastRoute::C);

        // set incoming interface
        newRouteG->setInInterface(new PIMMulticastRoute::PIMInInterface(newInIntG, neighborToRP->getAddress()));

        // create and set (*,G) ET timer
        PIMet* timerEt = createExpiryTimer(outInt->getInterfaceId(),HOLDTIME_HOST,multicastGroup,IPv4Address::UNSPECIFIED_ADDRESS, G);
        PIMet* timerEtNI = createExpiryTimer(NO_INT_TIMER,HOLDTIME,multicastGroup,IPv4Address::UNSPECIFIED_ADDRESS, G);
        PIMjt* timerJt = createJoinTimer(multicastGroup, this->getRPAddress(), neighborToRP->getAddress(),G);
        newRouteG->setJoinTimer(timerJt);
        newRouteG->setExpiryTimer(timerEtNI);

        // set outgoing interface to RP
        newRouteG->addOutInterface(new PIMMulticastRoute::PIMOutInterface(outInt,
                                    PIMMulticastRoute::Forward,
                                    PIMInterface::SparseMode,
                                    NULL,
                                    timerEt,
                                    PIMMulticastRoute::NoInfo,
                                    PIMMulticastRoute::NoInfoRS,true));
        rt->addMulticastRoute(newRouteG);

        // oilist != NULL -> send Join(*,G) to 224.0.0.13
        if (!newRouteG->isOilistNull())
            sendPIMJoinPrune(multicastGroup,this->getRPAddress(),neighborToRP->getAddress(),JoinMsg,G);
    }
    else                                                                                                        // add new outgoing interface to existing (*,G) route
    {
        InterfaceEntry *nextOutInt = ift->getInterfaceById(interfaceId);
        //PIMet* timerEt = createExpiryTimer(nextOutInt->getInterfaceId(),HOLDTIME,multGroup,IPv4Address::UNSPECIFIED_ADDRESS, G);

        newRouteG->addOutInterface(new PIMMulticastRoute::PIMOutInterface(nextOutInt,
                                    PIMMulticastRoute::Forward,
                                    PIMInterface::SparseMode,
                                    NULL,
                                    NULL,
                                    PIMMulticastRoute::NoInfo,
                                    PIMMulticastRoute::NoInfoRS,true));
        newRouteG->setFlags(PIMMulticastRoute::C);
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
            processJoinTimer(check_and_cast<PIMjt *> (timer));
            break;
        case PrunePendingTimer:
            EV << "PrunePendingTimer" << endl;
            processPrunePendingTimer(check_and_cast<PIMppt *> (timer));
            break;
        case ExpiryTimer:
            EV << "ExpiryTimer" << endl;
            processExpiryTimer(check_and_cast<PIMet *> (timer));
            break;
        case KeepAliveTimer:
            EV << "KeepAliveTimer" << endl;
            processKeepAliveTimer(check_and_cast<PIMkat *> (timer));
            break;
        case RegisterStopTimer:
            EV << "RegisterStopTimer" << endl;
            processRegisterStopTimer(check_and_cast<PIMrst *> (timer));
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
            // TODO for future use
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
    PIMMulticastRoute *route;
    IPv4Datagram *datagram;
    PIMInterface *pimInterface;

    if (signalID == NF_IPv4_MCAST_REGISTERED)
    {
        EV <<  "pimSM::receiveChangeNotification - NEW IGMP ADDED" << endl;
        IPv4MulticastGroupInfo *info = check_and_cast<IPv4MulticastGroupInfo*>(details);
        pimInterface = pimIft->getInterfaceById(info->ie->getInterfaceId());
        if (pimInterface && pimInterface->getMode() == PIMInterface::SparseMode)
            newMulticastReceiver(pimInterface, info->groupAddress);
    }
    else if (signalID == NF_IPv4_MCAST_UNREGISTERED)
    {
        EV <<  "pimSM::receiveChangeNotification - IGMP REMOVED" << endl;
        IPv4MulticastGroupInfo *info = check_and_cast<IPv4MulticastGroupInfo*>(details);
        pimInterface = pimIft->getInterfaceById(info->ie->getInterfaceId());
        if (pimInterface && pimInterface->getMode() == PIMInterface::SparseMode)
            removeMulticastReceiver(pimInterface, info->groupAddress);
    }
    // new multicast data appears in router
    else if (signalID == NF_IPv4_NEW_MULTICAST)
    {
        EV <<  "PimSM::receiveChangeNotification - NEW MULTICAST" << endl;
        datagram = check_and_cast<IPv4Datagram*>(details);
        IPv4Address srcAddr = datagram->getSrcAddress();
        IPv4Address destAddr = datagram->getDestAddress();
        newMulticastRegisterDR(srcAddr, destAddr);
    }
    // create PIM register packet
    else if (signalID == NF_IPv4_MDATA_REGISTER)
    {
        EV <<  "pimSM::receiveChangeNotification - REGISTER DATA" << endl;
        datagram = check_and_cast<IPv4Datagram*>(details);
        route = getRouteFor(datagram->getDestAddress(), datagram->getSrcAddress());
        if (route)
        {
            InterfaceEntry *intToRP = rt->getInterfaceForDestAddr(route->getRP());
            if (route->isFlagSet(PIMMulticastRoute::F) && route->isFlagSet(PIMMulticastRoute::P))
            {
                PIMOutInterface *outInterface = route->findOutInterfaceByInterfaceId(intToRP->getInterfaceId());
                if (outInterface && outInterface->regState == PIMMulticastRoute::Join)
                    sendPIMRegister(datagram);
            }
        }
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
                dataOnRpf(route);
            route = getRouteFor(datagram->getDestAddress(), datagram->getSrcAddress());
            if (route)
                dataOnRpf(route);
        }
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

bool PIMSM::deleteMulticastRoute(PIMMulticastRoute *route)
{
    IPv4MulticastRoute *routeFound = rt->removeMulticastRoute(route);
    if (routeFound == route)
    {
        cancelAndDelete(route->getStateRefreshTimer());
        cancelAndDelete(route->getGraftRetryTimer());
        cancelAndDelete(route->getSourceActiveTimer());
        cancelAndDelete(route->getKeepAliveTimer());
        cancelAndDelete(route->getExpiryTimer());
        cancelAndDelete(route->getJoinTimer());
        cancelAndDelete(route->getPrunePendingTimer());
        for (unsigned int j = 0;j < route->getNumOutInterfaces(); j++)
            cancelAndDelete(route->getPIMOutInterface(j)->pruneTimer);

        delete route;
        return true;
    }
    return false;
}
