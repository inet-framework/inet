//
// Copyright (C) 2013 Opensim Ltd.
// Author: Levente Meszaros
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

#include "inet/common/INETMath.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/routing/dymo/Dymo.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo.h"

namespace inet {

namespace dymo {

Define_Module(Dymo);

//
// construction
//

Dymo::Dymo() :
    clientAddresses(nullptr),
    useMulticastRREP(false),
    interfaces(nullptr),
    activeInterval(NaN),
    maxIdleTime(NaN),
    maxSequenceNumberLifetime(NaN),
    routeRREQWaitTime(NaN),
    rreqHolddownTime(NaN),
    maxHopCount(-1),
    discoveryAttemptsMax(-1),
    appendInformation(false),
    bufferSizePackets(-1),
    bufferSizeBytes(-1),
    maxJitter(NaN),
    sendIntermediateRREP(false),
    minHopLimit(-1),
    maxHopLimit(-1),
    host(nullptr),
    addressType(nullptr),
    interfaceTable(nullptr),
    routingTable(nullptr),
    networkProtocol(nullptr),
    expungeTimer(nullptr),
    sequenceNumber(0)
{
}

Dymo::~Dymo()
{
    for (auto & elem : targetAddressToRREQTimer)
        cancelAndDelete(elem.second);
    cancelAndDelete(expungeTimer);
}

//
// module interface
//

void Dymo::initialize(int stage)
{
    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        addressType = getSelfAddress().getAddressType();    // addressType need for handleStartOperation()  and handleStartOperation() called by RoutingProtocolBase::initialize();
    }

    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // Dymo parameters from RFC
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
        // Dymo extension parameters
        maxJitter = par("maxJitter");
        sendIntermediateRREP = par("sendIntermediateRREP");
        minHopLimit = par("minHopLimit");
        maxHopLimit = par("maxHopLimit");
        // context
        host = getContainingNode(this);
        interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        routingTable = getModuleFromPar<IRoutingTable>(par("routingTableModule"), this);
        networkProtocol = getModuleFromPar<INetfilter>(par("networkProtocolModule"), this);
        // internal
        expungeTimer = new cMessage("ExpungeTimer");
        L3AddressResolver addressResolver;
        cStringTokenizer tokenizer(clientAddresses);
        while (tokenizer.hasMoreTokens()) {
            const char *clientAddress = tokenizer.nextToken();
            char *slash = const_cast<char *>(strchr(clientAddress, '/'));
            if (slash)
                *slash = 0;
            const L3Address address = addressResolver.resolve(clientAddress);
            int prefixLength = address.getAddressType()->getMaxPrefixLength();
            if (slash) {
                int pLength = atoi(slash + 1);
                if (pLength < 0 || pLength > prefixLength)
                    throw cRuntimeError("invalid prefix length in 'clientAddresses' parameter: '%s/%s'", clientAddress, slash);
                prefixLength = pLength;
            }
            clientAddressAndPrefixLengthPairs.push_back(std::pair<L3Address, int>(address, prefixLength));
        }
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        registerService(Protocol::manet, nullptr, gate("ipIn"));
        registerProtocol(Protocol::manet, gate("ipOut"), nullptr);
        host->subscribe(linkBrokenSignal, this);
        networkProtocol->registerHook(0, this);
    }
}

void Dymo::handleMessageWhenUp(cMessage *message)
{
    if (message->isSelfMessage())
        processSelfMessage(message);
    else
        processMessage(message);
}

//
// handling messages
//

void Dymo::processSelfMessage(cMessage *message)
{
    if (message == expungeTimer)
        processExpungeTimer();
    else if (auto waitRrepTimer = dynamic_cast<RreqWaitRrepTimer *>(message))
        processRreqWaitRrepTimer(waitRrepTimer);
    else if (auto backoffTimer = dynamic_cast<RreqBackoffTimer *>(message))
        processRreqBackoffTimer(backoffTimer);
    else if (auto holddownTimer = dynamic_cast<RreqHolddownTimer *>(message))
        processRreqHolddownTimer(holddownTimer);
    else
        throw cRuntimeError("Unknown self message");
}

void Dymo::processMessage(cMessage *message)
{
    if (Packet *fp = dynamic_cast<Packet *>(message))
        processUdpPacket(fp);
    else
        throw cRuntimeError("Unknown message");
}

//
// route discovery
//

void Dymo::startRouteDiscovery(const L3Address& target)
{
    EV_INFO << "Starting route discovery: originator = " << getSelfAddress() << ", target = " << target << endl;
    ASSERT(!hasOngoingRouteDiscovery(target));
    sendRreq(createRreq(target, 0));
    scheduleRreqWaitRrepTimer(createRreqWaitRrepTimer(target, 0));
}

void Dymo::retryRouteDiscovery(const L3Address& target, int retryCount)
{
    EV_INFO << "Retrying route discovery: originator = " << getSelfAddress() << ", target = " << target << ", retry = " << retryCount << endl;
    ASSERT(hasOngoingRouteDiscovery(target));
    sendRreq(createRreq(target, retryCount));
    scheduleRreqWaitRrepTimer(createRreqWaitRrepTimer(target, retryCount));
}

void Dymo::completeRouteDiscovery(const L3Address& target)
{
    EV_INFO << "Completing route discovery: originator = " << getSelfAddress() << ", target = " << target << endl;
    ASSERT(hasOngoingRouteDiscovery(target));
    auto lt = targetAddressToDelayedPackets.lower_bound(target);
    auto ut = targetAddressToDelayedPackets.upper_bound(target);
    for (auto it = lt; it != ut; it++)
        reinjectDelayedDatagram(it->second);
    eraseDelayedDatagrams(target);
}

