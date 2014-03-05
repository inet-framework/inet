//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "AODVRouting.h"
#include "IPv4Route.h"
#include "Ieee80211Frame_m.h"
#include "IPSocket.h"
#include "UDPControlInfo.h"
#include "AODVDefs.h"
#include "NodeOperations.h"

Define_Module(AODVRouting);

void AODVRouting::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        lastBroadcastTime = 0;
        rebootTime = 0;
        rreqId = sequenceNum = 0;
        host = this->getParentModule();
        routingTable = check_and_cast<IRoutingTable *>(getModuleByPath(par("routingTablePath")));
        interfaceTable = check_and_cast<IInterfaceTable *>(getModuleByPath(par("interfaceTablePath")));
        networkProtocol = check_and_cast<INetfilter *>(getModuleByPath(par("networkProtocolPath")));
        AodvUDPPort = par("UDPPort");
        askGratuitousRREP = par("askGratuitousRREP");
        useHelloMessages = par("useHelloMessages");
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS)
    {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(this->getParentModule()->getSubmodule("status"));
        isOperational = !nodeStatus || nodeStatus->getState() == NodeStatus::UP;

        addressType = getSelfIPAddress().getAddressType();
        IPSocket socket(gate("ipOut"));
        socket.registerProtocol(IP_PROT_MANET);
        networkProtocol->registerHook(0, this);
        host->subscribe(NF_LINK_BREAK, this);

        if (useHelloMessages)
        {
            helloMsgTimer = new cMessage("HelloMsgTimer");
            scheduleAt(simTime() + HELLO_INTERVAL, helloMsgTimer);
        }
    }
}

void AODVRouting::handleMessage(cMessage *msg)
{
    if (!isOperational)
    {
        if (msg->isSelfMessage())
            throw cRuntimeError("Model error: self msg '%s' received when isOperational is false", msg->getName());

        EV_ERROR << "Application is turned off, dropping '" << msg->getName() << "' message\n";
        delete msg;
    }

    if (msg->isSelfMessage())
    {
        if (dynamic_cast<WaitForRREP *>(msg))
            handleWaitForRREP((WaitForRREP*)msg);
        else if (strcmp(msg->getName(), "HelloMsgTimer") == 0)
           sendHelloMessagesIfNeeded();
        else
            throw cRuntimeError("Unknown self message");
    }
    else
    {
        UDPPacket *udpPacket = check_and_cast<UDPPacket *>(msg);
        AODVControlPacket *ctrlPacket = check_and_cast<AODVControlPacket *>(udpPacket->decapsulate());
        unsigned int ctrlPacketType = ctrlPacket->getPacketType();
        INetworkProtocolControlInfo * udpProtocolCtrlInfo = dynamic_cast<INetworkProtocolControlInfo *>(udpPacket->getControlInfo());
        ASSERT(udpProtocolCtrlInfo != NULL);
        Address sourceAddr = udpProtocolCtrlInfo->getSourceAddress();
        unsigned int arrivalPacketTTL = udpProtocolCtrlInfo->getHopLimit();
        std::cout << udpPacket->getName() << this->getParentModule()->getFullName() << " packet arrived with type " << ctrlPacketType << " from " << sourceAddr << endl;
        switch (ctrlPacketType)
        {
            case RREQ:
                handleRREQ(check_and_cast<AODVRREQ *>(ctrlPacket),sourceAddr,arrivalPacketTTL);
                break;
            case RREP:
                handleRREP(check_and_cast<AODVRREP *>(ctrlPacket),sourceAddr);
                break;
            case RERR:
                handleRERR(check_and_cast<AODVRERR *>(ctrlPacket),sourceAddr);
                break;
            default:
                throw cRuntimeError("AODV Control Packet arrived with undefined packet type: %d", ctrlPacketType);
        }
        delete msg;
    }
}

INetfilter::IHook::Result AODVRouting::ensureRouteForDatagram(INetworkDatagram * datagram)
{
    Enter_Method("datagramPreRoutingHook");
    const Address& destAddr = datagram->getDestinationAddress();
    const Address& sourceAddr = datagram->getSourceAddress();

    if (destAddr.isBroadcast() || routingTable->isLocalAddress(destAddr) || destAddr.isMulticast())
        return ACCEPT;
    else
    {
        EV_INFO << "Finding route for source " << sourceAddr << " with destination " << destAddr << endl;
        IRoute* route = routingTable->findBestMatchingRoute(destAddr);
        AODVRouteData* routeData = route ? dynamic_cast<AODVRouteData *>(route->getProtocolData()) : NULL;
        bool isActive = routeData && routeData->isActive();

        if (route && !route->getNextHopAsGeneric().isUnspecified() && isActive)
        {
            EV_INFO << "Active route found: " << route << endl;
            if (routeData)
                routeData->setLastUsed(simTime());

            return ACCEPT;
        }
        else if (sourceAddr.isUnspecified() || routingTable->isLocalAddress(sourceAddr))
        {
            // A node disseminates a RREQ when it determines that it needs a route
            // to a destination and does not have one available.  This can happen if
            // the destination is previously unknown to the node, or if a previously
            // valid route to the destination expires or is marked as invalid.

            EV_INFO << (isActive ? "Inactive" : "Missing") << " route for destination " << destAddr << endl;
            delayDatagram(datagram);

            if (!hasOngoingRouteDiscovery(destAddr))
            {
                // When a new route to the same destination is required at a later time
                // (e.g., upon route loss), the TTL in the RREQ IP header is initially
                // set to the Hop Count plus TTL_INCREMENT.
                if (!isActive)
                    startRouteDiscovery(destAddr, route->getMetric() + TTL_INCREMENT);
                else
                    startRouteDiscovery(destAddr);
            }
            else
                EV_DETAIL << "Route discovery is in progress, originator " << getSelfIPAddress() << " target " << destAddr << endl;

            return QUEUE;
        }
        else
            return ACCEPT;
    }
}

void AODVRouting::startAODVRouting()
{

}

void AODVRouting::stopAODVRouting()
{

}

AODVRouting::AODVRouting()
{
    interfaceTable = NULL;
    host = NULL;
    routingTable = NULL;
    isOperational = false;
    networkProtocol = NULL;
    addressType = NULL;
    helloMsgTimer = NULL;
}

bool AODVRouting::hasOngoingRouteDiscovery(const Address& destAddr)
{
    return waitForRREPTimers.find(destAddr) != waitForRREPTimers.end();
}

