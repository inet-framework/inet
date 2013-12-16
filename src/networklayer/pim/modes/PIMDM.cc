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
// Authors: Veronika Rybova, Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)

#include "IPv4Datagram.h"
#include "PIMDM.h"

Define_Module(PIMDM);

using namespace std;

typedef IPv4MulticastRoute::OutInterface OutInterface;
typedef PIMMulticastRoute::PIMInInterface PIMInInterface;
typedef PIMMulticastRoute::PIMOutInterface PIMOutInterface;

/**
 * SEND PIM JOIN PRUNE
 *
 * The method is used to create PIMJoinPrune Packet and send it to next hop router.
 *
 * @param nextHop IP Address of receiver.
 * @param src IP address of multicast source.
 * @param grp IP address of multicast group.
 * @param intId ID of outgoing interface.
 * @see PIMJoinPrune()
 */
void PIMDM::sendPimJoinPrune(IPv4Address nextHop, IPv4Address src, IPv4Address grp, int intId)
{
	EV << "pimDM::sendPimJoinPrune" << endl;
	EV << "UpstreamNeighborAddress: " << nextHop << ", Source: " << src << ", Group: " << grp << ", IntId: " << intId << endl;

	PIMJoinPrune *msg = new PIMJoinPrune();
	msg->setName("PIMJoinPrune");
	msg->setUpstreamNeighborAddress(nextHop);
	msg->setHoldTime(pruneInterval);
	msg->setMulticastGroupsArraySize(1);

	//FIXME change to add also join groups
	// we do not need it at this time

	// set multicast groups
	MulticastGroup *group = new MulticastGroup();
    EncodedAddress address;
	group->setGroupAddress(grp);
	group->setJoinedSourceAddressArraySize(0);
	group->setPrunedSourceAddressArraySize(1);
	address.IPaddress = src;
	group->setPrunedSourceAddress(0, address);
	msg->setMulticastGroups(0, *group);

	// set IP Control info
	IPv4ControlInfo *ctrl = new IPv4ControlInfo();
	IPv4Address ga1("224.0.0.13");
	ctrl->setDestAddr(ga1);
	//ctrl->setProtocol(IP_PROT_PIM);
	ctrl->setProtocol(103);
	ctrl->setTimeToLive(1);
	ctrl->setInterfaceId(intId);
	msg->setControlInfo(ctrl);
	send(msg, "spiltterOut");
}

/**
 * SEND PIM GRAFT ACK
 *
 * The method is used to create PIMGraftAck packet and send it to next hop router.
 * PIMGraftAck pkt is copy of received PIMGraft pkt. Only type and IP control info
 * has to be changed.
 *
 * @param msg Pointer to PIMGraft packet.
 * @see PIMGraftAck()
 */
void PIMDM::sendPimGraftAck(PIMGraftAck *msg)
{
	msg->setName("PIMGraftAck");
	msg->setType(GraftAck);

	// set IP Control info
	IPv4ControlInfo *oldCtrl = check_and_cast<IPv4ControlInfo*>(msg->removeControlInfo());
	IPv4ControlInfo *ctrl = new IPv4ControlInfo();
	ctrl->setDestAddr(oldCtrl->getSrcAddr());
	ctrl->setSrcAddr(oldCtrl->getDestAddr());
	ctrl->setProtocol(103);
	ctrl->setTimeToLive(1);
	ctrl->setInterfaceId(oldCtrl->getInterfaceId());
	delete oldCtrl;
	msg->setControlInfo(ctrl);
	send(msg, "spiltterOut");
}

/**
 * SEND PIM GRAFT
 *
 * The method is used to create PIMGraft packet and send it to next hop router.
 * Only  JoinedSource part of message is used, because Graft pkt is sent when
 * router wants to join to multicast tree again.
 *
 * @param nextHop IP Address of receiver.
 * @param src IP address of multicast source.
 * @param grp IP address of multicast group.
 * @param intId ID of outgoing interface.
 * @see PIMGraft()
 */
void PIMDM::sendPimGraft(IPv4Address nextHop, IPv4Address src, IPv4Address grp, int intId)
{
	EV << "pimDM::sendPimGraft" << endl;
	EV << "UpstreamNeighborAddress: " << nextHop << ", Source: " << src << ", Group: " << grp << ", IntId: " << intId << endl;

	PIMGraft *msg = new PIMGraft();
	msg->setName("PIMGraft");
	msg->setHoldTime(0);
	msg->setUpstreamNeighborAddress(nextHop);
	msg->setMulticastGroupsArraySize(1);

	// set multicast groups
	MulticastGroup *group = new MulticastGroup();
	EncodedAddress address;
	group->setGroupAddress(grp);
	group->setJoinedSourceAddressArraySize(1);
	group->setPrunedSourceAddressArraySize(0);
    address.IPaddress = src;
	group->setJoinedSourceAddress(0, address);
	msg->setMulticastGroups(0, *group);

	// set IP Control info
	IPv4ControlInfo *ctrl = new IPv4ControlInfo();
	ctrl->setDestAddr(nextHop);
	//ctrl->setProtocol(IP_PROT_PIM);
	ctrl->setProtocol(103);
	ctrl->setTimeToLive(1);
	ctrl->setInterfaceId(intId);
	msg->setControlInfo(ctrl);
	send(msg, "spiltterOut");
}

/**
 * SEND PIM STATE REFRESH
 *
 * The method is used to create PIMStateRefresh packet and send it to next hop router.
 * By using the message we do not need to flood the network every 3 minutes.
 *
 * @param originator IP Address of source router.
 * @param src IP address of multicast source.
 * @param grp IP address of multicast group.
 * @param intId ID of outgoing interface.
 * @param P Indicator of pruned outgoing interface. If interface is pruned it is set to 1, otherwise to 0.
 * @see PIMStateRefresh()
 */
void PIMDM::sendPimStateRefresh(IPv4Address originator, IPv4Address src, IPv4Address grp, int intId, bool P)
{
	EV << "pimDM::sendPimStateRefresh" << endl;

	PIMStateRefresh *msg = new PIMStateRefresh();
	msg->setName("PIMStateRefresh");
	msg->setGroupAddress(grp);
	msg->setSourceAddress(src);
	msg->setOriginatorAddress(originator);
	msg->setInterval(stateRefreshInterval);
	msg->setP(P);

	// set IP Control info
	IPv4ControlInfo *ctrl = new IPv4ControlInfo();
	ctrl->setDestAddr(grp);
	//ctrl->setProtocol(IP_PROT_PIM);
	ctrl->setProtocol(103);
	ctrl->setTimeToLive(1);
	ctrl->setInterfaceId(intId);
	msg->setControlInfo(ctrl);
	send(msg, "spiltterOut");
}

