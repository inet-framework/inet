//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "xDYMO.h"
#include "InterfaceTableAccess.h"
#include "IPSocket.h"
#include "IPProtocolId_m.h"
#include "Ieee80211Frame_m.h"
#include "IPvXAddressResolver.h"
#include "IPv4ControlInfo.h"
#include "UDPControlInfo.h"
#include "NodeOperations.h"
#include "NotificationBoard.h"

DYMO_NAMESPACE_BEGIN

Define_Module(DYMO::xDYMO);

#define DYMO_EV EV << "DYMO at " << getHostName() << " "

//
// construction
//

xDYMO::xDYMO()
{
    interfaceTable = NULL;
    routingTable = NULL;
    networkProtocol = NULL;
    expungeTimer = NULL;
    nb = NULL;
}

xDYMO::~xDYMO()
{
    for (std::map<IPv4Address, RREQTimer *>::iterator it = targetAddressToRREQTimer.begin(); it != targetAddressToRREQTimer.end(); it++)
        cancelAndDelete(it->second);
    cancelAndDelete(expungeTimer);
    nb = NotificationBoardAccess().getIfExists(this);
    if (nb)
        nb->unsubscribe(this, NF_LINK_BREAK);
}

//
// module interface
//

void xDYMO::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == 0)
    {
        // DYMO parameters from RFC
        clientAddresses = par("clientAddresses");
        useMulticastRREP = par("useMulticastRREP");
        interfaces = par("interfaces");
        activeInterval = par("activeInterval");
        maxIdleTime = par("maxIdleTime");
        maxSequenceNumberLifetime = par("maxSequenceNumberLifetime");
        routeRREQWaitTime = par("routeRREQWaitTime");
        rreqHolddownTime = par("rreqHolddownTime");
        maxHopCount = par("maxHopCount");
        discoveryAttemptsMax = par("discoveryAttemptsMax");
        appendInformation = par("appendInformation");
        bufferSizePackets = par("bufferSizePackets");
        bufferSizeBytes = par("bufferSizeBytes");
        // DYMO extension parameters
        maxJitter = par("maxJitter");
        sendIntermediateRREP = par("sendIntermediateRREP");
        minHopLimit = par("minHopLimit");
        maxHopLimit = par("maxHopLimit");
        // context
        host = findContainingNode(this);
        nodeStatus = dynamic_cast<NodeStatus *>(host->getSubmodule("status"));
        interfaceTable = InterfaceTableAccess().get(this);
        routingTable = check_and_cast<IRoutingTable *>(getModuleByPath(par("routingTableModule")));
        networkProtocol = check_and_cast<INetfilter *>(getModuleByPath(par("networkProtocolModule")));
        // internal
        expungeTimer = new cMessage("ExpungeTimer");
    }
    else if (stage == 4)
    {
        IPvXAddressResolver addressResolver;
        cStringTokenizer tokenizer(clientAddresses);
        while (tokenizer.hasMoreTokens()) {
            const char * clientAddress = tokenizer.nextToken();
            char * slash = const_cast<char *>(strchr(clientAddress,'/'));
            if (slash)
                *slash = 0;
            const IPvXAddress address = addressResolver.resolve(clientAddress);
            int prefixLength = 32;
            if (slash)
            {
                int pLength = atoi(slash + 1);
                if (pLength < 0 || pLength > prefixLength)
                    throw cRuntimeError("invalid prefix length in 'clientAddresses' parameter: '%s/%s'", clientAddress, slash);
                prefixLength = pLength;
            }
            clientAddressAndPrefixLengthPairs.push_back(std::pair<IPv4Address, int>(address.get4(), prefixLength));
        }
    }
    else if (stage == 5)
    {
        IPSocket socket(gate("ipOut"));
        socket.registerProtocol(IP_PROT_MANET);

        nb = NotificationBoardAccess().get();
        nb->subscribe(this, NF_LINK_BREAK);
        networkProtocol->registerHook(0, this);
        if (isNodeUp())
            configureInterfaces();
    }
}

void xDYMO::handleMessage(cMessage * message)
{
    if (!isNodeUp())
        throw cRuntimeError("Routing protocol is not running");
    if (message->isSelfMessage())
        processSelfMessage(message);
    else
        processMessage(message);
}

//
// handling messages
//

void xDYMO::processSelfMessage(cMessage * message)
{
    if (message == expungeTimer)
        processExpungeTimer();
    else if (dynamic_cast<RREQWaitRREPTimer *>(message))
        processRREQWaitRREPTimer((RREQWaitRREPTimer *)message);
    else if (dynamic_cast<RREQBackoffTimer *>(message))
        processRREQBackoffTimer((RREQBackoffTimer *)message);
    else if (dynamic_cast<RREQHolddownTimer *>(message))
        processRREQHolddownTimer((RREQHolddownTimer *)message);
    else
        throw cRuntimeError("Unknown self message");
}

void xDYMO::processMessage(cMessage * message)
{
    if (dynamic_cast<UDPPacket *>(message))
        processUDPPacket((UDPPacket *)message);
    else
        throw cRuntimeError("Unknown message");
}

//
// route discovery
//

void xDYMO::startRouteDiscovery(const IPv4Address & target)
{
    DYMO_EV << "Starting route discovery: originator = " << getSelfAddress() << ", target = " << target << endl;
    ASSERT(!hasOngoingRouteDiscovery(target));
    sendRREQ(createRREQ(target, 0));
    scheduleRREQWaitRREPTimer(createRREQWaitRREPTimer(target, 0));
}

void xDYMO::retryRouteDiscovery(const IPv4Address & target, int retryCount)
{
    DYMO_EV << "Retrying route discovery: originator = " << getSelfAddress() << ", target = " << target << ", retry = " << retryCount << endl;
    ASSERT(hasOngoingRouteDiscovery(target));
    sendRREQ(createRREQ(target, retryCount));
    scheduleRREQWaitRREPTimer(createRREQWaitRREPTimer(target, retryCount));
}

void xDYMO::completeRouteDiscovery(const IPv4Address & target)
{
    DYMO_EV << "Completing route discovery: originator = " << getSelfAddress() << ", target = " << target << endl;
    ASSERT(hasOngoingRouteDiscovery(target));
    std::multimap<IPv4Address, IPv4Datagram *>::iterator lt = targetAddressToDelayedPackets.lower_bound(target);
    std::multimap<IPv4Address, IPv4Datagram *>::iterator ut = targetAddressToDelayedPackets.upper_bound(target);
    for (std::multimap<IPv4Address, IPv4Datagram *>::iterator it = lt; it != ut; it++)
        reinjectDelayedDatagram(it->second);
    eraseDelayedDatagrams(target);
}

void xDYMO::cancelRouteDiscovery(const IPv4Address & target)
{
    DYMO_EV << "Canceling route discovery: originator = " << getSelfAddress() << ", target = " << target << endl;
    ASSERT(hasOngoingRouteDiscovery(target));
    std::multimap<IPv4Address, IPv4Datagram *>::iterator lt = targetAddressToDelayedPackets.lower_bound(target);
    std::multimap<IPv4Address, IPv4Datagram *>::iterator ut = targetAddressToDelayedPackets.upper_bound(target);
    for (std::multimap<IPv4Address, IPv4Datagram *>::iterator it = lt; it != ut; it++)
        dropDelayedDatagram(it->second);
    eraseDelayedDatagrams(target);
}

bool xDYMO::hasOngoingRouteDiscovery(const IPv4Address & target)
{
    return targetAddressToRREQTimer.find(target) != targetAddressToRREQTimer.end();
}

//
// handling IP datagrams
//

void xDYMO::delayDatagram(IPv4Datagram * datagram)
{
    DYMO_EV << "Queuing datagram: source = " << datagram->getSrcAddress() << ", destination = " << datagram->getDestAddress() << endl;
    const IPv4Address & target = datagram->getDestAddress();
    targetAddressToDelayedPackets.insert(std::pair<IPv4Address, IPv4Datagram *>(target, datagram));
}