void AODVRouting::startRouteDiscovery(const Address& destAddr, unsigned timeToLive)
{

    EV_INFO << "Starting route discovery with originator " << getSelfIPAddress() << " and destination " << destAddr << endl;
    ASSERT(!hasOngoingRouteDiscovery(destAddr));
    AODVRREQ * rreq = createRREQ(destAddr);
    sendRREQ(rreq, addressType->getBroadcastAddress(), timeToLive);
}

Address AODVRouting::getSelfIPAddress()
{
    return routingTable->getRouterIdAsGeneric();
}

void AODVRouting::delayDatagram(INetworkDatagram* datagram)
{
    EV_DETAIL << "Queuing datagram, source " << datagram->getSourceAddress() << ", destination " << datagram->getDestinationAddress() << endl;
    const Address& target = datagram->getDestinationAddress();
    targetAddressToDelayedPackets.insert(std::pair<Address, INetworkDatagram *>(target, datagram));
}

void AODVRouting::sendRREQ(AODVRREQ * rreq, const Address& destAddr, unsigned int timeToLive)
{
    // In an expanding ring search, the originating node initially uses a TTL =
    // TTL_START in the RREQ packet IP header and sets the timeout for
    // receiving a RREP to RING_TRAVERSAL_TIME milliseconds.
    // RING_TRAVERSAL_TIME is calculated as described in section 10.  The
    // TTL_VALUE used in calculating RING_TRAVERSAL_TIME is set equal to the
    // value of the TTL field in the IP header.  If the RREQ times out
    // without a corresponding RREP, the originator broadcasts the RREQ
    // again with the TTL incremented by TTL_INCREMENT.  This continues
    // until the TTL set in the RREQ reaches TTL_THRESHOLD, beyond which a
    // TTL = NET_DIAMETER is used for each attempt.

    std::map<Address, WaitForRREP *>::iterator rrepTimer = waitForRREPTimers.find(rreq->getDestAddr());

    if (rrepTimer != waitForRREPTimers.end())
    {
        WaitForRREP * rrepTimerMsg = rrepTimer->second;
        unsigned int lastTTL = rrepTimerMsg->getLastTTL();
        rrepTimerMsg->setDestAddr(rreq->getDestAddr());

        // The Hop Count stored in an invalid routing table entry indicates the
        // last known hop count to that destination in the routing table.  When
        // a new route to the same destination is required at a later time
        // (e.g., upon route loss), the TTL in the RREQ IP header is initially
        // set to the Hop Count plus TTL_INCREMENT.  Thereafter, following each
        // timeout the TTL is incremented by TTL_INCREMENT until TTL =
        // TTL_THRESHOLD is reached.  Beyond this TTL = NET_DIAMETER is used.
        // Once TTL = NET_DIAMETER, the timeout for waiting for the RREP is set
        // to NET_TRAVERSAL_TIME, as specified in section 6.3.

        if (timeToLive != 0)
        {
            rrepTimerMsg->setLastTTL(timeToLive);
            rrepTimerMsg->setFromInvalidEntry(true);
            cancelEvent(rrepTimerMsg);
        }
        else if (lastTTL + TTL_INCREMENT < TTL_THRESHOLD)
        {
            ASSERT(!rrepTimerMsg->isScheduled());
            timeToLive = lastTTL + TTL_INCREMENT;
            rrepTimerMsg->setLastTTL(lastTTL + TTL_INCREMENT);
        }
        else
        {
            ASSERT(!rrepTimerMsg->isScheduled());
            timeToLive = NET_DIAMETER;
            rrepTimerMsg->setLastTTL(NET_DIAMETER);
        }

        if (rrepTimerMsg->getLastTTL() == NET_DIAMETER && rrepTimerMsg->getFromInvalidEntry())
            scheduleAt(simTime() + NET_TRAVERSAL_TIME, rrepTimerMsg);
        else
        {
            double ringTraversalTime = 2.0 * NODE_TRAVERSAL_TIME * (timeToLive + TIMEOUT_BUFFER);
            scheduleAt(simTime() + ringTraversalTime, rrepTimerMsg);
        }
    }
    else
    {
        WaitForRREP * newRREPTimerMsg = new WaitForRREP();
        waitForRREPTimers[rreq->getDestAddr()] = newRREPTimerMsg;
        ASSERT(hasOngoingRouteDiscovery(rreq->getDestAddr()));

        timeToLive = TTL_START;
        newRREPTimerMsg->setLastTTL(TTL_START);
        newRREPTimerMsg->setFromInvalidEntry(false);
        newRREPTimerMsg->setDestAddr(rreq->getDestAddr());

        // Each time, the timeout for receiving a RREP is RING_TRAVERSAL_TIME.
        double ringTraversalTime = 2.0 * NODE_TRAVERSAL_TIME * (TTL_START + TIMEOUT_BUFFER);

        scheduleAt(simTime() + ringTraversalTime, newRREPTimerMsg);

    }
    double delay = uniform(0,1);
    sendAODVPacket(rreq, destAddr, timeToLive, delay);
}


void AODVRouting::sendRREP(AODVRREP * rrep, const Address& destAddr, unsigned int timeToLive)
{
    EV_INFO << "Sending Route Reply to " << destAddr << " for node: " << getSelfIPAddress() << endl;

    // When any node transmits a RREP, the precursor list for the
    // corresponding destination node is updated by adding to it
    // the next hop node to which the RREP is forwarded.

    IRoute * destRoute = routingTable->findBestMatchingRoute(destAddr);
    const Address& nextHop = destRoute->getNextHopAsGeneric();
    AODVRouteData * destRouteData = dynamic_cast<AODVRouteData *>(destRoute->getProtocolData());
    destRouteData->addPrecursor(nextHop);

    sendAODVPacket(rrep, nextHop, timeToLive, 0); // FIXME: temporary TTL
}

/*
 * RFC 3561: 6.3. Generating Route Requests
 */