/**
 * CREATE PRUNE TIMER
 *
 * The method is used to create PIMPrune timer. The timer is set when outgoing interface
 * goes to pruned state. After expiration (usually 3min) interface goes back to forwarding
 * state. It is set to (S,G,I).
 *
 * @param source IP address of multicast source.
 * @param group IP address of multicast group.
 * @param intId ID of outgoing interface.
 * @param holdTime Time of expiration (usually 3min).
 * @return Pointer to new Prune Timer.
 * @see PIMpt()
 */
PIMpt* PIMDM::createPruneTimer(IPv4Address source, IPv4Address group, int intId, int holdTime)
{
	PIMpt *timer = new PIMpt("PimPruneTimer", PruneTimer);
	timer->setSource(source);
	timer->setGroup(group);
	timer->setIntId(intId);
	scheduleAt(simTime() + holdTime, timer);
	return timer;
}

/**
 * CREATE GRAFT RETRY TIMER
 *
 * The method is used to create PIMGraftRetry timer. The timer is set when router wants to join to
 * multicast tree again and send PIM Prune message to upstream. Router waits for Graft Retry Timer
 * (3 s) for PIM PruneAck message from upstream. If timer expires, router will send PIM Prune message
 * again. It is set to (S,G).
 *
 * @param source IP address of multicast source.
 * @param group IP address of multicast group.
 * @return Pointer to new Graft Retry Timer.
 * @see PIMgrt()
 */
PIMgrt* PIMDM::createGraftRetryTimer(IPv4Address source, IPv4Address group)
{
	PIMgrt *timer = new PIMgrt("PIMGraftRetryTimer", GraftRetryTimer);
	timer->setSource(source);
	timer->setGroup(group);
	scheduleAt(simTime() + graftRetryInterval, timer);
	return timer;
}

/**
 * CREATE SOURCE ACTIVE TIMER
 *
 * The method is used to create PIMSourceActive timer. The timer is set when source of multicast is
 * connected directly to the router.  If timer expires, router will remove the route from multicast
 * routing table. It is set to (S,G).
 *
 * @param source IP address of multicast source.
 * @param group IP address of multicast group.
 * @return Pointer to new Source Active Timer
 * @see PIMsat()
 */
PIMsat* PIMDM::createSourceActiveTimer(IPv4Address source, IPv4Address group)
{
	PIMsat *timer = new PIMsat("PIMSourceActiveTimer", SourceActiveTimer);
	timer->setSource(source);
	timer->setGroup(group);
	scheduleAt(simTime() + sourceActiveInterval, timer);
	return timer;
}

/**
 * CREATE STATE REFRESH TIMER
 *
 * The method is used to create PIMStateRefresh timer. The timer is set when source of multicast is
 * connected directly to the router. If timer expires, router will send StateRefresh message, which
 * will propagate through all network and wil reset Prune Timer. It is set to (S,G).
 *
 * @param source IP address of multicast source.
 * @param group IP address of multicast group.
 * @return Pointer to new Source Active Timer
 * @see PIMsrt()
 */
PIMsrt* PIMDM::createStateRefreshTimer(IPv4Address source, IPv4Address group)
{
	PIMsrt *timer = new PIMsrt("PIMStateRefreshTimer", StateRefreshTimer);
	timer->setSource(source);
	timer->setGroup(group);
	scheduleAt(simTime() + stateRefreshInterval, timer);
	return timer;
}


/**
 * PROCESS GRAFT PACKET
 *
 * The method is used to process PIMGraft packet. Packet means that downstream router wants to join to
 * multicast tree, so the packet cannot come to RPF interface. Router finds correct outgoig interface
 * towards downstream router. Change its state to forward if it was not before and cancel Prune Timer.
 * If route was in pruned state, router will send also Graft message to join multicast tree.
 *
 * @param source IP address of multicast source.
 * @param group IP address of multicast group.
 * @param sender IP Address of Graft packet sender.
 * @param intId ID of Graft packet incoming interface.
 * @see sendPimGraft()
 * @see createGraftRetryTimer()
 * @see PIMGraft()
 * @see PIMgrt()
 * @see processJoinPruneGraftPacket()
 */
void PIMDM::processGraftPacket(IPv4Address source, IPv4Address group, IPv4Address sender, int intId)
{
	EV << "pimDM::processGraftPacket" << endl;

	PIMMulticastRoute *route = getRouteFor(group, source);

	bool forward = false;

	// check if message come to non-RPF interface
	if (route->getPIMInInterface()->getInterfaceId() == intId)
	{
		EV << "ERROR: Graft message came to RPF interface." << endl;
		return;
	}

	// find outgoing interface to neighbor
	for (unsigned int l = 0; l < route->getNumOutInterfaces(); l++)
	{
	    PIMOutInterface *outInt = route->getPIMOutInterface(l);
		if(outInt->getInterfaceId() == intId)
		{
			forward = true;
			if (outInt->forwarding == PIMMulticastRoute::Pruned)
			{
				EV << "Interface " << outInt->getInterfaceId() << " transit to forwarding state (Graft)." << endl;
				outInt->forwarding = PIMMulticastRoute::Forward;

				//cancel Prune Timer
				PIMpt* timer = outInt->pruneTimer;
				cancelEvent(timer);
				delete timer;
				outInt->pruneTimer = NULL;
			}
		}
	}

	// if all route was pruned, remove prune flag
	// if upstrem is not source, send Graft message
	if (route->isFlagSet(PIMMulticastRoute::P) && forward && (route->getGraftRetryTimer() == NULL))
	{
		if (!route->isFlagSet(PIMMulticastRoute::A))
		{
			EV << "Route is not pruned any more, send Graft to upstream" << endl;
			PIMInInterface *inInterface = route->getPIMInInterface();
			sendPimGraft(inInterface->nextHop, source, group, inInterface->getInterfaceId());
			PIMgrt* timer = createGraftRetryTimer(source, group);
			route->setGraftRetryTimer(timer);
		}
		else
			route->clearFlag(PIMMulticastRoute::P);
	}
}

/**
 * PROCESS GRAFT ACK PACKET
 *
 * The method is used to process PIMGraftAck packet. Packet means that the router was successfully joined
 * to multicast tree. Route is now in forwarding state and Graft Retry timer was canceled.
 *
 * @param route Pointer to multicast route which GraftAck belongs to.
 * @see processJoinPruneGraftPacket()
 * @see PIMgrt()
 */
void PIMDM::processGraftAckPacket(PIMMulticastRoute *route)
{
	PIMgrt *grt = route->getGraftRetryTimer();
	if (grt != NULL)
	{
		cancelEvent(grt);
		delete grt;
		route->setGraftRetryTimer(NULL);
		route->clearFlag(PIMMulticastRoute::P);
	}
}