void xDYMO::reinjectDelayedDatagram(IPv4Datagram * datagram)
{
    DYMO_EV << "Sending queued datagram: source = " << datagram->getSrcAddress() << ", destination = " << datagram->getDestAddress() << endl;
    networkProtocol->reinjectQueuedDatagram(const_cast<const IPv4Datagram *>(datagram));
}

void xDYMO::dropDelayedDatagram(IPv4Datagram * datagram)
{
    DYMO_EV << "Dropping queued datagram: source = " << datagram->getSrcAddress() << ", destination = " << datagram->getDestAddress() << endl;
    networkProtocol->dropQueuedDatagram(const_cast<const IPv4Datagram *>(datagram));
}

void xDYMO::eraseDelayedDatagrams(const IPv4Address & target)
{
    std::multimap<IPv4Address, IPv4Datagram *>::iterator lt = targetAddressToDelayedPackets.lower_bound(target);
    std::multimap<IPv4Address, IPv4Datagram *>::iterator ut = targetAddressToDelayedPackets.upper_bound(target);
    targetAddressToDelayedPackets.erase(lt, ut);
}

bool xDYMO::hasDelayedDatagrams(const IPv4Address & target)
{
    return targetAddressToDelayedPackets.find(target) != targetAddressToDelayedPackets.end();
}

//
// handling RREQ timers
//

void xDYMO::cancelRREQTimer(const IPv4Address & target)
{
    std::map<IPv4Address, RREQTimer *>::iterator tt = targetAddressToRREQTimer.find(target);
    cancelEvent(tt->second);
}

void xDYMO::deleteRREQTimer(const IPv4Address & target)
{
    std::map<IPv4Address, RREQTimer *>::iterator tt = targetAddressToRREQTimer.find(target);
    delete tt->second;
}

void xDYMO::eraseRREQTimer(const IPv4Address & target)
{
    std::map<IPv4Address, RREQTimer *>::iterator tt = targetAddressToRREQTimer.find(target);
    targetAddressToRREQTimer.erase(tt);
}

//
// handling RREQ wait RREP timers
//

RREQWaitRREPTimer * xDYMO::createRREQWaitRREPTimer(const IPv4Address & target, int retryCount)
{
    RREQWaitRREPTimer * message = new RREQWaitRREPTimer("RREQWaitRREPTimer");
    message->setRetryCount(retryCount);
    message->setTarget(target);
    return message;
}

void xDYMO::scheduleRREQWaitRREPTimer(RREQWaitRREPTimer * message)
{
    DYMO_EV << "Scheduling RREQ wait RREP timer" << endl;
    targetAddressToRREQTimer[message->getTarget()] = message;
    scheduleAt(simTime() + routeRREQWaitTime, message);
}

void xDYMO::processRREQWaitRREPTimer(RREQWaitRREPTimer * message)
{
    DYMO_EV << "Processing RREQ wait RREP timer" << endl;
    const IPv4Address & target = message->getTarget();
    if (message->getRetryCount() == discoveryAttemptsMax - 1) {
        cancelRouteDiscovery(target);
        cancelRREQTimer(target);
        eraseRREQTimer(target);
        scheduleRREQHolddownTimer(createRREQHolddownTimer(target));
    }
    else
        scheduleRREQBackoffTimer(createRREQBackoffTimer(target, message->getRetryCount()));
    delete message;
}

//
// handling RREQ backoff timer
//

RREQBackoffTimer * xDYMO::createRREQBackoffTimer(const IPv4Address & target, int retryCount)
{
    RREQBackoffTimer * message = new RREQBackoffTimer("RREQBackoffTimer");
    message->setRetryCount(retryCount);
    message->setTarget(target);
    return message;
}

void xDYMO::scheduleRREQBackoffTimer(RREQBackoffTimer * message)
{
    DYMO_EV << "Scheduling RREQ backoff timer" << endl;
    targetAddressToRREQTimer[message->getTarget()] = message;
    scheduleAt(simTime() + computeRREQBackoffTime(message->getRetryCount()), message);
}

void xDYMO::processRREQBackoffTimer(RREQBackoffTimer * message)
{
    DYMO_EV << "Processing RREQ backoff timer" << endl;
    retryRouteDiscovery(message->getTarget(), message->getRetryCount() + 1);
    delete message;
}

simtime_t xDYMO::computeRREQBackoffTime(int retryCount)
{
    return pow(routeRREQWaitTime, retryCount);
}

//
// handling RREQ holddown timers
//

RREQHolddownTimer * xDYMO::createRREQHolddownTimer(const IPv4Address & target)
{
    RREQHolddownTimer * message = new RREQHolddownTimer("RREQHolddownTimer");
    message->setTarget(target);
    return message;
}

void xDYMO::scheduleRREQHolddownTimer(RREQHolddownTimer * message)
{
    DYMO_EV << "Scheduling RREQ holddown timer" << endl;
    targetAddressToRREQTimer[message->getTarget()] = message;
    scheduleAt(simTime() + rreqHolddownTime, message);
}

void xDYMO::processRREQHolddownTimer(RREQHolddownTimer * message)
{
    DYMO_EV << "Processing RREQ holddown timer" << endl;
    const IPv4Address & target = message->getTarget();
    eraseRREQTimer(target);
    if (hasDelayedDatagrams(target))
        startRouteDiscovery(target);
    delete message;
}

//
// handling UDP packets
//

void xDYMO::sendUDPPacket(UDPPacket * packet, double delay)
{
    if (delay == 0)
        send(packet, "ipOut");
    else
        sendDelayed(packet, delay, "ipOut");
}

void xDYMO::processUDPPacket(UDPPacket * packet)
{
    cPacket * encapsulatedPacket = packet->decapsulate();
    if (dynamic_cast<DYMOPacket *>(encapsulatedPacket)) {
        DYMOPacket * dymoPacket = (DYMOPacket *)encapsulatedPacket;
        dymoPacket->setControlInfo(packet->removeControlInfo());
        processDYMOPacket(dymoPacket);
    }
    else
        throw cRuntimeError("Unknown UDP packet");
    delete packet;
}

//
// handling DYMO packets
//

void xDYMO::sendDYMOPacket(DYMOPacket * packet, const InterfaceEntry * interfaceEntry, const IPv4Address & nextHop, double delay)
{
    IPv4ControlInfo * networkProtocolControlInfo = new IPv4ControlInfo();
    // 5.4. AODVv2 Packet Header Fields and Information Elements
    // In addition, IP Protocol Number 138 has been reserved for MANET protocols [RFC5498].
    networkProtocolControlInfo->setProtocol(IP_PROT_MANET);
    // The IPv4 TTL (IPv6 Hop Limit) field for all packets containing AODVv2 messages is set to 255.
    networkProtocolControlInfo->setTimeToLive(255);
    networkProtocolControlInfo->setDestAddr(nextHop);
    networkProtocolControlInfo->setSrcAddr(getSelfAddress());
    if (interfaceEntry)
        networkProtocolControlInfo->setInterfaceId(interfaceEntry->getInterfaceId());
    UDPPacket * udpPacket = new UDPPacket(packet->getName());
    udpPacket->encapsulate(packet);
    // In its default mode of operation, AODVv2 uses the UDP port 269 [RFC5498] to carry protocol packets.
    udpPacket->setSourcePort(DYMO_UDP_PORT);
    udpPacket->setDestinationPort(DYMO_UDP_PORT);
    udpPacket->setControlInfo(dynamic_cast<cObject *>(networkProtocolControlInfo));
    sendUDPPacket(udpPacket, delay);
}

void xDYMO::processDYMOPacket(DYMOPacket * packet)
{
    if (dynamic_cast<RREQ *>(packet))
        processRREQ((RREQ *)packet);
    else if (dynamic_cast<RREP *>(packet))
        processRREP((RREP *)packet);
    else if (dynamic_cast<RERR *>(packet))
        processRERR((RERR *)packet);
    else
        throw cRuntimeError("Unknown DYMO packet");
}

//
// handling RteMsg packets
//

