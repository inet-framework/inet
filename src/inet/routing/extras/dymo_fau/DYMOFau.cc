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

#include "inet/routing/extras/dymo_fau/DYMOFau.h"
#include "inet/transportlayer/contract/udp/UDPControlInfo_m.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "inet/networklayer/common/IPProtocolId_m.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/transportlayer/udp/UDPPacket.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/ipv4/IPv4.h"
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
    dymo_routingTable = NULL;
    timerMsg = NULL;
    ownSeqNumLossTimeout = NULL;
    ownSeqNumLossTimeoutMax = NULL;
    queuedDataPackets = NULL;
    rateLimiterRREQ = NULL;
    DYMO_INTERFACES = NULL;
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

        dymo_routingTable = new DYMO_RoutingTable(this, IPv4Address(myAddr));
        WATCH_PTR(dymo_routingTable);

        outstandingRREQList.delAll();
        WATCH_OBJ(outstandingRREQList);
        queuedDataPackets = new DYMO_DataQueue(this, BUFFER_SIZE_PACKETS, BUFFER_SIZE_BYTES);
        WATCH_PTR(queuedDataPackets);

        registerRoutingModule();

        // setSendToICMP(true);
        myAddr = getAddress().toIPv4().getInt();   //FIXME
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
    dymo_routingTable = 0;

    outstandingRREQList.delAll();

    delete ownSeqNumLossTimeout;
    ownSeqNumLossTimeout = NULL;
    delete ownSeqNumLossTimeoutMax;
    ownSeqNumLossTimeoutMax = NULL;

    delete rateLimiterRREQ;
    rateLimiterRREQ = NULL;

    // IPv4* ipLayer = queuedDataPackets->getIpLayer();
    delete queuedDataPackets;
    queuedDataPackets = NULL;
    // ipLayer->unregisterHook(0, this);

    cancelAndDelete(timerMsg);
    timerMsg = NULL;
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

void DYMOFau::handleMessage(cMessage* apMsg)
{

    cMessage * msg_aux = NULL;
    UDPPacket* udpPacket = NULL;

    if (apMsg->isSelfMessage())
    {
        handleSelfMsg(apMsg);
    }
    else
    {
        if (dynamic_cast<ControlManetRouting *>(apMsg))
        {
            ControlManetRouting * control = check_and_cast <ControlManetRouting *> (apMsg);
            if (control->getOptionCode() == MANET_ROUTE_NOROUTE)
            {
                IPv4Datagram * dgram = check_and_cast<IPv4Datagram*>(control->decapsulate());
                processPacket(dgram);
            }
            else if (control->getOptionCode() == MANET_ROUTE_UPDATE)
            {
                IPv4Address src = control->getSrcAddress().toIPv4();
                IPv4Address dst = control->getDestAddress().toIPv4();
                if (!src.isLimitedBroadcastAddress() && !src.isMulticast() && src.isUnspecified())
                    updateRouteLifetimes(control->getSrcAddress());
                if (!dst.isLimitedBroadcastAddress() && !dst.isMulticast() && !dst.isUnspecified())
                    updateRouteLifetimes(control->getDestAddress());
            }
            delete apMsg;
            return;
        }
        else if (dynamic_cast<UDPPacket *>(apMsg))
        {
            udpPacket = check_and_cast<UDPPacket*>(apMsg);
            if (udpPacket->getDestinationPort() != DYMO_PORT)
            {
                delete  apMsg;
                return;
            }
            msg_aux = udpPacket->decapsulate();

            INetworkProtocolControlInfo *controlInfo = check_and_cast<INetworkProtocolControlInfo*>(udpPacket->removeControlInfo());
            L3Address srcAddr = controlInfo->getSourceAddress();
            if (isLocalAddress(srcAddr) || srcAddr.isUnspecified())
            {
                // local address delete packet
                delete msg_aux;
                delete controlInfo;
                delete apMsg;
                return;
            }
            msg_aux->setControlInfo(check_and_cast<cObject *>(controlInfo));
        }
        else
        {
            EV_ERROR << "!!!!!!!!!!!!! Unknown Message type !!!!!!!!!! \n";
            delete apMsg;
            return;
        }

        if (udpPacket)
        {
            delete udpPacket;
            udpPacket = NULL;
        }

        if (!dynamic_cast<DYMO_PacketBBMessage  *>(msg_aux))
        {
            delete msg_aux;
            return;
        }
        cPacket* apPkt = PK(msg_aux);
        handleLowerMsg(apPkt);
    }
}