AODVRREQ * AODVRouting::createRREQ(const Address& destAddr)
{
    AODVRREQ *rreqPacket = new AODVRREQ("AODV-RREQ");

    rreqPacket->setGratuitousRREPFlag(askGratuitousRREP);
    IRoute *lastKnownRoute = routingTable->findBestMatchingRoute(destAddr);

    rreqPacket->setPacketType(RREQ);

    // The Originator Sequence Number in the RREQ message is the
    // node's own sequence number, which is incremented prior to
    // insertion in a RREQ.
    sequenceNum++;

    rreqPacket->setOriginatorSeqNum(sequenceNum);

    if (lastKnownRoute)
    {
        // The Destination Sequence Number field in the RREQ message is the last
        // known destination sequence number for this destination and is copied
        // from the Destination Sequence Number field in the routing table.

        AODVRouteData * routeData = dynamic_cast<AODVRouteData *>(lastKnownRoute->getProtocolData());
        if (routeData && routeData->hasValidDestNum())
        {
            rreqPacket->setDestSeqNum(routeData->getDestSeqNum());
            rreqPacket->setUnknownSeqNumFlag(false);
        }
        else
            rreqPacket->setUnknownSeqNumFlag(true);
    }
    else
        rreqPacket->setUnknownSeqNumFlag(true); // If no sequence number is known, the unknown sequence number flag MUST be set.


    rreqPacket->setOriginatorAddr(getSelfIPAddress());
    rreqPacket->setDestAddr(destAddr);

    // The RREQ ID field is incremented by one from the last RREQ ID used
    // by the current node. Each node maintains only one RREQ ID.
    rreqId++;
    rreqPacket->setRreqId(rreqId);

    // The Hop Count field is set to zero.
    rreqPacket->setHopCount(0);

    // Before broadcasting the RREQ, the originating node buffers the RREQ
    // ID and the Originator IP address (its own address) of the RREQ for
    // PATH_DISCOVERY_TIME.
    // In this way, when the node receives the packet again from its neighbors,
    // it will not reprocess and re-forward the packet.

    RREQIdentifier rreqIdentifier(getSelfIPAddress(),rreqId);
    rreqsArrivalTime[rreqIdentifier] = simTime();

    // TODO: G flag

    return rreqPacket;
}

AODVRREP* AODVRouting::createRREP(AODVRREQ * rreq, IRoute * route, const Address& sourceAddr)
{
    AODVRREP *rrep = new AODVRREP("AODV-RREP");
    rrep->setPacketType(RREP);

    // When generating a RREP message, a node copies the Destination IP
    // Address and the Originator Sequence Number from the RREQ message into
    // the corresponding fields in the RREP message.

    rrep->setDestAddr(rreq->getDestAddr());
    rrep->setOriginatorSeqNum(rreq->getOriginatorSeqNum());

    // OriginatorAddr = The IP address of the node which originated the RREQ
    // for which the route is supplied.
    rrep->setOriginatorAddr(rreq->getOriginatorAddr());

    // Processing is slightly different, depending on whether the node is
    // itself the requested destination (see section 6.6.1), or instead
    // if it is an intermediate node with an fresh enough route to the destination
    // (see section 6.6.2).

    if (rreq->getDestAddr() == getSelfIPAddress()) // node is itself the requested destination
    {
        // If the generating node is the destination itself, it MUST increment
        // its own sequence number by one if the sequence number in the RREQ
        // packet is equal to that incremented value.

        if (sequenceNum + 1 == rreq->getDestSeqNum())
            sequenceNum++;

        // The destination node places its (perhaps newly incremented)
        // sequence number into the Destination Sequence Number field of
        // the RREP,
        rrep->setDestSeqNum(sequenceNum);

        // and enters the value zero in the Hop Count field
        // of the RREP.
        rrep->setHopCount(0);

        // The destination node copies the value MY_ROUTE_TIMEOUT
        // into the Lifetime field of the RREP.
        rrep->setLifeTime(MY_ROUTE_TIMEOUT);

    }
    else // intermediate node
    {
        // it copies its known sequence number for the destination into
        // the Destination Sequence Number field in the RREP message.
        AODVRouteData *routeData = dynamic_cast<AODVRouteData *>(route->getProtocolData());
        rrep->setDestSeqNum(routeData->getDestSeqNum());

        // The intermediate node updates the forward route entry by placing the
        // last hop node (from which it received the RREQ, as indicated by the
        // source IP address field in the IP header) into the precursor list for
        // the forward route entry -- i.e., the entry for the Destination IP
        // Address.

        routeData->addPrecursor(sourceAddr);

        // The intermediate node places its distance in hops from the
        // destination (indicated by the hop count in the routing table)
        // Hop Count field in the RREP.

        rrep->setHopCount(route->getMetric());

        // The Lifetime field of the RREP is calculated by subtracting the
        // current time from the expiration time in its route table entry.

        rrep->setLifeTime(routeData->getLifeTime() - simTime());
    }
    return rrep;
}

/*
 * 6.6.3. Generating Gratuitous RREPs
 */
AODVRREP* AODVRouting::createGratuitousRREP(AODVRREQ* rreq, IRoute* route)
{
    ASSERT(route != NULL);
    AODVRREP *grrep = new AODVRREP("AODV-GRREP");
    AODVRouteData * routeData = dynamic_cast<AODVRouteData *>(route->getProtocolData());

    grrep->setPacketType(GRREP);
    grrep->setHopCount(route->getMetric());
    grrep->setDestAddr(rreq->getOriginatorAddr());
    grrep->setDestSeqNum(rreq->getOriginatorSeqNum());
    grrep->setOriginatorAddr(rreq->getDestAddr());

    // XXX: ? The remaining lifetime of the route
    // towards the originator of the RREQ,
    // as known by the intermediate node.
    grrep->setLifeTime(routeData->getLifeTime());

    return grrep;
}

/*
 * 6.7. Receiving and Forwarding Route Replies
 */