bool xDYMO::permissibleRteMsg(RteMsg * rteMsg)
{
    // 7.5. Handling a Received RteMsg
    AddressBlock & originatorNode = rteMsg->getOriginatorNode();
    AddressBlock & targetNode = rteMsg->getTargetNode();
    IPv4ControlInfo * networkProtocolControlInfo = check_and_cast<IPv4ControlInfo *>(rteMsg->getControlInfo());
    // 1. HandlingRtr MUST handle AODVv2 messages only from adjacent
    //    routers as specified in Section 5.4. AODVv2 messages from other
    //    sources MUST be disregarded.
    //    5.4. AODVv2 Packet Header Fields and Information Elements
    //    If a packet is received with a value other than 255, any AODVv2
    //    message contained in the packet MUST be disregarded by AODVv2.
    // FIXME: we should rather compare with 255 but unfortunately IPv4 decrements
    // FIXME: TTL too early in the sender, see http://en.wikipedia.org/wiki/Time_to_live
    if (networkProtocolControlInfo->getTimeToLive() != 254)
        return false;
    // 2. If the RteMsg.<msg-hop-limit> is equal to 0, then the message is disregarded.
    if (rteMsg->getHopLimit() == 0)
        return false;
    // 3. If the RteMsg.<msg-hop-count> is present, and RteMsg.<msg-hop-
    //    count> >= MAX_HOPCOUNT, then the message is disregarded.
    if (rteMsg->getHopCount() >= maxHopCount)
        return false;
    // 4. HandlingRtr examines the RteMsg to ascertain that it contains the
    //    required information: TargNode.Addr, OrigNode.Addr,
    //    RteMsg_Gen.Metric and RteMsg_Gen.SeqNum.  If the required
    //    information does not exist, the message is disregarded.
    if (dynamic_cast<RREQ *>(rteMsg) && (!originatorNode.getHasMetric() || !originatorNode.getHasSequenceNumber()))
        return false;
    else if (dynamic_cast<RREP *>(rteMsg) && (!targetNode.getHasMetric() || !targetNode.getHasSequenceNumber()))
        return false;
    // 5. HandlingRtr checks that OrigNode.Addr and TargNode.Addr are valid
    //    routable unicast addresses.  If not, the message is disregarded.
    const IPv4Address & originatorAddress = originatorNode.getAddress();
    const IPv4Address & targetAddress = targetNode.getAddress();
    if (originatorAddress.isUnspecified() || originatorAddress.isMulticast() || originatorAddress.isLimitedBroadcastAddress() ||
        targetAddress.isUnspecified() || targetAddress.isMulticast() || targetAddress.isLimitedBroadcastAddress())
        return false;
    // 6. HandlingRtr checks that the Metric Type associated with
    //    OrigNode.Metric and TargNode.Metric is known, and that Cost(L)
    //    can be computed.  If not, the message is disregarded.
    //     *  DISCUSSION: alternatively, can change the AddrBlk metric to
    //        use HopCount, measured from<msg-hop-limit>.
    if (originatorNode.getMetricType() != HOP_COUNT || targetNode.getMetricType() != HOP_COUNT)
        return false;
    // 7. If MAX_METRIC[RteMsg.MetricType] <= (RteMsg_Gen.Metric +
    //    Cost(L)), where 'L' is the incoming link, the RteMsg is
    //    disregarded.
    // TODO: implement
    return true;
}

void xDYMO::processRteMsg(RteMsg * rteMsg)
{
    // 7.5. Handling a Received RteMsg
    // 1. HandlingRtr MUST process the routing information contained in the
    //    RteMsg as speciied in Section 6.1.
    if (dynamic_cast<RREQ *>(rteMsg))
        updateRoutes(rteMsg, rteMsg->getOriginatorNode());
    else if (dynamic_cast<RREP *>(rteMsg))
        updateRoutes(rteMsg, rteMsg->getTargetNode());
    // 2. HandlingRtr MAY process AddedNode routing information (if
    //    present) as specified in Section 13.7.1 Otherwise, if AddedNode
    //    information is not processed, it MUST be deleted.
    int count = rteMsg->getAddedNodeArraySize();
    for (int i = 0; i < count; i++)
        updateRoutes(rteMsg, rteMsg->getAddedNode(i));
    // 3. By sending the updated RteMsg, HandlingRtr advertises that it
    //    will route for addresses contained in the outgoing RteMsg based
    //    on the information enclosed.  HandlingRtr MAY choose not to send
    //    the RteMsg, though not resending this RteMsg could decrease
    //    connectivity in the network or result in a nonoptimal path.  The
    //    circumstances under which HandlingRtr might choose to not re-
    //    transmit a RteMsg are not specified in this document.  Some
    //    examples might include the following:
    //    * HandlingRtr is already heavily loaded and does not want to
    //      advertise routing for the contained addresses
    //    * HandlingRtr recently transmitted identical routing information
    //      (e.g. in a RteMsg advertising the same metric)
    //    * HandlingRtr is low on energy and has to reduce energy expended
    //      for sending protocol messages or packet forwarding
    //    Unless HandlingRtr is prepared to send an updated RteMsg, it
    //    halts processing.  Otherwise, processing continues as follows.
    // TODO: why is this here and how could we halt here?
    // 4. HandlingRtr MUST decrement RteMsg.<msg-hop-limit>.  If
    //    RteMsg.<msg-hop-limit> is then zero (0), no further action is taken.
    rteMsg->setHopLimit(rteMsg->getHopLimit() - 1);
    // 5. HandlingRtr MUST increment RteMsg.<msg-hop-count>.
    rteMsg->setHopCount(rteMsg->getHopCount() + 1);
}

int xDYMO::computeRteMsgBitLength(RteMsg * rteMsg)
{
    // TODO: validityTime, metric, metricType, TLVs
    // 1. <address-block> := <num-addr> <addr-flags> (<head-length><head>?)? (<tail-length><tail>?)? <mid>* <prefix-length>*
    int addressBlock = 8 + 8;
    // head-length and head are not used
    // tail-length and tail are not used
    // mid contains the originator and target addresses
    addressBlock += (2 + rteMsg->getAddedNodeArraySize()) * 4 * 8;
    // prefix-length is not used (assuming 8 * address-length bits)
    // 2. <tlv> := <tlv-type> <tlv-flags> <tlv-type-ext>? (<index-start><index-stop>?)? (<length><value>?)?
    int addressTLV = 8 + 8;
    // tlv-type-ext is not used
    // index-start and index-stop are not used
    // length is set to 2 + added node count
    addressTLV += 8;
    // sequence number values are included
    addressTLV += (2 + rteMsg->getAddedNodeArraySize()) * 2 * 8;
    // 3. <tlv-block> := <tlvs-length> <tlv>*
    int addressTLVBlock = 16;
    // there's exactly one tlv in the block
    addressTLVBlock += addressTLV;
    // 4. <tlv-block> := <tlvs-length> <tlv>*
    int messageTLVBlock = 16;
    // there's no TLV in the message TLV block
    // 5. <msg-header> := <msg-type> <msg-flags> <msg-addr-length> <msg-size> <msg-orig-addr>? <msg-hop-limit>? <msg-hop-count>? <msg-seq-num>?
    int messageHeader = 8 + 4 + 4 + 16;
    // msg-orig-addr is not used
    // msg-hop-limit and msg-hop-count are always included in the message header
    messageHeader += 8 + 8;
    // msg-seq-num is not used
    // 6. <message> := <msg-header> <tlv-block> (<addr-block><tlv-block>)*
    int message = messageHeader + messageTLVBlock;
    // there's exactly one address-block with one tlv-block in the message
    message += (addressBlock + addressTLVBlock);
    // 7. <pkt-header> := <version> <pkt-flags> <pkt-seq-num>? <tlv-block>?
    int packetHeader = 4 + 4;
    // pkt-seq-num is not used
    // tlv-block is not used
    // 8. <packet> := <pkt-header> <message>*
    int packet = packetHeader;
    // there's exactly one message in the packet
    packet += message;
    return packet;
}

//
// handling RREQ packets
//

