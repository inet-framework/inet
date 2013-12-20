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
typedef PIMMulticastRoute::PIMOutInterface PIMOutInterface;

void PIMDM::sendPrunePacket(IPv4Address nextHop, IPv4Address src, IPv4Address grp, int intId)
{
    ASSERT(!src.isUnspecified());
    ASSERT(grp.isMulticast());

	EV_INFO << "Sending Prune(source = " << src << ", group = " << grp << ") message to neighbor '" << nextHop << "' on interface '" << intId << "'\n";

	PIMJoinPrune *packet = new PIMJoinPrune("PIMJoinPrune");
	packet->setUpstreamNeighborAddress(nextHop);
	packet->setHoldTime(pruneInterval);

	// set multicast groups
    packet->setMulticastGroupsArraySize(1);
	MulticastGroup &group = packet->getMulticastGroups(0);
	group.setGroupAddress(grp);
	group.setPrunedSourceAddressArraySize(1);
    EncodedAddress &address = group.getPrunedSourceAddress(0);
	address.IPaddress = src;

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
    EV_INFO << "Sending Graft(source = " << src << ", group = " << grp << ") message to neighbor '" << nextHop << "' on interface '" << intId << "'\n";

	PIMGraft *msg = new PIMGraft("PIMGraft");
	msg->setHoldTime(0);
	msg->setUpstreamNeighborAddress(nextHop);

    msg->setMulticastGroupsArraySize(1);
	MulticastGroup &group = msg->getMulticastGroups(0);
	group.setGroupAddress(grp);
	group.setJoinedSourceAddressArraySize(1);
    EncodedAddress &address = group.getJoinedSourceAddress(0);
    address.IPaddress = src;

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

    IPv4ControlInfo *oldCtrl = check_and_cast<IPv4ControlInfo*>(graftPacket->removeControlInfo());
    IPv4Address destAddr = oldCtrl->getSrcAddr();
    IPv4Address srcAddr = oldCtrl->getDestAddr();
    int outInterfaceId = oldCtrl->getInterfaceId();
    delete oldCtrl;

    PIMGraftAck *msg = new PIMGraftAck();
    *((PIMGraft*)msg) = *graftPacket;
    msg->setName("PIMGraftAck");
    msg->setType(GraftAck);

    sendToIP(msg, srcAddr, destAddr, outInterfaceId);
}

