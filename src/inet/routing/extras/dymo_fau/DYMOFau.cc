/*
 *  Copyright (C) 2005 Mohamed Louizi
 *  Copyright (C) 2006,2007 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
 *
 * Adapted to ManetRoutingBase by Alfonso Ariza
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/ipv4/IcmpHeader.h"
#include "inet/networklayer/ipv4/Ipv4Route.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/linklayer/common/MacAddressTag_m.h"

#include "inet/routing/extras/dymo_fau/DYMOFau.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/ipv4/Ipv4.h"
#include "inet/routing/extras/base/ControlManetRouting_m.h"

namespace inet {

namespace inetmanet {

Define_Module(DYMOFau);


#define DYMO_PORT 653
namespace
{
const int DYMO_RM_HEADER_LENGTH = 13; /**< length (in bytes) of a DYMO RM header */
const int DYMO_RBLOCK_LENGTH = 10; /**< length (in bytes) of one DYMO RBlock */
const int DYMO_RERR_HEADER_LENGTH = 4; /**< length (in bytes) of a DYMO RERR header */
const int DYMO_UBLOCK_LENGTH = 8; /**< length (in bytes) of one DYMO UBlock */
const int UDPPort = DYMO_PORT; //9000 /**< UDP Port to listen on (TBD) */
const double MAXJITTER = 0.001; /**< all messages sent to a lower layer are delayed by 0..MAXJITTER seconds (draft-ietf-manet-jitter-01) */
}

DYMOFau::DYMOFau()
{
}

void DYMOFau::initialize(int stage)
{
    ManetRoutingBase::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS)
    {
        ownSeqNumLossTimeout = new DYMO_Timer(this, "OwnSeqNumLossTimeout");
        WATCH_PTR(ownSeqNumLossTimeout);
        ownSeqNumLossTimeoutMax = new DYMO_Timer(this, "OwnSeqNumLossTimeoutMax");
        WATCH_PTR(ownSeqNumLossTimeoutMax);
        //TODO assume SeqNum loss when starting up?

        totalPacketsSent = 0;
        WATCH(totalPacketsSent);
        totalBytesSent = 0;
        WATCH(totalBytesSent);

        statsRREQSent = 0; /**< number of generated DYMO RREQs */
        statsRREPSent = 0; /**< number of generated DYMO RREPs */
        statsRERRSent = 0; /**< number of generated DYMO RERRs */

        statsRREQRcvd = 0; /**< number of consumed DYMO RREQs */
        statsRREPRcvd = 0; /**< number of consumed DYMO RREPs */
        statsRERRRcvd = 0; /**< number of consumed DYMO RERRs */

        statsRREQFwd = 0; /**< number of forwarded (and processed) DYMO RREQs */
        statsRREPFwd = 0; /**< number of forwarded (and processed) DYMO RREPs */
        statsRERRFwd = 0; /**< number of forwarded (and processed) DYMO RERRs */

        statsDYMORcvd = 0; /**< number of observed DYMO messages */

        discoveryLatency = 0;
        disSamples = 0;
        dataLatency = 0;
        dataSamples = 0;

        ownSeqNum = 1;

        rblockSize = 13;

        discoveryDelayVec.setName("Discovery delay");
        dataDelayVec.setName("Data delay");
        //dataLoadVec.setName("Data load");
        //controlLoadVec.setName("Control load");

        RESPONSIBLE_ADDRESSES_PREFIX = par("RESPONSIBLE_ADDRESSES_PREFIX");
        // DYMO_INTERFACES=par("DYMO_INTERFACES");
        //AUTOASSIGN_ADDRESS_BASE=IPv4Address(par("AUTOASSIGN_ADDRESS_BASE").stringValue());
        ROUTE_AGE_MIN_TIMEOUT = par("ROUTE_AGE_MIN_TIMEOUT");
        ROUTE_AGE_MAX_TIMEOUT = par("ROUTE_AGE_MAX_TIMEOUT");
        ROUTE_NEW_TIMEOUT = par("ROUTE_NEW_TIMEOUT");
        ROUTE_USED_TIMEOUT = par("ROUTE_USED_TIMEOUT");
        ROUTE_DELETE_TIMEOUT = par("ROUTE_DELETE_TIMEOUT");
        MIN_HOPLIMIT = par("MIN_HOPLIMIT");
        MAX_HOPLIMIT = par("MAX_HOPLIMIT");
        RREQ_RATE_LIMIT = par("RREQ_RATE_LIMIT");
        RREQ_BURST_LIMIT = par("RREQ_BURST_LIMIT");
        RREQ_WAIT_TIME = par("RREQ_WAIT_TIME");
        RREQ_TRIES = par("RREQ_TRIES");
        BUFFER_SIZE_PACKETS = par("BUFFER_SIZE_PACKETS");
        BUFFER_SIZE_BYTES = par("BUFFER_SIZE_BYTES");

        // myAddr = AUTOASSIGN_ADDRESS_BASE.getInt() + uint32(getParentModule()->getId());

        rateLimiterRREQ = new DYMO_TokenBucket(RREQ_RATE_LIMIT, RREQ_BURST_LIMIT, simTime());

        myAddr = getAddress();
        dymo_routingTable = new DYMO_RoutingTable(this, myAddr);
        WATCH_PTR(dymo_routingTable);

        outstandingRREQList.delAll();
        WATCH_OBJ(outstandingRREQList);
        queuedDataPackets = new DYMO_DataQueue(this, BUFFER_SIZE_PACKETS, BUFFER_SIZE_BYTES);
        WATCH_PTR(queuedDataPackets);

        registerRoutingModule();
        if (!isInMacLayer())
            registerHook();

        // setSendToICMP(true);
        linkLayerFeeback();
        timerMsg = new cMessage("DYMO_scheduler");
    }
}

void DYMOFau::finish()
{
    recordScalar("totalPacketsSent", totalPacketsSent);
    recordScalar("totalBytesSent", totalBytesSent);

    recordScalar("DYMO_RREQSent", statsRREQSent);
    recordScalar("DYMO_RREPSent", statsRREPSent);
    recordScalar("DYMO_RERRSent", statsRERRSent);

    recordScalar("DYMO_RREQRcvd", statsRREQRcvd);
    recordScalar("DYMO_RREPRcvd", statsRREPRcvd);
    recordScalar("DYMO_RERRRcvd", statsRERRRcvd);

    recordScalar("DYMO_RREQFwd", statsRREQFwd);
    recordScalar("DYMO_RREPFwd", statsRREPFwd);
    recordScalar("DYMO_RERRFwd", statsRERRFwd);

    recordScalar("DYMO_DYMORcvd", statsDYMORcvd);

    if (discoveryLatency > 0 && disSamples > 0)
        recordScalar("discovery latency", discoveryLatency/disSamples);
    if (dataLatency > 0 && dataSamples > 0)
        recordScalar("data latency", dataLatency/dataSamples);

    delete dymo_routingTable;
    dymo_routingTable = nullptr;

    outstandingRREQList.delAll();

    delete ownSeqNumLossTimeout;
    ownSeqNumLossTimeout = nullptr;
    delete ownSeqNumLossTimeoutMax;
    ownSeqNumLossTimeoutMax = nullptr;

    delete rateLimiterRREQ;
    rateLimiterRREQ = nullptr;

    // IPv4* ipLayer = queuedDataPackets->getIpLayer();
    delete queuedDataPackets;
    queuedDataPackets = nullptr;
    // ipLayer->unregisterHook(0, this);

    cancelAndDelete(timerMsg);
    timerMsg = nullptr;
}