RREQ * xDYMO::createRREQ(const IPv4Address & target, int retryCount)
{
    RREQ * rreq = new RREQ("RREQ");
    AddressBlock & originatorNode = rreq->getOriginatorNode();
    AddressBlock & targetNode = rreq->getTargetNode();
    // 7.3. RREQ Generation
    // 1. RREQ_Gen MUST increment its OwnSeqNum by one (1) according to the
    //    rules specified in Section 5.5.
    incrementSequenceNumber();
    // 2. OrigNode MUST be a unicast address.  If RREQ_Gen is not OrigNode,
    //    then OwnSeqNum will be used as the value of OrigNode.SeqNum. will
    //    be used by AODVv2 routers to create a route toward the OrigNode,
    //    enabling a RREP from TargRtr, and eventually used for proper
    //    forwarding of data packets.
    // 3. If RREQ_Gen requires that only TargRtr is allowed to generate a
    //    RREP, then RREQ_Gen includes the "Destination RREP Only" TLV as
    //    part of the RFC 5444 message header.  This also assures that
    //    TargRtr increments its sequence number.  Otherwise, intermediate
    //    AODVv2 routers MAY respond to the RREQ_Gen's RREQ if they have an
    //    valid route to TargNode (see Section 13.2).
    // 4. msg-hopcount MUST be set to 0.
    rreq->setHopCount(0);
    //    *  This RFC 5444 constraint causes the typical RteMsg payload
    //       incur additional enlargement.
    // 5. RREQ_Gen adds the TargNode.Addr to the RREQ.
    targetNode.setAddress(target);
    targetNode.setPrefixLength(32);
    // 6. If a previous value of the TargNode's SeqNum is known RREQ_Gen SHOULD
    //    include TargNode.SeqNum in all but the last RREQ attempt.
    std::map<IPv4Address, DYMOSequenceNumber>::iterator st = targetAddressToSequenceNumber.find(target);
    if (st != targetAddressToSequenceNumber.end() && retryCount < discoveryAttemptsMax - 1) {
        targetNode.setHasSequenceNumber(true);
        targetNode.setSequenceNumber(st->second);
    }
    else
        targetNode.setHasSequenceNumber(false);
    // 7. RREQ_Gen adds OrigNode.Addr, its prefix, and the RREQ_Gen.SeqNum (OwnSeqNum) to the RREQ.
    const IPv4Address & originator = getSelfAddress();
    originatorNode.setAddress(originator);
    originatorNode.setPrefixLength(32);
    originatorNode.setHasSequenceNumber(true);
    originatorNode.setSequenceNumber(sequenceNumber);
    // 8. If OrigNode.Metric is included it is set to the cost of the route
    //    between OrigNode and RREQ_Gen.
    originatorNode.setHasMetric(true);
    originatorNode.setMetric(0);
    originatorNode.setHasMetricType(true);
    originatorNode.setMetricType(HOP_COUNT);
    targetNode.setHasMetricType(true);
    targetNode.setMetricType(HOP_COUNT);
    // 13.1. Expanding Rings Multicast
    int hopLimit = minHopLimit + (maxHopLimit - minHopLimit) * retryCount / discoveryAttemptsMax;
    rreq->setHopLimit(hopLimit);
    return rreq;
}

void xDYMO::sendRREQ(RREQ * rreq)
{
    const IPv4Address & target = rreq->getTargetNode().getAddress();
    const IPv4Address & originator = rreq->getOriginatorNode().getAddress();
    rreq->setBitLength(computeRREQBitLength(rreq));
    DYMO_EV << "Sending RREQ: originator = " << originator << ", target = " << target << endl;
    sendDYMOPacket(rreq, NULL, IPv4Address::LL_MANET_ROUTERS, uniform(0, maxJitter).dbl());
}

void xDYMO::processRREQ(RREQ * rreqIncoming)
{
    const IPv4Address & target = rreqIncoming->getTargetNode().getAddress();
    const IPv4Address & originator = rreqIncoming->getOriginatorNode().getAddress();
    DYMO_EV << "Processing RREQ: originator = " << originator << ", target = " << target << endl;
    if (permissibleRteMsg(rreqIncoming)) {
        processRteMsg(rreqIncoming);
        // 7.5.1. Additional Handling for Outgoing RREQ
        // o If the upstream router is in the Blacklist, and Current_Time <
        //   BlacklistRmTime, then HandlingRtr MUST NOT transmit any outgoing
        //   RREQ, and processing is complete.
        // TODO: implement
        // o Otherwise, if the upstream router is in the Blacklist, and
        //   Current_Time >= BlacklistRmTime, then the upstream router SHOULD
        //   be removed from the Blacklist, and message processing continued.
        // TODO: implement
        if (isClientAddress(target)) {
            // o If TargNode is a client of HandlingRtr, then a RREP is generated
            //   by the HandlingRtr (i.e., TargRtr) and unicast to the upstream
            //   router towards the RREQ OrigNode, as specified in Section 7.4.
            //   Afterwards, TargRtr processing for the RREQ is complete.
            DYMO_EV << "Received RREQ for client: originator = " << originator << ", target = " << target << endl;
            if (useMulticastRREP)
                sendRREP(createRREP(rreqIncoming));
            else {
                IPv4Route * route = routingTable->findBestMatchingRoute(originator);
                RREP * rrep = createRREP(rreqIncoming, route);
                sendRREP(rrep, route);
            }
        }
        else {
            // o If HandlingRtr is not the TargetNode, then the outgoing RREQ (as
            //   altered by the procedure defined above) SHOULD be sent to the IP
            //   multicast address LL-MANET-Routers [RFC5498].  If the RREQ is
            //   unicast, the IP.DestinationAddress is set to the NextHopAddress.
            DYMO_EV << "Forwarding RREQ: originator = " << originator << ", target = " << target << endl;
            RREQ * rreqOutgoing = rreqIncoming->dup();
            if (appendInformation)
                addSelfNode(rreqOutgoing);
            sendRREQ(rreqOutgoing);
        }
    }
    else
        DYMO_EV << "Dropping non-permissible RREQ" << endl;
    delete rreqIncoming;
}

int xDYMO::computeRREQBitLength(RREQ * rreq)
{
    return computeRteMsgBitLength(rreq);
}

//
// handling RREP packets
//

RREP * xDYMO::createRREP(RteMsg * rteMsg)
{
    return createRREP(rteMsg, NULL);
}

RREP * xDYMO::createRREP(RteMsg * rteMsg, IPv4Route * route)
{
    DYMORouteData * routeData = check_and_cast<DYMORouteData *>(route->getProtocolData());
    RREP * rrep = new RREP("RREP");
    AddressBlock & originatorNode = rrep->getOriginatorNode();
    AddressBlock & targetNode = rrep->getTargetNode();
    // 1. RREP_Gen first uses the routing information to update its route
    //    table entry for OrigNode if necessary as specified in Section 6.2.
    // NOTE: this is already done
    // 2. RREP_Gen MUST increment its OwnSeqNum by one (1) according to
    //    the rules specified in Section 5.5.
    incrementSequenceNumber();
    // 3. RREP.AddrBlk[OrigNode] := RREQ.AddrBlk[OrigNode]
    originatorNode = AddressBlock(rteMsg->getOriginatorNode());
    // 4. RREP.AddrBlk[TargNode] := RREQ.AddrBlk[TargNode]
    targetNode = AddressBlock(rteMsg->getTargetNode());
    // 5. RREP.SeqNumTLV[OrigNode] := RREQ.SeqNumTLV[OrigNode]
    originatorNode.setHasSequenceNumber(true);
    originatorNode.setSequenceNumber(rteMsg->getOriginatorNode().getSequenceNumber());
    // 6. RREP.SeqNumTLV[TargNode] := OwnSeqNum
    targetNode.setHasSequenceNumber(true);
    targetNode.setSequenceNumber(sequenceNumber);
    // 7. If Route[TargNode].PfxLen/8 is equal to the number of bytes in
    //    the addresses of the RREQ (4 for IPv4, 16 for IPv6), then no
    //    <prefix-length> is included with the iRREP.  Otherwise,
    //    RREP.PfxLen[TargNode] := RREQ.PfxLen[TargNode] according to the
    //    rules of RFC 5444 AddrBlk encoding.
    // TODO: implement
    // 8. RREP.MetricType[TargNode] := Route[TargNode].MetricType
    targetNode.setHasMetricType(true);
    targetNode.setMetricType(routeData->getMetricType());
    // 9. RREP.Metric[TargNode] := Route[TargNode].Metric
    targetNode.setHasMetric(true);
    targetNode.setMetric(route->getMetric());
    // 10. <msg-hop-limit> SHOULD be set to RteMsg.<msg-hop-count>.
    rrep->setHopLimit(rteMsg->getHopCount());
    // 11. IP.DestinationAddr := Route[OrigNode].NextHop
    // NOTE: can't be done here, it is done later
    return rrep;
}