void DYMOFau::processPacket(const IPv4Datagram* datagram)
{
    Enter_Method("procces ip Packet (%s)", datagram->getName());

    IPv4Address destAddr = datagram->getDestAddress();
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
            delete datagram;
            return;
        }
        TargetSeqNum = entry->routeSeqNum;
        TargetHopCount = entry->routeDist;
    }

    if (!datagram->getSrcAddress().isUnspecified() && !isLocalAddress(L3Address(datagram->getSrcAddress())))
    {
        // It's not a packet of this node, send error to source
        sendRERR(destAddr.getInt(), TargetSeqNum);
        delete datagram;
        return;
    }

    // no route in the table found -> route discovery (if none is already underway)
    if (!outstandingRREQList.getByDestAddr(destAddr.getInt(), 32))
    {
        sendRREQ(destAddr.getInt(), MIN_HOPLIMIT, TargetSeqNum, TargetHopCount);
        /** queue the RREQ in a list and schedule a timeout in order to resend the RREQ when no RREP is received **/
        DYMO_OutstandingRREQ* outstandingRREQ = new DYMO_OutstandingRREQ;
        outstandingRREQ->tries = 1;
        outstandingRREQ->wait_time = new DYMO_Timer(this, "RREQ wait time");
        outstandingRREQ->wait_time->start(RREQ_WAIT_TIME);
        outstandingRREQ->destAddr = destAddr.getInt();
        outstandingRREQ->creationTime = simTime();
        outstandingRREQList.add(outstandingRREQ);
        rescheduleTimer();
    }
    queuedDataPackets->queuePacket(datagram);
}

void DYMOFau::handleLowerMsg(cPacket* apMsg)
{
    /**
     * check the type of received message
     1) Routing Message: RREQ or RREP
     2) Error Message: RERR
     3) Unsupported Message: UERR
     4) Data Message
     **/
    if (dynamic_cast<DYMO_RM*>(apMsg)) handleLowerRM(dynamic_cast<DYMO_RM*>(apMsg));
    else if (dynamic_cast<DYMO_RERR*>(apMsg)) handleLowerRERR(dynamic_cast<DYMO_RERR*>(apMsg));
    else if (dynamic_cast<DYMO_UERR*>(apMsg)) handleLowerUERR(dynamic_cast<DYMO_UERR*>(apMsg));
    else if (apMsg->getKind() == UDP_I_ERROR) { EV_INFO << "discarded UDP error message" << endl; delete apMsg; }
    else throw cRuntimeError("message is no DYMO Packet");
}

void DYMOFau::handleLowerRM(DYMO_RM *routingMsg)
{
    /** message is a routing message **/
    EV_INFO << "received message is a routing message" << endl;

    statsDYMORcvd++;

    /** routing message  preprocessing and updating routes from routing blocks **/
    if (updateRoutes(routingMsg) == NULL)
    {
        EV_INFO << "dropping received message" << endl;
        delete routingMsg;
        return;
    }

    /**
     * received message is a routing message.
     * check if the node is the destination
     * 1) YES - if the RM is a RREQ, then send a RREP to source
     * 2) NO - send message down to next node.
     **/
    if (myAddr == routingMsg->getTargetNode().getAddress())
    {
        handleLowerRMForMe(routingMsg);
        return;
    }
    else
    {
        handleLowerRMForRelay(routingMsg);
        return;
    }
}

uint32_t DYMOFau::getNextHopAddress(DYMO_RM *routingMsg)
{
    if (routingMsg->getAdditionalNodes().size() > 0)
    {
        return routingMsg->getAdditionalNodes().back().getAddress();
    }
    else
    {
        return routingMsg->getOrigNode().getAddress();
    }
}

InterfaceEntry* DYMOFau::getNextHopInterface(DYMO_PacketBBMessage* pkt)
{

    if (!pkt) throw cRuntimeError("getNextHopInterface called with NULL packet");

    INetworkProtocolControlInfo* controlInfo = check_and_cast_nullable<INetworkProtocolControlInfo *>(pkt->removeControlInfo());
    if (!controlInfo) throw cRuntimeError("received packet did not have INetworkProtocolControlInfo attached");

    int interfaceId = controlInfo->getInterfaceId();
    if (interfaceId == -1) throw cRuntimeError("received packet's UDPControlInfo did not have information on interfaceId");

    InterfaceEntry* srcIf = NULL;

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

    delete controlInfo;
    return srcIf;
}