DYMOFau::~DYMOFau()
{
    delete dymo_routingTable;

    outstandingRREQList.delAll();

    delete ownSeqNumLossTimeout;
    delete ownSeqNumLossTimeoutMax;

    delete rateLimiterRREQ;

    // IPv4* ipLayer = queuedDataPackets->getIpLayer();
    delete queuedDataPackets;

    cancelAndDelete(timerMsg);
}

void DYMOFau::rescheduleTimer()
{
    if (!timerMsg->isScheduled())
        scheduleAt(simTime()+1.0, timerMsg);
}

void DYMOFau::socketDataArrived(UdpSocket *socket, Packet *pkt)
{
    const auto & chunk = pkt->peekAtFront<Chunk>();
    L3Address srcAddr;
    L3Address destAddr;


    bool isDymo = false;

    if (dynamicPtrCast<const DYMO_PacketBBMessage>(chunk))
        isDymo = true;
    else if (dynamicPtrCast<const DYMO_Packet>(chunk))
        isDymo = true;

    if (!isDymo) {
        delete pkt;
        scheduleEvent();
        return;
    }

    if (!isInMacLayer()) {
        if (!isInMacLayer()) {
            srcAddr = pkt->getTag<L3AddressInd>()->getSrcAddress();
            // aodvMsg->setControlInfo(check_and_cast<cObject *>(controlInfo));
        }
        else {
            srcAddr = L3Address(pkt->getTag<MacAddressInd>()->getSrcAddress());
        }
    }

    if (isLocalAddress(srcAddr))
        delete pkt;
    else
        handleLowerMsg(pkt);

    scheduleEvent();
}

void DYMOFau::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

INetfilter::IHook::Result DYMOFau::ensureRouteForDatagram(Packet *datagram)
{

    if (isInMacLayer()) {
        // TODO: MAC layer
        throw cRuntimeError("Error AODV-uu HOOK");
        return ACCEPT;
    }

    const auto& networkHeader = getNetworkProtocolHeader(datagram);
    const L3Address& destAddr = networkHeader->getDestinationAddress();
    const L3Address& sourceAddr = networkHeader->getSourceAddress();

    if (destAddr.isBroadcast() || isLocalAddress(destAddr) || destAddr.isMulticast())
        return ACCEPT;

    INetfilter::IHook::Result res;
    IRoute *route = getInetRoutingTable()->findBestMatchingRoute(destAddr);
    if (route) {
        if (!sourceAddr.isBroadcast() && !sourceAddr.isMulticast() && sourceAddr.isUnspecified())
            updateRouteLifetimes(sourceAddr);
        if (!destAddr.isBroadcast() && !destAddr.isMulticast() && !destAddr.isUnspecified())
            updateRouteLifetimes(destAddr);
        return ACCEPT;
    }
    else
        res = processPacket(datagram);   // Data path
    scheduleEvent();
    return res;

}

INetfilter::IHook::Result DYMOFau::processPacket(const Packet* datagram)
{
    Enter_Method("procces ip Packet (%s)", datagram->getName());

    const auto& networkHeader = getNetworkProtocolHeader(const_cast<Packet *>(datagram));
    const L3Address& destAddr = networkHeader->getDestinationAddress();
    const L3Address& sourceAddr = networkHeader->getSourceAddress();

    int TargetSeqNum = 0;
    int TargetHopCount = 0;

    // look up routing table entry for packet destination
    DYMO_RoutingEntry* entry = dymo_routingTable->getForAddress(destAddr);
    if (entry)
    {
        // if a valid route exists, signal the queue to send all packets stored for this destination
        if (!entry->routeBroken)
        {
            //TODO: mark route as used when forwarding data packets? Draft says yes, but as we are using Route Timeout to "detect" link breaks, this seems to be a bad idea
            // update routes to destination
            // send queued packets
            throw cRuntimeError("Dymo has a valid entry route but ip doesn't have a entry route");
            //delete datagram;
            //return;
        }
        TargetSeqNum = entry->routeSeqNum;
        TargetHopCount = entry->routeDist;
    }

    if (!sourceAddr.isUnspecified() && !isLocalAddress(sourceAddr))
    {
        // It's not a packet of this node, send error to source
        sendRERR(destAddr, TargetSeqNum);
        return INetfilter::IHook::DROP;
    }

    // no route in the table found -> route discovery (if none is already underway)
    if (!outstandingRREQList.getByDestAddr(destAddr, 32))
    {
        sendRREQ(destAddr, MIN_HOPLIMIT, TargetSeqNum, TargetHopCount);
        /** queue the RREQ in a list and schedule a timeout in order to resend the RREQ when no RREP is received **/

        DYMO_OutstandingRREQ* outstandingRREQ = new DYMO_OutstandingRREQ;
        outstandingRREQ->tries = 1;
        outstandingRREQ->wait_time = new DYMO_Timer(this, "RREQ wait time");
        outstandingRREQ->wait_time->start(RREQ_WAIT_TIME);
        outstandingRREQ->destAddr = destAddr;
        outstandingRREQ->creationTime = simTime();
        outstandingRREQList.add(outstandingRREQ);
        rescheduleTimer();
    }

    queuedDataPackets->queuePacket(const_cast<Packet *>(datagram));
    return INetfilter::IHook::QUEUE;
}


void DYMOFau::handleLowerMsg(Packet* apMsg)
{
    /**
     * check the type of received message
     1) Routing Message: RREQ or RREP
     2) Error Message: RERR
     3) Unsupported Message: UERR
     4) Data Message
     **/
    const auto &chunk = apMsg->peekAtFront<Chunk>();
    if (dynamicPtrCast<const DYMO_RM>(chunk)) handleLowerRM(apMsg);
    else if (dynamicPtrCast<const DYMO_RERR>(chunk)) handleLowerRERR(apMsg);
    else if (dynamicPtrCast<const DYMO_UERR>(chunk)) handleLowerUERR(apMsg);
    else if (apMsg->getKind() == UDP_I_ERROR) { EV_INFO << "discarded UDP error message" << endl; delete apMsg; }
    else throw cRuntimeError("message is no DYMO Packet");
}