void xDYMO::sendRREP(RREP * rrep)
{
    const IPv4Address & target = rrep->getTargetNode().getAddress();
    const IPv4Address & originator = rrep->getOriginatorNode().getAddress();
    rrep->setBitLength(computeRREPBitLength(rrep));
    DYMO_EV << "Sending broadcast RREP: originator = " << originator << ", target = " << target << endl;
    sendDYMOPacket(rrep, NULL, IPv4Address::LL_MANET_ROUTERS, 0);
}

void xDYMO::sendRREP(RREP * rrep, IPv4Route * route)
{
    const IPv4Address & target = rrep->getTargetNode().getAddress();
    const IPv4Address & originator = rrep->getOriginatorNode().getAddress();
    const IPv4Address & nextHop = route->getGateway();
    rrep->setBitLength(computeRREPBitLength(rrep));
    DYMO_EV << "Sending unicast RREP: originator = " << originator << ", target = " << target << ", nextHop = " << nextHop << endl;
    sendDYMOPacket(rrep, route->getInterface(), nextHop, 0);
}

void xDYMO::processRREP(RREP * rrepIncoming)
{
    const IPv4Address & target = rrepIncoming->getTargetNode().getAddress();
    const IPv4Address & originator = rrepIncoming->getOriginatorNode().getAddress();
    DYMO_EV << "Processing RREP: originator = " << originator << ", target = " << target << endl;
    if (permissibleRteMsg(rrepIncoming)) {
        processRteMsg(rrepIncoming);
        // 7.5.2. Additional Handling for Outgoing RREP
        if (isClientAddress(originator)) {
            DYMO_EV << "Received RREP for client: originator = " << originator << ", target = " << target << endl;
            if (hasOngoingRouteDiscovery(target)) {
                completeRouteDiscovery(target);
                cancelRREQTimer(target);
                deleteRREQTimer(target);
                eraseRREQTimer(target);
            }
        }
        else {
            // o If HandlingRtr is not OrigRtr then the outgoing RREP is sent to
            //   the Route.NextHopAddress for the RREP.AddrBlk[OrigNode].  If no
            //   forwarding route exists to OrigNode, then a RERR SHOULD be
            //   transmitted to RREP.AddrBlk[TargNode].  See Table 1 for notational
            //   conventions; OrigRtr, OrigNode, and TargNode are routers named in
            //   the context of OrigRtr, that is, the router originating the RREQ
            //   to which the RREP is responding.
            DYMO_EV << "Forwarding RREP: originator = " << originator << ", target = " << target << endl;
            RREP * rrepOutgoing = rrepIncoming->dup();
            if (appendInformation)
                addSelfNode(rrepOutgoing);
            if (useMulticastRREP)
                sendRREP(rrepOutgoing);
            else {
                IPv4Route * route = routingTable->findBestMatchingRoute(originator);
                if (route)
                    sendRREP(rrepOutgoing, route);
                else
                    DYMO_EV << "No route found toward originator, dropping RREP: originator = " << originator << ", target = " << target << endl;
            }
        }
    }
    else
        DYMO_EV << "Dropping non-permissible RREQ" << endl;
    delete rrepIncoming;
}

int xDYMO::computeRREPBitLength(RREP * rrep)
{
    return computeRteMsgBitLength(rrep);
}

//
// handling RERR packets
//

RERR * xDYMO::createRERR(std::vector<IPv4Address> & unreachableAddresses)
{
    RERR * rerr = new RERR();
    for (int i = 0; i < (int)unreachableAddresses.size(); i++) {
        const IPv4Address & unreachableAddress = unreachableAddresses[i];
        AddressBlock * addressBlock = new AddressBlock();
        addressBlock->setAddress(unreachableAddress);
        addressBlock->setPrefixLength(32);
        addressBlock->setHasValidityTime(false);
        addressBlock->setHasMetric(false);
        addressBlock->setHasMetricType(false);
        std::map<IPv4Address, DYMOSequenceNumber>::iterator st = targetAddressToSequenceNumber.find(unreachableAddress);
        if (st != targetAddressToSequenceNumber.end()) {
            addressBlock->setHasSequenceNumber(true);
            addressBlock->setSequenceNumber(st->second);
        }
        else
            addressBlock->setHasSequenceNumber(false);
        int size = rerr->getUnreachableNodeArraySize();
        rerr->setUnreachableNodeArraySize(size + 1);
        rerr->setUnreachableNode(size, *addressBlock);
    }
    rerr->setHopLimit(maxHopLimit);
    return rerr;
}

void xDYMO::sendRERR(RERR * rerr)
{
    rerr->setBitLength(computeRERRBitLength(rerr));
    DYMO_EV << "Sending RERR: unreachableNodeCount = " << rerr->getUnreachableNodeArraySize() << endl;
    sendDYMOPacket(rerr, NULL, IPv4Address::LL_MANET_ROUTERS, 0);
}

void xDYMO::sendRERRForUndeliverablePacket(const IPv4Address & destination)
{
    DYMO_EV << "Sending RERR for undeliverable packet: destination = " << destination << endl;
    // 8.3.1. Case 1: Undeliverable Packet
    // The first case happens when the router receives a packet but does not
    // have a valid route for the destination of the packet.  In this case,
    // there is exactly one UnreachableNode to be included in the RERR's
    // AddrBlk.  RERR_dest SHOULD be the multicast address LL-MANET-Routers,
    // but RERR_Gen MAY instead set RERR_dest to be the next hop towards the
    // source IP address of the packet which was undeliverable.  In the
    // latter case, the PktSource MsgTLV MUST be included, containing the
    // the source IP address of the undeliverable packet.  If a value for
    // the UnreachableNode's SeqNum (UnreachableNode.SeqNum) is known, it
    // MUST be placed in the RERR.  Otherwise, if no Seqnum AddrTLV is
    // included, all nodes handling the RERR will assume their route through
    // RERR_Gen towards the UnreachableNode is no longer valid and flag
    // those routes as broken.  RERR_Gen MUST discard the packet or message
    // that triggered generation of the RERR.
    std::vector<IPv4Address> unreachableAddresses;
    unreachableAddresses.push_back(destination);
    sendRERR(createRERR(unreachableAddresses));
}