/**
 * PROCESS PRUNE PACKET
 *
 * The method process PIM Prune packet. First the method has to find correct outgoing interface
 * where PIM Prune packet came to. The method also checks if there is still any forwarding outgoing
 * interface. Forwarding interfaces, where Prune packet come to, goes to prune state. If all outgoing
 * interfaces are pruned, the router will prune from multicast tree.
 *
 * @param route Pointer to multicast route which GraftAck belongs to.
 * @param intId
 * @see processJoinPruneGraftPacket()
 * @see createPruneTimer()
 * @see sendPimJoinPrune()
 * @see PIMpt()
 */
void PIMDM::processPrunePacket(PIMMulticastRoute *route, int intId, int holdTime)
{
	EV << "pimDM::processPrunePacket" << endl;
	bool change = false;

	// we find correct outgoing interface
    PIMOutInterface *outInt = route->findOutInterfaceByInterfaceId(intId);
	if (outInt)
	{
		// if interface was already pruned, restart Prune Timer
		if (outInt->forwarding == PIMMulticastRoute::Pruned)
		{
			EV << "Outgoing interface is already pruned, restart Prune Timer." << endl;
			PIMpt* timer = outInt->pruneTimer;
			cancelEvent(timer);
			scheduleAt(simTime() + holdTime, timer);
		}
		// if interface is forwarding, transit its state to pruned and set Prune timer
		else
		{
			EV << "Outgoing interfaces is forwarding now -> change to Pruned." << endl;
			outInt->forwarding = PIMMulticastRoute::Pruned;
			PIMpt* timer = createPruneTimer(route->getOrigin(), route->getMulticastGroup(), intId, holdTime);
			outInt->pruneTimer = timer;
			change = true;
		}
	}

	// if there is no forwarding outgoing int, transit route to pruned state
	if (route->isOilistNull() && change)
	{
		EV << "Route is not forwarding any more, send Prune to upstream." << endl;
		route->setFlags(PIMMulticastRoute::P);

		// if GRT is running now, do not send Prune msg
		if (route->isFlagSet(PIMMulticastRoute::P) && (route->getGraftRetryTimer() != NULL))
		{
			cancelEvent(route->getGraftRetryTimer());
			delete route->getGraftRetryTimer();
			route->setGraftRetryTimer(NULL);
		}
		else if (!route->isFlagSet(PIMMulticastRoute::A))
		{
		    PIMInInterface *inInterface = route->getPIMInInterface();
			sendPimJoinPrune(inInterface->nextHop, route->getOrigin(), route->getMulticastGroup(), inInterface->getInterfaceId());
		}
	}
}


/**
 * PROCESS JOIN PRUNE GRAFT PACKET
 *
 * The method is used to process PIM JoinePrune, PIMGraft, and PIMGraftAck packet. All these packets have
 * the same structure. If packet is for this router the method goes through all multicast groups in message.
 * There could be list of joined and pruned sources for each multicast group. Joine Prune message can contains
 * both. Graft and Graft Ack can contain only join list. According to type of packet, appropriate method is
 * called to process info.
 *
 * @param pkt Pointer to packet.
 * @param type Type of packet.
 * @see processGraftPacket()
 * @see processGraftAckPacket()
 * @see processPrunePacket()
 * @see sendPimGraftAck()
 */
void PIMDM::processJoinPruneGraftPacket(PIMJoinPrune *pkt, PIMPacketType type)
{
	EV << "pimDM::processJoinePruneGraftPacket" << endl;

	IPv4ControlInfo *ctrl = check_and_cast<IPv4ControlInfo*>(pkt->getControlInfo());
	IPv4Address sender = ctrl->getSrcAddr();
	InterfaceEntry * nt = rt->getInterfaceForDestAddr(sender);
	vector<PIMNeighbor*> neighbors = pimNbt->getNeighborsOnInterface(nt->getInterfaceId());
	IPv4Address addr = nt->ipv4Data()->getIPAddress();

	// does packet belong to this router?
	if (pkt->getUpstreamNeighborAddress() != nt->ipv4Data()->getIPAddress() && type != GraftAck)
	{
		delete pkt;
		return;
	}

	// go through list of multicast groups
	for (unsigned int i = 0; i < pkt->getMulticastGroupsArraySize(); i++)
	{
		MulticastGroup group = pkt->getMulticastGroups(i);
		IPv4Address groupAddr = group.getGroupAddress();

		// go through list of joined sources
		//EV << "JoinedSourceAddressArraySize: " << group.getJoinedSourceAddressArraySize() << endl;
		for (unsigned int j = 0; j < group.getJoinedSourceAddressArraySize(); j++)
		{
			IPv4Address source = (group.getJoinedSourceAddress(j)).IPaddress;
			PIMMulticastRoute *route = getRouteFor(groupAddr, source);

			if (type == JoinPrune)
			{
				//FIXME join action
				// only if there is more than one PIM neighbor on one interface
				// interface change to forwarding state
				// cancel Prune Timer
				// send Graft to upstream
			}
			else if (type == Graft)
				processGraftPacket(source, groupAddr, sender, nt->getInterfaceId());
			else if (type == GraftAck)
				processGraftAckPacket(route);
		}

		// go through list of pruned sources (only for PIM Join Prune msg)
		if (type == JoinPrune)
		{
			//EV << "JoinedPrunedAddressArraySize: " << group.getPrunedSourceAddressArraySize() << endl;
			for(unsigned int k = 0; k < group.getPrunedSourceAddressArraySize(); k++)
			{
				IPv4Address source = (group.getPrunedSourceAddress(k)).IPaddress;
				PIMMulticastRoute *route = getRouteFor(groupAddr, source);

				if (source != route->getOrigin())
					continue;

				// if there could be more than one PIM neighbor on interface
				if (neighbors.size() > 1)
				{
					; //FIXME set PPT timer
				}
				// if there is only one PIM neighbor on interface
				else
					processPrunePacket(route, nt->getInterfaceId(), pkt->getHoldTime());
			}
		}
	}

	// Send GraftAck for this Graft message
	if (type == Graft)
		sendPimGraftAck(check_and_cast<PIMGraftAck*>(pkt));
}

/**
 * PROCESS STATE REFRESH PACKET
 *
 * The method is used to process PIMStateRefresh packet. The method checks if there is route in mroute
 * and that packet has came to RPF interface. Then it goes through all outgoing interfaces. If the
 * interface is pruned, it resets Prune Timer. For each interface State Refresh message is copied and
 * correct prune indicator is set according to state of outgoing interface (pruned/forwarding).
 *
 * State Refresh message is used to stop flooding of network each 3 minutes.
 *
 * @param pkt Pointer to packet.
 * @see sendPimJoinPrune()
 * @see sendPimStateRefresh()
 */