void DYMOFau::handleLowerRM(Packet *pkt)
{
    /** message is a routing message **/
    EV_INFO << "received message is a routing message" << endl;

    statsDYMORcvd++;


    /** routing message  preprocessing and updating routes from routing blocks **/
    if (updateRoutes(pkt) == nullptr)
    {
        EV_INFO << "dropping received message" << endl;
        delete pkt;
        return;
    }

    const auto &routingMsg = pkt->peekAtFront<DYMO_RM>();

    /**
     * received message is a routing message.
     * check if the node is the destination
     * 1) YES - if the RM is a RREQ, then send a RREP to source
     * 2) NO - send message down to next node.
     **/
    if (myAddr == routingMsg->getTargetNode().getAddress())
    {
        handleLowerRMForMe(pkt);
        return;
    }
    else
    {
        handleLowerRMForRelay(pkt);
        return;
    }
}

L3Address DYMOFau::getNextHopAddress(Packet *pkt)
{
    const auto& routingMsg = pkt->peekAtFront<DYMO_RM>();
    if (routingMsg->getAdditionalNodes().size() > 0)
    {
        return routingMsg->getAdditionalNodes().back().getAddress();
    }
    else
    {
        return routingMsg->getOrigNode().getAddress();
    }
}

InterfaceEntry* DYMOFau::getNextHopInterface(Packet* pktAux)
{

    const auto & pkt = pktAux->peekAtFront<DYMO_PacketBBMessage>();

    if (pkt == nullptr) throw cRuntimeError("getNextHopInterface called with nullptr packet");

    // get interface
    auto interfaceInd = pktAux->findTag<InterfaceInd>();
    if (nullptr == interfaceInd) throw cRuntimeError("received packet did not have InterfaceInd attached");

    int interfaceId = interfaceInd->getInterfaceId();
    if (interfaceId == -1) throw cRuntimeError("received packet's UDPControlInfo did not have information on interfaceId");

    InterfaceEntry* srcIf = nullptr;

    for (int i = 0; i < getNumWlanInterfaces(); i++)
    {
        InterfaceEntry *ie = getWlanInterfaceEntry(i);
        if (interfaceId == ie->getInterfaceId())
        {
            srcIf = ie;
            break;
        }
    }
    if (!srcIf) throw cRuntimeError("parent module interface table did not contain interface on which packet arrived");
    return srcIf;
}

void DYMOFau::handleLowerRMForMe(Packet *pkt)
{
    const auto &routingMsg = pkt->peekAtFront<DYMO_RM>();

    /** current node is the target **/
    if (dynamicPtrCast<const DYMO_RREQ>(routingMsg))
    {
        /** received message is a RREQ -> send a RREP to source **/
        sendReply(routingMsg->getOrigNode().getAddress(), (routingMsg->getTargetNode().hasSeqNum() ? routingMsg->getTargetNode().getSeqNum() : 0));
        statsRREQRcvd++;
        delete pkt;
    }
    else if (dynamicPtrCast<const DYMO_RREP>(routingMsg))
    {
        /** received message is a RREP **/
        statsRREPRcvd++;

        // signal the queue to dequeue waiting messages for this destination
        checkAndSendQueuedPkts(routingMsg->getOrigNode().getAddress(), (routingMsg->getOrigNode().hasPrefix() ? routingMsg->getOrigNode().getPrefix() : 32), getNextHopAddress(pkt));

        delete pkt;
    }
    else throw cRuntimeError("received unknown dymo message");
}

void DYMOFau::handleLowerRMForRelay(Packet *pkt)
{
    /** current node is not the message destination -> find route to destination **/
    EV_INFO << "current node is not the message destination -> find route to destination" << endl;

    const auto & routingMsg = pkt->peekAtFront<DYMO_RM>();

    L3Address targetAddr = routingMsg->getTargetNode().getAddress();
    unsigned int targetSeqNum = 0;

    // stores route entry of route to destination if a route exists, 0 otherwise
    DYMO_RoutingEntry* entry = dymo_routingTable->getForAddress(targetAddr);
    if (entry)
    {
        targetSeqNum = entry->routeSeqNum;

        //TODO: mark route as used when forwarding DYMO packets?
        //entry->routeUsed.start(ROUTE_USED_TIMEOUT);
        //entry->routeDelete.cancel();

        if (entry->routeBroken) entry = nullptr;
    }

    /** received routing message is an RREP and no routing entry was found **/
    if (dynamicPtrCast<const DYMO_RREP>(routingMsg) && (!entry))
    {
        /* do nothing, just drop the RREP */
        EV_INFO << "no route to destination of RREP was found. Sending RERR and dropping message." << endl;
        sendRERR(targetAddr, targetSeqNum);
        delete pkt;
        return;
    }

    // check if received message is a RREQ and a routing entry was found
    if (dynamicPtrCast<const DYMO_RREQ>(routingMsg) && (entry) && (routingMsg->getTargetNode().hasSeqNum()) && (!seqNumIsFresher(routingMsg->getTargetNode().getSeqNum(), entry->routeSeqNum)))
    {
        // yes, we can. Do intermediate DYMO router RREP creation
        EV_INFO << "route to destination of RREQ was found. Sending intermediate DYMO router RREP" << endl;
        sendReplyAsIntermediateRouter(routingMsg->getOrigNode(), routingMsg->getTargetNode(), entry);
        statsRREQRcvd++;
        delete pkt;
        return;
    }

    /** check whether a RREQ was sent to discover route to destination **/
    EV_INFO << "received message is a RREQ" << endl;
    EV_INFO << "trying to discover route to node " << targetAddr << endl;

    /** increment distance metric of existing AddressBlocks */
    std::vector<DYMO_AddressBlock> additional_nodes = routingMsg->getAdditionalNodes();
    std::vector<DYMO_AddressBlock> additional_nodes_to_relay;
    if (routingMsg->getOrigNode().hasDist() && (routingMsg->getOrigNode().getDist() >= 0xFF - 1))
    {
        EV_INFO << "passing on this message would overflow OrigNode.Dist -> dropping message" << endl;
        delete pkt;
        return;
    }

    auto routingMsgAux = pkt->removeAtFront<DYMO_RM>();
    auto origNode = routingMsgAux->getOrigNode();
    origNode.incrementDistIfAvailable();
    routingMsgAux->setOrigNode(origNode);

    for (auto & additional_node : additional_nodes)
    {
        if (additional_node.hasDist() && (additional_node.getDist() >= 0xFF - 1))
        {
            EV_INFO << "passing on additionalNode would overflow OrigNode.Dist -> dropping additionalNode" << endl;
            continue;
        }
        additional_node.incrementDistIfAvailable();
        additional_nodes_to_relay.push_back(additional_node);
    }

    // append additional routing information about this node
    DYMO_AddressBlock additional_node;
    additional_node.setDist(0);
    additional_node.setAddress(myAddr);
    if (RESPONSIBLE_ADDRESSES_PREFIX != -1) additional_node.setPrefix(RESPONSIBLE_ADDRESSES_PREFIX);
    incSeqNum();
    additional_node.setSeqNum(ownSeqNum);
    additional_nodes_to_relay.push_back(additional_node);

    routingMsgAux->setAdditionalNodes(additional_nodes_to_relay);
    routingMsgAux->setMsgHdrHopLimit(routingMsg->getMsgHdrHopLimit() - 1);

    int hopLimit = routingMsgAux->getMsgHdrHopLimit();
    pkt->insertAtFront(routingMsgAux);

    // check hop limit
    if (hopLimit < 1)
    {
        EV_INFO << "received message has reached hop limit -> delete message" << endl;
        delete pkt;
        return;
    }

    // do not transmit DYMO messages when we lost our sequence number
    if (ownSeqNumLossTimeout->isRunning())
    {
        EV_INFO << "node has lost sequence number -> not transmitting anything" << endl;
        delete pkt;
        return;
    }

    // do rate limiting
    if ((dynamicPtrCast<const DYMO_RREQ>(routingMsg)) && (!rateLimiterRREQ->consumeTokens(1, simTime())))
    {
        EV_INFO << "RREQ send rate exceeded maximum -> not transmitting RREQ" << endl;
        delete pkt;
        return;
    }

    /* transmit message -- RREP via unicast, RREQ via DYMOcast */
    sendDown(pkt, dynamicPtrCast<const DYMO_RREP>(routingMsg) ? (entry->routeNextHopAddress) : L3Address(Ipv4Address::LL_MANET_ROUTERS));

    /* keep statistics */
    if (dynamicPtrCast<const DYMO_RREP>(routingMsg))
    {
        statsRREPFwd++;
    }
    else
    {
        statsRREQFwd++;
    }
}