void xDYMO::sendRERRForBrokenLink(const InterfaceEntry * interfaceEntry, const IPv4Address & nextHop)
{
    DYMO_EV << "Sending RERR for broken link: nextHop = " << nextHop << endl;
    // 8.3.2. Case 2: Broken Link
    // The second case happens when the link breaks to an active downstream
    // neighbor (i.e., the next hop of an active route).  In this case,
    // RERR_dest MUST be the multicast address LL-MANET-Routers, except when
    // the optional feature of maintaining precursor lists is used as
    // specified in Section 13.3.  All Active, Idle and Expired routes that
    // use the broken link MUST be marked as Broken.  The set of
    // UnreachableNodes is initialized by identifying those Active routes
    // which use the broken link.  For each such Active Route, Route.Dest is
    // added to the set of Unreachable Nodes.  After the Active Routes using
    // the broken link have all been included as UnreachableNodes, idle
    // routes MAY also be included, as long as the packet size of the RERR
    // does not exceed the MTU of the physical medium.
    // If the set of UnreachableNodes is empty, no RERR is generated.
    // Otherwise, RERR_Gen generates a new RERR, and the address of each
    // UnreachableNode (IP.DestinationAddress from a data packet or
    // RREP.TargNode.IPv4Address) is inserted into an AddrBlock.  If a prefix is
    // known for the UnreachableNode.IPv4Address, it SHOULD be included.
    // Otherwise, the UnreachableNode.IPv4Address is assumed to be a host
    // address with a full length prefix.  The value for each
    // UnreachableNode's SeqNum (UnreachableNode.SeqNum) MUST be placed in a
    // SeqNum AddrTLV.  If none of UnreachableNode.Addr entries are
    // associated with known prefix lengths, then the AddrBLK SHOULD NOT
    // include any prefix-length information.  Otherwise, for each
    // UnreachableNode.Addr that does not have any associated prefix-length
    // information, the prefix-length for that address MUST be assigned to
    // zero.
    std::vector<IPv4Address> unreachableAddresses;
    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        IPv4Route * route = routingTable->getRoute(i);
        if (route->getSource() == this) {
            DYMORouteData * routeData = check_and_cast<DYMORouteData *>(route->getProtocolData());
            DYMORouteState routeState = getRouteState(routeData);
            if (routeState != BROKEN && route->getInterface() == interfaceEntry && route->getGateway() == nextHop)
            {
                DYMO_EV << "Marking route as broken: " << route << endl;
                // TODO delete route, but save its data for later update
                // route->setEnabled(false);
                routeData->setBroken(true);
                unreachableAddresses.push_back(route->getDestination());
            }
        }
    }
    if (unreachableAddresses.size() == 0)
        DYMO_EV << "No unreachable address found" << endl;
    else
        sendRERR(createRERR(unreachableAddresses));
}

void xDYMO::processRERR(RERR * rerrIncoming)
{
    DYMO_EV << "Processing RERR" << endl;
    // 8.4. Receiving and Handling RERR Messages
    // HandlingRtr examines the incoming RERR to assure that it contains
    // Msg.<msg-hop-limit> and at least one UnreachableNode.IPv4Address.  If the
    // required information does not exist, the incoming RERR message is
    // disregarded and further processing stopped.
    if (rerrIncoming->getHopLimit() == 0 || rerrIncoming->getUnreachableNodeArraySize() == 0)
        return;
    else {
        IPv4ControlInfo * networkProtocolControlInfo = check_and_cast<IPv4ControlInfo *>(rerrIncoming->getControlInfo());
        // Otherwise, for each UnreachableNode.IPv4Address, HandlingRtr searches its
        // route table for a route using longest prefix matching.  If no such
        // Route is found, processing is complete for that UnreachableNode.IPv4Address.
        std::vector<IPv4Address> unreachableAddresses;
        for (int i = 0; i < (int)rerrIncoming->getUnreachableNodeArraySize(); i++) {
            AddressBlock & addressBlock = rerrIncoming->getUnreachableNode(i);
            for (int j = 0; j < routingTable->getNumRoutes(); j++) {
                IPv4Route * route = routingTable->getRoute(j);
                if (route->getSource() == this) {
                    DYMORouteData *routeData = check_and_cast<DYMORouteData *>(route->getProtocolData());
                    const IPv4Address & unreachableAddress = addressBlock.getAddress();
                    // HandlingRtr verifies the following:
                    // 1. The UnreachableNode.IPv4Address is a routable unicast address.
                    // 2. Route.NextHopAddress is the same as RERR IP.SourceAddress.
                    // 3. Route.NextHopInterface is the same as the interface on which the
                    //    RERR was received.
                    // 4. The UnreachableNode.SeqNum is unknown, OR Route.SeqNum <=
                    //    UnreachableNode.SeqNum (using signed 16-bit arithmetic).
                    if (!unreachableAddress.isMulticast() && !unreachableAddress.isLimitedBroadcastAddress() &&
                        unreachableAddress == route->getDestination() &&
                        route->getGateway() == networkProtocolControlInfo->getSrcAddr() &&
                        route->getInterface()->getInterfaceId() == networkProtocolControlInfo->getInterfaceId() &&
                        (!addressBlock.getHasSequenceNumber() || routeData->getSequenceNumber() <= addressBlock.getSequenceNumber()))
                    {
                        // If the route satisfies all of the above conditions, HandlingRtr sets
                        // the Route.Broken flag for that route.
                        DYMO_EV << "Marking route as broken: " << route << endl;
                        // TODO delete route, but save its data for later update
                        // route->setEnabled(false);
                        routeData->setBroken(true);
                        unreachableAddresses.push_back(unreachableAddress);
                    }
                }
            }
        }
        if (unreachableAddresses.size() == 0)
            DYMO_EV << "No unreachable address found" << endl;
        else {
            // Furthermore, if Msg.<msg-hop-limit> is greater than 0, then HandlingRtr
            // adds the UnreachableNode address and TLV information to an AddrBlk for
            // delivery in the outgoing RERR message to one or more of HandlingRtr's
            // upstream neighbors.
            // If there are no UnreachableNode addresses to be transmitted in an
            // RERR to upstream routers, HandlingRtr MUST discard the RERR, and no
            // further action is taken.
            // Otherwise, Msg.<msg-hop-limit> is decremented by one (1) and
            RERR * rerrOutgoing = createRERR(unreachableAddresses);
            rerrOutgoing->setHopLimit(rerrIncoming->getHopLimit() - 1);
            // processing continues as follows:
            // o If precursor lists are (optionally) maintained, the outgoing RERR
            //   SHOULD be sent to the active precursors of the broken route as
            //   specified in Section 13.3.
            // o Otherwise, if the incoming RERR message was received at the LL-
            //   MANET-Routers [RFC5498] multicast address, the outgoing RERR
            //   SHOULD also be sent to LL-MANET-Routers.
            // o Otherwise, if the PktSource MsgTLV is present, and HandlingRtr has
            //   a Route to PktSource.Addr, then HandlingRtr MUST send the outgoing
            //   RERR to Route[PktSource.Addr].NextHop.
            // o Otherwise, the outgoing RERR MUST be sent to LL-MANET-Routers.
            sendRERR(rerrOutgoing);
        }
    }
    delete rerrIncoming;
}

int xDYMO::computeRERRBitLength(RERR * rerr)
{
    // TODO: validityTime, metric, metricType, TLVs
    // 1. <address-block> := <num-addr> <addr-flags> (<head-length><head>?)? (<tail-length><tail>?)? <mid>* <prefix-length>*
    int addressBlock = 8 + 8;
    // head-length and head are not used
    // tail-length and tail are not used
    // mid contains the originator and target addresses
    addressBlock += rerr->getUnreachableNodeArraySize() * 4 * 8;
    // prefix-length is not used (assuming 8 * address-length bits)
    // 2. <tlv> := <tlv-type> <tlv-flags> <tlv-type-ext>? (<index-start><index-stop>?)? (<length><value>?)?
    int addressTLV = 8 + 8;
    // tlv-type-ext is not used
    // index-start and index-stop are not used
    // length is set to 2 + added node count
    addressTLV += 8;
    // sequence number values are included
    addressTLV += rerr->getUnreachableNodeArraySize() * 2 * 8;
    // 3. <tlv-block> := <tlvs-length> <tlv>*
    int addressTLVBlock = 16;
    // there's exactly one tlv in the block
    addressTLVBlock += addressTLV;
    // 4. <tlv-block> := <tlvs-length> <tlv>*
    int messageTLVBlock = 16;
    // there's no TLV in the message TLV block
    // 5. <msg-header> := <msg-type> <msg-flags> <msg-addr-length> <msg-size> <msg-orig-addr>? <msg-hop-limit>? <msg-hop-count>? <msg-seq-num>?
    int messageHeader = 8 + 4 + 4 + 16;
    // msg-orig-addr is not used
    // msg-hop-limit is always included in the message header
    messageHeader += 8 + 8;
    // msg-hop-count is not used
    // msg-seq-num is not used
    // 6. <message> := <msg-header> <tlv-block> (<addr-block><tlv-block>)*
    int message = messageHeader + messageTLVBlock;
    // there's exactly one address-block with one tlv-block in the message
    message += (addressBlock + addressTLVBlock);
    // 7. <pkt-header> := <version> <pkt-flags> <pkt-seq-num>? <tlv-block>?
    int packetHeader = 4 + 4;
    // pkt-seq-num is not used
    // tlv-block is not used
    // 8. <packet> := <pkt-header> <message>*
    int packet = packetHeader;
    // there's exactly one message in the packet
    packet += message;
    return packet;
}