void AODVRouting::handleRREP(AODVRREP* rrep, const Address& sourceAddr)
{
    EV_INFO << "Route Reply arrived from " << sourceAddr << endl;

    if (rrep->getOriginatorAddr().isUnspecified())
    {
        EV_INFO << "This Route Reply is a Hello Message" << endl;
        handleHelloMessage(rrep);
        return;
    }
    // When a node receives a RREP message, it searches (using longest-
    // prefix matching) for a route to the previous hop.

    // If needed, a route is created for the previous hop,
    // but without a valid sequence number (see section 6.2)

    IRoute * previousHopRoute = routingTable->findBestMatchingRoute(sourceAddr);

    if (!previousHopRoute || previousHopRoute->getSource() != this)
    {
        // create without valid sequence number
        createRoute(sourceAddr,sourceAddr,1,false,-1,true,simTime() + ACTIVE_ROUTE_TIMEOUT);
    }
    else
    {
        AODVRouteData * previousHopProtocolData = dynamic_cast<AODVRouteData *>(previousHopRoute->getProtocolData());
        updateRoutingTable(previousHopRoute,sourceAddr,1,false,0,previousHopProtocolData->isActive(),simTime() + ACTIVE_ROUTE_TIMEOUT);
    }

    // Next, the node then increments the hop count value in the RREP by one,
    // to account for the new hop through the intermediate node
    unsigned int newHopCount = rrep->getHopCount() + 1;
    rrep->setHopCount(newHopCount);

    // Then the forward route for this destination is created if it does not
    // already exist.

    IRoute * route = routingTable->findBestMatchingRoute(rrep->getDestAddr());
    AODVRouteData * routeData = NULL;
    simtime_t lifeTime = rrep->getLifeTime();
    unsigned int destSeqNum = rrep->getDestSeqNum();

    if (route && route->getSource() == this) // already exists
    {

        routeData = dynamic_cast<AODVRouteData *>(route->getProtocolData());
        // Upon comparison, the existing entry is updated only in the following circumstances:

        // (i) the sequence number in the routing table is marked as
        //     invalid in route table entry.

        if (!routeData->hasValidDestNum())
        {
            updateRoutingTable(route, sourceAddr, newHopCount, true, destSeqNum, false, simTime() + lifeTime);
            /*
               If the route table entry to the destination is created or updated,
               then the following actions occur:

               -  the route is marked as active,

               -  the destination sequence number is marked as valid,

               -  the next hop in the route entry is assigned to be the node from
                  which the RREP is received, which is indicated by the source IP
                  address field in the IP header,

               -  the hop count is set to the value of the New Hop Count,

               -  the expiry time is set to the current time plus the value of the
                  Lifetime in the RREP message,

               -  and the destination sequence number is the Destination Sequence
                  Number in the RREP message.
             */
        }
        // (ii) the Destination Sequence Number in the RREP is greater than
        //      the node's copy of the destination sequence number and the
        //      known value is valid, or
        else if (destSeqNum > routeData->getDestSeqNum())
        {
            updateRoutingTable(route, sourceAddr, newHopCount, true, destSeqNum, false, simTime() + lifeTime);
        }
        else
        {
            // (iii) the sequence numbers are the same, but the route is
            //       marked as inactive, or
            if (destSeqNum == routeData->getDestSeqNum() && !routeData->isActive())
            {
                updateRoutingTable(route, sourceAddr, newHopCount, true, destSeqNum, false, simTime() + lifeTime);
            }
            // (iv) the sequence numbers are the same, and the New Hop Count is
            //      smaller than the hop count in route table entry.
            else if (destSeqNum == routeData->getDestSeqNum() && rrep->getHopCount() < (unsigned int) route->getMetric())
            {
                updateRoutingTable(route, sourceAddr, newHopCount, true, destSeqNum, false, simTime() + lifeTime);
            }
        }
    }
    else // create forward route for the destination: this path will be used by the originator to send data packets
    {
        route = createRoute(rrep->getDestAddr(),sourceAddr,newHopCount,true,destSeqNum,true,simTime() + lifeTime);
        routeData = dynamic_cast<AODVRouteData *>(route->getProtocolData());
    }

    // If the current node is not the node indicated by the Originator IP
    // Address in the RREP message AND a forward route has been created or
    // updated as described above, the node consults its route table entry
    // for the originating node to determine the next hop for the RREP
    // packet, and then forwards the RREP towards the originator using the
    // information in that route table entry.

    if (getSelfIPAddress() != rrep->getOriginatorAddr())
    {
        // If a node forwards a RREP over a link that is likely to have errors or
        // be unidirectional, the node SHOULD set the 'A' flag to require that the
        // recipient of the RREP acknowledge receipt of the RREP by sending a RREP-ACK
        // message back (see section 6.8).

        IRoute * forwardRREPRoute = routingTable->findBestMatchingRoute(rrep->getOriginatorAddr());
        if (forwardRREPRoute)
        {
            if (rrep->getAckRequiredFlag())
            {
                // TODO: send RREP-ACK: (simTime() > rebootTime + DELETE_PERIOD || rebootTime == 0)
                rrep->setAckRequiredFlag(false);
            }

            AODVRouteData * forwardRREPRouteData = dynamic_cast<AODVRouteData *>(forwardRREPRoute->getProtocolData());

            // Also, at each node the (reverse) route used to forward a
            // RREP has its lifetime changed to be the maximum of (existing-
            // lifetime, (current time + ACTIVE_ROUTE_TIMEOUT).

            simtime_t existingLifeTime = forwardRREPRouteData->getLifeTime();
            forwardRREPRouteData->setLifeTime(std::max(simTime() + ACTIVE_ROUTE_TIMEOUT, existingLifeTime));


            // Finally, the precursor list for the next hop towards the
            // destination is updated to contain the next hop towards the
            // source.

            routeData->addPrecursor(forwardRREPRoute->getNextHopAsGeneric());


            if (simTime() > rebootTime + DELETE_PERIOD || rebootTime == 0)
                forwardRREP(rrep, forwardRREPRoute->getNextHopAsGeneric(), 100);
        }
        else
            EV_ERROR << "Reverse route doesn't exist. Dropping the RREP message" << endl;

    }
    else
    {
        if (hasOngoingRouteDiscovery(rrep->getDestAddr()))
        {
            EV_INFO << "The Route Reply has arrived for our Route Request to node " << rrep->getDestAddr() << endl;
            completeRouteDiscovery(rrep->getDestAddr());
        }
        delete rrep;
    }

}
void AODVRouting::updateRoutingTable(IRoute * route, const Address& nextHop, unsigned int hopCount, bool hasValidDestNum, unsigned int destSeqNum, bool isActive, simtime_t lifeTime)
{
    EV_DETAIL << "Updating the Routing Table with ..." << endl;
    route->setNextHop(nextHop);
    route->setMetric(hopCount);
    AODVRouteData * routingData = dynamic_cast<AODVRouteData *>(route->getProtocolData());

    ASSERT(routingData != NULL);

    routingData->setLifeTime(lifeTime);
    routingData->setDestSeqNum(destSeqNum);
    routingData->setIsActive(isActive);
    routingData->setHasValidDestNum(hasValidDestNum);
}

void AODVRouting::sendAODVPacket(AODVControlPacket* packet, const Address& destAddr, unsigned int timeToLive, double delay)
{

    ASSERT(timeToLive != 0);

    INetworkProtocolControlInfo * networkProtocolControlInfo = addressType->createNetworkProtocolControlInfo();

    networkProtocolControlInfo->setHopLimit(timeToLive);

    networkProtocolControlInfo->setTransportProtocol(IP_PROT_MANET);
    networkProtocolControlInfo->setDestinationAddress(destAddr);
    networkProtocolControlInfo->setSourceAddress(getSelfIPAddress());

    InterfaceEntry * ifEntry = interfaceTable->getInterfaceByName("wlan0"); // FIXME
    networkProtocolControlInfo->setInterfaceId(ifEntry->getInterfaceId());

    UDPPacket * udpPacket = new UDPPacket(packet->getName());
    udpPacket->encapsulate(packet);
    udpPacket->setSourcePort(AodvUDPPort);
    udpPacket->setDestinationPort(AodvUDPPort);
    udpPacket->setControlInfo(dynamic_cast<cObject *>(networkProtocolControlInfo));

    if (destAddr.isBroadcast())
        lastBroadcastTime = simTime();

    if (delay == 0)
        send(udpPacket, "ipOut");
    else
        sendDelayed(udpPacket, delay, "ipOut");
}