void DYMOFau::handleLowerRMForMe(DYMO_RM *routingMsg)
{
    /** current node is the target **/
    if (dynamic_cast<DYMO_RREQ*>(routingMsg))
    {
        /** received message is a RREQ -> send a RREP to source **/
        sendReply(routingMsg->getOrigNode().getAddress(), (routingMsg->getTargetNode().hasSeqNum() ? routingMsg->getTargetNode().getSeqNum() : 0));
        statsRREQRcvd++;
        delete routingMsg;
    }
    else if (dynamic_cast<DYMO_RREP*>(routingMsg))
    {
        /** received message is a RREP **/
        statsRREPRcvd++;

        // signal the queue to dequeue waiting messages for this destination
        checkAndSendQueuedPkts(routingMsg->getOrigNode().getAddress(), (routingMsg->getOrigNode().hasPrefix() ? routingMsg->getOrigNode().getPrefix() : 32), getNextHopAddress(routingMsg));

        delete routingMsg;
    }
    else throw cRuntimeError("received unknown dymo message");
}

void DYMOFau::handleLowerRMForRelay(DYMO_RM *routingMsg)
{
    /** current node is not the message destination -> find route to destination **/
    EV_INFO << "current node is not the message destination -> find route to destination" << endl;

    unsigned int targetAddr = routingMsg->getTargetNode().getAddress();
    unsigned int targetSeqNum = 0;

    // stores route entry of route to destination if a route exists, 0 otherwise
    DYMO_RoutingEntry* entry = dymo_routingTable->getForAddress(IPv4Address(targetAddr));
    if (entry)
    {
        targetSeqNum = entry->routeSeqNum;

        //TODO: mark route as used when forwarding DYMO packets?
        //entry->routeUsed.start(ROUTE_USED_TIMEOUT);
        //entry->routeDelete.cancel();

        if (entry->routeBroken) entry = 0;
    }

    /** received routing message is an RREP and no routing entry was found **/
    if (dynamic_cast<DYMO_RREP*>(routingMsg) && (!entry))
    {
        /* do nothing, just drop the RREP */
        EV_INFO << "no route to destination of RREP was found. Sending RERR and dropping message." << endl;
        sendRERR(targetAddr, targetSeqNum);
        delete routingMsg;
        return;
    }

    // check if received message is a RREQ and a routing entry was found
    if (dynamic_cast<DYMO_RREQ*>(routingMsg) && (entry) && (routingMsg->getTargetNode().hasSeqNum()) && (!seqNumIsFresher(routingMsg->getTargetNode().getSeqNum(), entry->routeSeqNum)))
    {
        // yes, we can. Do intermediate DYMO router RREP creation
        EV_INFO << "route to destination of RREQ was found. Sending intermediate DYMO router RREP" << endl;
        sendReplyAsIntermediateRouter(routingMsg->getOrigNode(), routingMsg->getTargetNode(), entry);
        statsRREQRcvd++;
        delete routingMsg;
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
        delete routingMsg;
        return;
    }
    routingMsg->getOrigNode().incrementDistIfAvailable();
    for (unsigned int i = 0; i < additional_nodes.size(); i++)
    {
        if (additional_nodes[i].hasDist() && (additional_nodes[i].getDist() >= 0xFF - 1))
        {
            EV_INFO << "passing on additionalNode would overflow OrigNode.Dist -> dropping additionalNode" << endl;
            continue;
        }
        additional_nodes[i].incrementDistIfAvailable();
        additional_nodes_to_relay.push_back(additional_nodes[i]);
    }

    // append additional routing information about this node
    DYMO_AddressBlock additional_node;
    additional_node.setDist(0);
    additional_node.setAddress(myAddr);
    if (RESPONSIBLE_ADDRESSES_PREFIX != -1) additional_node.setPrefix(RESPONSIBLE_ADDRESSES_PREFIX);
    incSeqNum();
    additional_node.setSeqNum(ownSeqNum);
    additional_nodes_to_relay.push_back(additional_node);

    routingMsg->setAdditionalNodes(additional_nodes_to_relay);
    routingMsg->setMsgHdrHopLimit(routingMsg->getMsgHdrHopLimit() - 1);

    // check hop limit
    if (routingMsg->getMsgHdrHopLimit() < 1)
    {
        EV_INFO << "received message has reached hop limit -> delete message" << endl;
        delete routingMsg;
        return;
    }

    // do not transmit DYMO messages when we lost our sequence number
    if (ownSeqNumLossTimeout->isRunning())
    {
        EV_INFO << "node has lost sequence number -> not transmitting anything" << endl;
        delete routingMsg;
        return;
    }

    // do rate limiting
    if ((dynamic_cast<DYMO_RREQ*>(routingMsg)) && (!rateLimiterRREQ->consumeTokens(1, simTime())))
    {
        EV_INFO << "RREQ send rate exceeded maximum -> not transmitting RREQ" << endl;
        delete routingMsg;
        return;
    }

    /* transmit message -- RREP via unicast, RREQ via DYMOcast */
    sendDown(routingMsg, dynamic_cast<DYMO_RREP*>(routingMsg) ? (entry->routeNextHopAddress).getInt() : IPv4Address::LL_MANET_ROUTERS.getInt());

    /* keep statistics */
    if (dynamic_cast<DYMO_RREP*>(routingMsg))
    {
        statsRREPFwd++;
    }
    else
    {
        statsRREQFwd++;
    }
}