//
// handling routes
//

void xDYMO::updateRoutes(RteMsg * rteMsg, AddressBlock & addressBlock)
{
    const IPv4Address & address = addressBlock.getAddress();
    if (address == getSelfAddress())
        // we don't need to manage routes for our own address
        return;
    else {
        // 6.1. Evaluating Incoming Routing Information
        // HandRtr searches its route table to see if there is a route table
        // entry with the same MetricType of the RteMsg, matching RteMsg.Addr.
        IPv4Route * route = NULL;
        for (int i = 0; i < routingTable->getNumRoutes(); i++) {
            IPv4Route * routeCandidate = routingTable->getRoute(i);
            if (routeCandidate->getSource() == this) {
                DYMORouteData *routeData = check_and_cast<DYMORouteData *>(routeCandidate->getProtocolData());
                if (routeCandidate->getDestination() == address && routeData->getMetricType() == addressBlock.getMetricType()) {
                    route = routeCandidate;
                    break;
                }
            }
        }
        // If not, HandRtr creates a route table entry for RteMsg.Addr as described
        // in Section 6.2. Otherwise, HandRtr compares the incoming routing information
        // in RteMsg against the already stored routing information in the route table
        // entry (Route) for RteMsg.Addr, as described below.
        if (!route) {
            IPv4Route * route = createRoute(rteMsg, addressBlock);
            DYMO_EV << "Adding new route: " << route << endl;
            routingTable->addRoute(route);
        }
        else {
            DYMORouteData * routeData = check_and_cast<DYMORouteData *>(route->getProtocolData());
            // Offers improvement if
            // (RteMsg.SeqNum > Route.SeqNum) OR
            // {(RteMsg.SeqNum == Route.SeqNum) AND
            // [(RteMsg.Metric < Route.Metric) OR
            // ((Route.Broken == TRUE) && LoopFree (RteMsg, Route))]}    if
            if ((addressBlock.getSequenceNumber() > routeData->getSequenceNumber()) ||
                (addressBlock.getSequenceNumber() == routeData->getSequenceNumber() && addressBlock.getMetric() < route->getMetric()) ||
                (routeData->getBroken() && isLoopFree(rteMsg, route)))
            {
                // it's more recent, or it's not stale and is shorter, or it can safely repair a broken route
                // TODO: should we simply update the route instead? only if the route change notification is sent exactly once
                routingTable->removeRoute(route);
                DYMO_EV << "Updating existing route: " << route << endl;
                updateRoute(rteMsg, addressBlock, route);
                DYMO_EV << "Route updated: " << route << endl;
                routingTable->addRoute(route);
            }
        }
    }
}

IPv4Route * xDYMO::createRoute(RteMsg * rteMsg, AddressBlock & addressBlock)
{
    IPv4Route * route = new IPv4Route();
    route->setSourceType(IPv4Route::DYMO);
    route->setSource(this);
    route->setProtocolData(new DYMORouteData());
    updateRoute(rteMsg, addressBlock, route);
    return route;
}

void xDYMO::updateRoute(RteMsg * rteMsg, AddressBlock & addressBlock, IPv4Route * route)
{
    // 6.2. Applying Route Updates To Route Table Entries
    IPv4ControlInfo * networkProtocolControlInfo = check_and_cast<IPv4ControlInfo *>(rteMsg->getControlInfo());
    DYMORouteData * routeData = check_and_cast<DYMORouteData *>(route->getProtocolData());
    // Route.IPv4Address := RteMsg.Addr
    const IPv4Address & address = addressBlock.getAddress();
    route->setDestination(address);
    // If (RteMsg.PfxLen != 0), then Route.PfxLen := RteMsg.PfxLen
    route->setNetmask(IPv4Address::ALLONES_ADDRESS);
    // Route.SeqNum := RteMsg.SeqNum
    DYMOSequenceNumber sequenceNumber = addressBlock.getSequenceNumber();
    routeData->setSequenceNumber(sequenceNumber);
    targetAddressToSequenceNumber[address] = sequenceNumber;
    // Route.NextHopAddress := IP.SourceAddress (i.e., an address of the node from which the RteMsg was received)
    // note that DYMO packets are not routed on the IP level, so we can use the source address here
    route->setGateway(networkProtocolControlInfo->getSrcAddr());
    // Route.NextHopInterface is set to the interface on which RteMsg was received
    InterfaceEntry *interfaceEntry = interfaceTable->getInterfaceById(networkProtocolControlInfo->getInterfaceId());
    if (interfaceEntry)
        route->setInterface(interfaceEntry);
    // Route.Broken flag := FALSE
    routeData->setBroken(false);
    // If RteMsg.MetricType is included, then Route.MetricType := RteMsg.MetricType.  Otherwise, Route.MetricType := DEFAULT_METRIC_TYPE.
    // Route.MetricType := RteMsg.MetricType
    if (addressBlock.getHasMetricType())
        routeData->setMetricType(addressBlock.getMetricType());
    else
        routeData->setMetricType(HOP_COUNT);
    // Route.Metric := RteMsg.Metric
    route->setMetric(addressBlock.getMetric());
    // Route.LastUsed := Current_Time
    routeData->setLastUsed(simTime());
    // If RteMsg.VALIDITY_TIME is not included, then Route.ExpirationTime := MAXTIME, otherwise Route.ExpirationTime := Current_Time + RteMsg.VALIDITY_TIME
    if (addressBlock.getHasValidityTime())
        routeData->setExpirationTime(simTime() + addressBlock.getValidityTime());
    else
        routeData->setExpirationTime(SimTime::getMaxTime());
    scheduleExpungeTimer();
}

// TODO: use
int xDYMO::getLinkCost(const InterfaceEntry * interfaceEntry, DYMOMetricType metricType)
{
    switch (metricType) {
        case HOP_COUNT:
            return 1;
        default:
            throw cRuntimeError("Unknown metric type");
    }
}

bool xDYMO::isLoopFree(RteMsg * rteMsg, IPv4Route * route)
{
    // TODO: implement
    return true;
}

//
// handling expunge timer
//

void xDYMO::processExpungeTimer()
{
    DYMO_EV << "Processing expunge timer" << endl;
    expungeRoutes();
    scheduleExpungeTimer();
}

void xDYMO::scheduleExpungeTimer()
{
    DYMO_EV << "Scheduling expunge timer" << endl;
    simtime_t nextExpungeTime = getNextExpungeTime();
    if (nextExpungeTime == SimTime::getMaxTime()) {
        if (expungeTimer->isScheduled())
            cancelEvent(expungeTimer);
    }
    else {
        if (!expungeTimer->isScheduled())
            scheduleAt(nextExpungeTime, expungeTimer);
        else {
            if (expungeTimer->getArrivalTime() != nextExpungeTime) {
                cancelEvent(expungeTimer);
                scheduleAt(nextExpungeTime, expungeTimer);
            }
        }
    }
}

void xDYMO::expungeRoutes()
{
    DYMO_EV << "Expunging routes from routing table: routeCount = " << routingTable->getNumRoutes() << endl;
    // 6.3. Route Table Entry Timeouts
    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        IPv4Route * route = routingTable->getRoute(i);
        if (route->getSource() == this) {
            DYMORouteData * routeData = check_and_cast<DYMORouteData *>(route->getProtocolData());
            // An Active route MUST NOT be expunged
            // An Idle route SHOULD NOT be expunged
            // An Expired route MAY be expunged (least recently used first)
            // A route MUST be expunged if (Current_Time - Route.LastUsed) >= MAX_SEQNUM_LIFETIME.
            // A route MUST be expunged if Current_Time >= Route.ExpirationTime
            if ((getRouteState(routeData) == EXPIRED) ||
                (simTime() - routeData->getLastUsed() >= maxSequenceNumberLifetime) ||
                (simTime() >= routeData->getExpirationTime()))
            {
                DYMO_EV << "Expunging route: " << route << endl;
                routingTable->deleteRoute(route);
                i--;
            }
        }
    }
}