void AODVRouting::handleRREQ(AODVRREQ* rreq, const Address& sourceAddr, unsigned int timeToLive)
{
    EV_INFO << "AODV Route Request arrived from " << sourceAddr << endl;
    // When a node receives a RREQ, it first creates or updates a route to
    // the previous hop without a valid sequence number (see section 6.2).

    IRoute * previousHopRoute = routingTable->findBestMatchingRoute(sourceAddr);
    unsigned int destSeqNum = rreq->getDestSeqNum();

    if (!previousHopRoute || previousHopRoute->getSource() != this)
    {
        // create without valid sequence number
        createRoute(sourceAddr,sourceAddr,1,false,-1,true,simTime() + ACTIVE_ROUTE_TIMEOUT);

    }
    else
    {
        AODVRouteData * previousHopProtocolData = dynamic_cast<AODVRouteData *>(previousHopRoute->getProtocolData());
        updateRoutingTable(previousHopRoute,sourceAddr,1,false,0,previousHopProtocolData->isActive(),simTime() + ACTIVE_ROUTE_TIMEOUT);
    }

    // then checks to determine whether it has received a RREQ with the same
    // Originator IP Address and RREQ ID within at least the last PATH_DISCOVERY_TIME.
    // If such a RREQ has been received, the node silently discards the newly received RREQ.

    RREQIdentifier rreqIdentifier(rreq->getOriginatorAddr(), rreq->getRreqId());
    std::map<RREQIdentifier, simtime_t, RREQIdentifierCompare>::iterator checkRREQArrivalTime = rreqsArrivalTime.find(rreqIdentifier);
    if (checkRREQArrivalTime != rreqsArrivalTime.end() && simTime() - checkRREQArrivalTime->second <= PATH_DISCOVERY_TIME)
    {
        EV_WARN << "The same packet has arrived within PATH_DISCOVERY_TIME. Discarding it." << endl;
        delete rreq;
        return;
    }

    // update or create
    rreqsArrivalTime[rreqIdentifier] = simTime();

    // Otherwise, if a node does generate a RREP, then the node discards the
    // RREQ.

    // A node generates a RREP if either:
    //
    // (i)       it is itself the destination, or
    //
    // (ii)      it has an active route to the destination, the destination
    //           sequence number in the node's existing route table entry
    //           for the destination is valid and greater than or equal to
    //           the Destination Sequence Number of the RREQ (comparison
    //           using signed 32-bit arithmetic), and the "destination only"
    //           ('D') flag is NOT set.

    IRoute * destRoute = routingTable->findBestMatchingRoute(rreq->getDestAddr());
    AODVRouteData * destRouteData = NULL;
    if (destRoute)
        destRouteData = dynamic_cast<AODVRouteData *>(destRoute->getProtocolData());

    // First, it first increments the hop count value in the RREQ by one, to
    // account for the new hop through the intermediate node.

    rreq->setHopCount(rreq->getHopCount() + 1);

    // Then the node searches for a reverse route to the Originator IP Address (see
    // section 6.2), using longest-prefix matching.

    IRoute * reverseRoute = routingTable->findBestMatchingRoute(rreq->getOriginatorAddr());

    // If need be, the route is created, or updated using the Originator Sequence Number from the
    // RREQ in its routing table.
    //
    // When the reverse route is created or updated, the following actions on
    // the route are also carried out:
    //
    //   1. the Originator Sequence Number from the RREQ is compared to the
    //      corresponding destination sequence number in the route table entry
    //      and copied if greater than the existing value there
    //
    //   2. the valid sequence number field is set to true;
    //
    //   3. the next hop in the routing table becomes the node from which the
    //      RREQ was received (it is obtained from the source IP address in
    //      the IP header and is often not equal to the Originator IP Address
    //      field in the RREQ message);
    //
    //   4. the hop count is copied from the Hop Count in the RREQ message;
    //
    //   Whenever a RREQ message is received, the Lifetime of the reverse
    //   route entry for the Originator IP address is set to be the maximum of
    //   (ExistingLifetime, MinimalLifetime), where
    //
    //   MinimalLifetime = (current time + 2*NET_TRAVERSAL_TIME - 2*HopCount*NODE_TRAVERSAL_TIME).

    unsigned int hopCount = rreq->getHopCount();
    simtime_t minimalLifeTime = simTime() + 2 * NET_TRAVERSAL_TIME - 2 * hopCount * NODE_TRAVERSAL_TIME;
    simtime_t newLifeTime = std::max(simTime(), minimalLifeTime);
    unsigned int newDestSeqNum = rreq->getOriginatorSeqNum() > destSeqNum ? rreq->getOriginatorSeqNum() : destSeqNum; // 1. action (see above)

    if (rreq->getOriginatorAddr() != getSelfIPAddress())
    {
        if (!reverseRoute || reverseRoute->getSource() != this) // create
        {
            // This reverse route will be needed if the node receives a RREP back to the
            // node that originated the RREQ (identified by the Originator IP Address).
            createRoute(rreq->getOriginatorAddr(),sourceAddr,hopCount,true,newDestSeqNum,true,newLifeTime);
        }
        else
        {
            // if need be (6.2. Route Table Entries and Precursor Lists), update.
            AODVRouteData * reverseRouteData = dynamic_cast<AODVRouteData*>(reverseRoute->getProtocolData());

            if (destSeqNum > reverseRouteData->getDestSeqNum())
            {
                updateRoutingTable(reverseRoute,sourceAddr,hopCount,true,newDestSeqNum,reverseRouteData->isActive(),newLifeTime);
            }
            else if(destSeqNum == reverseRouteData->getDestSeqNum() && hopCount + 1 < (unsigned int) reverseRoute->getMetric())
            {
                updateRoutingTable(reverseRoute,sourceAddr,hopCount,true,newDestSeqNum,reverseRouteData->isActive(),newLifeTime);
            }
            else if(!reverseRouteData->hasValidDestNum())
            {
                updateRoutingTable(reverseRoute,sourceAddr,hopCount,true,newDestSeqNum,reverseRouteData->isActive(),newLifeTime);
            }

        }

        // check (i) - (ii)
        if (rreq->getDestAddr() == getSelfIPAddress() ||
            (destRouteData && destRouteData->isActive() && destRouteData->hasValidDestNum() && destRouteData->getDestSeqNum() >= rreq->getDestSeqNum()))
        {
            EV_INFO << "It is the destination node for which the route is desired" << endl;
            // create RREP
            AODVRREP * rrep = createRREP(rreq, destRoute, sourceAddr);

            // send to the originator
            sendRREP(rrep, rreq->getOriginatorAddr(), 100); // FIXME: temporary, we set the TTL value to 100

            // After a node receives a RREQ and responds with a RREP, it discards
            // the RREQ.  If the RREQ has the 'G' flag set, and the intermediate
            // node returns a RREP to the originating node, it MUST also unicast a
            // gratuitous RREP to the destination node.

            if (rreq->getGratuitousRREPFlag() && rreq->getDestAddr() != getSelfIPAddress())
            {
                // The gratuitous RREP is then sent to the next hop along the path to
                // the destination node, just as if the destination node had already
                // issued a RREQ for the originating node and this RREP was produced in
                // response to that (fictitious) RREQ.

                IRoute * originatorRoute = routingTable->findBestMatchingRoute(rreq->getOriginatorAddr());
                AODVRREP * grrep = createGratuitousRREP(rreq, originatorRoute);
                sendGRREP(grrep, rreq->getDestAddr(), 100);
            }

            delete rreq;
            return; // discard RREQ, in this case, we do not forward it.
        }
    }
    // If a node does not generate a RREP (following the processing rules in
    // section 6.6), and if the incoming IP header has TTL larger than 1,
    // the node updates and broadcasts the RREQ to address 255.255.255.255
    // on each of its configured interfaces (see section 6.14).  To update
    // the RREQ, the TTL or hop limit field in the outgoing IP header is
    // decreased by one, and the Hop Count field in the RREQ message is
    // incremented by one, to account for the new hop through the
    // intermediate node. (!) Lastly, the Destination Sequence number for the
    // requested destination is set to the maximum of the corresponding
    // value received in the RREQ message, and the destination sequence
    // value currently maintained by the node for the requested destination.
    // However, the forwarding node MUST NOT modify its maintained value for
    // the destination sequence number, even if the value received in the
    // incoming RREQ is larger than the value currently maintained by the
    // forwarding node.

    if (destRouteData && !destRouteData->isActive()) // (!)
        rreq->setDestSeqNum(std::max(destRouteData->getDestSeqNum(), rreq->getDestSeqNum()));

    if (timeToLive > 1 && (simTime() > rebootTime + DELETE_PERIOD || rebootTime == 0))
    {
        forwardRREQ(rreq, timeToLive - 1);
    }
    else
    {
        EV_WARN << "Can't forward the RREQ because of its small TTL: " << timeToLive << " or the AODV reboot has not completed yet"  << endl;
        delete rreq;
    }
}