void PIMDM::processStateRefreshPacket(PIMStateRefresh *pkt)
{
	EV << "pimDM::processStateRefreshPacket" << endl;

	// FIXME actions of upstream automat according to pruned/forwarding state and Prune Indicator from msg

	// first check if there is route for given group address and source
	PIMMulticastRoute *route = getRouteFor(pkt->getGroupAddress(), pkt->getSourceAddress());
	if (route == NULL)
	{
		delete pkt;
		return;
	}
	bool pruneIndicator;

	// chceck if State Refresh msg has came to RPF interface
	IPv4ControlInfo *ctrl = check_and_cast<IPv4ControlInfo*>(pkt->getControlInfo());
	PIMInInterface *inInterface = route->getPIMInInterface();
	if (ctrl->getInterfaceId() != inInterface->getInterfaceId())
	{
		delete pkt;
		return;
	}

	// this router is pruned, but outgoing int of upstream router leading to this router is forwarding
	if (route->isFlagSet(PIMMulticastRoute::P) && !pkt->getP())
	{
		// send Prune msg to upstream
		if (route->getGraftRetryTimer() == NULL)
		{
		    PIMInInterface *inInterface = route->getPIMInInterface();
			sendPimJoinPrune(inInterface->nextHop, route->getOrigin(), route->getMulticastGroup(), inInterface->getInterfaceId());
		}
		else
		{
			cancelEvent(route->getGraftRetryTimer());
			delete route->getGraftRetryTimer();
			route->setGraftRetryTimer(NULL);
			///////delete P
		}
	}

	// go through all outgoing interfaces, reser Prune Timer and send out State Refresh msg
	for (unsigned int i = 0; i < route->getNumOutInterfaces(); i++)
	{
	    PIMOutInterface *outInt = route->getPIMOutInterface(i);
		if (outInt->forwarding == PIMMulticastRoute::Pruned)
		{
			// P = true
			pruneIndicator = true;
			// reset PT
			cancelEvent(outInt->pruneTimer);
			scheduleAt(simTime() + pruneInterval, outInt->pruneTimer);
		}
		else if (outInt->forwarding == PIMMulticastRoute::Forward)
		{
			// P = false
			pruneIndicator = false;
		}
		sendPimStateRefresh(pkt->getOriginatorAddress(), pkt->getSourceAddress(), pkt->getGroupAddress(), outInt->getInterfaceId(), pruneIndicator);
	}
	delete pkt;
}


/**
 * PROCESS PRUNE TIMER
 *
 * The method is used to process PIM Prune timer. It is (S,G,I) timer. When Prune timer expires, it
 * means that outgoing interface transits back to forwarding state. If the router is pruned from
 * multicast tree, join again.
 *
 * @param timer Pointer to Prune timer.
 * @see PIMpt()
 * @see sendPimGraft()
 * @see createGraftRetryTimer()
 */
void PIMDM::processPruneTimer(PIMpt *timer)
{
	EV << "pimDM::processPruneTimer" << endl;

	IPv4Address source = timer->getSource();
	IPv4Address group = timer->getGroup();
	int intId = timer->getIntId();

	// find correct (S,G) route which timer belongs to
	PIMMulticastRoute *route = getRouteFor(group, source);
	if (route == NULL)
	{
		delete timer;
		return;
	}

	// state of interface is changed to forwarding
    PIMOutInterface *outInt = route->findOutInterfaceByInterfaceId(intId);
	if (outInt)
	{
		delete timer;
		outInt->pruneTimer = NULL;
		outInt->forwarding = PIMMulticastRoute::Forward;
		route->clearFlag(PIMMulticastRoute::P);

		// if the router is pruned from multicast tree, join again
		/*if (route->isFlagSet(P) && (route->getGrt() == NULL))
		{
			if (!route->isFlagSet(A))
			{
				EV << "Pruned cesta prejde do forwardu, posli Graft" << endl;
				sendPimGraft(route->getInIntNextHop(), source, group, route->getInIntId());
				PIMgrt* timer = createGraftRetryTimer(source, group);
				route->setGrt(timer);
			}
			else
				route->removeFlag(P);
		}*/
	}
}

/**
 * PROCESS GRAFT RETRY TIMER
 *
 * The method is used to process PIM Graft Retry Timer. It is (S,G) timer. When Graft Retry timer expires,
 * it means that the router did'n get GraftAck message from upstream router. The router has to send Prune
 * packet again.
 *
 * @param timer Pointer to Graft Retry timer.
 * @see PIMgrt()
 * @see sendPimGraft()
 * @see createGraftRetryTimer()
 */
void PIMDM::processGraftRetryTimer(PIMgrt *timer)
{
	PIMMulticastRoute *route = getRouteFor(timer->getGroup(), timer->getSource());
	PIMInInterface *inInterface = route->getPIMInInterface();
	sendPimGraft(inInterface->nextHop, timer->getSource(), timer->getGroup(), inInterface->getInterfaceId());
	timer = createGraftRetryTimer(timer->getSource(), timer->getGroup());
}

/**
 * PROCESS GRAFT RETRY TIMER
 *
 * The method is used to process PIM Source Active Timer. It is (S,G) timer. When Source Active Timer expires,
 * route is removed from multicast routing table.
 *
 * @param timer Pointer to Source Active Timer.
 * @see PIMsat()
 */
void PIMDM::processSourceActiveTimer(PIMsat * timer)
{
	EV << "pimDM::processSourceActiveTimer: route will be deleted" << endl;
	PIMMulticastRoute *route = getRouteFor(timer->getGroup(), timer->getSource());

	delete timer;
	route->setSourceActiveTimer(NULL);
	deleteMulticastRoute(route);
}

/**
 * PROCESS STATE REFRESH TIMER
 *
 * The method is used to process PIM State Refresh Timer. It is (S,G) timer and it is used only on router
 * whoch is connected directly to the source of multicast. When State Refresh Timer expires, the State Refresh
 * messages are sent to all outgoing interfaces. Prune indicator in each msg is set according to state of the
 * outgoing interface. State Refresh Timer is then reset.
 *
 * @param timer Pointer to Source Active Timer.
 * @see PIMsrt()
 * @see sendPimStateRefresh()
 * @see createStateRefreshTimer()
 */