void Dymo::cancelRouteDiscovery(const L3Address& target)
{
    EV_INFO << "Canceling route discovery: originator = " << getSelfAddress() << ", target = " << target << endl;
    ASSERT(hasOngoingRouteDiscovery(target));
    auto lt = targetAddressToDelayedPackets.lower_bound(target);
    auto ut = targetAddressToDelayedPackets.upper_bound(target);
    for (auto it = lt; it != ut; it++)
        dropDelayedDatagram(it->second);
    eraseDelayedDatagrams(target);
}

bool Dymo::hasOngoingRouteDiscovery(const L3Address& target)
{
    return targetAddressToRREQTimer.find(target) != targetAddressToRREQTimer.end();
}

//
// handling IP datagrams
//

void Dymo::delayDatagram(Packet *datagram)
{
    const auto& networkHeader = getNetworkProtocolHeader(datagram);
    EV_INFO << "Queuing datagram: source = " << networkHeader->getSourceAddress() << ", destination = " << networkHeader->getDestinationAddress() << endl;
    const L3Address& target = networkHeader->getDestinationAddress();
    targetAddressToDelayedPackets.insert(std::pair<L3Address, Packet *>(target, datagram));
}

void Dymo::reinjectDelayedDatagram(Packet *datagram)
{
    const auto& networkHeader = getNetworkProtocolHeader(datagram);
    EV_INFO << "Sending queued datagram: source = " << networkHeader->getSourceAddress() << ", destination = " << networkHeader->getDestinationAddress() << endl;
    networkProtocol->reinjectQueuedDatagram(const_cast<const Packet *>(datagram));
}

void Dymo::dropDelayedDatagram(Packet *datagram)
{
    const auto& networkHeader = getNetworkProtocolHeader(datagram);
    EV_WARN << "Dropping queued datagram: source = " << networkHeader->getSourceAddress() << ", destination = " << networkHeader->getDestinationAddress() << endl;
    networkProtocol->dropQueuedDatagram(const_cast<const Packet *>(datagram));
}

void Dymo::eraseDelayedDatagrams(const L3Address& target)
{
    EV_DEBUG << "Erasing the list of delayed datagrams: originator = " << getSelfAddress() << ", destination = " << target << endl;
    auto lt = targetAddressToDelayedPackets.lower_bound(target);
    auto ut = targetAddressToDelayedPackets.upper_bound(target);
    targetAddressToDelayedPackets.erase(lt, ut);
}

bool Dymo::hasDelayedDatagrams(const L3Address& target)
{
    return targetAddressToDelayedPackets.find(target) != targetAddressToDelayedPackets.end();
}

//
// handling RREQ timers
//

void Dymo::cancelRreqTimer(const L3Address& target)
{
    auto tt = targetAddressToRREQTimer.find(target);
    cancelEvent(tt->second);
}

void Dymo::deleteRreqTimer(const L3Address& target)
{
    auto tt = targetAddressToRREQTimer.find(target);
    delete tt->second;
}

void Dymo::eraseRreqTimer(const L3Address& target)
{
    auto tt = targetAddressToRREQTimer.find(target);
    targetAddressToRREQTimer.erase(tt);
}

//
// handling RREQ wait RREP timers
//

RreqWaitRrepTimer *Dymo::createRreqWaitRrepTimer(const L3Address& target, int retryCount)
{
    RreqWaitRrepTimer *message = new RreqWaitRrepTimer("RreqWaitRrepTimer");
    message->setRetryCount(retryCount);
    message->setTarget(target);
    return message;
}

void Dymo::scheduleRreqWaitRrepTimer(RreqWaitRrepTimer *message)
{
    EV_DETAIL << "Scheduling RREQ wait RREP timer" << endl;
    targetAddressToRREQTimer[message->getTarget()] = message;
    scheduleAt(simTime() + routeRREQWaitTime, message);
}

void Dymo::processRreqWaitRrepTimer(RreqWaitRrepTimer *message)
{
    EV_DETAIL << "Processing RREQ wait RREP timer" << endl;
    const L3Address& target = message->getTarget();
    if (message->getRetryCount() == discoveryAttemptsMax - 1) {
        cancelRouteDiscovery(target);
        cancelRreqTimer(target);
        eraseRreqTimer(target);
        scheduleRreqHolddownTimer(createRreqHolddownTimer(target));
    }
    else
        scheduleRreqBackoffTimer(createRreqBackoffTimer(target, message->getRetryCount()));
    delete message;
}

//
// handling RREQ backoff timer
//

RreqBackoffTimer *Dymo::createRreqBackoffTimer(const L3Address& target, int retryCount)
{
    RreqBackoffTimer *message = new RreqBackoffTimer("RreqBackoffTimer");
    message->setRetryCount(retryCount);
    message->setTarget(target);
    return message;
}

void Dymo::scheduleRreqBackoffTimer(RreqBackoffTimer *message)
{
    EV_DETAIL << "Scheduling RREQ backoff timer" << endl;
    targetAddressToRREQTimer[message->getTarget()] = message;
    scheduleAt(simTime() + computeRreqBackoffTime(message->getRetryCount()), message);
}