IRoute * AODVRouting::createRoute(const Address& destAddr, const Address& nextHop,
        unsigned int hopCount, bool hasValidDestNum, unsigned int destSeqNum,
        bool isActive, simtime_t lifeTime)
{
    IRoute * newRoute = routingTable->createRoute();
    AODVRouteData * newProtocolData = new AODVRouteData();

    newProtocolData->setHasValidDestNum(hasValidDestNum);

    // active route
    newProtocolData->setIsActive(isActive);

    // A route towards a destination that has a routing table entry
    // that is marked as valid.  Only active routes can be used to
    // forward data packets.

    newProtocolData->setLifeTime(lifeTime);
    newProtocolData->setDestSeqNum(destSeqNum);

    InterfaceEntry * ifEntry = interfaceTable->getInterfaceByName("wlan0"); // TODO:
    if (ifEntry)
        newRoute->setInterface(ifEntry);

    newRoute->setDestination(destAddr);
    newRoute->setSourceType(IRoute::AODV);
    newRoute->setSource(this);
    newRoute->setProtocolData(newProtocolData);
    newRoute->setMetric(hopCount);
    newRoute->setNextHop(nextHop);
    newRoute->setPrefixLength(addressType->getMaxPrefixLength()); // TODO:

    routingTable->addRoute(newRoute);
    return newRoute;
}

void AODVRouting::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj)
{
    Enter_Method("receiveChangeNotification");
    if (signalID == NF_LINK_BREAK)
    {
        EV_DETAIL << "Received link break signal" << endl;
        Ieee80211Frame* ieee80211Frame = dynamic_cast<Ieee80211Frame *>(const_cast<cObject*>(obj));
        if (ieee80211Frame)
        {
            INetworkDatagram * datagram = dynamic_cast<INetworkDatagram *>(ieee80211Frame->getEncapsulatedPacket());
            const Address& unreachableAddr = datagram->getDestinationAddress();
            if (unreachableAddr.getAddressType() == addressType)
            {
                // A node initiates processing for a RERR message in three situations:
                //
                //   (i)     if it detects a link break for the next hop of an active
                //           route in its routing table while transmitting data (and
                //           route repair, if attempted, was unsuccessful), or

                //handleLinkBreakSendRERR(unreachableAddr);

            }
        }
    }
}