void DYMOFau::handleLowerRERR(DYMO_RERR *my_rerr)
{
    /** message is a RERR. **/
    statsDYMORcvd++;

    // get RERR's IPv4.SourceAddress
    INetworkProtocolControlInfo* controlInfo = check_and_cast<INetworkProtocolControlInfo*>(my_rerr->getControlInfo());
    IPv4Address sourceAddr = controlInfo->getSourceAddress().toIPv4();

    // get RERR's SourceInterface
    InterfaceEntry* sourceInterface = getNextHopInterface(my_rerr);

    EV_INFO << "Received RERR from " << sourceAddr << endl;

    // iterate over all unreachableNode entries
    std::vector<DYMO_AddressBlock> unreachableNodes = my_rerr->getUnreachableNodes();
    std::vector<DYMO_AddressBlock> unreachableNodesToForward;
    for (unsigned int i = 0; i < unreachableNodes.size(); i++)
    {
        const DYMO_AddressBlock& unreachableNode = unreachableNodes[i];

        if (IPv4Address(unreachableNode.getAddress()).isMulticast()) continue;

        // check whether this invalidates entries in our routing table
        std::vector<DYMO_RoutingEntry *> RouteVector = dymo_routingTable->getRoutingTable();
        for (unsigned int i = 0; i < RouteVector.size(); i++)
        {
            DYMO_RoutingEntry* entry = RouteVector[i];

            // skip if route has no associated Forwarding Route
            if (entry->routeBroken) continue;

            // skip if this route isn't to the unreachableNode Address mentioned in the RERR
            if (!entry->routeAddress.prefixMatches(IPv4Address(unreachableNode.getAddress()), entry->routePrefix)) continue;

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
        delete my_rerr;
        return;
    }

    // discard RERR if ownSeqNum was lost
    if (ownSeqNumLossTimeout->isRunning())
    {
        statsRERRRcvd++;
        delete my_rerr;
        return;
    }

    // discard RERR if msgHdrHopLimit has reached 1
    if (my_rerr->getMsgHdrHopLimit() <= 1)
    {
        statsRERRRcvd++;
        delete my_rerr;
        return;
    }

    // forward RERR with unreachableNodesToForward
    my_rerr->setUnreachableNodes(unreachableNodesToForward);
    my_rerr->setMsgHdrHopLimit(my_rerr->getMsgHdrHopLimit() - 1);

    EV_INFO << "send down RERR" << endl;
    sendDown(my_rerr, IPv4Address::LL_MANET_ROUTERS.getInt());

    statsRERRFwd++;
}

void DYMOFau::handleLowerUERR(DYMO_UERR *my_uerr)
{
    /** message is a UERR. **/
    statsDYMORcvd++;

    EV_INFO << "Received unsupported UERR message" << endl;
    // to be finished
    delete my_uerr;
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
        while ((outstandingRREQ = outstandingRREQList.getExpired()) != NULL )
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

void DYMOFau::sendDown(cPacket* apMsg, int destAddr)
{
    // all messages sent to a lower layer are delayed by 0..MAXJITTER seconds (draft-ietf-manet-jitter-01)
    simtime_t jitter = dblrand() * MAXJITTER;

    apMsg->setKind(UDP_C_DATA);

    // set byte size of message
    const DYMO_RM* re = dynamic_cast<const DYMO_RM*>(apMsg);
    const DYMO_RERR* rerr = dynamic_cast<const DYMO_RERR*>(apMsg);
    if (re)
    {
        apMsg->setByteLength(DYMO_RM_HEADER_LENGTH + ((1 + re->getAdditionalNodes().size()) * DYMO_RBLOCK_LENGTH));
    }
    else if (rerr)
    {
        apMsg->setByteLength(DYMO_RERR_HEADER_LENGTH + (rerr->getUnreachableNodes().size() * DYMO_UBLOCK_LENGTH));
    }
    else
    {
        throw cRuntimeError("tried to send unsupported message type");
    }
    // keep statistics
    totalPacketsSent++;
    totalBytesSent += apMsg->getByteLength();
    if (IPv4Address::LL_MANET_ROUTERS.getInt()==(unsigned int)destAddr)
    {
        destAddr = IPv4Address::ALLONES_ADDRESS.getInt();
        sendToIp(apMsg, UDPPort, L3Address(IPv4Address(destAddr)), UDPPort, 1, SIMTIME_DBL(jitter));
    }
    else
    {
        sendToIp(apMsg, UDPPort, L3Address(IPv4Address(destAddr)), UDPPort, 1, 0.0);
    }
}

void DYMOFau::sendRREQ(unsigned int destAddr, int msgHdrHopLimit, unsigned int targetSeqNum, unsigned int targetDist)
{
    /** generate a new RREQ with the given pararmeter **/
    EV_INFO << "send a RREQ to discover route to destination node " << destAddr << endl;

    DYMO_RM *my_rreq = new DYMO_RREQ("RREQ");
    my_rreq->setMsgHdrHopLimit(msgHdrHopLimit);
    my_rreq->getTargetNode().setAddress(destAddr);
    if (targetSeqNum != 0) my_rreq->getTargetNode().setSeqNum(targetSeqNum);
    if (targetDist != 0) my_rreq->getTargetNode().setDist(targetDist);

    my_rreq->getOrigNode().setDist(0);
    my_rreq->getOrigNode().setAddress(myAddr);
    if (RESPONSIBLE_ADDRESSES_PREFIX != -1) my_rreq->getOrigNode().setPrefix(RESPONSIBLE_ADDRESSES_PREFIX);
    incSeqNum();
    my_rreq->getOrigNode().setSeqNum(ownSeqNum);

    // do not transmit DYMO messages when we lost our sequence number
    if (ownSeqNumLossTimeout->isRunning())
    {
        EV_INFO << "node has lost sequence number -> not transmitting RREQ" << endl;
        delete my_rreq;
        return;
    }

    // do rate limiting
    if (!rateLimiterRREQ->consumeTokens(1, simTime()))
    {
        EV_INFO << "RREQ send rate exceeded maximum -> not transmitting RREQ" << endl;
        delete my_rreq;
        return;
    }

    sendDown(my_rreq, IPv4Address::LL_MANET_ROUTERS.getInt());
    statsRREQSent++;
}

void DYMOFau::sendReply(unsigned int destAddr, unsigned int tSeqNum)
{
    /** create a new RREP and send it to given destination **/
    EV_INFO << "send a reply to destination node " << destAddr << endl;

    DYMO_RM * rrep = new DYMO_RREP("RREP");
    DYMO_RoutingEntry *entry = dymo_routingTable->getForAddress(IPv4Address(destAddr));
    if (!entry) throw cRuntimeError("Tried sending RREP via a route that just vanished");

    rrep->setMsgHdrHopLimit(MAX_HOPLIMIT);
    rrep->getTargetNode().setAddress(destAddr);
    rrep->getTargetNode().setSeqNum(entry->routeSeqNum);
    rrep->getTargetNode().setDist(entry->routeDist);

    // check if ownSeqNum should be incremented
    if ((tSeqNum == 0) || (seqNumIsFresher(ownSeqNum, tSeqNum))) incSeqNum();

    rrep->getOrigNode().setAddress(myAddr);
    if (RESPONSIBLE_ADDRESSES_PREFIX != -1) rrep->getOrigNode().setPrefix(RESPONSIBLE_ADDRESSES_PREFIX);
    rrep->getOrigNode().setSeqNum(ownSeqNum);
    rrep->getOrigNode().setDist(0);

    // do not transmit DYMO messages when we lost our sequence number
    if (ownSeqNumLossTimeout->isRunning())
    {
        EV_WARN << "node has lost sequence number -> not transmitting anything" << endl;
        return;
    }

    sendDown(rrep, (entry->routeNextHopAddress).getInt());

    statsRREPSent++;
}

void DYMOFau::sendReplyAsIntermediateRouter(const DYMO_AddressBlock& origNode, const DYMO_AddressBlock& targetNode, const DYMO_RoutingEntry* routeToTargetNode)
{
    /** create a new RREP and send it to given destination **/
    EV_INFO << "sending a reply to OrigNode " << origNode.getAddress() << endl;

    DYMO_RoutingEntry* routeToOrigNode = dymo_routingTable->getForAddress(IPv4Address(origNode.getAddress()));
    if (!routeToOrigNode) throw cRuntimeError("no route to OrigNode found");

    // increment ownSeqNum.
    // TODO: The draft is unclear about when to increment ownSeqNum for intermediate DYMO router RREP creation
    incSeqNum();

    // create rrepToOrigNode
    DYMO_RREP* rrepToOrigNode = new DYMO_RREP("RREP");
    rrepToOrigNode->setMsgHdrHopLimit(MAX_HOPLIMIT);
    rrepToOrigNode->getTargetNode().setAddress(origNode.getAddress());
    rrepToOrigNode->getTargetNode().setSeqNum(origNode.getSeqNum());
    if (origNode.hasDist()) rrepToOrigNode->getTargetNode().setDist(origNode.getDist() + 1);

    rrepToOrigNode->getOrigNode().setAddress(myAddr);
    if (RESPONSIBLE_ADDRESSES_PREFIX != -1) rrepToOrigNode->getOrigNode().setPrefix(RESPONSIBLE_ADDRESSES_PREFIX);
    rrepToOrigNode->getOrigNode().setSeqNum(ownSeqNum);
    rrepToOrigNode->getOrigNode().setDist(0);

    DYMO_AddressBlock additionalNode;
    additionalNode.setAddress(routeToTargetNode->routeAddress.getInt());
    if (routeToTargetNode->routeSeqNum != 0) additionalNode.setSeqNum(routeToTargetNode->routeSeqNum);
    if (routeToTargetNode->routeDist != 0) additionalNode.setDist(routeToTargetNode->routeDist);
    rrepToOrigNode->getAdditionalNodes().push_back(additionalNode);

    // create rrepToTargetNode
    DYMO_RREP* rrepToTargetNode = new DYMO_RREP("RREP");
    rrepToTargetNode->setMsgHdrHopLimit(MAX_HOPLIMIT);
    rrepToTargetNode->getTargetNode().setAddress(targetNode.getAddress());
    if (targetNode.hasSeqNum()) rrepToTargetNode->getTargetNode().setSeqNum(targetNode.getSeqNum());
    if (targetNode.hasDist()) rrepToTargetNode->getTargetNode().setDist(targetNode.getDist());

    rrepToTargetNode->getOrigNode().setAddress(myAddr);
    if (RESPONSIBLE_ADDRESSES_PREFIX != -1) rrepToTargetNode->getOrigNode().setPrefix(RESPONSIBLE_ADDRESSES_PREFIX);
    rrepToTargetNode->getOrigNode().setSeqNum(ownSeqNum);
    rrepToTargetNode->getOrigNode().setDist(0);

    DYMO_AddressBlock additionalNode2;
    additionalNode2.setAddress(origNode.getAddress());
    additionalNode2.setSeqNum(origNode.getSeqNum());
    if (origNode.hasDist()) additionalNode2.setDist(origNode.getDist() + 1);
    rrepToTargetNode->getAdditionalNodes().push_back(additionalNode2);

    // do not transmit DYMO messages when we lost our sequence number
    if (ownSeqNumLossTimeout->isRunning())
    {
        EV_WARN << "node has lost sequence number -> not transmitting anything" << endl;
        return;
    }

    sendDown(rrepToOrigNode, (routeToOrigNode->routeNextHopAddress).getInt());
    sendDown(rrepToTargetNode, (routeToTargetNode->routeNextHopAddress).getInt());

    statsRREPSent++;
}

void DYMOFau::sendRERR(unsigned int targetAddr, unsigned int targetSeqNum)
{
    EV_INFO << "generating an RERR" << endl;
    DYMO_RERR *rerr = new DYMO_RERR("RERR");
    std::vector<DYMO_AddressBlock> unode_vec;

    // add target node as first unreachableNode
    DYMO_AddressBlock unode;
    unode.setAddress(targetAddr);
    if (targetSeqNum != 0) unode.setSeqNum(targetSeqNum);
    unode_vec.push_back(unode);

    // set hop limit
    rerr->setMsgHdrHopLimit(MAX_HOPLIMIT);

    // add additional unreachableNode entries for all route entries that use the same routeNextHopAddress and routeNextHopInterface
    DYMO_RoutingEntry* brokenEntry = dymo_routingTable->getForAddress(IPv4Address(targetAddr));
    if (brokenEntry)
    {
        // sanity check
        if (!brokenEntry->routeBroken) throw std::runtime_error("sendRERR called for targetAddr that has a perfectly fine routing table entry");

        // add route entries with same routeNextHopAddress as broken route
        std::vector<DYMO_RoutingEntry *> RouteVector = dymo_routingTable->getRoutingTable();
        for (unsigned int i = 0; i < RouteVector.size(); i++)
        {
            DYMO_RoutingEntry* entry = RouteVector[i];
            if ((entry->routeNextHopAddress != brokenEntry->routeNextHopAddress) || (entry->routeNextHopInterface != brokenEntry->routeNextHopInterface)) continue;

            EV_DETAIL << "Including in RERR route to " << entry->routeAddress << " via " << entry->routeNextHopAddress << endl;

            DYMO_AddressBlock unode;
            unode.setAddress(entry->routeAddress.getInt());
            if (entry->routeSeqNum != 0) unode.setSeqNum(entry->routeSeqNum);
            unode_vec.push_back(unode);
        }
    }

    // wrap up and send
    rerr->setUnreachableNodes(unode_vec);
    sendDown(rerr, IPv4Address::LL_MANET_ROUTERS.getInt());

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
    DYMO_RoutingEntry* entry = dymo_routingTable->getForAddress(targetAddr.toIPv4());
    if (!entry) return;

    // TODO: not specified in draft, but seems to make sense
    if (entry->routeBroken) return;

    entry->routeUsed.start(ROUTE_USED_TIMEOUT);
    entry->routeDelete.cancel();
    rescheduleTimer();

    dymo_routingTable->maintainAssociatedRoutingTable();
    EV_INFO << "lifetimes of route to destination node " << targetAddr << " are up to date "  << endl;

    checkAndSendQueuedPkts(entry->routeAddress.getInt(), entry->routePrefix, (entry->routeNextHopAddress).getInt());
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
        DYMO_RoutingEntry* entry = dymo_routingTable->getForAddress(IPv4Address(outstandingRREQ.destAddr));
        if (entry && (!entry->routeBroken))
        {
            /** an entry was found in the routing table -> get control data from the table, encapsulate message **/
            EV_INFO << "RREQ timed out and we DO have a route" << endl;

            checkAndSendQueuedPkts(entry->routeAddress.getInt(), entry->routePrefix, entry->routeNextHopAddress.getInt());

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

        std::list<IPv4Datagram*> datagrams;
        // drop packets bound for the expired RREQ's destination
        dymo_routingTable->maintainAssociatedRoutingTable();
        queuedDataPackets->dropPacketsTo(IPv4Address(outstandingRREQ.destAddr), 32, &datagrams);
        while (!datagrams.empty())
        {
            IPv4Datagram* dgram = datagrams.front();
            datagrams.pop_front();
            sendICMP(dgram);
        }

        // clean up outstandingRREQList
        outstandingRREQList.del(&outstandingRREQ);

        return;
    }

    return;
}

bool DYMOFau::updateRoutesFromAddressBlock(const DYMO_AddressBlock& ab, bool isRREQ, uint32_t nextHopAddress, InterfaceEntry* nextHopInterface)
{
    DYMO_RoutingEntry* entry = dymo_routingTable->getForAddress(IPv4Address(ab.getAddress()));
    if (entry && !isRBlockBetter(entry, ab, isRREQ)) return false;

    if (!entry)
    {
        EV_INFO << "adding routing entry for " << IPv4Address(ab.getAddress()) << endl;
        entry = new DYMO_RoutingEntry(this);
        dymo_routingTable->addRoute(entry);
    }
    else
    {
        EV_INFO << "updating routing entry for " << IPv4Address(ab.getAddress()) << endl;
    }

    entry->routeAddress = IPv4Address(ab.getAddress());
    entry->routeSeqNum = ab.getSeqNum();
    entry->routeDist = ab.hasDist() ? (ab.getDist() + 1) : 0;  // incremented by one, because draft -10 says to first increment, then compare
    entry->routeNextHopAddress = IPv4Address(nextHopAddress);
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

    checkAndSendQueuedPkts(entry->routeAddress.getInt(), entry->routePrefix, nextHopAddress);

    return true;
}

DYMO_RM* DYMOFau::updateRoutes(DYMO_RM * pkt)
{
    EV_INFO << "starting update routes from routing blocks in the received message" << endl;
    std::vector<DYMO_AddressBlock> additional_nodes = pkt->getAdditionalNodes();
    std::vector<DYMO_AddressBlock> new_additional_nodes;

    bool isRREQ = (dynamic_cast<DYMO_RREQ*>(pkt) != 0);
    uint32_t nextHopAddress = getNextHopAddress(pkt);
    InterfaceEntry* nextHopInterface = getNextHopInterface(pkt);

    if (pkt->getOrigNode().getAddress()==myAddr) return NULL;
    bool origNodeEntryWasSuperior = updateRoutesFromAddressBlock(pkt->getOrigNode(), isRREQ, nextHopAddress, nextHopInterface);

    for (unsigned int i = 0; i < additional_nodes.size(); i++)
    {
        // TODO: not specified in draft, but seems to make sense
        if (additional_nodes[i].getAddress()==myAddr) return NULL;

        if (updateRoutesFromAddressBlock(additional_nodes[i], isRREQ, nextHopAddress, nextHopInterface))
        {
            /** read routing block is valid -> save block to the routing message **/
            new_additional_nodes.push_back(additional_nodes[i]);
        }
        else
        {
            EV_DETAIL << "AdditionalNode AddressBlock has no valid information  -> dropping block from routing message" << endl;
        }
    }

    if (!origNodeEntryWasSuperior)
    {
        EV_INFO << "OrigNode AddressBlock had no valid information -> deleting received routing message" << endl;
        return NULL;
    }

    pkt->setAdditionalNodes(new_additional_nodes);

    return pkt;
}

void DYMOFau::checkAndSendQueuedPkts(unsigned int destinationAddress, int prefix, unsigned int /*nextHopAddress*/)
{
    dymo_routingTable->maintainAssociatedRoutingTable();
    queuedDataPackets->dequeuePacketsTo(IPv4Address(destinationAddress), prefix);

    // clean up outstandingRREQList: remove those with matching destAddr
    DYMO_OutstandingRREQ* o = outstandingRREQList.getByDestAddr(destinationAddress, prefix);
    if (o) outstandingRREQList.del(o);
}

void DYMOFau::setMyAddr(unsigned int myAddr)
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

cModule* DYMOFau::getRouterByAddress(IPv4Address address)
{
    return dynamic_cast<cModule*>(simulation.getModule(address.getInt() - AUTOASSIGN_ADDRESS_BASE.getInt()));
}

/* Called for packets whose delivery fails at the link layer */
void DYMOFau::packetFailed(const IPv4Datagram *dgram)
{
    /* We don't care about link failures for broadcast or non-data packets */
    if (dgram->getDestAddress() == IPv4Address::ALLONES_ADDRESS || dgram->getDestAddress() == IPv4Address::LL_MANET_ROUTERS)
    {
        return;
    }
    EV_WARN << "LINK FAILURE for dest=" << dgram->getSrcAddress();
    DYMO_RoutingEntry *entry = dymo_routingTable->getByAddress(dgram->getDestAddress());
    if (entry)
    {
        IPv4Address nextHop = entry->routeNextHopAddress;
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

void DYMOFau::processLinkBreak(const cObject *details)
{
    const IPv4Datagram *dgram = dynamic_cast<const IPv4Datagram *>(details);
    if (dgram)
        packetFailed(dgram);
}

} // namespace inetmanet

} // namespace inet