void Dymo::processRreqBackoffTimer(RreqBackoffTimer *message)
{
    EV_DETAIL << "Processing RREQ backoff timer" << endl;
    retryRouteDiscovery(message->getTarget(), message->getRetryCount() + 1);
    delete message;
}

simtime_t Dymo::computeRreqBackoffTime(int retryCount)
{
    return pow(routeRREQWaitTime, retryCount);
}

//
// handling RREQ holddown timers
//

RreqHolddownTimer *Dymo::createRreqHolddownTimer(const L3Address& target)
{
    RreqHolddownTimer *message = new RreqHolddownTimer("RreqHolddownTimer");
    message->setTarget(target);
    return message;
}

void Dymo::scheduleRreqHolddownTimer(RreqHolddownTimer *message)
{
    EV_DETAIL << "Scheduling RREQ holddown timer" << endl;
    targetAddressToRREQTimer[message->getTarget()] = message;
    scheduleAt(simTime() + rreqHolddownTime, message);
}

void Dymo::processRreqHolddownTimer(RreqHolddownTimer *message)
{
    EV_DETAIL << "Processing RREQ holddown timer" << endl;
    const L3Address& target = message->getTarget();
    eraseRreqTimer(target);
    if (hasDelayedDatagrams(target))
        startRouteDiscovery(target);
    delete message;
}

//
// handling Udp packets
//

void Dymo::sendUdpPacket(cPacket *packet, double delay)
{
    if (delay == 0)
        send(packet, "ipOut");
    else
        sendDelayed(packet, delay, "ipOut");
}

void Dymo::processUdpPacket(Packet *packet)
{
    packet->popAtFront<UdpHeader>();
    processDymoPacket(packet, packet->peekDataAt<DymoPacket>(b(0), packet->getDataLength()));
    delete packet;
}

//
// handling Dymo packets
//

void Dymo::sendDymoPacket(const Ptr<DymoPacket>& packet, const InterfaceEntry *interfaceEntry, const L3Address& nextHop, double delay)
{
    // 5.4. AODVv2 Packet Header Fields and Information Elements
    // In addition, IP Protocol Number 138 has been reserved for MANET protocols [RFC5498].
    auto className = packet->getClassName();
    Packet *udpPacket = new Packet(!strncmp("inet::", className, 6) ? className + 6 : className);
    auto udpHeader = makeShared<UdpHeader>();
    udpPacket->addTag<PacketProtocolTag>()->setProtocol(&Protocol::manet);
    // In its default mode of operation, AODVv2 uses the Udp port 269 [RFC5498] to carry protocol packets.
    udpHeader->setSourcePort(DYMO_UDP_PORT);
    udpHeader->setDestinationPort(DYMO_UDP_PORT);
    udpHeader->setCrcMode(CRC_DISABLED);
    udpPacket->addTag<DispatchProtocolReq>()->setProtocol(addressType->getNetworkProtocol());
    if (interfaceEntry)
        udpPacket->addTag<InterfaceReq>()->setInterfaceId(interfaceEntry->getInterfaceId());
    auto addresses = udpPacket->addTag<L3AddressReq>();
    addresses->setSrcAddress(getSelfAddress());
    addresses->setDestAddress(nextHop);
    // The Ipv4 TTL (Ipv6 Hop Limit) field for all packets containing AODVv2 messages is set to 255.
    udpPacket->addTag<HopLimitReq>()->setHopLimit(255);
    udpPacket->insertAtFront(udpHeader);
    udpPacket->insertAtBack(packet);
    sendUdpPacket(udpPacket, delay);
}

void Dymo::processDymoPacket(Packet *packet, const Ptr<const DymoPacket>& dymoPacket)
{
    auto dymoPacketCopy = dymoPacket->dupShared();
    if (auto rreq = dynamicPtrCast<const Rreq>(dymoPacketCopy))
        processRreq(packet, rreq);
    else if (auto rrep = dynamicPtrCast<const Rrep>(dymoPacketCopy))
        processRrep(packet, rrep);
    else if (auto rerr = dynamicPtrCast<const Rerr>(dymoPacketCopy))
        processRerr(packet, rerr);
    else
        throw cRuntimeError("Unknown Dymo packet");
}

//
// handling RteMsg packets
//