void AODVRouting::handleLinkBreakSendRERR(const Address& unreachableAddr)
{
    // For case (i), the node first makes a list of unreachable destinations
    // consisting of the unreachable neighbor and any additional
    // destinations (or subnets, see section 7) in the local routing table
    // that use the unreachable neighbor as the next hop.

    // Just before transmitting the RERR, certain updates are made on the
    // routing table that may affect the destination sequence numbers for
    // the unreachable destinations.  For each one of these destinations,
    // the corresponding routing table entry is updated as follows:
    //
    // 1. The destination sequence number of this routing entry, if it
    //    exists and is valid, is incremented for cases (i) and (ii) above,
    //    and copied from the incoming RERR in case (iii) above.
    //
    // 2. The entry is invalidated by marking the route entry as invalid
    //
    // 3. The Lifetime field is updated to current time plus DELETE_PERIOD.
    //    Before this time, the entry SHOULD NOT be deleted.

    std::vector<Address> unreachableNeighbors;
    std::vector<unsigned int> unreachableNeighborsDestSeqNum;

    IRoute * unreachableRoute = routingTable->findBestMatchingRoute(unreachableAddr);
    ASSERT(unreachableRoute != NULL);
    AODVRouteData * unreachableRouteData = dynamic_cast<AODVRouteData *>(unreachableRoute->getProtocolData());
    unreachableNeighbors.push_back(unreachableAddr); // Note that, in spite of this name, we include the unreachableAddr also for the easier implementation
    unreachableNeighborsDestSeqNum.push_back(unreachableRouteData->getDestSeqNum());

    // For case (i), the node first makes a list of unreachable destinations
    // consisting of the unreachable neighbor and any additional destinations
    // (or subnets, see section 7) in the local routing table
    // that use the unreachable neighbor as the next hop.

    for (int i = 0; i < routingTable->getNumRoutes(); i++)
    {
        IRoute * route = routingTable->getRoute(i);

        if (route->getNextHopAsGeneric() == unreachableAddr)
        {
            AODVRouteData * routeData = dynamic_cast<AODVRouteData *>(route->getProtocolData());

            if (routeData->hasValidDestNum())
                routeData->setDestSeqNum(routeData->getDestSeqNum() + 1);

            EV_DETAIL << "Marking route to " << route->getDestinationAsGeneric() << " as inactive" << endl;

            routeData->setIsActive(false);
            routeData->setLifeTime(simTime() + DELETE_PERIOD);

            // Note that, there is a bijection between unreachableNeighbors and unreachableNeighborsDestSeqNum
            unreachableNeighbors.push_back(route->getDestinationAsGeneric());
            unreachableNeighborsDestSeqNum.push_back(routeData->getDestSeqNum());
        }
    }

    AODVRERR * rerr = createRERR(unreachableNeighbors,unreachableNeighborsDestSeqNum);

    // The neighboring node(s) that should receive the RERR are all those
    // that belong to a precursor list of at least one of the unreachable
    // destination(s) in the newly created RERR.  In case there is only one
    // unique neighbor that needs to receive the RERR, the RERR SHOULD be
    // unicast toward that neighbor.  Otherwise the RERR is typically sent
    // to the local broadcast address (Destination IP == 255.255.255.255,
    // TTL == 1) with the unreachable destinations, and their corresponding
    // destination sequence numbers, included in the packet.

    // broadcast
    sendAODVPacket(rerr, addressType->getBroadcastAddress(), 1, 0);
}

AODVRERR* AODVRouting::createRERR(const std::vector<Address>& unreachableNeighbors, const std::vector<unsigned int>& unreachableNeighborsDestSeqNum)
{
    AODVRERR * rerr = new AODVRERR("AODV-RERR");
    unsigned int destCount = unreachableNeighbors.size();

    rerr->setPacketType(RERR);
    rerr->setDestCount(destCount);
    rerr->setUnreachableDestAddrsArraySize(destCount);
    rerr->setUnreachableDestAddrsArraySize(destCount);

    for (unsigned int i = 0; i < destCount; i++)
    {
        rerr->setUnreachableDestAddrs(i, unreachableNeighbors[i]);
        rerr->setUnreachableSeqNum(i, unreachableNeighborsDestSeqNum[i]);
    }

    return rerr;
}

void AODVRouting::handleRERR(AODVRERR* rerr, const Address& sourceAddr)
{
    // A node initiates processing for a RERR message in three situations:
    // (iii)   if it receives a RERR from a neighbor for one or more
    //         active routes.

    unsigned int unreachableArraySize = rerr->getUnreachableDestAddrsArraySize();
    std::vector<Address> unreachableNeighbors;
    std::vector<unsigned int> unreachableNeighborsDestSeqNum;

    for (int i = 0; i < routingTable->getNumRoutes(); i++)
    {
        IRoute * route = routingTable->getRoute(i);
        AODVRouteData * routeData = dynamic_cast<AODVRouteData *>(route);

        // For case (iii), the list should consist of those destinations in the RERR
        // for which there exists a corresponding entry in the local routing
        // table that has the transmitter of the received RERR as the next hop.

        // The RERR should contain those destinations that are part of
        // the created list of unreachable destinations and have a non-empty
        // precursor list.

        if (route->getNextHopAsGeneric() == sourceAddr && routeData->getPrecursorList().size() > 0)
        {
            for (unsigned int j = 0; j < unreachableArraySize; j++)
            {
                if (route->getDestinationAsGeneric() == rerr->getUnreachableDestAddrs(j))
                {
                    // 1. The destination sequence number of this routing entry, if it
                    // exists and is valid, is incremented for cases (i) and (ii) above,
                    // ! and copied from the incoming RERR in case (iii) above.

                    routeData->setDestSeqNum(rerr->getUnreachableSeqNum(j));
                    routeData->setIsActive(false); // it means invalid, see 3. AODV Terminology p.3. in RFC 3561
                    routeData->setLifeTime(simTime() + DELETE_PERIOD);

                    unreachableNeighbors.push_back(route->getDestinationAsGeneric());
                    unreachableNeighborsDestSeqNum.push_back(routeData->getDestSeqNum());

                    break;
                }
            }
        }
    }

    if (unreachableNeighbors.size() > 0 && (simTime() > rebootTime + DELETE_PERIOD || rebootTime == 0))
    {
       AODVRERR * newRERR = createRERR(unreachableNeighbors,unreachableNeighborsDestSeqNum);
       sendAODVPacket(newRERR,addressType->getBroadcastAddress(),1,0);
    }
}


bool AODVRouting::handleOperationStage(LifecycleOperation* operation, int stage, IDoneCallback* doneCallback)
{

    // TODO (localInHook??): If the node
    // receives a data packet for some other destination, it SHOULD
    // broadcast a RERR as described in subsection 6.11 and MUST reset the
    // waiting timer to expire after current time plus DELETE_PERIOD.

    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation))
    {
        if (stage == NodeStartOperation::STAGE_APPLICATION_LAYER)
        {
            isOperational = true;
            rebootTime = simTime();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation))
    {
        if (stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER)
        {
            isOperational = false;
            clearState();
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation))
    {
        if (stage == NodeCrashOperation::STAGE_CRASH)
        {
            isOperational = false;
            clearState();
        }
    }
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());

    return true;
}

void AODVRouting::clearState()
{
    rreqId = sequenceNum = 0;

    for (std::map<Address, WaitForRREP *>::iterator it = waitForRREPTimers.begin(); it != waitForRREPTimers.end(); ++it)
        cancelAndDelete(it->second);

    waitForRREPTimers.clear();
    rreqsArrivalTime.clear();
    cancelEvent(helloMsgTimer);

}

