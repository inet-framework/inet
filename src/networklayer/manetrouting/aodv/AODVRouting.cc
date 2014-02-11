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
#include "IPSocket.h"
#include "UDPControlInfo.h"
#include "AODVDefs.h"

Define_Module(AODVRouting);

void AODVRouting::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        rreqId = sequenceNum = 0;
        host = this->getParentModule();
        routingTable = check_and_cast<IRoutingTable *>(getModuleByPath(par("routingTablePath")));
        interfaceTable = check_and_cast<IInterfaceTable *>(getModuleByPath(par("interfaceTablePath")));
        networkProtocol = check_and_cast<INetfilter *>(getModuleByPath(par("networkProtocolPath")));
        socket.setOutputGate(gate("udpOut"));
        AodvUDPPort = par("UDPPort");
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS)
    {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(this->getParentModule()->getSubmodule("status"));
        isOperational = !nodeStatus || nodeStatus->getState() == NodeStatus::UP;

        addressType = getSelfIPAddress().getAddressType();
        IPSocket socket(gate("ipOut"));
        socket.registerProtocol(IP_PROT_MANET);
        networkProtocol->registerHook(0, this);
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
    }
    else
    {
        UDPPacket *udpPacket = check_and_cast<UDPPacket *>(msg);
        AODVControlPacket *ctrlPacket = check_and_cast<AODVControlPacket *>(udpPacket->decapsulate());
        unsigned int ctrlPacketType = ctrlPacket->getPacketType();
        INetworkProtocolControlInfo * udpProtocolCtrlInfo = dynamic_cast<INetworkProtocolControlInfo *>(udpPacket->getControlInfo());
        ASSERT(udpProtocolCtrlInfo != NULL);
        Address sourceAddr = udpProtocolCtrlInfo->getSourceAddress();

        switch (ctrlPacketType)
        {
            case RREQ:
                break;
            case RREP:
                handleRREP(check_and_cast<AODVRREP *>(ctrlPacket),sourceAddr);
                break;
            case RERR:
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
    const Address& sourceAddr = datagram->getSourceAddress();
    const Address& destAddr = datagram->getDestinationAddress();

    if (sourceAddr.isBroadcast() || routingTable->isLocalAddress(destAddr))
        return ACCEPT;
    else
    {
        EV_INFO << "Finding route for source " << sourceAddr << " with destination " << destAddr << endl;
        IRoute* route = routingTable->findBestMatchingRoute(destAddr);
        AODVRouteData* routeData = route ? dynamic_cast<AODVRouteData *>(route->getProtocolData()) : NULL;
        bool isValid = routeData->isValid();

        if (route && !route->getNextHopAsGeneric().isUnspecified() && isValid)
        {
            EV_INFO << "Valid route found: " << route << endl;
            if (routeData)
            {
                routeData->setLastUsed(simTime());
            }

            return ACCEPT;
        }
        else
        {
            // A node disseminates a RREQ when it determines that it needs a route
            // to a destination and does not have one available.  This can happen if
            // the destination is previously unknown to the node, or if a previously
            // valid route to the destination expires or is marked as invalid.

            EV_INFO << (isValid ? "Invalid" : "Missing") << " route for source " << sourceAddr << " with destination " << destAddr << endl;
            //delayDatagram(datagram);

            if (!hasOngoingRouteDiscovery(destAddr))
            {
                // When a new route to the same destination is required at a later time
                // (e.g., upon route loss), the TTL in the RREQ IP header is initially
                // set to the Hop Count plus TTL_INCREMENT.
                if (!isValid)
                    startRouteDiscovery(destAddr, route->getMetric() + TTL_INCREMENT);
                else
                    startRouteDiscovery(destAddr);
            }
            else
                EV_DETAIL << "Route discovery is in progress: originator " << getSelfIPAddress() << "target " << destAddr << endl;

            return QUEUE;
        }
    }
}

void AODVRouting::startAODVRouting()
{
    socket.bind(AodvUDPPort); // todo: multicast loop
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
}

bool AODVRouting::hasOngoingRouteDiscovery(const Address& destAddr)
{
    return waitForRREPTimers.find(destAddr) != waitForRREPTimers.end();
}

void AODVRouting::startRouteDiscovery(const Address& destAddr, unsigned timeToLive)
{
    EV_INFO << "Starting route discovery with originator " << getSelfIPAddress() << " and destination " << destAddr << endl;
    ASSERT(!hasOngoingRouteDiscovery(destAddr));

}

Address AODVRouting::getSelfIPAddress()
{
    return routingTable->getRouterIdAsGeneric();
}

void AODVRouting::delayDatagram(INetworkDatagram* datagram)
{

}

void AODVRouting::sendRREQ(AODVRREP * rrep, const Address& destAddr, unsigned int timeToLive)
{
    sendAODVPacket(rrep,destAddr,timeToLive);
}

void AODVRouting::sendRERR()
{

}

void AODVRouting::sendRREP()
{

}

/*
 * RFC 3561: 6.3. Generating Route Requests
 */
AODVRREQ * AODVRouting::createRREQ(const Address& destAddr, unsigned int timeToLive)
{
    AODVRREQ *rreqPacket = new AODVRREQ("ADOV RREQ Control Packet");
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
            rreqPacket->setDestSeqNum(routeData->getDestSeqNum());
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

    // TODO:
    // Before broadcasting the RREQ, the originating node buffers the RREQ
    // ID and the Originator IP address (its own address) of the RREQ for
    // PATH_DISCOVERY_TIME.
    // In this way, when the node receives the packet again from its neighbors,
    // it will not reprocess and re-forward the packet.

    // TODO: G flag

    return rreqPacket;
}

AODVRREP* AODVRouting::createRREP(AODVRREQ * rreq, IRoute * route)
{
    AODVRREP *rrep = new AODVRREP("AODV RREP Control Packet");

    // When generating a RREP message, a node copies the Destination IP
    // Address and the Originator Sequence Number from the RREQ message into
    // the corresponding fields in the RREP message.

    rrep->setDestAddr(rreq->getDestAddr());
    rrep->setDestSeqNum(rreq->getOriginatorSeqNum()); // FIXME

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

        // TODO: The destination node copies the value MY_ROUTE_TIMEOUT
        // into the Lifetime field of the RREP.

    }
    else // intermediate node
    {
        // it copies its known sequence number for the destination into
        // the Destination Sequence Number field in the RREP message.
        AODVRouteData *routeData = dynamic_cast<AODVRouteData *>(route->getProtocolData());
        rrep->setDestSeqNum(routeData->getDestSeqNum());

        // TODO: The intermediate node updates the forward route entry by placing the
        // last hop node (from which it received the RREQ, as indicated by the
        // source IP address field in the IP header) into the precursor list for
        // the forward route entry -- i.e., the entry for the Destination IP
        // Address.

        // The intermediate node places its distance in hops from the
        // destination (indicated by the hop count in the routing table) Count
        // field in the RREP.

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
    AODVRREP *rrep = new AODVRREP("AODV Gratuitous RREP Control Packet");
    AODVRouteData * routeData = dynamic_cast<AODVRouteData *>(route->getProtocolData());

    rrep->setHopCount(route->getMetric());
    rrep->setDestAddr(rreq->getOriginatorAddr());
    rrep->setDestSeqNum(rreq->getOriginatorSeqNum());
    rrep->setOriginatorAddr(rreq->getDestAddr());
    rrep->setLifeTime(routeData->getLifeTime()); // XXX

    return rrep;
}

/*
 * 6.7. Receiving and Forwarding Route Replies
 */
void AODVRouting::handleRREP(AODVRREP* rrep, const Address& nextHop)
{
    // When a node receives a RREP message, it searches (using longest-
    // prefix matching) for a route to the previous hop.

    // TODO: If needed, a route is created for the previous hop,
    // but without a valid sequence number (see section 6.2)

    // Next, the node then increments the hop count value in the RREP by one,
    // to account for the new hop through the intermediate node
    unsigned int newHopCount = rrep->getHopCount() + 1;
    rrep->setHopCount(newHopCount);

    // Then the forward route for this destination is created if it does not
    // already exist.

    IRoute * route = routingTable->findBestMatchingRoute(rrep->getDestAddr());
    AODVRouteData * routeData = dynamic_cast<AODVRouteData *>(route->getProtocolData());
    simtime_t lifeTime = rrep->getLifeTime();
    unsigned destSeqNum = rrep->getDestSeqNum();

    if (route) // already exists
    {

        // Upon comparison, the existing entry is updated only in the following circumstances:

        // (i) the sequence number in the routing table is marked as
        //     invalid in route table entry.

        if (!routeData->hasValidDestNum())
        {
            updateRoutingTable(route, nextHop, newHopCount, true, destSeqNum, false, simTime() + lifeTime);
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
            updateRoutingTable(route, nextHop, newHopCount, true, destSeqNum, false, simTime() + lifeTime);
        }
        else
        {
            // (iii) the sequence numbers are the same, but the route is
            //       marked as inactive, or
            if (destSeqNum == routeData->getDestSeqNum() && routeData->isInactive())
            {
                updateRoutingTable(route, nextHop, newHopCount, true, destSeqNum, false, simTime() + lifeTime);
            }
            // (iv) the sequence numbers are the same, and the New Hop Count is
            //      smaller than the hop count in route table entry.
            else if (destSeqNum == routeData->getDestSeqNum() && rrep->getHopCount() < route->getMetric())
            {
                updateRoutingTable(route, nextHop, newHopCount, true, destSeqNum, false, simTime() + lifeTime);
            }
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
                    // TODO: send RREP-ACK
                    rrep->setAckRequiredFlag(false);
                }

                AODVRouteData * forwardRREPRouteData = dynamic_cast<AODVRouteData *>(route->getProtocolData());

                // Also, at each node the (reverse) route used to forward a
                // RREP has its lifetime changed to be the maximum of (existing-
                // lifetime, (current time + ACTIVE_ROUTE_TIMEOUT).

                simtime_t existingLifeTime = forwardRREPRouteData->getLifeTime();
                forwardRREPRouteData->setLifeTime(std::max(simTime() + ACTIVE_ROUTE_TIMEOUT, existingLifeTime));

                // TODO: forward
            }
            else
                EV_ERROR << "Reverse route doesn't exist. Dropping the RREP message" << endl;

        }
    }
    else // create forward route for the destination: this path will be used by the originator to send data packets
    {
        IRoute * forwardRoute = routingTable->createRoute();
        AODVRouteData * forwardProtocolData = new AODVRouteData();

        forwardProtocolData->setHasValidDestNum(true);
        forwardProtocolData->setIsInactive(false);
        forwardProtocolData->setLifeTime(simTime() + lifeTime);
        forwardProtocolData->setDestSeqNum(destSeqNum);

        forwardRoute->setSourceType(IRoute::AODV);
        forwardRoute->setSource(this);
        forwardRoute->setProtocolData(forwardProtocolData);
        forwardRoute->setMetric(newHopCount);
        forwardRoute->setNextHop(nextHop); // TODO: "which is indicated by the source IP address field in the IP header"

        // TODO:
    }
    // TODO: precursor list
}