bool Dymo::permissibleRteMsg(Packet *packet, const Ptr<const RteMsg>& rteMsg)
{
    // 7.5. Handling a Received RteMsg
    const AddressBlock& originatorNode = rteMsg->getOriginatorNode();
    const AddressBlock& targetNode = rteMsg->getTargetNode();
    // 1. HandlingRtr MUST handle AODVv2 messages only from adjacent
    //    routers as specified in Section 5.4. AODVv2 messages from other
    //    sources MUST be disregarded.
    //    5.4. AODVv2 Packet Header Fields and Information Elements
    //    If a packet is received with a value other than 255, any AODVv2
    //    message contained in the packet MUST be disregarded by AODVv2.
    if (packet->getTag<HopLimitInd>()->getHopLimit() != 255)
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
    if (dynamicPtrCast<const Rreq>(rteMsg) && (!originatorNode.getHasMetric() || !originatorNode.getHasSequenceNumber()))
        return false;
    else if (dynamicPtrCast<const Rrep>(rteMsg) && (!targetNode.getHasMetric() || !targetNode.getHasSequenceNumber()))
        return false;
    // 5. HandlingRtr checks that OrigNode.Addr and TargNode.Addr are valid
    //    routable unicast addresses.  If not, the message is disregarded.
    const L3Address& originatorAddress = originatorNode.getAddress();
    const L3Address& targetAddress = targetNode.getAddress();
    if (originatorAddress.isUnspecified() || originatorAddress.isMulticast() || originatorAddress.isBroadcast() ||
        targetAddress.isUnspecified() || targetAddress.isMulticast() || targetAddress.isBroadcast())
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

void Dymo::processRteMsg(Packet *packet, const Ptr<const RteMsg>& rteMsg)
{
    // 7.5. Handling a Received RteMsg
    // 1. HandlingRtr MUST process the routing information contained in the
    //    RteMsg as speciied in Section 6.1.
    if (dynamicPtrCast<const Rreq>(rteMsg))
        updateRoutes(packet, rteMsg, rteMsg->getOriginatorNode());
    else if (dynamicPtrCast<const Rrep>(rteMsg))
        updateRoutes(packet, rteMsg, rteMsg->getTargetNode());
    // 2. HandlingRtr MAY process AddedNode routing information (if
    //    present) as specified in Section 13.7.1 Otherwise, if AddedNode
    //    information is not processed, it MUST be deleted.
    int count = rteMsg->getAddedNodeArraySize();
    for (int i = 0; i < count; i++)
        updateRoutes(packet, rteMsg, rteMsg->getAddedNode(i));
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
    // KLUDGE: TODO: constPtrCast<RteMsg>(rteMsg)
    constPtrCast<RteMsg>(rteMsg)->setHopLimit(rteMsg->getHopLimit() - 1);
    // 5. HandlingRtr MUST increment RteMsg.<msg-hop-count>.
    constPtrCast<RteMsg>(rteMsg)->setHopCount(rteMsg->getHopCount() + 1);
}

b Dymo::computeRteMsgLength(const Ptr<RteMsg>& rteMsg)
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
    return b(packet);
}

//
// handling RREQ packets
//

const Ptr<Rreq> Dymo::createRreq(const L3Address& target, int retryCount)
{
    auto rreq = makeShared<Rreq>(); // TODO: "RREQ");
    AddressBlock& originatorNode = rreq->getOriginatorNodeForUpdate();
    AddressBlock& targetNode = rreq->getTargetNodeForUpdate();
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
    targetNode.setPrefixLength(addressType->getMaxPrefixLength());
    // 6. If a previous value of the TargNode's SeqNum is known RREQ_Gen SHOULD
    //    include TargNode.SeqNum in all but the last RREQ attempt.
    auto st = targetAddressToSequenceNumber.find(target);
    if (st != targetAddressToSequenceNumber.end() && retryCount < discoveryAttemptsMax - 1) {
        targetNode.setHasSequenceNumber(true);
        targetNode.setSequenceNumber(st->second);
    }
    else
        targetNode.setHasSequenceNumber(false);
    // 7. RREQ_Gen adds OrigNode.Addr, its prefix, and the RREQ_Gen.SeqNum (OwnSeqNum) to the RREQ.
    const L3Address& originator = getSelfAddress();
    originatorNode.setAddress(originator);
    originatorNode.setPrefixLength(addressType->getMaxPrefixLength());
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

void Dymo::sendRreq(const Ptr<Rreq> &rreq)
{
    const L3Address& target = rreq->getTargetNode().getAddress();
    const L3Address& originator = rreq->getOriginatorNode().getAddress();
    rreq->setChunkLength(computeRreqLength(rreq));
    EV_DETAIL << "Sending RREQ: originator = " << originator << ", target = " << target << endl;
    sendDymoPacket(rreq, nullptr, addressType->getLinkLocalManetRoutersMulticastAddress(), uniform(0, maxJitter).dbl());
}

void Dymo::processRreq(Packet *packet, const Ptr<const Rreq>& rreqIncoming)
{
    const L3Address& target = rreqIncoming->getTargetNode().getAddress();
    const L3Address& originator = rreqIncoming->getOriginatorNode().getAddress();
    EV_DETAIL << "Processing RREQ: originator = " << originator << ", target = " << target << endl;
    if (permissibleRteMsg(packet, rreqIncoming)) {
        processRteMsg(packet, rreqIncoming);
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
            EV_DETAIL << "Received RREQ for client: originator = " << originator << ", target = " << target << endl;
            if (useMulticastRREP)
                sendRrep(createRrep(rreqIncoming));
            else {
                IRoute *route = routingTable->findBestMatchingRoute(originator);
                auto rrep = createRrep(rreqIncoming, route);
                sendRrep(rrep, route);
            }
        }
        else {
            // o If HandlingRtr is not the TargetNode, then the outgoing RREQ (as
            //   altered by the procedure defined above) SHOULD be sent to the IP
            //   multicast address LL-MANET-Routers [RFC5498].  If the RREQ is
            //   unicast, the IP.DestinationAddress is set to the NextHopAddress.
            EV_DETAIL << "Forwarding RREQ: originator = " << originator << ", target = " << target << endl;
            auto rreqOutgoing = dynamicPtrCast<Rreq>(rreqIncoming->dupShared());
            if (appendInformation)
                addSelfNode(rreqOutgoing);
            sendRreq(rreqOutgoing);
        }
    }
    else
        EV_WARN << "Dropping non-permissible RREQ" << endl;
}

b Dymo::computeRreqLength(const Ptr<Rreq>& rreq)
{
    return computeRteMsgLength(rreq);
}

//
// handling RREP packets
//