simtime_t xDYMO::getNextExpungeTime()
{
    simtime_t nextExpirationTime = SimTime::getMaxTime();
    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        IPv4Route * route = routingTable->getRoute(i);
        if (route->getSource() == this) {
            DYMORouteData * routeData = check_and_cast<DYMORouteData *>(route->getProtocolData());
            const simtime_t & expirationTime = routeData->getExpirationTime();
            if (expirationTime < nextExpirationTime)
                nextExpirationTime = expirationTime;
            const simtime_t & defaultExpirationTime = routeData->getLastUsed() + maxSequenceNumberLifetime;
            if (defaultExpirationTime < nextExpirationTime)
                nextExpirationTime = defaultExpirationTime;
        }
    }
    return nextExpirationTime;
}

DYMORouteState xDYMO::getRouteState(DYMORouteData * routeData)
{
    simtime_t lastUsed = routeData->getLastUsed();
    if (routeData->getBroken())
        return BROKEN;
    else if (lastUsed - simTime() <= activeInterval)
        return ACTIVE;
    else if (routeData->getExpirationTime() != SimTime::getMaxTime()) {
        if (simTime() >= routeData->getExpirationTime())
            return EXPIRED;
        else
            return TIMED;
    }
    else if (lastUsed - simTime() <= maxIdleTime)
        return IDLE;
    else
        return EXPIRED;
}

//
// configuration
//

bool xDYMO::isNodeUp()
{
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

void xDYMO::configureInterfaces()
{
    // join multicast groups
    cPatternMatcher interfaceMatcher(interfaces, false, true, false);
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        InterfaceEntry * interfaceEntry = interfaceTable->getInterface(i);
        if (interfaceEntry->isMulticast() && interfaceMatcher.matches(interfaceEntry->getName()))
            // Most AODVv2 messages are sent with the IP destination address set to the link-local
            // multicast address LL-MANET-Routers [RFC5498] unless otherwise specified. Therefore,
            // all AODVv2 routers MUST subscribe to LL-MANET-Routers [RFC5498] to receiving AODVv2 messages.
            interfaceEntry->joinMulticastGroup(IPv4Address::LL_MANET_ROUTERS);
    }
}

//
// address
//

std::string xDYMO::getHostName()
{
    return host->getFullName();
}

IPv4Address xDYMO::getSelfAddress()
{
    return routingTable->getRouterId();
}

bool xDYMO::isClientAddress(const IPv4Address & address)
{
    if (routingTable->isLocalAddress(address))
        return true;
    else {
        for (std::vector<std::pair<IPv4Address, int> >::iterator it = clientAddressAndPrefixLengthPairs.begin(); it != clientAddressAndPrefixLengthPairs.end(); it++)
            // TODO: check for prefix length too
            if (it->first == address)
                return true;
        return false;
    }
}

//
// added node
//

void xDYMO::addSelfNode(RteMsg * rteMsg)
{
    const IPv4Address & address = getSelfAddress();
    AddressBlock * addressBlock = new AddressBlock();
    addressBlock->setAddress(address);
    addressBlock->setPrefixLength(32);
    addressBlock->setHasValidityTime(false);
    addressBlock->setValidityTime(-1);
    addressBlock->setHasMetric(true);
    addressBlock->setMetric(0);
    addressBlock->setHasMetricType(true);
    addressBlock->setMetricType(HOP_COUNT);
    addressBlock->setHasSequenceNumber(true);
    addressBlock->setSequenceNumber(sequenceNumber);
    addNode(rteMsg, *addressBlock);
}

void xDYMO::addNode(RteMsg * rteMsg, AddressBlock & addressBlock)
{
    int size = rteMsg->getAddedNodeArraySize();
    rteMsg->setAddedNodeArraySize(size + 1);
    rteMsg->setAddedNode(size, addressBlock);
}

//
// sequence number
//

void xDYMO::incrementSequenceNumber()
{
    // 5.5.  AODVv2 Sequence Numbers
    // Most of the time, OwnSeqNum is incremented by simply adding one (1).
    // But to increment OwnSeqNum when it has the value of the largest possible
    // number representable as a 16-bit unsigned integer (i.e., 65,535), it MUST
    // be set to one (1). In other words, the sequence number after 65,535 is 1.
    sequenceNumber++;
    if (sequenceNumber == 0)
        sequenceNumber = 1;
}

//
// routing
//

INetfilter::IHook::Result xDYMO::ensureRouteForDatagram(IPv4Datagram * datagram)
{
    const IPv4Address & source = datagram->getSrcAddress();
    const IPv4Address & destination = datagram->getDestAddress();
    if (destination.isMulticast() || destination.isLimitedBroadcastAddress() || routingTable->isLocalAddress(destination))
        return ACCEPT;
    else {
        DYMO_EV << "Finding route: source = " << source << ", destination = " << destination << endl;
        IPv4Route * route = routingTable->findBestMatchingRoute(destination);
        DYMORouteData * routeData = route ? dynamic_cast<DYMORouteData *>(route->getProtocolData()) : NULL;
        bool broken = routeData && routeData->getBroken();
        if (route && !route->getGateway().isUnspecified() && !broken) {
            DYMO_EV << "Route found: source = " << source << ", destination = " << destination << ", route: " << route << endl;
            if (routeData)
                // 8.1. Handling Route Lifetimes During Packet Forwarding
                // Route.LastUsed := Current_Time, and the packet is forwarded to the route's next hop.
                routeData->setLastUsed(simTime());
            return ACCEPT;
        }
        else if (source.isUnspecified() || isClientAddress(source)) {
            DYMO_EV << (broken ? "Broken" : "Missing") << " route: source = " << source << ", destination = " << destination << endl;
            delayDatagram(datagram);
            if (!hasOngoingRouteDiscovery(destination))
                startRouteDiscovery(destination);
            else
                DYMO_EV << "Route discovery is in progress: originator = " << getSelfAddress() << ", target = " << destination << endl;
            return QUEUE;
        }
        else
            // the actual routing decision will be repeated in the network protocol
            return ACCEPT;
    }
}

//
// lifecycle
//


bool xDYMO::handleOperationStage(LifecycleOperation * operation, int stage, IDoneCallback * doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_APPLICATION_LAYER)
            configureInterfaces();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER)
            // TODO: send a RERR to notify peers about broken routes
            for (std::map<IPv4Address, RREQTimer *>::iterator it = targetAddressToRREQTimer.begin(); it != targetAddressToRREQTimer.end(); it++)
                cancelRouteDiscovery(it->first);
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (stage == NodeCrashOperation::STAGE_CRASH) {
            targetAddressToSequenceNumber.clear();
            targetAddressToRREQTimer.clear();
            targetAddressToDelayedPackets.clear();
        }
    }
    else throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

//
// notification
//

void xDYMO::receiveChangeNotification(int signalID, const cObject *obj)
{
    Enter_Method("receiveChangeNotification");
    if (signalID == NF_LINK_BREAK) {
        DYMO_EV << "Received link break" << endl;
        Ieee80211Frame *ieee80211Frame = dynamic_cast<Ieee80211Frame *>(const_cast<cObject*>(obj));
        if (ieee80211Frame) {
            IPv4Datagram * datagram = dynamic_cast<IPv4Datagram *>(ieee80211Frame->getEncapsulatedPacket());
            if (datagram) {
                // TODO: get nexthop and interface from the packet
                // IPv4ControlInfo * networkProtocolControlInfo = dynamic_cast<IPv4ControlInfo *>(datagram->getControlInfo());
                const IPv4Address & destination = datagram->getDestAddress();
                {
                    IPv4Route * route = routingTable->findBestMatchingRoute(destination);
                    if (route) {
                        const IPv4Address & nextHop = route->getGateway();
                        sendRERRForBrokenLink(route->getInterface(), nextHop);
                    }
                }
            }
        }
    }
}

DYMO_NAMESPACE_END