void PIMDM::processStateRefreshTimer(PIMsrt * timer)
{
	EV << "pimDM::processStateRefreshTimer" << endl;
	PIMMulticastRoute *route = getRouteFor(timer->getGroup(), timer->getSource());
	bool pruneIndicator;

	for (unsigned int i = 0; i < route->getNumOutInterfaces(); i++)
	{
	    PIMOutInterface *outInt = route->getPIMOutInterface(i);
		if (outInt->forwarding == PIMMulticastRoute::Pruned)
		{
			// P = true
			pruneIndicator = true;
			// reset PT
			cancelEvent(outInt->pruneTimer);
			scheduleAt(simTime() + pruneInterval, outInt->pruneTimer);
		}
		else if (outInt->forwarding == PIMMulticastRoute::Forward)
		{
			pruneIndicator = false;
		}
		int intId = outInt->getInterfaceId();
		sendPimStateRefresh(ift->getInterfaceById(intId)->ipv4Data()->getIPAddress(), timer->getSource(), timer->getGroup(), intId, pruneIndicator);
	}
	delete timer;
	route->setStateRefreshTimer(createStateRefreshTimer(route->getOrigin(), route->getMulticastGroup()));
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
void PIMDM::processPIMTimer(cMessage *timer)
{
	EV << "pimDM::processPIMTimer: ";

	switch(timer->getKind())
	{
	    case HelloTimer:
	        processHelloTimer(timer);
           break;
		case AssertTimer:
			EV << "AssertTimer" << endl;
			break;
		case PruneTimer:
			EV << "PruneTimer" << endl;
			processPruneTimer(check_and_cast<PIMpt *> (timer));
			break;
		case PrunePendingTimer:
			EV << "PrunePendingTimer" << endl;
			break;
		case GraftRetryTimer:
			EV << "GraftRetryTimer" << endl;
			processGraftRetryTimer(check_and_cast<PIMgrt *> (timer));
			break;
		case UpstreamOverrideTimer:
			EV << "UpstreamOverrideTimer" << endl;
			break;
		case PruneLimitTimer:
			EV << "PruneLimitTimer" << endl;
			break;
		case SourceActiveTimer:
			EV << "SourceActiveTimer" << endl;
			processSourceActiveTimer(check_and_cast<PIMsat *> (timer));
			break;
		case StateRefreshTimer:
			EV << "StateRefreshTimer" << endl;
			processStateRefreshTimer(check_and_cast<PIMsrt *> (timer));
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
 * @see processJoinPruneGraftPacket()
 */
void PIMDM::processPIMPkt(PIMPacket *pkt)
{
	EV << "pimDM::processPIMPkt: ";

	switch(pkt->getType())
	{
	    case Hello:
	        processHelloPacket(check_and_cast<PIMHello*>(pkt));
	        break;
		case JoinPrune:
			EV << "JoinPrune" << endl;
			processJoinPruneGraftPacket(check_and_cast<PIMJoinPrune *> (pkt), (PIMPacketType) pkt->getType());
			break;
		case Assert:
			EV << "Assert" << endl;
			// FIXME for future use
			break;
		case Graft:
			EV << "Graft" << endl;
			processJoinPruneGraftPacket(check_and_cast<PIMJoinPrune *> (pkt), (PIMPacketType) pkt->getType());
			break;
		case GraftAck:
			EV << "GraftAck" << endl;
			processJoinPruneGraftPacket(check_and_cast<PIMJoinPrune *> (pkt), (PIMPacketType) pkt->getType());
			break;
		case StateRefresh:
			EV << "StateRefresh" << endl;
			processStateRefreshPacket(check_and_cast<PIMStateRefresh *> (pkt));
			break;
		default:
			EV << "BAD TYPE, DROPPED" << endl;
			delete pkt;
			break;
	}
}


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
void PIMDM::handleMessage(cMessage *msg)
{
	// self message (timer)
   if (msg->isSelfMessage())
   {
	   EV << "PIMDM::handleMessage:Timer" << endl;
	   processPIMTimer(msg);
   }
   // PIM packet from PIM neighbor
   else if (dynamic_cast<PIMPacket *>(msg))
   {
	   EV << "PIMDM::handleMessage: PIM-DM packet" << endl;
	   PIMPacket *pkt = check_and_cast<PIMPacket *>(msg);
	   processPIMPkt(pkt);
   }
   // wrong message, mistake
   else
	   EV << "PIMDM::handleMessage: Wrong message" << endl;
}

/**
 * INITIALIZE
 *
 * The method initializes PIM-DM module. It get access to all needed tables and other objects.
 * It subscribes to important notifications. If there is no PIM interface, all module can be
 * disabled.
 *
 * @param stage Stage of initialization.
 */
void PIMDM::initialize(int stage)
{
    PIMBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        pruneInterval = par("pruneInterval");
        graftRetryInterval = par("graftRetryInterval");
        sourceActiveInterval = par("sourceActiveInterval");
        stateRefreshInterval = par("stateRefreshInterval");
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS)
	{
        // is PIM enabled?
        if (pimIft->getNumInterfaces() == 0)
        {
            EV << "PIM is NOT enabled on device " << endl;
        }
        else
        {
            // subscribe for notifications
            cModule *host = findContainingNode(this);
            if (host != NULL)
            {
                host->subscribe(NF_IPv4_NEW_MULTICAST, this);
                host->subscribe(NF_IPv4_MCAST_REGISTERED, this);
                host->subscribe(NF_IPv4_MCAST_UNREGISTERED, this);
                host->subscribe(NF_IPv4_DATA_ON_NONRPF, this);
                host->subscribe(NF_IPv4_DATA_ON_RPF, this);
                //host->subscribe(NF_IPv4_RPF_CHANGE, this);
                host->subscribe(NF_ROUTE_ADDED, this);
                host->subscribe(NF_INTERFACE_STATE_CHANGED, this);
            }
        }
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
void PIMDM::receiveSignal(cComponent *source, simsignal_t signalID, cObject *details)
{
	// ignore notifications during initialize
	if (simulation.getContextType()==CTX_INITIALIZE)
		return;

	// PIM needs addition info for each notification
	if (details == NULL)
		return;

	Enter_Method_Silent();
    printNotificationBanner(signalID, details);
	IPv4Datagram *datagram;
	PIMInterface *pimInterface;

    // new multicast data appears in router
    if (signalID == NF_IPv4_NEW_MULTICAST)
    {
        EV <<  "PimDM::receiveChangeNotification - NEW MULTICAST" << endl;
        datagram = check_and_cast<IPv4Datagram*>(details);
        IPv4Address srcAddr = datagram->getSrcAddress();
        IPv4Address destAddr = datagram->getDestAddress();
        newMulticast(srcAddr, destAddr);
    }
    // configuration of interface changed, it means some change from IGMP, address were added.
    else if (signalID == NF_IPv4_MCAST_REGISTERED)
    {
        EV << "pimDM::receiveChangeNotification - IGMP change - address were added." << endl;
        IPv4MulticastGroupInfo *info = check_and_cast<IPv4MulticastGroupInfo*>(details);
        pimInterface = pimIft->getInterfaceById(info->ie->getInterfaceId());
        if (pimInterface && pimInterface->getMode() == PIMInterface::DenseMode)
            newMulticastAddr(pimInterface, info->groupAddress);
    }
    // configuration of interface changed, it means some change from IGMP, address were removed.
    else if (signalID == NF_IPv4_MCAST_UNREGISTERED)
    {
        EV << "pimDM::receiveChangeNotification - IGMP change - address were removed." << endl;
        IPv4MulticastGroupInfo *info = check_and_cast<IPv4MulticastGroupInfo*>(details);
        pimInterface = pimIft->getInterfaceById(info->ie->getInterfaceId());
        if (pimInterface && pimInterface->getMode() == PIMInterface::DenseMode)
            oldMulticastAddr(pimInterface, info->groupAddress);
    }
    // data come to non-RPF interface
    else if (signalID == NF_IPv4_DATA_ON_NONRPF)
    {
        EV << "pimDM::receiveChangeNotification - Data appears on non-RPF interface." << endl;
        datagram = check_and_cast<IPv4Datagram*>(details);
        pimInterface = getIncomingInterface(datagram);
        dataOnNonRpf(datagram->getDestAddress(), datagram->getSrcAddress(), pimInterface? pimInterface->getInterfaceId():-1);
    }
    // data come to RPF interface
    else if (signalID == NF_IPv4_DATA_ON_RPF)
    {
        EV << "pimDM::receiveChangeNotification - Data appears on RPF interface." << endl;
        datagram = check_and_cast<IPv4Datagram*>(details);
        pimInterface = getIncomingInterface(datagram);
        if (pimInterface && pimInterface->getMode() == PIMInterface::DenseMode)
            dataOnRpf(datagram);
    }
    // RPF interface has changed
    else if (signalID == NF_ROUTE_ADDED)
    {
        EV << "pimDM::receiveChangeNotification - RPF interface has changed." << endl;
        IPv4Route *entry = check_and_cast<IPv4Route*>(details);
        vector<PIMMulticastRoute*> routes = getRoutesForSource(entry->getDestination());
        for (unsigned int i = 0; i < routes.size(); i++)
            rpfIntChange(routes[i]);
    }
}

/**
 * RPF INTERFACE CHANGE
 *
 * The method process notification about interface change. Multicast routing table will be
 * changed if RPF interface has changed. New RPF interface is set to route and is removed
 * from outgoing interfaces. On the other hand, old RPF interface is added to outgoing
 * interfaces. If route was not pruned, the router has to join to the multicast tree again
 * (by different path).
 *
 * @param newRoute Pointer to new entry in the multicast routing table.
 * @see sendPimGraft()
 * @see createGraftRetryTimer()
 */
void PIMDM::rpfIntChange(PIMMulticastRoute *route)
{
	IPv4Address source = route->getOrigin();
	IPv4Address group = route->getMulticastGroup();
	InterfaceEntry *newRpf = rt->getInterfaceForDestAddr(source);
	int rpfId = newRpf->getInterfaceId();
	PIMInInterface *inInterface = route->getPIMInInterface();

	// is there any change?
	if (rpfId == inInterface->getInterfaceId())
		return;
	EV << "New RPF int for group " << group << " source " << source << " is " << rpfId << endl;

	// set new RPF
	InterfaceEntry *oldRpfInt = route->getInInterface() ? route->getInInterface()->getInterface() : NULL;
	PIMInInterface *newInInterface = new PIMMulticastRoute::PIMInInterface(newRpf, rpfId, pimNbt->getNeighborsOnInterface(rpfId)[0]->getAddress());
	route->setInInterface(newInInterface);

	// route was not pruned, join to the multicast tree again
	if (!route->isFlagSet(PIMMulticastRoute::P))
	{
		sendPimGraft(newInInterface->nextHop, source, group, rpfId);
		PIMgrt* timer = createGraftRetryTimer(source, group);
		route->setGraftRetryTimer(timer);
	}

	// find rpf int in outgoing imterfaces and delete it
	for(unsigned int i = 0; i < route->getNumOutInterfaces(); i++)
	{
	    PIMOutInterface *outInt = route->getPIMOutInterface(i);
		if (outInt->getInterfaceId() == rpfId)
		{
			if (outInt->pruneTimer != NULL)
			{
				cancelEvent(outInt->pruneTimer);
				delete outInt->pruneTimer;
				outInt->pruneTimer = NULL;
			}
			route->removeOutInterface(i);
			break;
		}
	}

	// old RPF should be now outgoing interface if it is not down
	if (oldRpfInt && oldRpfInt->isUp())
	{
	    PIMMulticastRoute::PIMOutInterface *newOutInt = new PIMOutInterface(oldRpfInt);
		newOutInt->pruneTimer = NULL;
		newOutInt->forwarding = PIMMulticastRoute::Forward;
		newOutInt->mode = PIMInterface::DenseMode;
		route->addOutInterface(newOutInt);
	}
}


/**
 * DATA ON RPF INTERFACE
 *
 * The method process notification about data which appears on RPF interface. It means that source
 * is still active. The resault is resetting of Source Active Timer.
 *
 * @param newRoute Pointer to new entry in the multicast routing table.
 * @see PIMsat()
 */
void PIMDM::dataOnRpf(IPv4Datagram *datagram)
{
    PIMMulticastRoute *route = getRouteFor(datagram->getDestAddress(), datagram->getSrcAddress());
	cancelEvent(route->getSourceActiveTimer());
	scheduleAt(simTime() + sourceActiveInterval, route->getSourceActiveTimer());

    if (route->getNumOutInterfaces() == 0 || route->isFlagSet(PIMMulticastRoute::P))
    {
        EV << "Route does not have any outgoing interface or it is pruned." << endl;
        dataOnPruned(datagram->getDestAddress(), datagram->getSrcAddress());
    }
}

/**
 * DATA ON NON-RPF INTERFACE
 *
 * The method has to solve the problem when multicast data appears on non-RPF interface. It can
 * happen when there is loop in the network. In this case, router has to prune from the neighbor,
 * so it sends Prune message.
 *
 * @param group Multicast group IP address.
 * @param source Source IP address.
 * @param intID Identificator of incoming interface.
 * @see sendPimJoinPrune()
 * @see createPruneTimer()
 */
void PIMDM::dataOnNonRpf(IPv4Address group, IPv4Address source, int intId)
{
	EV << "pimDM::dataOnNonRpf, intID: " << intId << endl;

	// load route from mroute
	PIMMulticastRoute *route = getRouteFor(group, source);
	if (route == NULL)
		return;

	// in case of p2p link, send prune
	// FIXME There should be better indicator of P2P link
	if (pimNbt->getNumNeighborsOnInterface(intId) == 1)
	{
		// send Prune msg to the neighbor who sent these multicast data
		IPv4Address nextHop = (pimNbt->getNeighborsOnInterface(intId))[0]->getAddress();
		sendPimJoinPrune(nextHop, source, group, intId);

		// the incoming interface has to change its state to Pruned
		PIMOutInterface *outInt = route->findOutInterfaceByInterfaceId(intId);
		if (outInt && outInt->forwarding == PIMMulticastRoute::Forward)
		{
			outInt->forwarding = PIMMulticastRoute::Pruned;
			PIMpt* timer = createPruneTimer(route->getOrigin(), route->getMulticastGroup(), intId, pruneInterval);
			outInt->pruneTimer = timer;

			// if there is no outgoing interface, Prune msg has to be sent on upstream
			if (route->isOilistNull())
			{
				EV << "Route is not forwarding any more, send Prune to upstream" << endl;
				route->setFlags(PIMMulticastRoute::P);
				if (!route->isFlagSet(PIMMulticastRoute::A))
				{
				    PIMInInterface *inInterface = route->getPIMInInterface();
					sendPimJoinPrune(inInterface->nextHop, route->getOrigin(), route->getMulticastGroup(), inInterface->getInterfaceId());
				}
			}
		}
	}

	//FIXME in case of LAN
}

/**
 * DATA ON PRUNED
 *
 * The method has to solve the problem when multicast data appears on RPF interface and
 * route is pruned. In this case, new PIM JoinPrune has to be sent to upstream.
 *
 * @param group Multicast group IP address.
 * @param source Source IP address.
 * @see sendPimJoinPrune()
 */
void PIMDM::dataOnPruned(IPv4Address group, IPv4Address source)
{
	EV << "pimDM::dataOnPruned" << endl;
	PIMMulticastRoute *route = getRouteFor(group, source);
    if (route->isFlagSet(PIMMulticastRoute::A))
        return;

    // if GRT is running now, do not send Prune msg
	if (route->isFlagSet(PIMMulticastRoute::P) && (route->getGraftRetryTimer() != NULL))
	{
		cancelEvent(route->getGraftRetryTimer());
		delete route->getGraftRetryTimer();
		route->setGraftRetryTimer(NULL);
	}
	// otherwise send Prune msg to upstream router
	else
	{
	    PIMInInterface *inInterface = route->getPIMInInterface();
		sendPimJoinPrune(inInterface->nextHop, source, group, inInterface->getInterfaceId());
	}
}

/**
 * OLD MULTICAST ADDRESS
 *
 * The method process notification about multicast groups removed from interface. For each
 * old address it tries to find route. If there is route, it finds interface in list of outgoing
 * interfaces. If the interface is in the list it will be removed. If the router was not pruned
 * and there is no outgoing interface, the router will prune from the multicast tree.
 *
 * @param members Structure containing old multicast IP addresses.
 * @see sendPimJoinPrune()
 */
void PIMDM::oldMulticastAddr(PIMInterface *pimInt, IPv4Address oldAddr)
{
	EV << "pimDM::oldMulticastAddr" << endl;
	bool connected = false;

	// go through all old multicast addresses assigned to interface
    EV << "Removed multicast address: " << oldAddr << endl;
    vector<PIMMulticastRoute*> routes = getRouteFor(oldAddr);

    // go through all multicast routes
    for (unsigned int j = 0; j < routes.size(); j++)
    {
        PIMMulticastRoute *route = routes[j];
        unsigned int k;

        // is interface in list of outgoing interfaces?
        for (k = 0; k < route->getNumOutInterfaces();)
        {
            PIMOutInterface *outInt = route->getPIMOutInterface(k);
            if (outInt->getInterfaceId() == pimInt->getInterfaceId())
            {
                EV << "Interface is present, removing it from the list of outgoing interfaces." << endl;
                route->removeOutInterface(k);
            }
            else if(outInt->forwarding == PIMMulticastRoute::Forward)
            {
                if ((pimNbt->getNeighborsOnInterface(outInt->getInterfaceId())).size() == 0)
                    connected = true;
                k++;
            }
            else
                k++;
        }

        // if there is no directly connected member of group
        if (!connected)
            route->clearFlag(PIMMulticastRoute::C);

        // there is no receiver of multicast, prune the router from the multicast tree
        if (route->isOilistNull())
        {
            EV << "Route is not forwarding any more, send Prune to upstream" << endl;
            // if GRT is running now, do not send Prune msg
            if (route->isFlagSet(PIMMulticastRoute::P) && (route->getGraftRetryTimer() != NULL))
            {
                cancelEvent(route->getGraftRetryTimer());
                delete route->getGraftRetryTimer();
                route->setGraftRetryTimer(NULL);
                PIMInInterface *inInterface = route->getPIMInInterface();
                sendPimJoinPrune(inInterface->nextHop, route->getOrigin(), route->getMulticastGroup(), inInterface->getInterfaceId());
            }

            // if the source is not directly connected, sent Prune msg
            if (!route->isFlagSet(PIMMulticastRoute::A) && !route->isFlagSet(PIMMulticastRoute::P))
            {
                PIMInInterface *inInterface = route->getPIMInInterface();
                sendPimJoinPrune(inInterface->nextHop, route->getOrigin(), route->getMulticastGroup(), inInterface->getInterfaceId());
            }

            route->setFlags(PIMMulticastRoute::P);
        }
    }
}

/**
 * NEW MULTICAST ADDRESS
 *
 * The method process notification about new multicast groups aasigned to interface. For each
 * new address it tries to find route. If there is route, it finds interface in list of outgoing
 * interfaces. If the interface is not in the list it will be added. if the router was pruned
 * from multicast tree, join again.
 *
 * @param members Structure containing new multicast IP addresses.
 * @see sendPimGraft()
 * @see createGraftRetryTimer()
 * @see addRemoveAddr
 */
void PIMDM::newMulticastAddr(PIMInterface *pimInt, IPv4Address newAddr)
{
	EV << "pimDM::newMulticastAddr" << endl;
	bool forward = false;

    EV << "New multicast address: " << newAddr << endl;
    vector<PIMMulticastRoute*> routes = getRouteFor(newAddr);

    // go through all multicast routes
    for (unsigned int j = 0; j < routes.size(); j++)
    {
        PIMMulticastRoute *route = routes[j];
        //PIMMulticastRoute::AnsaOutInterfaceVector outInt = route->getOutInt();
        unsigned int k;

        // check on RPF interface
        PIMInInterface *rpfInterface = route->getPIMInInterface();
        if (rpfInterface->getInterface() == pimInt->getInterfacePtr())
            continue;

        // is interface in list of outgoing interfaces?
        for (k = 0; k < route->getNumOutInterfaces(); k++)
        {
            PIMOutInterface *outInt = route->getPIMOutInterface(k);
            if (outInt->getInterfaceId() == pimInt->getInterfaceId())
            {
                EV << "Interface is already on list of outgoing interfaces" << endl;
                if (outInt->forwarding == PIMMulticastRoute::Pruned)
                    outInt->forwarding = PIMMulticastRoute::Forward;
                forward = true;
                break;
            }
        }

        // interface is not in list of outgoing interfaces
        if (k == route->getNumOutInterfaces())
        {
            EV << "Interface is not on list of outgoing interfaces yet, it will be added" << endl;
            PIMMulticastRoute::PIMOutInterface *newInt = new PIMOutInterface(pimInt->getInterfacePtr());
            newInt->mode = PIMInterface::DenseMode;
            newInt->forwarding = PIMMulticastRoute::Forward;
            newInt->pruneTimer = NULL;
            route->addOutInterface(newInt);
            forward = true;
        }
        route->setFlags(PIMMulticastRoute::C);

        // route was pruned, has to be added to multicast tree
        if (route->isFlagSet(PIMMulticastRoute::P) && forward)
        {
            EV << "Route is not pruned any more, send Graft to upstream" << endl;

            // if source is not directly connected, send Graft to upstream
            if (!route->isFlagSet(PIMMulticastRoute::A))
            {
                PIMInInterface *inInterface = route->getPIMInInterface();
                sendPimGraft(inInterface->nextHop, route->getOrigin(), route->getMulticastGroup(), inInterface->getInterfaceId());
                PIMgrt *timer = createGraftRetryTimer(route->getOrigin(), route->getMulticastGroup());
                route->setGraftRetryTimer(timer);
            }
            else
                route->clearFlag(PIMMulticastRoute::P);
        }
    }
}

/**
 * NEW MULTICAST
 *
 * The method process notification about new multicast data stream. It goes through all PIM
 * interfaces and tests them if they can be added to the list of outgoing interfaces. If there
 * is no interface on the list at the end, the router will prune from the multicast tree.
 *
 * @param newRoute Pointer to new entry in the multicast routing table.
 * @see sendPimGraft()
 * @see createGraftRetryTimer()
 * @see addRemoveAddr
 */
void PIMDM::newMulticast(IPv4Address srcAddr, IPv4Address destAddr)
{
    ASSERT(!srcAddr.isUnspecified());
    ASSERT(destAddr.isMulticast());

    EV << "pimDM::newMulticast" << endl;

	IPv4Route *routeToSrc = rt->findBestMatchingRoute(srcAddr);
	if (!routeToSrc || !routeToSrc->getInterface())
	{
        EV << "ERROR: PIMDM::newMulticast(): cannot find RPF interface, routing information is missing.";
        return;
	}

    PIMInterface *rpfInterface = pimIft->getInterfaceById(routeToSrc->getInterface()->getInterfaceId());
    if (!rpfInterface || rpfInterface->getMode() != PIMInterface::DenseMode)
        return;

    EV << "PIMDM::newMulticast - group: " << destAddr << ", source: " << srcAddr << endl;

    // gateway is unspecified for directly connected destinations
    IPv4Address rpfNeighbor = routeToSrc->getGateway().isUnspecified() ? srcAddr : routeToSrc->getGateway();

    // create new multicast route
    PIMMulticastRoute *newRoute = new PIMMulticastRoute(srcAddr, destAddr);
    newRoute->setInInterface(new PIMMulticastRoute::PIMInInterface(rpfInterface->getInterfacePtr(), rpfInterface->getInterfaceId(), rpfNeighbor));
    if (routeToSrc->getSourceType() == IPv4Route::IFACENETMASK)
        newRoute->setFlags(PIMMulticastRoute::A);


	// only outgoing interfaces are missing
	bool pruned = true;

	// insert all PIM interfaces except rpf int
	for (int i = 0; i < pimIft->getNumInterfaces(); i++)
	{
		PIMInterface *pimIntTemp = pimIft->getInterface(i);
		int intId = pimIntTemp->getInterfaceId();

		//check if PIM interface is not RPF interface
		if (pimIntTemp == rpfInterface)
			continue;

		// create new outgoing interface
		PIMMulticastRoute::PIMOutInterface *newOutInt = new PIMOutInterface(pimIntTemp->getInterfacePtr());
		newOutInt->pruneTimer = NULL;

		switch (pimIntTemp->getMode())
		{
			case PIMInterface::DenseMode:
				newOutInt->mode = PIMInterface::DenseMode;
				break;
			case PIMInterface::SparseMode:
				newOutInt->mode = PIMInterface::SparseMode;
				break;
		}

		// if there are neighbors on interface, we will forward
		if((pimNbt->getNeighborsOnInterface(intId)).size() > 0)
		{
			newOutInt->forwarding = PIMMulticastRoute::Forward;
			pruned = false;
			newRoute->addOutInterface(newOutInt);
		}
		// if there is member of group, we will forward
		else if (pimIntTemp->getInterfacePtr()->ipv4Data()->hasMulticastListener(newRoute->getMulticastGroup()))
		{
			newOutInt->forwarding = PIMMulticastRoute::Forward;
			pruned = false;
			newRoute->setFlags(PIMMulticastRoute::C);
			newRoute->addOutInterface(newOutInt);
		}
		// in any other case interface is not involved
	}

	// directly connected to source, set State Refresh Timer
	if (newRoute->isFlagSet(PIMMulticastRoute::A) && rpfInterface->getSR())
	{
	    PIMsrt* timerSrt = createStateRefreshTimer(newRoute->getOrigin(), newRoute->getMulticastGroup());
	    newRoute->setStateRefreshTimer(timerSrt);
	}

	// set Source Active Timer (liveness of route)
	PIMsat* timerSat = createSourceActiveTimer(newRoute->getOrigin(), newRoute->getMulticastGroup());
    newRoute->setSourceActiveTimer(timerSat);

	// if there is no outgoing interface, prune from multicast tree
	if (pruned)
	{
		EV << "pimDM::newMulticast: There is no outgoing interface for multicast, send Prune msg to upstream" << endl;
		newRoute->setFlags(PIMMulticastRoute::P);

		// Prune message is sent from the forwarding hook (NF_IPv4_DATA_ON_RPF), see dataOnRpf()
	}

	// add new route record to multicast routing table
	rt->addMulticastRoute(newRoute);
	EV << "PimSplitter::newMulticast: New route was added to the multicast routing table." << endl;
}

PIMInterface *PIMDM::getIncomingInterface(IPv4Datagram *datagram)
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