const Ptr<Rrep> Dymo::createRrep(const Ptr<const RteMsg>& rteMsg)
{
    return createRrep(rteMsg, nullptr);
}

const Ptr<Rrep> Dymo::createRrep(const Ptr<const RteMsg>& rteMsg, IRoute *route)
{
    DymoRouteData *routeData = check_and_cast<DymoRouteData *>(route->getProtocolData());
    auto rrep = makeShared<Rrep>(); // TODO: "RREP");
    AddressBlock& originatorNode = rrep->getOriginatorNodeForUpdate();
    AddressBlock& targetNode = rrep->getTargetNodeForUpdate();
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
    //    the addresses of the RREQ (4 for Ipv4, 16 for Ipv6), then no
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

void Dymo::sendRrep(const Ptr<Rrep>& rrep)
{
    const L3Address& target = rrep->getTargetNode().getAddress();
    const L3Address& originator = rrep->getOriginatorNode().getAddress();
    rrep->setChunkLength(computeRrepLength(rrep));
    EV_DETAIL << "Sending broadcast RREP: originator = " << originator << ", target = " << target << endl;
    sendDymoPacket(rrep, nullptr, addressType->getLinkLocalManetRoutersMulticastAddress(), 0);
}

void Dymo::sendRrep(const Ptr<Rrep>& rrep, IRoute *route)
{
    const L3Address& target = rrep->getTargetNode().getAddress();
    const L3Address& originator = rrep->getOriginatorNode().getAddress();
    const L3Address& nextHop = route->getNextHopAsGeneric();
    rrep->setChunkLength(computeRrepLength(rrep));
    EV_DETAIL << "Sending unicast RREP: originator = " << originator << ", target = " << target << ", nextHop = " << nextHop << endl;
    sendDymoPacket(rrep, route->getInterface(), nextHop, 0);
}

void Dymo::processRrep(Packet *packet, const Ptr<const Rrep>& rrepIncoming)
{
    const L3Address& target = rrepIncoming->getTargetNode().getAddress();
    const L3Address& originator = rrepIncoming->getOriginatorNode().getAddress();
    EV_DETAIL << "Processing RREP: originator = " << originator << ", target = " << target << endl;
    if (permissibleRteMsg(packet, rrepIncoming)) {
        processRteMsg(packet, rrepIncoming);
        // 7.5.2. Additional Handling for Outgoing RREP
        if (isClientAddress(originator)) {
            EV_DETAIL << "Received RREP for client: originator = " << originator << ", target = " << target << endl;
            if (hasOngoingRouteDiscovery(target)) {
                completeRouteDiscovery(target);
                cancelRreqTimer(target);
                deleteRreqTimer(target);
                eraseRreqTimer(target);
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
            EV_DETAIL << "Forwarding RREP: originator = " << originator << ", target = " << target << endl;
            auto rrepOutgoing = staticPtrCast<Rrep>(rrepIncoming->dupShared());
            if (appendInformation)
                addSelfNode(rrepOutgoing);
            if (useMulticastRREP)
                sendRrep(rrepOutgoing);
            else {
                IRoute *route = routingTable->findBestMatchingRoute(originator);
                if (route)
                    sendRrep(rrepOutgoing, route);
                else
                    EV_WARN << "No route found toward originator, dropping RREP: originator = " << originator << ", target = " << target << endl;
            }
        }
    }
    else
        EV_WARN << "Dropping non-permissible RREQ" << endl;
}

b Dymo::computeRrepLength(const Ptr<Rrep>& rrep)
{
    return computeRteMsgLength(rrep);
}

//
// handling RERR packets
//

const Ptr<Rerr> Dymo::createRerr(std::vector<L3Address>& unreachableAddresses)
{
    auto rerr = makeShared<Rerr>(); // TODO: "RERR");
    for (auto & unreachableAddresse : unreachableAddresses) {
        const L3Address& unreachableAddress = unreachableAddresse;
        AddressBlock *addressBlock = new AddressBlock();
        addressBlock->setAddress(unreachableAddress);
        addressBlock->setPrefixLength(addressType->getMaxPrefixLength());
        addressBlock->setHasValidityTime(false);
        addressBlock->setHasMetric(false);
        addressBlock->setHasMetricType(false);
        auto st = targetAddressToSequenceNumber.find(unreachableAddress);
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

void Dymo::sendRerr(const Ptr<Rerr>& rerr)
{
    rerr->setChunkLength(computeRerrLength(rerr));
    EV_DETAIL << "Sending RERR: unreachableNodeCount = " << rerr->getUnreachableNodeArraySize() << endl;
    sendDymoPacket(rerr, nullptr, addressType->getLinkLocalManetRoutersMulticastAddress(), 0);
}

void Dymo::sendRerrForUndeliverablePacket(const L3Address& destination)
{
    EV_DETAIL << "Sending RERR for undeliverable packet: destination = " << destination << endl;
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
    std::vector<L3Address> unreachableAddresses;
    unreachableAddresses.push_back(destination);
    sendRerr(createRerr(unreachableAddresses));
}

void Dymo::sendRerrForBrokenLink(const InterfaceEntry *interfaceEntry, const L3Address& nextHop)
{
    EV_DETAIL << "Sending RERR for broken link: nextHop = " << nextHop << endl;
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
    // RREP.TargNode.Address) is inserted into an AddrBlock.  If a prefix is
    // known for the UnreachableNode.Address, it SHOULD be included.
    // Otherwise, the UnreachableNode.Address is assumed to be a host
    // address with a full length prefix.  The value for each
    // UnreachableNode's SeqNum (UnreachableNode.SeqNum) MUST be placed in a
    // SeqNum AddrTLV.  If none of UnreachableNode.Addr entries are
    // associated with known prefix lengths, then the AddrBLK SHOULD NOT
    // include any prefix-length information.  Otherwise, for each
    // UnreachableNode.Addr that does not have any associated prefix-length
    // information, the prefix-length for that address MUST be assigned to
    // zero.
    std::vector<L3Address> unreachableAddresses;
    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        IRoute *route = routingTable->getRoute(i);
        if (route->getSource() == this) {
            DymoRouteData *routeData = check_and_cast<DymoRouteData *>(route->getProtocolData());
            DymoRouteState routeState = getRouteState(routeData);
            if (routeState != BROKEN && route->getInterface() == interfaceEntry && route->getNextHopAsGeneric() == nextHop) {
                EV_DETAIL << "Marking route as broken: " << route << endl;
                // TODO delete route, but save its data for later update
                // route->setEnabled(false);
                routeData->setBroken(true);
                unreachableAddresses.push_back(route->getDestinationAsGeneric());
            }
        }
    }
    if (unreachableAddresses.size() == 0)
        EV_DETAIL << "No unreachable address found" << endl;
    else
        sendRerr(createRerr(unreachableAddresses));
}

void Dymo::processRerr(Packet *packet, const Ptr<const Rerr>& rerrIncoming)
{
    EV_DETAIL << "Processing RERR" << endl;
    // 8.4. Receiving and Handling RERR Messages
    // HandlingRtr examines the incoming RERR to assure that it contains
    // Msg.<msg-hop-limit> and at least one UnreachableNode.Address.  If the
    // required information does not exist, the incoming RERR message is
    // disregarded and further processing stopped.
    if (rerrIncoming->getHopLimit() == 0 || rerrIncoming->getUnreachableNodeArraySize() == 0)
        return;
    else {
        L3Address srcAddr = packet->getTag<L3AddressInd>()->getSrcAddress();
        auto incomingIfTag = packet->getTag<InterfaceInd>();
        // Otherwise, for each UnreachableNode.Address, HandlingRtr searches its
        // route table for a route using longest prefix matching.  If no such
        // Route is found, processing is complete for that UnreachableNode.Address.
        std::vector<L3Address> unreachableAddresses;
        for (int i = 0; i < (int)rerrIncoming->getUnreachableNodeArraySize(); i++) {
            const AddressBlock& addressBlock = rerrIncoming->getUnreachableNode(i);
            for (int j = 0; j < routingTable->getNumRoutes(); j++) {
                IRoute *route = routingTable->getRoute(j);
                if (route->getSource() == this) {
                    DymoRouteData *routeData = check_and_cast<DymoRouteData *>(route->getProtocolData());
                    const L3Address& unreachableAddress = addressBlock.getAddress();
                    // HandlingRtr verifies the following:
                    // 1. The UnreachableNode.Address is a routable unicast address.
                    // 2. Route.NextHopAddress is the same as RERR IP.SourceAddress.
                    // 3. Route.NextHopInterface is the same as the interface on which the
                    //    RERR was received.
                    // 4. The UnreachableNode.SeqNum is unknown, OR Route.SeqNum <=
                    //    UnreachableNode.SeqNum (using signed 16-bit arithmetic).
                    if (unreachableAddress.isUnicast() &&
                        unreachableAddress == route->getDestinationAsGeneric() &&
                        route->getNextHopAsGeneric() == srcAddr &&
                        route->getInterface()->getInterfaceId() == incomingIfTag->getInterfaceId() &&
                        (!addressBlock.getHasSequenceNumber() || routeData->getSequenceNumber() <= addressBlock.getSequenceNumber()))
                    {
                        // If the route satisfies all of the above conditions, HandlingRtr sets
                        // the Route.Broken flag for that route.
                        EV_DETAIL << "Marking route as broken: " << route << endl;
                        // TODO delete route, but save its data for later update
                        // route->setEnabled(false);
                        routeData->setBroken(true);
                        unreachableAddresses.push_back(unreachableAddress);
                    }
                }
            }
        }
        if (unreachableAddresses.size() == 0)
            EV_DETAIL << "No unreachable address found" << endl;
        else {
            // Furthermore, if Msg.<msg-hop-limit> is greater than 0, then HandlingRtr
            // adds the UnreachableNode address and TLV information to an AddrBlk for
            // delivery in the outgoing RERR message to one or more of HandlingRtr's
            // upstream neighbors.
            // If there are no UnreachableNode addresses to be transmitted in an
            // RERR to upstream routers, HandlingRtr MUST discard the RERR, and no
            // further action is taken.
            // Otherwise, Msg.<msg-hop-limit> is decremented by one (1) and
            auto rerrOutgoing = createRerr(unreachableAddresses);
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
            sendRerr(rerrOutgoing);
        }
    }
}

b Dymo::computeRerrLength(const Ptr<Rerr>& rerr)
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
    return b(packet);
}

//
// handling routes
//

void Dymo::updateRoutes(Packet *packet, const Ptr<const RteMsg>& rteMsg, const AddressBlock& addressBlock)
{
    const L3Address& address = addressBlock.getAddress();
    if (address == getSelfAddress())
        // we don't need to manage routes for our own address
        return;
    else {
        // 6.1. Evaluating Incoming Routing Information
        // HandRtr searches its route table to see if there is a route table
        // entry with the same MetricType of the RteMsg, matching RteMsg.Addr.
        IRoute *route = nullptr;
        for (int i = 0; i < routingTable->getNumRoutes(); i++) {
            IRoute *routeCandidate = routingTable->getRoute(i);
            if (routeCandidate->getSource() == this) {
                DymoRouteData *routeData = check_and_cast<DymoRouteData *>(routeCandidate->getProtocolData());
                if (routeCandidate->getDestinationAsGeneric() == address && routeData->getMetricType() == addressBlock.getMetricType()) {
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
            IRoute *route = createRoute(packet, rteMsg, addressBlock);
            EV_DETAIL << "Adding new route: " << route << endl;
            routingTable->addRoute(route);
        }
        else {
            DymoRouteData *routeData = check_and_cast<DymoRouteData *>(route->getProtocolData());
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
                EV_DETAIL << "Updating existing route: " << route << endl;
                updateRoute(packet, rteMsg, addressBlock, route);
                EV_DETAIL << "Route updated: " << route << endl;
                routingTable->addRoute(route);
            }
        }
    }
}

IRoute *Dymo::createRoute(Packet *packet, const Ptr<const  RteMsg>& rteMsg, const AddressBlock& addressBlock)
{
    IRoute *route = routingTable->createRoute();
    route->setSourceType(IRoute::DYMO);
    route->setSource(this);
    route->setProtocolData(new DymoRouteData());
    updateRoute(packet, rteMsg, addressBlock, route);
    return route;
}

void Dymo::updateRoute(Packet *packet, const Ptr<const RteMsg>& rteMsg, const AddressBlock& addressBlock, IRoute *route)
{
    // 6.2. Applying Route Updates To Route Table Entries
    DymoRouteData *routeData = check_and_cast<DymoRouteData *>(route->getProtocolData());
    // Route.Address := RteMsg.Addr
    const L3Address& address = addressBlock.getAddress();
    route->setDestination(address);
    // If (RteMsg.PfxLen != 0), then Route.PfxLen := RteMsg.PfxLen
    route->setPrefixLength(addressBlock.getPrefixLength());
    // Route.SeqNum := RteMsg.SeqNum
    DymoSequenceNumber sequenceNumber = addressBlock.getSequenceNumber();
    routeData->setSequenceNumber(sequenceNumber);
    targetAddressToSequenceNumber[address] = sequenceNumber;
    // Route.NextHopAddress := IP.SourceAddress (i.e., an address of the node from which the RteMsg was received)
    // note that Dymo packets are not routed on the IP level, so we can use the source address here
    L3Address srcAddr = packet->getTag<L3AddressInd>()->getSrcAddress();
    route->setNextHop(srcAddr);
    // Route.NextHopInterface is set to the interface on which RteMsg was received
    InterfaceEntry *interfaceEntry = interfaceTable->getInterfaceById((packet->getTag<InterfaceInd>())->getInterfaceId());
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
int Dymo::getLinkCost(const InterfaceEntry *interfaceEntry, DymoMetricType metricType)
{
    switch (metricType) {
        case HOP_COUNT:
            return 1;

        default:
            throw cRuntimeError("Unknown metric type");
    }
}

bool Dymo::isLoopFree(const Ptr<const RteMsg>& rteMsg, IRoute *route)
{
    // TODO: implement
    return true;
}

//
// handling expunge timer
//

void Dymo::processExpungeTimer()
{
    EV_DETAIL << "Processing expunge timer" << endl;
    expungeRoutes();
    scheduleExpungeTimer();
}

void Dymo::scheduleExpungeTimer()
{
    EV_DETAIL << "Scheduling expunge timer" << endl;
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

void Dymo::expungeRoutes()
{
    EV_DETAIL << "Expunging routes from routing table: routeCount = " << routingTable->getNumRoutes() << endl;
    // 6.3. Route Table Entry Timeouts
    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        IRoute *route = routingTable->getRoute(i);
        if (route->getSource() == this) {
            DymoRouteData *routeData = check_and_cast<DymoRouteData *>(route->getProtocolData());
            // An Active route MUST NOT be expunged
            // An Idle route SHOULD NOT be expunged
            // An Expired route MAY be expunged (least recently used first)
            // A route MUST be expunged if (Current_Time - Route.LastUsed) >= MAX_SEQNUM_LIFETIME.
            // A route MUST be expunged if Current_Time >= Route.ExpirationTime
            if ((getRouteState(routeData) == EXPIRED) ||
                (simTime() - routeData->getLastUsed() >= maxSequenceNumberLifetime) ||
                (simTime() >= routeData->getExpirationTime()))
            {
                EV_DETAIL << "Expunging route: " << route << endl;
                routingTable->deleteRoute(route);
                i--;
            }
        }
    }
}

simtime_t Dymo::getNextExpungeTime()
{
    simtime_t nextExpirationTime = SimTime::getMaxTime();
    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        IRoute *route = routingTable->getRoute(i);
        if (route->getSource() == this) {
            DymoRouteData *routeData = check_and_cast<DymoRouteData *>(route->getProtocolData());
            const simtime_t& expirationTime = routeData->getExpirationTime();
            if (expirationTime < nextExpirationTime)
                nextExpirationTime = expirationTime;
            const simtime_t& defaultExpirationTime = routeData->getLastUsed() + maxSequenceNumberLifetime;
            if (defaultExpirationTime < nextExpirationTime)
                nextExpirationTime = defaultExpirationTime;
        }
    }
    return nextExpirationTime;
}

DymoRouteState Dymo::getRouteState(DymoRouteData *routeData)
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

void Dymo::configureInterfaces()
{
    // join multicast groups
    cPatternMatcher interfaceMatcher(interfaces, false, true, false);
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        InterfaceEntry *interfaceEntry = interfaceTable->getInterface(i);
        if (interfaceEntry->isMulticast() && interfaceMatcher.matches(interfaceEntry->getInterfaceName()))
            // Most AODVv2 messages are sent with the IP destination address set to the link-local
            // multicast address LL-MANET-Routers [RFC5498] unless otherwise specified. Therefore,
            // all AODVv2 routers MUST subscribe to LL-MANET-Routers [RFC5498] to receiving AODVv2 messages.
            interfaceEntry->joinMulticastGroup(addressType->getLinkLocalManetRoutersMulticastAddress());
    }
}

//
// address
//

L3Address Dymo::getSelfAddress()
{
    return routingTable->getRouterIdAsGeneric();
}

bool Dymo::isClientAddress(const L3Address& address)
{
    if (routingTable->isLocalAddress(address))
        return true;
    else {
        for (auto & elem : clientAddressAndPrefixLengthPairs)
            // TODO: check for prefix length too
            if (elem.first == address)
                return true;

        return false;
    }
}

//
// added node
//

void Dymo::addSelfNode(const Ptr<RteMsg>& rteMsg)
{
    const L3Address& address = getSelfAddress();
    AddressBlock addressBlock;
    addressBlock.setAddress(address);
    addressBlock.setPrefixLength(addressType->getMaxPrefixLength());
    addressBlock.setHasValidityTime(false);
    addressBlock.setValidityTime(-1);
    addressBlock.setHasMetric(true);
    addressBlock.setMetric(0);
    addressBlock.setHasMetricType(true);
    addressBlock.setMetricType(HOP_COUNT);
    addressBlock.setHasSequenceNumber(true);
    addressBlock.setSequenceNumber(sequenceNumber);
    addNode(rteMsg, addressBlock);
}

void Dymo::addNode(const Ptr<RteMsg>& rteMsg, AddressBlock& addressBlock)
{
    int size = rteMsg->getAddedNodeArraySize();
    rteMsg->setAddedNodeArraySize(size + 1);
    rteMsg->setAddedNode(size, addressBlock);
}

//
// sequence number
//

void Dymo::incrementSequenceNumber()
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

INetfilter::IHook::Result Dymo::ensureRouteForDatagram(Packet *datagram)
{
    const auto& networkHeader = getNetworkProtocolHeader(datagram);
    const L3Address& source = networkHeader->getSourceAddress();
    const L3Address& destination = networkHeader->getDestinationAddress();
    if (destination.isMulticast() || destination.isBroadcast() || routingTable->isLocalAddress(destination))
        return ACCEPT;
    else {
        EV_DETAIL << "Finding route: source = " << source << ", destination = " << destination << endl;
        IRoute *route = routingTable->findBestMatchingRoute(destination);
        DymoRouteData *routeData = route ? dynamic_cast<DymoRouteData *>(route->getProtocolData()) : nullptr;
        bool broken = routeData && routeData->getBroken();
        if (route && !route->getNextHopAsGeneric().isUnspecified() && !broken) {
            EV_DETAIL << "Route found: source = " << source << ", destination = " << destination << ", route: " << route << endl;
            if (routeData)
                // 8.1. Handling Route Lifetimes During Packet Forwarding
                // Route.LastUsed := Current_Time, and the packet is forwarded to the route's next hop.
                routeData->setLastUsed(simTime());
            return ACCEPT;
        }
        else if (source.isUnspecified() || isClientAddress(source)) {
            EV_DETAIL << (broken ? "Broken" : "Missing") << " route: source = " << source << ", destination = " << destination << endl;
            delayDatagram(datagram);
            if (!hasOngoingRouteDiscovery(destination))
                startRouteDiscovery(destination);
            else
                EV_INFO << "Route discovery is in progress: originator = " << getSelfAddress() << ", target = " << destination << endl;
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

void Dymo::handleStartOperation(LifecycleOperation *operation)
{
    configureInterfaces();
}

void Dymo::handleStopOperation(LifecycleOperation *operation)
{
    // TODO: send a RERR to notify peers about broken routes
    for (auto & elem : targetAddressToRREQTimer)
        cancelRouteDiscovery(elem.first);
}

void Dymo::handleCrashOperation(LifecycleOperation *operation)
{
    targetAddressToSequenceNumber.clear();
    targetAddressToRREQTimer.clear();
    targetAddressToDelayedPackets.clear();
}

//
// notification
//

void Dymo::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("receiveChangeNotification");
    if (signalID == linkBrokenSignal) {
        EV_WARN << "Received link break" << endl;
        Packet *datagram = check_and_cast<Packet *>(obj);
        const auto& networkHeader = findNetworkProtocolHeader(datagram);
        if (networkHeader != nullptr) {
            const L3Address& destination = networkHeader->getDestinationAddress();
            if (destination.getAddressType() == addressType) {
                IRoute *route = routingTable->findBestMatchingRoute(destination);
                if (route) {
                    const L3Address& nextHop = route->getNextHopAsGeneric();
                    sendRerrForBrokenLink(route->getInterface(), nextHop);
                }
            }
        }
    }
}

} // namespace dymo

} // namespace inet