void DYMOFau::handleLowerRERR(Packet *pkt)
{

    // get RERR's SourceInterface
    InterfaceEntry* sourceInterface = getNextHopInterface(pkt);

    /** message is a RERR. **/
    auto my_rerr = pkt->removeAtFront<DYMO_RERR>();
    statsDYMORcvd++;

    auto l3AddressInd = pkt->getTag<L3AddressInd>();
    // get RERR's IPv4.SourceAddress

    L3Address sourceAddr = l3AddressInd->getSrcAddress();


    EV_INFO << "Received RERR from " << sourceAddr << endl;

    // iterate over all unreachableNode entries
    std::vector<DYMO_AddressBlock> unreachableNodes = my_rerr->getUnreachableNodes();
    std::vector<DYMO_AddressBlock> unreachableNodesToForward;
    for (auto & unreachableNodes_i : unreachableNodes)
    {
        const DYMO_AddressBlock& unreachableNode = unreachableNodes_i;

        if (unreachableNode.getAddress().isMulticast()) continue;

        // check whether this invalidates entries in our routing table
        std::vector<DYMO_RoutingEntry *> RouteVector = dymo_routingTable->getRoutingTable();
        for (auto & elem : RouteVector)
        {
            DYMO_RoutingEntry* entry = elem;

            // skip if route has no associated Forwarding Route
            if (entry->routeBroken) continue;

            // skip if this route isn't to the unreachableNode Address mentioned in the RERR
            if (!entry->routeAddress.toIpv4().prefixMatches(unreachableNode.getAddress().toIpv4(), entry->routePrefix)) continue;

            // skip if route entry isn't via the RERR sender
            if (entry->routeNextHopAddress != sourceAddr) continue;
            if (entry->routeNextHopInterface != sourceInterface) continue;

            // skip if route entry is fresher
            if (!((entry->routeSeqNum == 0) || (!unreachableNode.hasSeqNum()) || (!seqNumIsFresher(entry->routeSeqNum, unreachableNode.getSeqNum())))) continue;

            EV_DETAIL << "RERR invalidates route to " << entry->routeAddress << " via " << entry->routeNextHopAddress << endl;

            // mark as broken and delete associated forwarding route
            entry->routeBroken = true;
            dymo_routingTable->maintainAssociatedRoutingTable();

            // start delete timer
            // TODO: not specified in draft, but seems to make sense
            entry->routeDelete.start(ROUTE_DELETE_TIMEOUT);
            rescheduleTimer();

            // update unreachableNode.SeqNum
            // TODO: not specified in draft, but seems to make sense
            DYMO_AddressBlock unreachableNodeToForward;
            unreachableNodeToForward.setAddress(unreachableNode.getAddress());
            if (unreachableNode.hasSeqNum()) unreachableNodeToForward.setSeqNum(unreachableNode.getSeqNum());
            if (entry->routeSeqNum != 0) unreachableNodeToForward.setSeqNum(entry->routeSeqNum);

            // forward this unreachableNode entry
            unreachableNodesToForward.push_back(unreachableNodeToForward);
        }
    }


    // discard RERR if there are no entries to forward
    if (unreachableNodesToForward.size() <= 0)
    {
        statsRERRRcvd++;
        delete pkt;
        return;
    }

    // discard RERR if ownSeqNum was lost
    if (ownSeqNumLossTimeout->isRunning())
    {
        statsRERRRcvd++;
        delete pkt;
        return;
    }

    // discard RERR if msgHdrHopLimit has reached 1
    if (my_rerr->getMsgHdrHopLimit() <= 1)
    {
        statsRERRRcvd++;
        delete pkt;
        return;
    }

    // forward RERR with unreachableNodesToForward
    my_rerr->setUnreachableNodes(unreachableNodesToForward);
    my_rerr->setMsgHdrHopLimit(my_rerr->getMsgHdrHopLimit() - 1);

    pkt->insertAtFront(my_rerr);
    EV_INFO << "send down RERR" << endl;
    sendDown(pkt, L3Address(Ipv4Address::LL_MANET_ROUTERS));

    statsRERRFwd++;
}

void DYMOFau::handleLowerUERR(Packet *pkt)
{
    /** message is a UERR. **/
    statsDYMORcvd++;

    // DYMO_UERR *my_uerr

    EV_INFO << "Received unsupported UERR message" << endl;
    // to be finished
    delete pkt;
}