void PIMDM::sendStateRefreshPacket(IPv4Address originator, IPv4Address src, IPv4Address grp, int intId, bool P)
{
    EV_INFO << "Sending StateRefresh(source = " << src << ", group = " << grp << ") message on interface '" << intId << "'\n";

	PIMStateRefresh *msg = new PIMStateRefresh("PIMStateRefresh");
	msg->setGroupAddress(grp);
	msg->setSourceAddress(src);
	msg->setOriginatorAddress(originator);
	msg->setInterval(stateRefreshInterval);
	msg->setP(P);

	sendToIP(msg, IPv4Address::UNSPECIFIED_ADDRESS, grp, intId);
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

/**
 * The method is used to process PIMGraft packet. Packet means that downstream router wants to join to
 * multicast tree, so the packet cannot come to RPF interface. Router finds correct outgoig interface
 * towards downstream router. Change its state to forward if it was not before and cancel Prune Timer.
 * If route was in pruned state, router will send also Graft message to join multicast tree.
 */
void PIMDM::processGraft(IPv4Address source, IPv4Address group, IPv4Address sender, int incomingInterfaceId)
{
	EV_DEBUG << "Processing Graft, source=" << source << ", group=" << group << ", sender=" << sender << "incoming if=" << incomingInterfaceId << endl;

	SourceGroupState *sgState = getSourceGroupState(source, group);

	UpstreamInterface *upstream = sgState->upstreamInterface;

	// check if message come to non-RPF interface
	if (upstream->ie->getInterfaceId() == incomingInterfaceId)
	{
		EV << "ERROR: Graft message came to RPF interface." << endl;
		return;
	}

    DownstreamInterface *downstream = sgState->findDownstreamInterfaceByInterfaceId(incomingInterfaceId);
    if (!downstream)
        return;

    // downstream state machine
    // Note: GraftAck is sent in processGraftPacket()
    switch (downstream->pruneState)
    {
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
            break;
    }

    // if all route was pruned, remove prune flag
    // if upstrem is not source, send Graft message
    if (sgState->isFlagSet(PIMMulticastRoute::P) && !upstream->graftRetryTimer)
    {
        if (!sgState->isFlagSet(PIMMulticastRoute::A))
        {
            EV << "Route is not pruned any more, send Graft to upstream" << endl;
            UpstreamInterface *inInterface = sgState->upstreamInterface;
            sendGraftPacket(inInterface->nextHop, source, group, inInterface->getInterfaceId());
            inInterface->startGraftRetryTimer();
        }
        else
            sgState->clearFlag(PIMMulticastRoute::P);
    }
}

/**
 * The method process PIM Prune packet. First the method has to find correct outgoing interface
 * where PIM Prune packet came to. The method also checks if there is still any forwarding outgoing
 * interface. Forwarding interfaces, where Prune packet come to, goes to prune state. If all outgoing
 * interfaces are pruned, the router will prune from multicast tree.
 */
void PIMDM::processPrune(SourceGroupState *sgState, int intId, int holdTime, int numRpfNeighbors)
{
	EV_INFO << "Processing Prune " << endl;

	// we find correct outgoing interface
    DownstreamInterface *downstream = sgState->findDownstreamInterfaceByInterfaceId(intId);
    if (!downstream)
        return;

    // Downstream state machine
    switch (downstream->pruneState)
    {
        case DownstreamInterface::PRUNED:
            EV << "Outgoing interface is already pruned, restart Prune Timer." << endl;
            restartTimer(downstream->pruneTimer, holdTime);
            break;
        case DownstreamInterface::PRUNE_PENDING:
            // do nothing
            break;
        case DownstreamInterface::NO_INFO:

            // if there could be more than one PIM neighbor on interface
            if (numRpfNeighbors > 1)
            {
                //FIXME set PPT timer
            }
            // if there is only one PIM neighbor on interface
            else
            {
                EV << "Outgoing interfaces is forwarding now -> change to Pruned." << endl;
                downstream->pruneState = DownstreamInterface::PRUNED;
                downstream->startPruneTimer(holdTime);

                // if there is no forwarding outgoing int, transit route to pruned state
                if (sgState->isOilistNull())
                {
                    EV << "Route is not forwarding any more, send Prune to upstream." << endl;
                    sgState->setFlags(PIMMulticastRoute::P);

                    // if GRT is running now, do not send Prune msg
                    UpstreamInterface *upstream = sgState->upstreamInterface;
                    if (sgState->isFlagSet(PIMMulticastRoute::P) && upstream->graftRetryTimer)
                    {
                        cancelAndDelete(upstream->graftRetryTimer);
                        upstream->graftRetryTimer = NULL;
                    }
                    else if (!sgState->isFlagSet(PIMMulticastRoute::A))
                    {
                        UpstreamInterface *upstream = sgState->upstreamInterface;
                        sendPrunePacket(upstream->nextHop, sgState->source, sgState->group, upstream->getInterfaceId());
                    }
                }
            }
            break;
    }
}

void PIMDM::processJoin(SourceGroupState *sgState, int intId, int numRpfNeighbors)
{
    //FIXME join action
    // only if there is more than one PIM neighbor on one interface
    // interface change to forwarding state
    // cancel Prune Timer
    // send Graft to upstream
}


void PIMDM::processJoinPrunePacket(PIMJoinPrune *pkt)
{
    EV_INFO << "Received JoinPrune packet.\n";

    IPv4ControlInfo *ctrlInfo = check_and_cast<IPv4ControlInfo*>(pkt->getControlInfo());
    IPv4Address sender = ctrlInfo->getSrcAddr();
    InterfaceEntry *rpfInterface = rt->getInterfaceForDestAddr(sender);

    // does packet belong to this router?
    if (!rpfInterface || pkt->getUpstreamNeighborAddress() != rpfInterface->ipv4Data()->getIPAddress())
    {
        delete pkt;
        return;
    }

    int numRpfNeighbors = pimNbt->getNumNeighborsOnInterface(rpfInterface->getInterfaceId());

    for (unsigned int i = 0; i < pkt->getMulticastGroupsArraySize(); i++)
    {
        MulticastGroup group = pkt->getMulticastGroups(i);
        IPv4Address groupAddr = group.getGroupAddress();

        // go through list of joined sources
        for (unsigned int j = 0; j < group.getJoinedSourceAddressArraySize(); j++)
        {
            EncodedAddress &source = group.getJoinedSourceAddress(j);
            SourceGroupState *sgState = getSourceGroupState(source.IPaddress, groupAddr);
            processJoin(sgState, rpfInterface->getInterfaceId(), numRpfNeighbors);
        }

        // go through list of pruned sources
        for(unsigned int j = 0; j < group.getPrunedSourceAddressArraySize(); j++)
        {
            EncodedAddress &source = group.getPrunedSourceAddress(j);
            SourceGroupState *sgState = getSourceGroupState(source.IPaddress, groupAddr);
            processPrune(sgState, rpfInterface->getInterfaceId(), pkt->getHoldTime(), numRpfNeighbors);
        }
    }

    delete pkt;
}

void PIMDM::processGraftPacket(PIMGraft *pkt)
{
    EV_INFO << "Received Graft packet.\n";

    IPv4ControlInfo *ctrl = check_and_cast<IPv4ControlInfo*>(pkt->getControlInfo());
    IPv4Address sender = ctrl->getSrcAddr();
    InterfaceEntry * rpfInterface = rt->getInterfaceForDestAddr(sender);

    // does packet belong to this router?
    if (pkt->getUpstreamNeighborAddress() != rpfInterface->ipv4Data()->getIPAddress())
    {
        delete pkt;
        return;
    }

    for (unsigned int i = 0; i < pkt->getMulticastGroupsArraySize(); i++)
    {
        MulticastGroup &group = pkt->getMulticastGroups(i);
        IPv4Address groupAddr = group.getGroupAddress();

        for (unsigned int j = 0; j < group.getJoinedSourceAddressArraySize(); j++)
        {
            EncodedAddress &source = group.getJoinedSourceAddress(j);
            processGraft(source.IPaddress, groupAddr, sender, rpfInterface->getInterfaceId());
        }
    }

    // Send GraftAck for this Graft message
    sendGraftAckPacket(pkt);

    delete pkt;
}

void PIMDM::processGraftAckPacket(PIMGraftAck *pkt)
{
    EV_INFO << "Received GraftAck packet.\n";

    for (unsigned int i = 0; i < pkt->getMulticastGroupsArraySize(); i++)
    {
        MulticastGroup &group = pkt->getMulticastGroups(i);
        IPv4Address groupAddr = group.getGroupAddress();

        for (unsigned int j = 0; j < group.getJoinedSourceAddressArraySize(); j++)
        {
            EncodedAddress &source = group.getJoinedSourceAddress(j);
            SourceGroupState *sgState = getSourceGroupState(source.IPaddress, groupAddr);

            UpstreamInterface *upstream = sgState->upstreamInterface;
            if (upstream->graftRetryTimer)
            {
                cancelAndDelete(upstream->graftRetryTimer);
                upstream->graftRetryTimer = NULL;
                sgState->clearFlag(PIMMulticastRoute::P);
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

	// FIXME actions of upstream automat according to pruned/forwarding state and Prune Indicator from msg

	// first check if there is route for given group address and source
	SourceGroupState *sgState = getSourceGroupState(pkt->getSourceAddress(), pkt->getGroupAddress());
	if (sgState == NULL)
	{
		delete pkt;
		return;
	}
	bool pruneIndicator;

	// chceck if State Refresh msg has came to RPF interface
	IPv4ControlInfo *ctrl = check_and_cast<IPv4ControlInfo*>(pkt->getControlInfo());
	UpstreamInterface *inInterface = sgState->upstreamInterface;
	if (ctrl->getInterfaceId() != inInterface->getInterfaceId())
	{
		delete pkt;
		return;
	}

	// this router is pruned, but outgoing int of upstream router leading to this router is forwarding
	if (sgState->isFlagSet(PIMMulticastRoute::P) && !pkt->getP())
	{
		// send Prune msg to upstream
		if (!inInterface->graftRetryTimer)
		{
		    UpstreamInterface *inInterface = sgState->upstreamInterface;
			sendPrunePacket(inInterface->nextHop, sgState->source, sgState->group, inInterface->getInterfaceId());
		}
		else
		{
			cancelEvent(inInterface->graftRetryTimer);
			delete inInterface->graftRetryTimer;
			inInterface->graftRetryTimer = NULL;
			///////delete P
		}
	}

	// go through all outgoing interfaces, reser Prune Timer and send out State Refresh msg
	for (unsigned int i = 0; i < sgState->downstreamInterfaces.size(); i++)
	{
	    DownstreamInterface *outInt = sgState->downstreamInterfaces[i];
		if (outInt->pruneState == DownstreamInterface::PRUNED)
		{
			// P = true
			pruneIndicator = true;
			// reset PT
			restartTimer(outInt->pruneTimer, pruneInterval);
		}
		else if (outInt->pruneState == DownstreamInterface::NO_INFO)
		{
			// P = false
			pruneIndicator = false;
		}
		sendStateRefreshPacket(pkt->getOriginatorAddress(), pkt->getSourceAddress(), pkt->getGroupAddress(), outInt->ie->getInterfaceId(), pruneIndicator);
	}
	delete pkt;
}

void PIMDM::processAssertPacket(PIMAssert *pkt)
{
    EV_INFO << "Received Assert packet.\n";

    ASSERT(false); // not yet implemented

    delete pkt;
}

/*
 * The method is used to process PIM Prune timer. It is (S,G,I) timer. When Prune timer expires, it
 * means that outgoing interface transits back to forwarding state. If the router is pruned from
 * multicast tree, join again.
 */
void PIMDM::processPruneTimer(cMessage *timer)
{
	EV_INFO << "PruneTimer expired.\n";

	DownstreamInterface *downstream = static_cast<DownstreamInterface*>(timer->getContextPointer());
    ASSERT(timer == downstream->pruneTimer);

    SourceGroupState *sgState = downstream->owner;
	IPv4Address source = sgState->source;
	IPv4Address group = sgState->group;

	// state of interface is changed to forwarding
    downstream->stopPruneTimer();
    downstream->pruneState = DownstreamInterface::NO_INFO;
    sgState->clearFlag(PIMMulticastRoute::P);

    // if the router is pruned from multicast tree, join again
    /*if (sgState->isFlagSet(P) && (sgState->getGrt() == NULL))
    {
        if (!sgState->isFlagSet(A))
        {
            EV << "Pruned cesta prejde do forwardu, posli Graft" << endl;
            sendPimGraft(sgState->getInIntNextHop(), source, group, sgState->getInIntId());
            PIMgrt* timer = createGraftRetryTimer(source, group);
            sgState->setGrt(timer);
        }
        else
            sgState->removeFlag(P);
    }*/
}

// See RFC 3973 4.4.2.2
void PIMDM::processPrunePendingTimer(cMessage *timer)
{
    DownstreamInterface *downstream = static_cast<DownstreamInterface*>(timer->getContextPointer());
    ASSERT(timer == downstream->pruneTimer);
    ASSERT(downstream->pruneState == DownstreamInterface::PRUNE_PENDING);

    SourceGroupState *sgState = downstream->owner;
    IPv4Address source = sgState->source;
    IPv4Address group = sgState->group;

    EV_INFO << "PrunePendingTimer of (source=" << source << ", group=" << group << ") has expired.\n";

    // go to pruned state
    downstream->pruneState = DownstreamInterface::PRUNED;
    double holdTime = pruneInterval - overrideInterval; // XXX should be received HoldTime - computed override interval;
    downstream->startPruneTimer(holdTime);

    // TODO optionally send PruneEcho
}

void PIMDM::processGraftRetryTimer(cMessage *timer)
{
    EV_INFO << "GraftRetryTimer expired.\n";

    // send Graft message to upstream router
    UpstreamInterface *upstream = static_cast<UpstreamInterface*>(timer->getContextPointer());
    IPv4Address source = upstream->owner->source;
    IPv4Address group = upstream->owner->group;
	sendGraftPacket(upstream->nextHop, source, group, upstream->getInterfaceId());
    scheduleAt(simTime() + graftRetryInterval, timer);
}

void PIMDM::processSourceActiveTimer(cMessage * timer)
{
    EV_INFO << "SourceActiveTimer expired.\n";

    // delete the route, because there are no more packets
	UpstreamInterface *upstream = static_cast<UpstreamInterface*>(timer->getContextPointer());
	IPv4Address source = upstream->owner->source;
	IPv4Address group = upstream->owner->group;
	deleteSourceGroupState(source, group);
	PIMMulticastRoute *route = getRouteFor(group, source);
	if (route)
	    rt->deleteMulticastRoute(route);
}

/*
 * State Refresh Timer is used only on router which is connected directly to the source of multicast.
 * When State Refresh Timer expires, State Refresh messages are sent on all downstream interfaces.
 */
void PIMDM::processStateRefreshTimer(cMessage *timer)
{
	EV_INFO << "StateRefreshTimer expired, sending StateRefresh packets on downstream interfaces.\n";

	UpstreamInterface *upstream = static_cast<UpstreamInterface*>(timer->getContextPointer());
	SourceGroupState *sgState = upstream->owner;
	for (unsigned int i = 0; i < sgState->downstreamInterfaces.size(); i++)
	{
	    DownstreamInterface *downstream = sgState->downstreamInterfaces[i];
	    bool isPruned = downstream->pruneState == DownstreamInterface::PRUNED;
	    if (isPruned)
			restartTimer(downstream->pruneTimer, pruneInterval);

	    IPv4Address originator = downstream->ie->ipv4Data()->getIPAddress();
		sendStateRefreshPacket(originator, sgState->source, sgState->group, downstream->ie->getInterfaceId(), isPruned);
	}

    scheduleAt(simTime() + stateRefreshInterval, timer);
}

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
			processPruneTimer(timer);
			break;
		case PrunePendingTimer:
			EV << "PrunePendingTimer" << endl;
			processPrunePendingTimer(timer);
			break;
		case GraftRetryTimer:
			EV << "GraftRetryTimer" << endl;
			processGraftRetryTimer(timer);
			break;
		case UpstreamOverrideTimer:
			EV << "UpstreamOverrideTimer" << endl;
			break;
		case PruneLimitTimer:
			EV << "PruneLimitTimer" << endl;
			break;
		case SourceActiveTimer:
			EV << "SourceActiveTimer" << endl;
			processSourceActiveTimer(timer);
			break;
		case StateRefreshTimer:
			EV << "StateRefreshTimer" << endl;
			processStateRefreshTimer(timer);
			break;
		default:
			EV << "BAD TYPE, DROPPED" << endl;
			delete timer;
			break;
	}
}

void PIMDM::processPIMPacket(PIMPacket *pkt)
{
	switch(pkt->getType())
	{
	    case Hello:
	        processHelloPacket(check_and_cast<PIMHello*>(pkt));
	        break;
		case JoinPrune:
			processJoinPrunePacket(check_and_cast<PIMJoinPrune*>(pkt));
			break;
		case Assert:
			processAssertPacket(check_and_cast<PIMAssert*>(pkt));
			break;
		case Graft:
			processGraftPacket(check_and_cast<PIMGraft*>(pkt));
			break;
		case GraftAck:
			processGraftAckPacket(check_and_cast<PIMGraftAck*>(pkt));
			break;
		case StateRefresh:
			processStateRefreshPacket(check_and_cast<PIMStateRefresh *> (pkt));
			break;
		default:
			EV_WARN << "Dropping packet " << pkt->getName() << ".\n";
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
	   processPIMPacket(pkt);
   }
   // wrong message, mistake
   else
	   EV << "PIMDM::handleMessage: Wrong message" << endl;
}

void PIMDM::initialize(int stage)
{
    PIMBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        pruneInterval = par("pruneInterval");
        pruneLimitInterval = par("pruneLimitInterval");
        overrideInterval = par("overrideInterval");
        graftRetryInterval = par("graftRetryInterval");
        sourceActiveInterval = par("sourceActiveInterval");
        stateRefreshInterval = par("stateRefreshInterval");
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS)
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

void PIMDM::receiveSignal(cComponent *source, simsignal_t signalID, cObject *details)
{
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
        unroutableMulticastPacketArrived(srcAddr, destAddr);
    }
    // configuration of interface changed, it means some change from IGMP, address were added.
    else if (signalID == NF_IPv4_MCAST_REGISTERED)
    {
        EV << "pimDM::receiveChangeNotification - IGMP change - address were added." << endl;
        IPv4MulticastGroupInfo *info = check_and_cast<IPv4MulticastGroupInfo*>(details);
        pimInterface = pimIft->getInterfaceById(info->ie->getInterfaceId());
        if (pimInterface && pimInterface->getMode() == PIMInterface::DenseMode)
            multicastReceiverAdded(pimInterface, info->groupAddress);
    }
    // configuration of interface changed, it means some change from IGMP, address were removed.
    else if (signalID == NF_IPv4_MCAST_UNREGISTERED)
    {
        EV << "pimDM::receiveChangeNotification - IGMP change - address were removed." << endl;
        IPv4MulticastGroupInfo *info = check_and_cast<IPv4MulticastGroupInfo*>(details);
        pimInterface = pimIft->getInterfaceById(info->ie->getInterfaceId());
        if (pimInterface && pimInterface->getMode() == PIMInterface::DenseMode)
            multicastReceiverRemoved(pimInterface, info->groupAddress);
    }
    // data come to non-RPF interface
    else if (signalID == NF_IPv4_DATA_ON_NONRPF)
    {
        EV << "pimDM::receiveChangeNotification - Data appears on non-RPF interface." << endl;
        datagram = check_and_cast<IPv4Datagram*>(details);
        pimInterface = getIncomingInterface(datagram);
        multicastPacketArrivedOnNonRpfInterface(datagram->getDestAddress(), datagram->getSrcAddress(), pimInterface? pimInterface->getInterfaceId():-1);
    }
    // data come to RPF interface
    else if (signalID == NF_IPv4_DATA_ON_RPF)
    {
        EV << "pimDM::receiveChangeNotification - Data appears on RPF interface." << endl;
        datagram = check_and_cast<IPv4Datagram*>(details);
        pimInterface = getIncomingInterface(datagram);
        if (pimInterface && pimInterface->getMode() == PIMInterface::DenseMode)
            multicastPacketArrivedOnRpfInterface(datagram->getDestAddress(), datagram->getSrcAddress(), pimInterface ? pimInterface->getInterfaceId():-1);
    }
    // RPF interface has changed
    else if (signalID == NF_ROUTE_ADDED)
    {
        EV << "pimDM::receiveChangeNotification - RPF interface has changed." << endl;
        IPv4Route *entry = check_and_cast<IPv4Route*>(details);
        IPv4Address routeSource = entry->getDestination();
        IPv4Address routeNetmask = entry->getNetmask();

        int numRoutes = rt->getNumMulticastRoutes();
        for (int i = 0; i < numRoutes; i++)
        {
            // find multicast routes whose source are on the destination of the new unicast route
            PIMMulticastRoute *route = dynamic_cast<PIMMulticastRoute*>(rt->getMulticastRoute(i));
            if (route && route->getSource() == this && IPv4Address::maskedAddrAreEqual(route->getOrigin(), routeSource, routeNetmask))
            {
                IPv4Address source = route->getOrigin();
                InterfaceEntry *newRpfInterface = rt->getInterfaceForDestAddr(source);
                InterfaceEntry *oldRpfInterface = route->getInInterface()->getInterface();

                // is there any change?
                if (newRpfInterface != oldRpfInterface)
                    rpfInterfaceHasChanged(route, newRpfInterface);
            }
        }
    }
}

/**
 * The method process notification about new multicast data stream. It goes through all PIM
 * interfaces and tests them if they can be added to the list of outgoing interfaces. If there
 * is no interface on the list at the end, the router will prune from the multicast tree.
 */
void PIMDM::unroutableMulticastPacketArrived(IPv4Address source, IPv4Address group)
{
    ASSERT(!source.isUnspecified());
    ASSERT(group.isMulticast());

    EV_DETAIL << "New multicast source observed: source=" << source << ", group=" << group << ".\n";

    IPv4Route *routeToSrc = rt->findBestMatchingRoute(source);
    if (!routeToSrc || !routeToSrc->getInterface())
    {
        EV << "ERROR: PIMDM::newMulticast(): cannot find RPF interface, routing information is missing.";
        return;
    }

    PIMInterface *rpfInterface = pimIft->getInterfaceById(routeToSrc->getInterface()->getInterfaceId());
    if (!rpfInterface || rpfInterface->getMode() != PIMInterface::DenseMode)
        return;

    // gateway is unspecified for directly connected destinations
    IPv4Address rpfNeighbor = routeToSrc->getGateway().isUnspecified() ? source : routeToSrc->getGateway();

    SourceGroupState *sgState = &sgStates[SourceAndGroup(source, group)];
    sgState->owner = this;
    sgState->source = source;
    sgState->group = group;
    sgState->upstreamInterface = new UpstreamInterface(sgState, rpfInterface->getInterfacePtr(), rpfNeighbor);

    if (routeToSrc->getSourceType() == IPv4Route::IFACENETMASK)
        sgState->setFlags(PIMMulticastRoute::A);


    bool allDownstreamInterfacesArePruned = true;

    // insert all PIM interfaces except rpf int
    for (int i = 0; i < pimIft->getNumInterfaces(); i++)
    {
        PIMInterface *pimInterface = pimIft->getInterface(i);

        //check if PIM-DM interface and it is not RPF interface
        if (pimInterface == rpfInterface || pimInterface->getMode() != PIMInterface::DenseMode) // XXX original code added downstream if data for PIM-SM interfaces too
            continue;

        bool hasPIMNeighbors = pimNbt->getNumNeighborsOnInterface(pimInterface->getInterfaceId()) > 0;
        bool hasConnectedReceivers = pimInterface->getInterfacePtr()->ipv4Data()->hasMulticastListener(group);

        // if there are neighbors on interface, we will forward
        if(hasPIMNeighbors || hasConnectedReceivers)
        {
            // create new outgoing interface
            sgState->createDownstreamInterface(pimInterface->getInterfacePtr());
            allDownstreamInterfacesArePruned = false;
        }

        if (hasConnectedReceivers)
            sgState->setFlags(PIMMulticastRoute::C);
    }

    // directly connected to source, set State Refresh Timer
    if (sgState->isFlagSet(PIMMulticastRoute::A) && rpfInterface->getSR())
    {
        sgState->upstreamInterface->startStateRefreshTimer();
    }

    // XXX sourceActiveTimer should be created only in routers directly connected to the source
    // set Source Active Timer (liveness of route)
    sgState->upstreamInterface->startSourceActiveTimer();

    // if there is no outgoing interface, prune from multicast tree
    if (allDownstreamInterfacesArePruned)
    {
        EV_DETAIL << "There is no outgoing interface for multicast, will send Prune message to upstream.\n";
        sgState->setFlags(PIMMulticastRoute::P);

        // Prune message is sent from the forwarding hook (NF_IPv4_DATA_ON_RPF), see multicastPacketArrivedOnRpfInterface()
    }

    // create new multicast route
    PIMMulticastRoute *newRoute = new PIMMulticastRoute(source, group);
    newRoute->setSourceType(IMulticastRoute::PIM_DM);
    newRoute->setSource(this);
    newRoute->setInInterface(new IMulticastRoute::InInterface(sgState->upstreamInterface->ie));
    for (unsigned int i = 0; i < sgState->downstreamInterfaces.size(); ++i)
    {
        DownstreamInterface *downstream = sgState->downstreamInterfaces[i];
        newRoute->addOutInterface(new PIMDMOutInterface(downstream->ie, downstream));
    }

    rt->addMulticastRoute(newRoute);
    EV_DETAIL << "New route was added to the multicast routing table.\n";
}

void PIMDM::multicastPacketArrivedOnRpfInterface(IPv4Address group, IPv4Address source, int interfaceId)
{
    EV_DETAIL << "Multicast datagram arrived: source=" << source << ", group=" << group << ".\n";

    SourceGroupState *sgState = getSourceGroupState(source, group);
    ASSERT(sgState);
    UpstreamInterface *upstream = sgState->upstreamInterface;
	restartTimer(upstream->sourceActiveTimer, sourceActiveInterval);

    if (sgState->downstreamInterfaces.size() == 0 || sgState->isFlagSet(PIMMulticastRoute::P))
    {
        EV << "Route does not have any outgoing interface or it is pruned.\n";

        if (sgState->isFlagSet(PIMMulticastRoute::A))
            return;

        // if GRT is running now, do not send Prune msg
        if (sgState->isFlagSet(PIMMulticastRoute::P) && upstream->graftRetryTimer)
        {
            cancelAndDelete(upstream->graftRetryTimer);
            upstream->graftRetryTimer = NULL;
        }
        // otherwise send Prune msg to upstream router
        else
        {
            sendPrunePacket(upstream->nextHop, source, group, upstream->getInterfaceId());
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
	EV_DETAIL << "Received multicast datagram (source="<< source << ", group=" << group <<") on non-RPF interface: " << interfaceId << ".\n";

	SourceGroupState *sgState = getSourceGroupState(source, group);
	ASSERT(sgState);

	// in case of p2p link, send prune
	// FIXME There should be better indicator of P2P link
	if (pimNbt->getNumNeighborsOnInterface(interfaceId) == 1)
	{
		// send Prune msg to the neighbor who sent these multicast data
		IPv4Address nextHop = (pimNbt->getNeighborsOnInterface(interfaceId))[0]->getAddress();
		sendPrunePacket(nextHop, source, group, interfaceId);

		// the incoming interface has to change its state to Pruned
		DownstreamInterface *outInt = sgState->findDownstreamInterfaceByInterfaceId(interfaceId);
		if (outInt && outInt->pruneState == DownstreamInterface::NO_INFO)
		{
			outInt->pruneState = DownstreamInterface::PRUNED;
			outInt->startPruneTimer(pruneInterval);

			// if there is no outgoing interface, Prune msg has to be sent on upstream
			if (sgState->isOilistNull())
			{
				EV << "Route is not forwarding any more, send Prune to upstream" << endl;
				sgState->setFlags(PIMMulticastRoute::P);
				if (!sgState->isFlagSet(PIMMulticastRoute::A))
				{
				    UpstreamInterface *inInterface = sgState->upstreamInterface;
					sendPrunePacket(inInterface->nextHop, source, group, inInterface->getInterfaceId());
				}
			}
		}
	}

	//FIXME in case of LAN
}

/*
 * The method process notification about new multicast groups aasigned to interface. For each
 * new address it tries to find route. If there is route, it finds interface in list of outgoing
 * interfaces. If the interface is not in the list it will be added. if the router was pruned
 * from multicast tree, join again.
 */
void PIMDM::multicastReceiverAdded(PIMInterface *pimInterface, IPv4Address group)
{
    EV_DETAIL << "Multicast receiver added for group " << group << ".\n";

    int numRoutes = rt->getNumMulticastRoutes();
    for (int i = 0; i < numRoutes; i++)
    {
        PIMMulticastRoute *route = dynamic_cast<PIMMulticastRoute*>(rt->getMulticastRoute(i));

        // check group
        if (!route || route->getSource() != this || route->getMulticastGroup() != group)
            continue;

        SourceGroupState *sgState = getSourceGroupState(route->getOrigin(), group);
        ASSERT(sgState);

        // check on RPF interface
        UpstreamInterface *rpfInterface = sgState->upstreamInterface;
        if (rpfInterface->ie == pimInterface->getInterfacePtr())
            continue;

        // is interface in list of outgoing interfaces?
        DownstreamInterface *downstream = sgState->findDownstreamInterfaceByInterfaceId(pimInterface->getInterfaceId());
        if (downstream)
        {
            EV << "Interface is already on list of outgoing interfaces" << endl;
            if (downstream->pruneState == DownstreamInterface::PRUNED)
                downstream->pruneState = DownstreamInterface::NO_INFO;
        }
        else
        {
            // create new downstream data
            EV << "Interface is not on list of outgoing interfaces yet, it will be added" << endl;
            DownstreamInterface *downstream = sgState->createDownstreamInterface(pimInterface->getInterfacePtr());
            route->addOutInterface(new PIMDMOutInterface(pimInterface->getInterfacePtr(), downstream));
        }

        sgState->setFlags(PIMMulticastRoute::C);

        // route was pruned, has to be added to multicast tree
        if (sgState->isFlagSet(PIMMulticastRoute::P))
        {
            EV << "Route is not pruned any more, send Graft to upstream" << endl;

            // if source is not directly connected, send Graft to upstream
            if (!sgState->isFlagSet(PIMMulticastRoute::A))
            {
                UpstreamInterface *inInterface = sgState->upstreamInterface;
                sendGraftPacket(inInterface->nextHop, sgState->source, group, inInterface->getInterfaceId());
                inInterface->startGraftRetryTimer();
            }
            else
                sgState->clearFlag(PIMMulticastRoute::P);
        }
    }
}

/**
 * The method process notification about multicast groups removed from interface. For each
 * old address it tries to find route. If there is route, it finds interface in list of outgoing
 * interfaces. If the interface is in the list it will be removed. If the router was not pruned
 * and there is no outgoing interface, the router will prune from the multicast tree.
 */
void PIMDM::multicastReceiverRemoved(PIMInterface *pimInt, IPv4Address group)
{
    EV_DETAIL << "No more receiver for group " << group << ".\n";

    // delete pimInt from outgoing interfaces of multicast routes for group
    int numRoutes = rt->getNumMulticastRoutes();
    for (int i = 0; i < numRoutes; i++)
    {
        PIMMulticastRoute *route = dynamic_cast<PIMMulticastRoute*>(rt->getMulticastRoute(i));

        // check group
        if (route && route->getMulticastGroup() != group)
            continue;

        SourceGroupState *sgState = getSourceGroupState(route->getOrigin(), group);

        // remove pimInt from the list of outgoing interfaces
        DownstreamInterface *downstream = sgState->findDownstreamInterfaceByInterfaceId(pimInt->getInterfaceId());
        if (downstream)
        {
            // EV << "Interface is present, removing it from the list of outgoing interfaces." << endl;
            sgState->removeDownstreamInterface(downstream->ie->getInterfaceId());
            route->removeOutInterface(downstream->ie); // will delete downstream, XXX method should be named deleteOutInterface()
        }

        bool connected = false;
        for (unsigned int k = 0; k < sgState->downstreamInterfaces.size(); k++)
        {
            DownstreamInterface *outInt = sgState->downstreamInterfaces[k];
            if(outInt->pruneState == DownstreamInterface::NO_INFO &&
               pimNbt->getNumNeighborsOnInterface(outInt->ie->getInterfaceId()) == 0)
            {
                connected = true;
                break;
            }
        }

        // if there is no directly connected member of group
        if (!connected)
            sgState->clearFlag(PIMMulticastRoute::C);

        // there is no receiver of multicast, prune the router from the multicast tree
        if (sgState->isOilistNull())
        {
            EV << "Route is not forwarding any more, send Prune to upstream" << endl;
            // if GRT is running now, do not send Prune msg
            UpstreamInterface *upstream = sgState->upstreamInterface;
            if (sgState->isFlagSet(PIMMulticastRoute::P) && upstream->graftRetryTimer)
            {
                cancelAndDelete(upstream->graftRetryTimer);
                upstream->graftRetryTimer = NULL;
                sendPrunePacket(upstream->nextHop, sgState->source, sgState->group, upstream->getInterfaceId());
            }

            // if the source is not directly connected, sent Prune msg
            if (!sgState->isFlagSet(PIMMulticastRoute::A) && !sgState->isFlagSet(PIMMulticastRoute::P))
            {
                sendPrunePacket(upstream->nextHop, sgState->source, sgState->group, upstream->getInterfaceId());
            }

            sgState->setFlags(PIMMulticastRoute::P);
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
void PIMDM::rpfInterfaceHasChanged(PIMMulticastRoute *route, InterfaceEntry *newRpf)
{
    IPv4Address source = route->getOrigin();
    IPv4Address group = route->getMulticastGroup();
    int rpfId = newRpf->getInterfaceId();

    EV_DETAIL << "New RPF interface for group=" << group << " source=" << source << " is " << newRpf->getName() << endl;

    SourceGroupState *sgState = getSourceGroupState(source, group);
    ASSERT(sgState);

    // delete old upstream interface data
    UpstreamInterface *oldUpstreamInterface = sgState->upstreamInterface;
    InterfaceEntry *oldRpfInterface = oldUpstreamInterface ? oldUpstreamInterface->ie : NULL;
    delete oldUpstreamInterface;
    delete route->getInInterface();
    route->setInInterface(NULL);

    // set new upstream interface data
    IPv4Address newRpfNeighbor = pimNbt->getNeighborsOnInterface(rpfId)[0]->getAddress(); // XXX what happens if no neighbors?
    sgState->upstreamInterface = new UpstreamInterface(sgState, newRpf, newRpfNeighbor);
    route->setInInterface(new IMulticastRoute::InInterface(newRpf));

    // delete rpf interface from the downstream interfaces
    sgState->removeDownstreamInterface(newRpf->getInterfaceId());
    route->removeOutInterface(newRpf); // will delete downstream data, XXX method should be called deleteOutInterface()

    // route was not pruned, join to the multicast tree again
    if (!sgState->isFlagSet(PIMMulticastRoute::P))
    {
        sendGraftPacket(sgState->upstreamInterface->nextHop, source, group, rpfId);
        sgState->upstreamInterface->startGraftRetryTimer();
    }

    // old RPF interface should be now a downstream interface if it is not down
    if (oldRpfInterface && oldRpfInterface->isUp())
    {
        DownstreamInterface *downstream = sgState->createDownstreamInterface(oldRpfInterface);
        route->addOutInterface(new PIMDMOutInterface(oldRpfInterface, downstream));
    }
}

//----------------------------------------------------------------------------
//           Helpers
//----------------------------------------------------------------------------

void PIMDM::restartTimer(cMessage *timer, double interval)
{
    cancelEvent(timer);
    scheduleAt(simTime() + interval, timer);
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

PIMMulticastRoute *PIMDM::getRouteFor(IPv4Address group, IPv4Address source)
{
    int numRoutes = rt->getNumMulticastRoutes();
    for (int i = 0; i < numRoutes; i++)
    {
        PIMMulticastRoute *route = dynamic_cast<PIMMulticastRoute*>(rt->getMulticastRoute(i));
        if (route && route->getMulticastGroup() == group && route->getOrigin() == source)
            return route;
    }
    return NULL;
}

PIMDM::SourceGroupState *PIMDM::getSourceGroupState(IPv4Address source, IPv4Address group)
{
    SGStateMap::iterator it = sgStates.find(SourceAndGroup(source, group));
    return it != sgStates.end() ? &(it->second) : NULL;
}

void PIMDM::deleteSourceGroupState(IPv4Address source, IPv4Address group)
{
    sgStates.erase(SourceAndGroup(source, group));
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
    owner->owner->scheduleAt(simTime() + owner->owner->graftRetryInterval, graftRetryTimer);
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
    owner->owner->scheduleAt(simTime() + owner->owner->sourceActiveInterval, sourceActiveTimer);
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
    owner->owner->scheduleAt(simTime() + owner->owner->stateRefreshInterval, stateRefreshTimer);
}

PIMDM::DownstreamInterface::~DownstreamInterface()
{
    owner->owner->cancelAndDelete(pruneTimer);
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
    if (pruneTimer)
    {
        if (pruneTimer->isScheduled())
            owner->owner->cancelEvent(pruneTimer);
        delete pruneTimer;
        pruneTimer = NULL;
    }
}

void PIMDM::DownstreamInterface::startPrunePendingTimer(double overrideInterval)
{
    cMessage *timer = new cMessage("PimPrunePendingTimer", PrunePendingTimer);
    timer->setContextPointer(this);
    owner->owner->scheduleAt(simTime() + overrideInterval, timer);
    prunePendingTimer = timer;
}

void PIMDM::DownstreamInterface::stopPrunePendingTimer()
{
    if (prunePendingTimer)
    {
        if (prunePendingTimer->isScheduled())
            owner->owner->cancelEvent(prunePendingTimer);
        delete prunePendingTimer;
        prunePendingTimer = NULL;
    }
}

PIMDM::SourceGroupState::~SourceGroupState()
{
    delete upstreamInterface;
    for (unsigned int i = 0; i < downstreamInterfaces.size(); ++i)
        delete downstreamInterfaces[i];
    downstreamInterfaces.clear();
}

PIMDM::DownstreamInterface *PIMDM::SourceGroupState::findDownstreamInterfaceByInterfaceId(int interfaceId) const
{
    for (unsigned int i = 0; i < downstreamInterfaces.size(); ++i)
        if (downstreamInterfaces[i]->ie->getInterfaceId() == interfaceId)
            return downstreamInterfaces[i];
    return NULL;
}

PIMDM::DownstreamInterface *PIMDM::SourceGroupState::createDownstreamInterface(InterfaceEntry *ie)
{
    DownstreamInterface *downstream = new DownstreamInterface(this, ie);
    downstreamInterfaces.push_back(downstream);
    return downstream;
}


void PIMDM::SourceGroupState::removeDownstreamInterface(int interfaceId)
{
    for (std::vector<DownstreamInterface*>::iterator it = downstreamInterfaces.begin(); it != downstreamInterfaces.end(); ++it)
    {
        if ((*it)->ie->getInterfaceId() == interfaceId)
        {
            downstreamInterfaces.erase(it);
            break;
        }
    }
}

bool PIMDM::SourceGroupState::isOilistNull()
{
    for (unsigned int i = 0; i < downstreamInterfaces.size(); i++)
    {
        if (downstreamInterfaces[i]->pruneState != DownstreamInterface::PRUNED)
            return false;
    }
    return true;
}