void AODVRouting::handleWaitForRREP(WaitForRREP* rrepTimer)
{
    AODVRREQ * rreq = createRREQ(rrepTimer->getDestAddr());
    sendRREQ(rreq, addressType->getBroadcastAddress(), 0);
}

void AODVRouting::forwardRREP(AODVRREP* rrep, const Address& destAddr, unsigned int timeToLive)
{
    EV_INFO << "Forwarding the Route Reply to the node " << rrep->getOriginatorAddr() << " which originated the Route Request." << endl;
    sendAODVPacket(rrep, destAddr, 100, 0);
}

void AODVRouting::forwardRREQ(AODVRREQ* rreq, unsigned int timeToLive)
{
    double delay = uniform(0,1);
    sendAODVPacket(rreq->dup(), addressType->getBroadcastAddress(), timeToLive, delay);
    delete rreq;
}

void AODVRouting::completeRouteDiscovery(const Address& target)
{
    EV_DETAIL << "Completing route discovery, originator " << getSelfIPAddress() << ", target " << target << endl;
    ASSERT(hasOngoingRouteDiscovery(target));

    std::multimap<Address, INetworkDatagram *>::iterator lt = targetAddressToDelayedPackets.lower_bound(target);
    std::multimap<Address, INetworkDatagram *>::iterator ut = targetAddressToDelayedPackets.upper_bound(target);

    // reinject the delayed datagrams
    for (std::multimap<Address, INetworkDatagram *>::iterator it = lt; it != ut; it++)
    {
        INetworkDatagram * datagram = it->second;
        EV_DETAIL << "Sending queued datagram: source " << datagram->getSourceAddress() << ", destination " << datagram->getDestinationAddress() << endl;
        networkProtocol->reinjectQueuedDatagram(const_cast<const INetworkDatagram *>(datagram));
    }

    // clear the multimap
    targetAddressToDelayedPackets.erase(lt, ut);

    // we have a route for the destination, thus we must cancel the WaitForRREPTimer events
    std::map<Address, WaitForRREP *>::iterator waitRREPIter = waitForRREPTimers.find(target);
    ASSERT(waitRREPIter != waitForRREPTimers.end());
    cancelAndDelete(waitRREPIter->second);
    waitForRREPTimers.erase(waitRREPIter);
}

void AODVRouting::sendGRREP(AODVRREP* grrep, const Address& destAddr, unsigned int timeToLive)
{
    IRoute * destRoute = routingTable->findBestMatchingRoute(destAddr);
    const Address& nextHop = destRoute->getNextHopAsGeneric();

    sendAODVPacket(grrep, nextHop, timeToLive, 0); // TODO: temporary ttl
}

AODVRREP* AODVRouting::createHelloMessage()
{
    // called a Hello message, with the RREP
    // message fields set as follows:
    //
    //    Destination IP Address         The node's IP address.
    //
    //    Destination Sequence Number    The node's latest sequence number.
    //
    //    Hop Count                      0
    //
    //    Lifetime                       ALLOWED_HELLO_LOSS * HELLO_INTERVAL

    AODVRREP * helloMessage = new AODVRREP("AODV-HelloMsg");
    helloMessage->setDestAddr(getSelfIPAddress());
    helloMessage->setDestSeqNum(sequenceNum);
    helloMessage->setHopCount(0);
    helloMessage->setLifeTime(ALLOWED_HELLO_LOSS * HELLO_INTERVAL);

    return helloMessage;
}

void AODVRouting::sendHelloMessagesIfNeeded()
{
    ASSERT(useHelloMessages);

    // Every HELLO_INTERVAL milliseconds, the node checks whether it has
    // sent a broadcast (e.g., a RREQ or an appropriate layer 2 message)
    // within the last HELLO_INTERVAL.  If it has not, it MAY broadcast
    // a RREP with TTL = 1

    if (lastBroadcastTime == 0 || simTime() - lastBroadcastTime > HELLO_INTERVAL)
    {
        AODVRREP * helloMessage = createHelloMessage();
        double delay = uniform(0.0, 1.0);
        sendAODVPacket(helloMessage, addressType->getBroadcastAddress(), 1, delay);
    }
    scheduleAt(simTime() + HELLO_INTERVAL, helloMsgTimer);
}

void AODVRouting::handleHelloMessage(AODVRREP* helloMessage)
{
    const Address& helloOriginatorAddr = helloMessage->getDestAddr();
    IRoute * routeHelloOriginator = routingTable->findBestMatchingRoute(helloOriginatorAddr);

    // Whenever a node receives a Hello message from a neighbor, the node
    // SHOULD make sure that it has an active route to the neighbor, and
    // create one if necessary.  If a route already exists, then the
    // Lifetime for the route should be increased, if necessary, to be at
    // least ALLOWED_HELLO_LOSS * HELLO_INTERVAL.  The route to the
    // neighbor, if it exists, MUST subsequently contain the latest
    // Destination Sequence Number from the Hello message.  The current node
    // can now begin using this route to forward data packets.  Routes that
    // are created by hello messages and not used by any other active routes
    // will have empty precursor lists and would not trigger a RERR message
    // if the neighbor moves away and a neighbor timeout occurs.

    unsigned int latestDestSeqNum = helloMessage->getDestSeqNum();

    if (!routeHelloOriginator || routeHelloOriginator->getSource() != this)
    {
        createRoute(helloOriginatorAddr, helloOriginatorAddr, 1, true, latestDestSeqNum, true, ALLOWED_HELLO_LOSS * HELLO_INTERVAL);
    }
    else
    {
        AODVRouteData * routeData = dynamic_cast<AODVRouteData *>(routeHelloOriginator->getProtocolData());
        simtime_t newLifeTime = routeData->getLifeTime();

        if (newLifeTime < ALLOWED_HELLO_LOSS * HELLO_INTERVAL)
            newLifeTime = ALLOWED_HELLO_LOSS * HELLO_INTERVAL;

        updateRoutingTable(routeHelloOriginator, helloOriginatorAddr, 1, true, latestDestSeqNum, true, newLifeTime);
    }

    // TODO:
    // A node MAY determine connectivity by listening for packets from its
    // set of neighbors.  If, within the past DELETE_PERIOD, it has received
    // a Hello message from a neighbor, and then for that neighbor does not
    // receive any packets (Hello messages or otherwise) for more than
    // ALLOWED_HELLO_LOSS * HELLO_INTERVAL milliseconds, the node SHOULD
    // assume that the link to this neighbor is currently lost.  When this
    // happens, the node SHOULD proceed as in Section 6.11.
}

AODVRouting::~AODVRouting()
{
    clearState();
    delete helloMsgTimer;
}