void DYMOFau::handleSelfMsg(cMessage* apMsg)
{
    EV_DEBUG << "handle self message" << endl;
    if (apMsg == timerMsg)
    {
        bool hasActive = false;

        // Something timed out. Let's find out what.

        // Maybe it's a ownSeqNumLossTimeout
        if (ownSeqNumLossTimeout->stopWhenExpired() || ownSeqNumLossTimeoutMax->stopWhenExpired())
        {
            ownSeqNumLossTimeout->cancel();
            ownSeqNumLossTimeoutMax->cancel();
            ownSeqNum = 1;
        }
        hasActive = ownSeqNumLossTimeout->isActive() || ownSeqNumLossTimeoutMax->isActive();

        // Maybe it's a outstanding RREQ
        DYMO_OutstandingRREQ* outstandingRREQ;
        while ((outstandingRREQ = outstandingRREQList.getExpired()) != nullptr )
            handleRREQTimeout(*outstandingRREQ);

        if (!hasActive)
            hasActive = outstandingRREQList.hasActive();

        // Maybe it's a DYMO_RoutingEntry
        for (int i = 0; i < dymo_routingTable->getNumRoutes(); )
        {
            DYMO_RoutingEntry *entry = dymo_routingTable->getRoute(i);
            bool deleted = false;

            entry->routeAgeMin.stopWhenExpired();

            if (entry->routeAgeMax.stopWhenExpired())
            {
                dymo_routingTable->deleteRoute(entry);
                // if other timeouts also expired, they will have gotten their own DYMO_Timeout scheduled, so it's ok to stop here
                deleted = true;
            }
            else
            {
                bool routeNewStopped = entry->routeNew.stopWhenExpired();
                bool routeUsedStopped = entry->routeUsed.stopWhenExpired();

                if ((routeNewStopped || routeUsedStopped) && !(entry->routeUsed.isRunning() || entry->routeNew.isRunning()))
                    entry->routeDelete.start(ROUTE_DELETE_TIMEOUT);

                if (entry->routeDelete.stopWhenExpired())
                {
                    dymo_routingTable->deleteRoute(entry);
                    deleted = true;
                }
            }

            if (!deleted)
            {
                if (!hasActive)
                    hasActive = entry->hasActiveTimer();
                i++;
            }
        }
        if (hasActive)
            rescheduleTimer();
    }
    else throw cRuntimeError("unknown message type");
}

void DYMOFau::sendDown(Packet* apMsg, L3Address destAddr)
{
    // all messages sent to a lower layer are delayed by 0..MAXJITTER seconds (draft-ietf-manet-jitter-01)
    simtime_t jitter = dblrand() * MAXJITTER;

    // set byte size of message
    auto chunk = apMsg->removeAtFront<FieldsChunk>();
    auto re = dynamicPtrCast<DYMO_RM>(chunk);
    auto rerr = dynamicPtrCast<DYMO_RERR>(chunk);
    if (re)
    {
        chunk->setChunkLength(B(DYMO_RM_HEADER_LENGTH + ((1 + re->getAdditionalNodes().size()) * DYMO_RBLOCK_LENGTH)));
    }
    else if (rerr)
    {
        chunk->setChunkLength(B(DYMO_RERR_HEADER_LENGTH + (rerr->getUnreachableNodes().size() * DYMO_UBLOCK_LENGTH)));
    }
    else
    {
        throw cRuntimeError("tried to send unsupported message type");
    }
    apMsg->insertAtFront(chunk);

    // keep statistics
    totalPacketsSent++;
    totalBytesSent += apMsg->getByteLength();
    if (L3Address(Ipv4Address::LL_MANET_ROUTERS) == destAddr)
    {
        sendToIp(apMsg, UDPPort, destAddr, UDPPort, 1, SIMTIME_DBL(jitter));
    }
    else
    {
        sendToIp(apMsg, UDPPort, destAddr, UDPPort, 1, 0.0);
    }
}

void DYMOFau::sendRREQ(L3Address destAddr, int msgHdrHopLimit, unsigned int targetSeqNum, unsigned int targetDist)
{
    /** generate a new RREQ with the given pararmeter **/
    EV_INFO << "send a RREQ to discover route to destination node " << destAddr << endl;

    // do not transmit DYMO messages when we lost our sequence number
    if (ownSeqNumLossTimeout->isRunning())
    {
        EV_INFO << "node has lost sequence number -> not transmitting RREQ" << endl;
        return;
    }

    // do rate limiting
    if (!rateLimiterRREQ->consumeTokens(1, simTime()))
    {
        EV_INFO << "RREQ send rate exceeded maximum -> not transmitting RREQ" << endl;
        return;
    }


    auto my_rreq = makeShared<DYMO_RREQ>(); //("RREQ");

    my_rreq->setMsgHdrHopLimit(msgHdrHopLimit);
    auto target = my_rreq->getTargetNode();
    target.setAddress(destAddr);
    if (targetSeqNum != 0) target.setSeqNum(targetSeqNum);
    if (targetDist != 0) target.setDist(targetDist);

    my_rreq->setTargetNode(target);

    auto orig = my_rreq->getOrigNode();
    orig.setDist(0);
    orig.setAddress(myAddr);
    if (RESPONSIBLE_ADDRESSES_PREFIX != -1) orig.setPrefix(RESPONSIBLE_ADDRESSES_PREFIX);
    incSeqNum();
    orig.setSeqNum(ownSeqNum);
    my_rreq->setOrigNode(orig);

    my_rreq->setChunkLength(B(DYMO_RM_HEADER_LENGTH + ((1 + my_rreq->getAdditionalNodes().size()) * DYMO_RBLOCK_LENGTH)));

    auto pkt = new Packet("RREQ");
    pkt->insertAtFront(my_rreq);

    sendDown(pkt, L3Address(Ipv4Address::LL_MANET_ROUTERS));
    statsRREQSent++;
}

void DYMOFau::sendReply(L3Address destAddr, unsigned int tSeqNum)
{
    /** create a new RREP and send it to given destination **/
    EV_INFO << "send a reply to destination node " << destAddr << endl;

    if (ownSeqNumLossTimeout->isRunning())
    {
        EV_WARN << "node has lost sequence number -> not transmitting anything" << endl;
        return;
    }


    auto rrep = makeShared<DYMO_RREP>();
    DYMO_RoutingEntry *entry = dymo_routingTable->getForAddress(destAddr);
    if (!entry) throw cRuntimeError("Tried sending RREP via a route that just vanished");

    rrep->setMsgHdrHopLimit(MAX_HOPLIMIT);
    auto target = rrep->getTargetNode();
    target.setAddress(destAddr);
    target.setSeqNum(entry->routeSeqNum);
    target.setDist(entry->routeDist);
    rrep->setTargetNode(target);

    // check if ownSeqNum should be incremented
    if ((tSeqNum == 0) || (seqNumIsFresher(ownSeqNum, tSeqNum))) incSeqNum();

    auto orig =  rrep->getOrigNode();
    orig.setAddress(myAddr);
    if (RESPONSIBLE_ADDRESSES_PREFIX != -1)
        orig.setPrefix(RESPONSIBLE_ADDRESSES_PREFIX);
    orig.setSeqNum(ownSeqNum);
    orig.setDist(0);
    rrep->setOrigNode(orig);

    // do not transmit DYMO messages when we lost our sequence number
    rrep->setChunkLength(B(DYMO_RM_HEADER_LENGTH + ((1 + rrep->getAdditionalNodes().size()) * DYMO_RBLOCK_LENGTH)));

    auto pkt = new Packet("RREP");
    pkt->insertAtFront(rrep);

    sendDown(pkt, entry->routeNextHopAddress);

    statsRREPSent++;
}