void AODVRouting::updateRoutingTable(IRoute * route, const Address& nextHop, unsigned int hopCount, bool hasValidDestNum, unsigned int destSeqNum, bool isInactive, simtime_t lifeTime)
{
    EV_DETAIL << "Updating the Routing Table with ..." << endl;
    route->setNextHop(nextHop);
    route->setMetric(hopCount);
    AODVRouteData * routingData = dynamic_cast<AODVRouteData *>(route->getProtocolData());

    ASSERT(routingData != NULL);

    routingData->setLifeTime(lifeTime);
    routingData->setDestSeqNum(destSeqNum);
    routingData->setIsInactive(isInactive);
    routingData->setHasValidDestNum(hasValidDestNum);
}

void AODVRouting::sendAODVPacket(AODVControlPacket* packet, const Address& destAddr, unsigned int timeToLive)
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

    INetworkProtocolControlInfo * networkProtocolControlInfo = addressType->createNetworkProtocolControlInfo();

    if (packet->getPacketType() == RREQ)
    {
        std::map<Address, WaitForRREP *>::iterator rrepTimer = waitForRREPTimers.find(destAddr);

        if (rrepTimer != waitForRREPTimers.end())
        {
            WaitForRREP * rrepTimerMsg = rrepTimer->second;
            unsigned int lastTTL = rrepTimerMsg->getLastTTL();

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
                networkProtocolControlInfo->setHopLimit(timeToLive);
                rrepTimerMsg->setLastTTL(timeToLive);
                rrepTimerMsg->setFromInvalidEntry(true);
                cancelEvent(rrepTimerMsg);
            }
            else if (lastTTL + TTL_INCREMENT < TTL_THRESHOLD)
            {
                ASSERT(!rrepTimerMsg->isScheduled());
                networkProtocolControlInfo->setHopLimit(lastTTL + TTL_INCREMENT);
                rrepTimerMsg->setLastTTL(lastTTL + TTL_INCREMENT);
            }
            else
            {
                ASSERT(!rrepTimerMsg->isScheduled());
                networkProtocolControlInfo->setHopLimit(NET_DIAMETER);
                rrepTimerMsg->setLastTTL(NET_DIAMETER);
            }

            if (rrepTimerMsg->getLastTTL() == NET_DIAMETER && rrepTimerMsg->getFromInvalidEntry())
                scheduleAt(simTime() + NET_TRAVERSAL_TIME, rrepTimerMsg);
            else
                scheduleAt(simTime() + RING_TRAVERSAL_TIME, rrepTimerMsg);
        }
        else
        {
            WaitForRREP * newRREPTimerMsg = new WaitForRREP();
            waitForRREPTimers[destAddr] = newRREPTimerMsg;
            networkProtocolControlInfo->setHopLimit(TTL_START);
            newRREPTimerMsg->setLastTTL(TTL_START);
            newRREPTimerMsg->setFromInvalidEntry(false);
            // Each time, the timeout for receiving a RREP is RING_TRAVERSAL_TIME.
            scheduleAt(simTime() + RING_TRAVERSAL_TIME, newRREPTimerMsg);

        }

    }

    networkProtocolControlInfo->setTransportProtocol(IP_PROT_MANET);


    networkProtocolControlInfo->setDestinationAddress(destAddr);
    networkProtocolControlInfo->setSourceAddress(getSelfIPAddress());

    UDPPacket * udpPacket = new UDPPacket(packet->getName());
    udpPacket->encapsulate(packet);

    udpPacket->setSourcePort(AodvUDPPort);
    udpPacket->setDestinationPort(AodvUDPPort);
    udpPacket->setControlInfo(dynamic_cast<cObject *>(networkProtocolControlInfo));

    send(udpPacket, "udpOut");
}

AODVRouting::~AODVRouting()
{

}