void DYMOFau::sendReplyAsIntermediateRouter(const DYMO_AddressBlock& origNode, const DYMO_AddressBlock& targetNode, const DYMO_RoutingEntry* routeToTargetNode)
{
    /** create a new RREP and send it to given destination **/
    EV_INFO << "sending a reply to OrigNode " << origNode.getAddress() << endl;

    // do not transmit DYMO messages when we lost our sequence number
    if (ownSeqNumLossTimeout->isRunning())
    {
        EV_WARN << "node has lost sequence number -> not transmitting anything" << endl;
        return;
    }

    DYMO_RoutingEntry* routeToOrigNode = dymo_routingTable->getForAddress(origNode.getAddress());
    if (!routeToOrigNode) throw cRuntimeError("no route to OrigNode found");

    // increment ownSeqNum.
    // TODO: The draft is unclear about when to increment ownSeqNum for intermediate DYMO router RREP creation
    incSeqNum();

    // create rrepToOrigNode
    auto rrepToOrigNode = makeShared<DYMO_RREP>();
    rrepToOrigNode->setMsgHdrHopLimit(MAX_HOPLIMIT);


    auto targetToOrigNode = rrepToOrigNode->getTargetNode();
    targetToOrigNode.setAddress(origNode.getAddress());
    targetToOrigNode.setSeqNum(origNode.getSeqNum());
    if (origNode.hasDist()) targetToOrigNode.setDist(origNode.getDist() + 1);
    targetToOrigNode.setAddress(myAddr);
    if (RESPONSIBLE_ADDRESSES_PREFIX != -1) targetToOrigNode.setPrefix(RESPONSIBLE_ADDRESSES_PREFIX);
    targetToOrigNode.setSeqNum(ownSeqNum);
    targetToOrigNode.setDist(0);
    rrepToOrigNode->setTargetNode(targetToOrigNode);

    DYMO_AddressBlock additionalNode;
    additionalNode.setAddress(routeToTargetNode->routeAddress);
    if (routeToTargetNode->routeSeqNum != 0) additionalNode.setSeqNum(routeToTargetNode->routeSeqNum);
    if (routeToTargetNode->routeDist != 0) additionalNode.setDist(routeToTargetNode->routeDist);
    auto aditionalNodes = rrepToOrigNode->getAdditionalNodes();
    aditionalNodes.push_back(additionalNode);
    rrepToOrigNode->setAdditionalNodes(aditionalNodes);





    // create rrepToTargetNode
    auto rrepToTargetNode = makeShared<DYMO_RREP>();
    rrepToTargetNode->setMsgHdrHopLimit(MAX_HOPLIMIT);

    auto targetToNode = rrepToTargetNode->getTargetNode();
    targetToNode.setAddress(targetNode.getAddress());
    if (targetNode.hasSeqNum()) targetToNode.setSeqNum(targetNode.getSeqNum());
    if (targetNode.hasDist()) targetToNode.setDist(targetNode.getDist());

    targetToNode.setAddress(myAddr);
    if (RESPONSIBLE_ADDRESSES_PREFIX != -1) targetToNode.setPrefix(RESPONSIBLE_ADDRESSES_PREFIX);
    targetToNode.setSeqNum(ownSeqNum);
    targetToNode.setDist(0);
    rrepToTargetNode->setTargetNode(targetToNode);

    DYMO_AddressBlock additionalNode2;
    additionalNode2.setAddress(origNode.getAddress());
    additionalNode2.setSeqNum(origNode.getSeqNum());
    if (origNode.hasDist()) additionalNode2.setDist(origNode.getDist() + 1);
    aditionalNodes = rrepToTargetNode->getAdditionalNodes();
    aditionalNodes.push_back(additionalNode2);
    rrepToTargetNode->setAdditionalNodes(aditionalNodes);


    rrepToOrigNode->setChunkLength(B(DYMO_RM_HEADER_LENGTH + ((1 + rrepToOrigNode->getAdditionalNodes().size()) * DYMO_RBLOCK_LENGTH)));
    rrepToTargetNode->setChunkLength(B(DYMO_RM_HEADER_LENGTH + ((1 + rrepToTargetNode->getAdditionalNodes().size()) * DYMO_RBLOCK_LENGTH)));

    auto pktToOrigin = new Packet("RREP");
    auto pktToTarget = new Packet("RREP");
    pktToOrigin->insertAtFront(rrepToOrigNode);
    pktToTarget->insertAtFront(rrepToTargetNode);

    sendDown(pktToOrigin, routeToOrigNode->routeNextHopAddress);
    sendDown(pktToTarget, routeToTargetNode->routeNextHopAddress);

    statsRREPSent++;
}

void DYMOFau::sendRERR(L3Address targetAddr, unsigned int targetSeqNum)
{
    EV_INFO << "generating an RERR" << endl;
    auto rerr = makeShared<DYMO_RERR>();//("RERR");
    std::vector<DYMO_AddressBlock> unode_vec;

    // add target node as first unreachableNode
    DYMO_AddressBlock unode;
    unode.setAddress(targetAddr);
    if (targetSeqNum != 0) unode.setSeqNum(targetSeqNum);
    unode_vec.push_back(unode);

    // set hop limit
    rerr->setMsgHdrHopLimit(MAX_HOPLIMIT);

    // add additional unreachableNode entries for all route entries that use the same routeNextHopAddress and routeNextHopInterface
    DYMO_RoutingEntry* brokenEntry = dymo_routingTable->getForAddress(targetAddr);
    if (brokenEntry)
    {
        // sanity check
        if (!brokenEntry->routeBroken) throw std::runtime_error("sendRERR called for targetAddr that has a perfectly fine routing table entry");

        // add route entries with same routeNextHopAddress as broken route
        std::vector<DYMO_RoutingEntry *> RouteVector = dymo_routingTable->getRoutingTable();
        for (auto & elem : RouteVector)
        {
            DYMO_RoutingEntry* entry = elem;
            if ((entry->routeNextHopAddress != brokenEntry->routeNextHopAddress) || (entry->routeNextHopInterface != brokenEntry->routeNextHopInterface)) continue;

            EV_DETAIL << "Including in RERR route to " << entry->routeAddress << " via " << entry->routeNextHopAddress << endl;

            DYMO_AddressBlock unode;
            unode.setAddress(entry->routeAddress);
            if (entry->routeSeqNum != 0) unode.setSeqNum(entry->routeSeqNum);
            unode_vec.push_back(unode);
        }
    }

    // wrap up and send
    rerr->setUnreachableNodes(unode_vec);
    rerr->setChunkLength(B(DYMO_RERR_HEADER_LENGTH + (rerr->getUnreachableNodes().size() * DYMO_UBLOCK_LENGTH)));
    auto pkt = new Packet("RERR");
    pkt->insertAtFront(rerr);

    sendDown(pkt, L3Address(Ipv4Address::LL_MANET_ROUTERS));

    // keep statistics
    statsRERRSent++;
}

void DYMOFau::incSeqNum()
{
    if (ownSeqNum == 0xffff)
    {
        ownSeqNum = 0x0100;
    }
    else
    {
        ownSeqNum++;
    }
}

bool DYMOFau::seqNumIsFresher(unsigned int seqNumInQuestion, unsigned int referenceSeqNum)
{
    return ((int16_t)referenceSeqNum - (int16_t)seqNumInQuestion < 0);
}

simtime_t DYMOFau::computeBackoff(simtime_t backoff_var)
{
    return backoff_var * 2;
}

void DYMOFau::updateRouteLifetimes(const L3Address& targetAddr)
{
    DYMO_RoutingEntry* entry = dymo_routingTable->getForAddress(targetAddr);
    if (!entry) return;

    // TODO: not specified in draft, but seems to make sense
    if (entry->routeBroken) return;

    entry->routeUsed.start(ROUTE_USED_TIMEOUT);
    entry->routeDelete.cancel();
    rescheduleTimer();

    dymo_routingTable->maintainAssociatedRoutingTable();
    EV_INFO << "lifetimes of route to destination node " << targetAddr << " are up to date "  << endl;

    checkAndSendQueuedPkts(entry->routeAddress, entry->routePrefix, entry->routeNextHopAddress);
}

bool DYMOFau::isRBlockBetter(DYMO_RoutingEntry * entry, DYMO_AddressBlock ab, bool isRREQ)
{
    //TODO: check handling of unknown SeqNum values

    // stale?
    if (seqNumIsFresher(entry->routeSeqNum, ab.getSeqNum())) return false;

    // loop-possible or inferior?
    if (ab.getSeqNum() == (int)entry->routeSeqNum)
    {
        int nodeDist = ab.hasDist() ? (ab.getDist() + 1) : 0; // incremented by one, because draft -10 says to first increment, then compare
        int routeDist = entry->routeDist;

        // loop-possible?
        if (nodeDist == 0) return false;
        if (routeDist == 0) return false;
        if (nodeDist > routeDist + 1) return false;

        // inferior?
        if (nodeDist > routeDist) return false;
        if ((nodeDist == routeDist) && (!entry->routeBroken) && (isRREQ)) return false;
    }

    // superior
    return true;
}

void DYMOFau::handleRREQTimeout(DYMO_OutstandingRREQ& outstandingRREQ)
{
    EV_INFO << "Handling RREQ Timeouts for RREQ to " << outstandingRREQ.destAddr << endl;

    if (outstandingRREQ.tries < RREQ_TRIES)
    {
        DYMO_RoutingEntry* entry = dymo_routingTable->getForAddress(outstandingRREQ.destAddr);
        if (entry && (!entry->routeBroken))
        {
            /** an entry was found in the routing table -> get control data from the table, encapsulate message **/
            EV_INFO << "RREQ timed out and we DO have a route" << endl;

            checkAndSendQueuedPkts(entry->routeAddress, entry->routePrefix, entry->routeNextHopAddress);

            return;
        }
        else
        {
            EV_INFO << "RREQ timed out and we do not have a route yet" << endl;
            /** number of tries is less than RREQ_TRIES -> backoff and send the rreq **/
            outstandingRREQ.tries = outstandingRREQ.tries + 1;
            outstandingRREQ.wait_time->start(computeBackoff(outstandingRREQ.wait_time->getInterval()));

            /* update seqNum */
            incSeqNum();

            unsigned int targetSeqNum = 0;
            // if a targetSeqNum is known, include it in all but the last RREQ attempt
            if (entry && (outstandingRREQ.tries < RREQ_TRIES)) targetSeqNum = entry->routeSeqNum;

            // expanding ring search
            int msgHdrHopLimit = MIN_HOPLIMIT + (MAX_HOPLIMIT - MIN_HOPLIMIT) * (outstandingRREQ.tries - 1) / (RREQ_TRIES - 1);

            sendRREQ(outstandingRREQ.destAddr, msgHdrHopLimit, targetSeqNum, (entry?(entry->routeDist):0));

            return;
        }
    }
    else
    {
        /** RREQ_TRIES is reached **/

        std::list<Packet*> datagrams;
        // drop packets bound for the expired RREQ's destination
        dymo_routingTable->maintainAssociatedRoutingTable();
        queuedDataPackets->dropPacketsTo(outstandingRREQ.destAddr, 32, &datagrams);
        while (!datagrams.empty())
        {
            auto* dgram = datagrams.front();

            if (getNetworkProtocol())
                getNetworkProtocol()->dropQueuedDatagram(dgram);
            else
                delete dgram;
            datagrams.pop_front();
            //sendICMP(dgram);
        }

        // clean up outstandingRREQList
        outstandingRREQList.del(&outstandingRREQ);

        return;
    }

    return;
}

bool DYMOFau::updateRoutesFromAddressBlock(const DYMO_AddressBlock& ab, bool isRREQ, L3Address nextHopAddress, InterfaceEntry* nextHopInterface)
{
    DYMO_RoutingEntry* entry = dymo_routingTable->getForAddress(ab.getAddress());
    if (entry && !isRBlockBetter(entry, ab, isRREQ)) return false;

    if (!entry)
    {
        EV_INFO << "adding routing entry for " << ab.getAddress() << endl;
        entry = new DYMO_RoutingEntry(this);
        dymo_routingTable->addRoute(entry);
    }
    else
    {
        EV_INFO << "updating routing entry for " << ab.getAddress() << endl;
    }

    entry->routeAddress = ab.getAddress();
    entry->routeSeqNum = ab.getSeqNum();
    entry->routeDist = ab.hasDist() ? (ab.getDist() + 1) : 0;  // incremented by one, because draft -10 says to first increment, then compare
    entry->routeNextHopAddress = nextHopAddress;
    entry->routeNextHopInterface = nextHopInterface;
    entry->routePrefix = ab.hasPrefix() ? ab.getPrefix() : 32;
    entry->routeBroken = false;
    entry->routeAgeMin.start(ROUTE_AGE_MIN_TIMEOUT);
    entry->routeAgeMax.start(ROUTE_AGE_MAX_TIMEOUT);
    entry->routeNew.start(ROUTE_NEW_TIMEOUT);
    entry->routeUsed.cancel();
    entry->routeDelete.cancel();

    rescheduleTimer();

    dymo_routingTable->maintainAssociatedRoutingTable();

    checkAndSendQueuedPkts(entry->routeAddress, entry->routePrefix, nextHopAddress);

    return true;
}

Packet* DYMOFau::updateRoutes(Packet * pkt)
{
    EV_INFO << "starting update routes from routing blocks in the received message" << endl;

    const auto & re = pkt->peekAtFront<DYMO_RM>();

    std::vector<DYMO_AddressBlock> additional_nodes = re->getAdditionalNodes();
    std::vector<DYMO_AddressBlock> new_additional_nodes;



    bool isRREQ = (dynamicPtrCast<const DYMO_RREQ>(re) != nullptr);
    L3Address nextHopAddress = getNextHopAddress(pkt);
    InterfaceEntry* nextHopInterface = getNextHopInterface(pkt);

    if (re->getOrigNode().getAddress()==myAddr) return nullptr;
    bool origNodeEntryWasSuperior = updateRoutesFromAddressBlock(re->getOrigNode(), isRREQ, nextHopAddress, nextHopInterface);

    for (auto & additional_node : additional_nodes)
    {
        // TODO: not specified in draft, but seems to make sense
        if (additional_node.getAddress()==myAddr) return nullptr;

        if (updateRoutesFromAddressBlock(additional_node, isRREQ, nextHopAddress, nextHopInterface))
        {
            /** read routing block is valid -> save block to the routing message **/
            new_additional_nodes.push_back(additional_node);
        }
        else
        {
            EV_DETAIL << "AdditionalNode AddressBlock has no valid information  -> dropping block from routing message" << endl;
        }
    }

    if (!origNodeEntryWasSuperior)
    {
        EV_INFO << "OrigNode AddressBlock had no valid information -> deleting received routing message" << endl;
        return nullptr;
    }

    auto reAux = pkt->removeAtFront<DYMO_RM>();
    reAux->setAdditionalNodes(new_additional_nodes);
    pkt->insertAtFront(reAux);

    return pkt;
}

void DYMOFau::checkAndSendQueuedPkts(L3Address destinationAddress, int prefix, L3Address /*nextHopAddress*/)
{
    dymo_routingTable->maintainAssociatedRoutingTable();
    queuedDataPackets->dequeuePacketsTo(destinationAddress, prefix);

    // clean up outstandingRREQList: remove those with matching destAddr
    DYMO_OutstandingRREQ* o = outstandingRREQList.getByDestAddr(destinationAddress, prefix);
    if (o) outstandingRREQList.del(o);
}

void DYMOFau::setMyAddr(L3Address myAddr)
{
    // Check if this node has already participated in DYMO
    if (statsRREQSent || statsRREPSent || statsRERRSent)
    {
        // TODO: Send RERRs, cold-start DYMO, lose sequence number instead?
        EV_INFO << "Ignoring IPv4 Address change request. This node has already participated in DYMO." << endl;
        return;
    }

    EV_INFO << "Now assuming this node is reachable at address " << myAddr << " (was " << this->myAddr << ")" << endl;
    this->myAddr = myAddr;

    // TODO: if IInterfaceTable was autoconfigured, change IPv4 Address there?
}

DYMO_RoutingTable* DYMOFau::getDYMORoutingTable()
{
    return dymo_routingTable;
}

cModule* DYMOFau::getRouterByAddress(L3Address address)
{
    return L3AddressResolver().findHostWithAddress(address);
    //return dynamic_cast<cModule*>(getSimulation()->getModule(address.getInt() - AUTOASSIGN_ADDRESS_BASE.getInt()));
}

/* Called for packets whose delivery fails at the link layer */
void DYMOFau::packetFailed(const Packet *dgram)
{
    const auto& networkHeader = getNetworkProtocolHeader(const_cast<Packet *>(dgram));
    if (networkHeader == nullptr)
        return;

    const L3Address& destAddr = networkHeader->getDestinationAddress();
    //const L3Address& sourceAddr = networkHeader->getSourceAddress();



    /* We don't care about link failures for broadcast or non-data packets */
    if (destAddr.isBroadcast() || destAddr.isMulticast())
    {
        return;
    }
    EV_WARN << "LINK FAILURE for dest=" << destAddr;
    DYMO_RoutingEntry *entry = dymo_routingTable->getByAddress(destAddr);
    if (entry)
    {
        L3Address nextHop = entry->routeNextHopAddress;
        for (int i = 0; i < dymo_routingTable->getNumRoutes(); i++)
        {
            DYMO_RoutingEntry *entry = dymo_routingTable->getRoute(i);
            if (entry->routeNextHopAddress==nextHop)
            {
                entry->routeBroken = true;
                //sendRERR(entry->routeAddress.getInt(),entry->routeSeqNum);
            }
        }
    }
    dymo_routingTable->maintainAssociatedRoutingTable();
}

void DYMOFau::processLinkBreak(const Packet *dgram)
{
    const auto& networkHeader = getNetworkProtocolHeader(const_cast<Packet *>(dgram));
    const auto &header = dynamicPtrCast<const Ipv4Header>(networkHeader);
    if (header)
        packetFailed(dgram);
}

void  DYMOFau::handleStartOperation(LifecycleOperation *operation)
{

    rateLimiterRREQ = new DYMO_TokenBucket(RREQ_RATE_LIMIT, RREQ_BURST_LIMIT, simTime());
    dymo_routingTable = new DYMO_RoutingTable(this, myAddr);
    queuedDataPackets = new DYMO_DataQueue(this, BUFFER_SIZE_PACKETS, BUFFER_SIZE_BYTES);
}

void DYMOFau::handleStopOperation(LifecycleOperation *operation)
{
    delete dymo_routingTable;
    outstandingRREQList.delAll();
    delete rateLimiterRREQ;
    delete queuedDataPackets;
    cancelEvent(timerMsg);
}

void DYMOFau::handleCrashOperation(LifecycleOperation *operation)
{
    delete dymo_routingTable;
    outstandingRREQList.delAll();
    delete rateLimiterRREQ;
    delete queuedDataPackets;
    cancelEvent(timerMsg);
}


} // namespace inetmanet

} // namespace inet

