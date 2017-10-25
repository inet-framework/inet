//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
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

#include <stdlib.h>
#include <string.h>

#include "inet/networklayer/ipv4/IPv4.h"

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/INETUtils.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/serializer/TCPIPchecksum.h"
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/networklayer/common/EcnTag_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/common/MulticastTag_m.h"
#include "inet/networklayer/common/NextHopAddressTag_m.h"
#include "inet/networklayer/contract/IARP.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/L3SocketCommand_m.h"
#include "inet/networklayer/ipv4/IcmpHeader_m.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4Header.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MACAddressTag_m.h"

namespace inet {

Define_Module(IPv4);

//TODO TRANSLATE
// a multicast cimek eseten hianyoznak bizonyos NetFilter hook-ok
// a local interface-k hasznalata eseten szinten hianyozhatnak bizonyos NetFilter hook-ok

IPv4::IPv4() :
    isUp(true)
{
}

IPv4::~IPv4()
{
    for (auto it : socketIdToSocketDescriptor)
        delete it.second;
    flush();
}

void IPv4::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        QueueBase::initialize();

        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        rt = getModuleFromPar<IIPv4RoutingTable>(par("routingTableModule"), this);
        arp = getModuleFromPar<IARP>(par("arpModule"), this);
        icmp = getModuleFromPar<ICMP>(par("icmpModule"), this);

        transportInGateBaseId = gateBaseId("transportIn");

        const char *crcModeString = par("crcMode");
        if (!strcmp(crcModeString, "declared"))
            crcMode = CRC_DECLARED_CORRECT;
        else if (!strcmp(crcModeString, "computed"))
            crcMode = CRC_COMPUTED;
        else
            throw cRuntimeError("Unknown crc mode: '%s'", crcModeString);

        defaultTimeToLive = par("timeToLive");
        defaultMCTimeToLive = par("multicastTimeToLive");
        fragmentTimeoutTime = par("fragmentTimeout");
        forceBroadcast = par("forceBroadcast");
        useProxyARP = par("useProxyARP");

        curFragmentId = 0;
        lastCheckTime = 0;

        numMulticast = numLocalDeliver = numDropped = numUnroutable = numForwarded = 0;

        // NetFilter:
        hooks.clear();
        queuedDatagramsForHooks.clear();

        pendingPackets.clear();
        cModule *arpModule = check_and_cast<cModule *>(arp);
        arpModule->subscribe(IARP::completedARPResolutionSignal, this);
        arpModule->subscribe(IARP::failedARPResolutionSignal, this);

        WATCH(numMulticast);
        WATCH(numLocalDeliver);
        WATCH(numDropped);
        WATCH(numUnroutable);
        WATCH(numForwarded);
        WATCH_MAP(pendingPackets);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        isUp = isNodeUp();
        registerProtocol(Protocol::ipv4, gate("transportOut"));
        registerProtocol(Protocol::ipv4, gate("queueOut"));
    }
}

void IPv4::handleRegisterProtocol(const Protocol& protocol, cGate *gate)
{
    Enter_Method("handleRegisterProtocol");
    mapping.addProtocolMapping(ProtocolGroup::ipprotocol.getProtocolNumber(&protocol), gate->getIndex());
}

void IPv4::refreshDisplay() const
{
    char buf[80] = "";
    if (numForwarded > 0)
        sprintf(buf + strlen(buf), "fwd:%d ", numForwarded);
    if (numLocalDeliver > 0)
        sprintf(buf + strlen(buf), "up:%d ", numLocalDeliver);
    if (numMulticast > 0)
        sprintf(buf + strlen(buf), "mcast:%d ", numMulticast);
    if (numDropped > 0)
        sprintf(buf + strlen(buf), "DROP:%d ", numDropped);
    if (numUnroutable > 0)
        sprintf(buf + strlen(buf), "UNROUTABLE:%d ", numUnroutable);
    getDisplayString().setTagArg("t", 0, buf);
}

void IPv4::handleMessage(cMessage *msg)
{
    if (L3SocketBindCommand *command = dynamic_cast<L3SocketBindCommand *>(msg->getControlInfo())) {
        int socketId = msg->getMandatoryTag<SocketReq>()->getSocketId();
        SocketDescriptor *descriptor = new SocketDescriptor(socketId, command->getProtocolId());
        socketIdToSocketDescriptor[socketId] = descriptor;
        protocolIdToSocketDescriptors.insert(std::pair<int, SocketDescriptor *>(command->getProtocolId(), descriptor));
        delete msg;
    }
    else if (dynamic_cast<L3SocketCloseCommand *>(msg->getControlInfo()) != nullptr) {
        int socketId = msg->getMandatoryTag<SocketReq>()->getSocketId();
        auto it = socketIdToSocketDescriptor.find(socketId);
        if (it != socketIdToSocketDescriptor.end()) {
            int protocol = it->second->protocolId;
            auto lowerBound = protocolIdToSocketDescriptors.lower_bound(protocol);
            auto upperBound = protocolIdToSocketDescriptors.upper_bound(protocol);
            for (auto jt = lowerBound; jt != upperBound; jt++) {
                if (it->second == jt->second) {
                    protocolIdToSocketDescriptors.erase(jt);
                    break;
                }
            }
            delete it->second;
            socketIdToSocketDescriptor.erase(it);
        }
        delete msg;
    }
    else
        QueueBase::handleMessage(msg);
}

void IPv4::endService(cPacket *packet)
{
    if (!isUp) {
        EV_ERROR << "IPv4 is down -- discarding message\n";
        delete packet;
        return;
    }
    if (packet->getArrivalGate()->isName("transportIn")) {    //TODO packet->getArrivalGate()->getBaseId() == transportInGateBaseId
        handlePacketFromHL(check_and_cast<Packet*>(packet));
    }
    else {    // from network
        EV_INFO << "Received " << packet << " from network.\n";
        handleIncomingDatagram(check_and_cast<Packet*>(packet));
    }
}

bool IPv4::verifyCrc(const Ptr<const Ipv4Header>& ipv4Header)
{
    switch (ipv4Header->getCrcMode()) {
        case CRC_DECLARED_CORRECT: {
            // if the CRC mode is declared to be correct, then the check passes if and only if the chunk is correct
            return ipv4Header->isCorrect();
        }
        case CRC_DECLARED_INCORRECT:
            // if the CRC mode is declared to be incorrect, then the check fails
            return false;
        case CRC_COMPUTED: {
            if (ipv4Header->isCorrect()) {
                // compute the CRC, the check passes if the result is 0xFFFF (includes the received CRC) and the chunks are correct
                auto ipv4HeaderBytes = ipv4Header->Chunk::peek<BytesChunk>(B(0), ipv4Header->getChunkLength());
                auto bufferLength = B(ipv4HeaderBytes->getChunkLength()).get();
                auto buffer = new uint8_t[bufferLength];
                // 1. fill in the data
                ipv4HeaderBytes->copyToBuffer(buffer, bufferLength);
                // 2. compute the CRC
                auto computedCrc = inet::serializer::TCPIPchecksum::checksum(buffer, bufferLength);
                delete [] buffer;
                return computedCrc == 0;
            }
            else
                return false;
        }
        default:
            throw cRuntimeError("Unknown CRC mode");
    }
}

const InterfaceEntry *IPv4::getSourceInterface(cPacket *packet)
{
    auto tag = packet->getTag<InterfaceInd>();
    return tag != nullptr ? ift->getInterfaceById(tag->getInterfaceId()) : nullptr;
}

const InterfaceEntry *IPv4::getDestInterface(cPacket *packet)
{
    auto tag = packet->getTag<InterfaceReq>();
    return tag != nullptr ? ift->getInterfaceById(tag->getInterfaceId()) : nullptr;
}

IPv4Address IPv4::getNextHop(cPacket *packet)
{
    auto tag = packet->getTag<NextHopAddressReq>();
    return tag != nullptr ? tag->getNextHopAddress().toIPv4() : IPv4Address::UNSPECIFIED_ADDRESS;
}

void IPv4::handleIncomingDatagram(Packet *packet)
{
    ASSERT(packet);
    int interfaceId = packet->getMandatoryTag<InterfaceInd>()->getInterfaceId();
    emit(LayeredProtocolBase::packetReceivedFromLowerSignal, packet);

    //
    // "Prerouting"
    //

    const auto& ipv4Header = packet->peekHeader<Ipv4Header>();
    packet->ensureTag<NetworkProtocolInd>()->setProtocol(&Protocol::ipv4);
    packet->ensureTag<NetworkProtocolInd>()->setNetworkProtocolHeader(ipv4Header);

    if (!verifyCrc(ipv4Header)) {
        EV_WARN << "CRC error found, drop packet\n";
        delete packet;
        return;
    }

    if (ipv4Header->getTotalLengthField() > packet->getByteLength()) {
        EV_WARN << "length error found, sending ICMP_PARAMETER_PROBLEM\n";
        sendIcmpError(packet, interfaceId, ICMP_PARAMETER_PROBLEM, 0);
        return;
    }

    // remove lower layer paddings:
    if (ipv4Header->getTotalLengthField() < packet->getByteLength()) {
        packet->setTrailerPopOffset(packet->getHeaderPopOffset() + B(ipv4Header->getTotalLengthField()));
    }

    // check for header biterror
    if (packet->hasBitError()) {
        // probability of bit error in header = size of header / size of total message
        // (ignore bit error if in payload)
        double relativeHeaderLength = ipv4Header->getHeaderLength() / (double)B(ipv4Header->getChunkLength()).get();
        if (dblrand() <= relativeHeaderLength) {
            EV_WARN << "bit error found, sending ICMP_PARAMETER_PROBLEM\n";
            sendIcmpError(packet, interfaceId, ICMP_PARAMETER_PROBLEM, 0);
            return;
        }
    }

    EV_DETAIL << "Received datagram `" << ipv4Header->getName() << "' with dest=" << ipv4Header->getDestAddress() << "\n";

    if (datagramPreRoutingHook(packet) == INetfilter::IHook::ACCEPT)
        preroutingFinish(packet);
}

Packet *IPv4::prepareForForwarding(Packet *packet) const
{
    const auto& ipv4Header = removeNetworkProtocolHeader<Ipv4Header>(packet);
    ipv4Header->setTimeToLive(ipv4Header->getTimeToLive() - 1);
    insertNetworkProtocolHeader(packet, Protocol::ipv4, ipv4Header);
    return packet;
}

void IPv4::preroutingFinish(Packet *packet)
{
    const InterfaceEntry *fromIE = ift->getInterfaceById(packet->getMandatoryTag<InterfaceInd>()->getInterfaceId());
    IPv4Address nextHopAddr = getNextHop(packet);

    const auto& ipv4Header = packet->peekHeader<Ipv4Header>();
    ASSERT(ipv4Header);
    const IPv4Address& destAddr = ipv4Header->getDestAddress();

    // route packet

    if (fromIE->isLoopback()) {
        reassembleAndDeliver(packet);
    }
    else if (destAddr.isMulticast()) {
        // check for local delivery
        // Note: multicast routers will receive IGMP datagrams even if their interface is not joined to the group
        if (fromIE->ipv4Data()->isMemberOfMulticastGroup(destAddr) ||
            (rt->isMulticastForwardingEnabled() && ipv4Header->getProtocolId() == IP_PROT_IGMP))
            reassembleAndDeliver(packet->dup());
        else
            EV_WARN << "Skip local delivery of multicast datagram (input interface not in multicast group)\n";

        // don't forward if IP forwarding is off, or if dest address is link-scope
        if (!rt->isMulticastForwardingEnabled()) {
            EV_WARN << "Skip forwarding of multicast datagram (forwarding disabled)\n";
            delete packet;
        }
        else if (destAddr.isLinkLocalMulticast()) {
            EV_WARN << "Skip forwarding of multicast datagram (packet is link-local)\n";
            delete packet;
        }
        else if (ipv4Header->getTimeToLive() <= 1) {      // TTL before decrement
            EV_WARN << "Skip forwarding of multicast datagram (TTL reached 0)\n";
            delete packet;
        }
        else
            forwardMulticastPacket(prepareForForwarding(packet));
    }
    else {
        const InterfaceEntry *broadcastIE = nullptr;

        // check for local delivery; we must accept also packets coming from the interfaces that
        // do not yet have an IP address assigned. This happens during DHCP requests.
        if (rt->isLocalAddress(destAddr) || fromIE->ipv4Data()->getIPAddress().isUnspecified()) {
            reassembleAndDeliver(packet);
        }
        else if (destAddr.isLimitedBroadcastAddress() || (broadcastIE = rt->findInterfaceByLocalBroadcastAddress(destAddr))) {
            // broadcast datagram on the target subnet if we are a router
            if (broadcastIE && fromIE != broadcastIE && rt->isForwardingEnabled()) {
                auto packetCopy = prepareForForwarding(packet->dup());
                packetCopy->ensureTag<InterfaceReq>()->setInterfaceId(broadcastIE->getInterfaceId());
                packetCopy->ensureTag<NextHopAddressReq>()->setNextHopAddress(IPv4Address::ALLONES_ADDRESS);
                fragmentPostRouting(packetCopy);
            }

            EV_INFO << "Broadcast received\n";
            reassembleAndDeliver(packet);
        }
        else if (!rt->isForwardingEnabled()) {
            EV_WARN << "forwarding off, dropping packet\n";
            numDropped++;
            PacketDropDetails details;
            details.setReason(FORWARDING_DISABLED);
            emit(packetDropSignal, packet, &details);
            delete packet;
        }
        else {
            packet->ensureTag<NextHopAddressReq>()->setNextHopAddress(nextHopAddr);
            routeUnicastPacket(prepareForForwarding(packet));
        }
    }
}

void IPv4::handlePacketFromHL(Packet *packet)
{
    EV_INFO << "Received " << packet << " from upper layer.\n";
    emit(LayeredProtocolBase::packetReceivedFromUpperSignal, packet);

    // if no interface exists, do not send datagram
    if (ift->getNumInterfaces() == 0) {
        EV_ERROR << "No interfaces exist, dropping packet\n";
        numDropped++;
        PacketDropDetails details;
        details.setReason(NO_INTERFACE_FOUND);
        emit(packetDropSignal, packet, &details);
        delete packet;
        return;
    }

    // encapsulate
    encapsulate(packet);

    // TODO:
    L3Address nextHopAddr(IPv4Address::UNSPECIFIED_ADDRESS);
    if (datagramLocalOutHook(packet) == INetfilter::IHook::ACCEPT)
        datagramLocalOut(packet);
}

void IPv4::datagramLocalOut(Packet *packet)
{
    const InterfaceEntry *destIE = getDestInterface(packet);
    IPv4Address requestedNextHopAddress = getNextHop(packet);

    const auto& ipv4Header = packet->peekHeader<Ipv4Header>();
    bool multicastLoop = false;
    MulticastReq *mcr = packet->getTag<MulticastReq>();
    if (mcr != nullptr) {
        multicastLoop = mcr->getMulticastLoop();
    }

    // send
    const IPv4Address& destAddr = ipv4Header->getDestAddress();

    EV_DETAIL << "Sending datagram '" << packet->getName() << "' with destination = " << destAddr << "\n";

    if (ipv4Header->getDestAddress().isMulticast()) {
        destIE = determineOutgoingInterfaceForMulticastDatagram(ipv4Header, destIE);
        packet->ensureTag<InterfaceReq>()->setInterfaceId(destIE ? destIE->getInterfaceId() : -1);

        // loop back a copy
        if (multicastLoop && (!destIE || !destIE->isLoopback())) {
            const InterfaceEntry *loopbackIF = ift->getFirstLoopbackInterface();
            if (loopbackIF) {
                auto packetCopy = packet->dup();
                packetCopy->ensureTag<InterfaceReq>()->setInterfaceId(loopbackIF->getInterfaceId());
                packetCopy->ensureTag<NextHopAddressReq>()->setNextHopAddress(destAddr);
                fragmentPostRouting(packetCopy);
            }
        }

        if (destIE) {
            numMulticast++;
            packet->ensureTag<InterfaceReq>()->setInterfaceId(destIE->getInterfaceId());        //FIXME KLUDGE is it needed?
            packet->ensureTag<NextHopAddressReq>()->setNextHopAddress(destAddr);
            fragmentPostRouting(packet);
        }
        else {
            EV_ERROR << "No multicast interface, packet dropped\n";
            numUnroutable++;
            PacketDropDetails details;
            details.setReason(NO_INTERFACE_FOUND);
            emit(packetDropSignal, packet, &details);
            delete packet;
        }
    }
    else {    // unicast and broadcast
              // check for local delivery
        if (rt->isLocalAddress(destAddr)) {
            EV_INFO << "Delivering " << packet << " locally.\n";
            if (destIE && !destIE->isLoopback()) {
                EV_DETAIL << "datagram destination address is local, ignoring destination interface specified in the control info\n";
                destIE = nullptr;
                packet->ensureTag<InterfaceReq>()->setInterfaceId(-1);
            }
            if (!destIE) {
                destIE = ift->getFirstLoopbackInterface();
                packet->ensureTag<InterfaceReq>()->setInterfaceId(destIE ? destIE->getInterfaceId() : -1);
            }
            ASSERT(destIE);
            packet->ensureTag<NextHopAddressReq>()->setNextHopAddress(destAddr);
            routeUnicastPacket(packet);
        }
        else if (destAddr.isLimitedBroadcastAddress() || rt->isLocalBroadcastAddress(destAddr))
            routeLocalBroadcastPacket(packet);
        else {
            packet->ensureTag<NextHopAddressReq>()->setNextHopAddress(requestedNextHopAddress);
            routeUnicastPacket(packet);
        }
    }
}

/* Choose the outgoing interface for the muticast datagram:
 *   1. use the interface specified by MULTICAST_IF socket option (received in the control info)
 *   2. lookup the destination address in the routing table
 *   3. if no route, choose the interface according to the source address
 *   4. or if the source address is unspecified, choose the first MULTICAST interface
 */
const InterfaceEntry *IPv4::determineOutgoingInterfaceForMulticastDatagram(const Ptr<const Ipv4Header>& ipv4Header, const InterfaceEntry *multicastIFOption)
{
    const InterfaceEntry *ie = nullptr;
    if (multicastIFOption) {
        ie = multicastIFOption;
        EV_DETAIL << "multicast packet routed by socket option via output interface " << ie->getInterfaceName() << "\n";
    }
    if (!ie) {
        IPv4Route *route = rt->findBestMatchingRoute(ipv4Header->getDestAddress());
        if (route)
            ie = route->getInterface();
        if (ie)
            EV_DETAIL << "multicast packet routed by routing table via output interface " << ie->getInterfaceName() << "\n";
    }
    if (!ie) {
        ie = rt->getInterfaceByAddress(ipv4Header->getSrcAddress());
        if (ie)
            EV_DETAIL << "multicast packet routed by source address via output interface " << ie->getInterfaceName() << "\n";
    }
    if (!ie) {
        ie = ift->getFirstMulticastInterface();
        if (ie)
            EV_DETAIL << "multicast packet routed via the first multicast interface " << ie->getInterfaceName() << "\n";
    }
    return ie;
}

void IPv4::routeUnicastPacket(Packet *packet)
{
    const InterfaceEntry *fromIE = getSourceInterface(packet);
    const InterfaceEntry *destIE = getDestInterface(packet);
    IPv4Address nextHopAddress = getNextHop(packet);

    const auto& ipv4Header = packet->peekHeader<Ipv4Header>();
    IPv4Address destAddr = ipv4Header->getDestAddress();
    EV_INFO << "Routing " << packet << " with destination = " << destAddr << ", ";

    // if output port was explicitly requested, use that, otherwise use IPv4 routing
    if (destIE) {
        EV_DETAIL << "using manually specified output interface " << destIE->getInterfaceName() << "\n";
        // and nextHopAddr remains unspecified
        if (!nextHopAddress.isUnspecified()) {
            // do nothing, next hop address already specified
        }
        // special case ICMP reply
        else if (destIE->isBroadcast()) {
            // if the interface is broadcast we must search the next hop
            const IPv4Route *re = rt->findBestMatchingRoute(destAddr);
            if (re && re->getInterface() == destIE) {
                packet->ensureTag<NextHopAddressReq>()->setNextHopAddress(re->getGateway());
            }
        }
    }
    else {
        // use IPv4 routing (lookup in routing table)
        const IPv4Route *re = rt->findBestMatchingRoute(destAddr);
        if (re) {
            destIE = re->getInterface();
            packet->ensureTag<InterfaceReq>()->setInterfaceId(destIE->getInterfaceId());
            packet->ensureTag<NextHopAddressReq>()->setNextHopAddress(re->getGateway());
        }
    }

    if (!destIE) {    // no route found
        EV_WARN << "unroutable, sending ICMP_DESTINATION_UNREACHABLE, dropping packet\n";
        numUnroutable++;
        PacketDropDetails details;
        details.setReason(NO_ROUTE_FOUND);
        emit(packetDropSignal, packet, &details);
        sendIcmpError(packet, fromIE ? fromIE->getInterfaceId() : -1, ICMP_DESTINATION_UNREACHABLE, 0);
    }
    else {    // fragment and send
        if (fromIE != nullptr) {
            if (datagramForwardHook(packet) != INetfilter::IHook::ACCEPT)
                return;
        }

        routeUnicastPacketFinish(packet);
    }
}

void IPv4::routeUnicastPacketFinish(Packet *packet)
{
    EV_INFO << "output interface = " << getDestInterface(packet)->getInterfaceName() << ", next hop address = " << getNextHop(packet) << "\n";
    numForwarded++;
    fragmentPostRouting(packet);
}

void IPv4::routeLocalBroadcastPacket(Packet *packet)
{
    auto interfaceReq = packet->getTag<InterfaceReq>();
    const InterfaceEntry *destIE = interfaceReq != nullptr ? ift->getInterfaceById(interfaceReq->getInterfaceId()) : nullptr;
    // The destination address is 255.255.255.255 or local subnet broadcast address.
    // We always use 255.255.255.255 as nextHopAddress, because it is recognized by ARP,
    // and mapped to the broadcast MAC address.
    if (destIE != nullptr) {
        packet->ensureTag<InterfaceReq>()->setInterfaceId(destIE->getInterfaceId());    //FIXME KLUDGE is it needed?
        packet->ensureTag<NextHopAddressReq>()->setNextHopAddress(IPv4Address::ALLONES_ADDRESS);
        fragmentPostRouting(packet);
    }
    else if (forceBroadcast) {
        // forward to each interface including loopback
        for (int i = 0; i < ift->getNumInterfaces(); i++) {
            const InterfaceEntry *ie = ift->getInterface(i);
            auto packetCopy = packet->dup();
            packetCopy->ensureTag<InterfaceReq>()->setInterfaceId(ie->getInterfaceId());
            packetCopy->ensureTag<NextHopAddressReq>()->setNextHopAddress(IPv4Address::ALLONES_ADDRESS);
            fragmentPostRouting(packetCopy);
        }
        delete packet;
    }
    else {
        numDropped++;
        PacketDropDetails details;
        details.setReason(NO_INTERFACE_FOUND);
        emit(packetDropSignal, packet, &details);
        delete packet;
    }
}

const InterfaceEntry *IPv4::getShortestPathInterfaceToSource(const Ptr<const Ipv4Header>& ipv4Header) const
{
    return rt->getInterfaceForDestAddr(ipv4Header->getSrcAddress());
}

void IPv4::forwardMulticastPacket(Packet *packet)
{
    const InterfaceEntry *fromIE = ift->getInterfaceById(packet->getMandatoryTag<InterfaceInd>()->getInterfaceId());
    const auto& ipv4Header = packet->peekHeader<Ipv4Header>();
    const IPv4Address& srcAddr = ipv4Header->getSrcAddress();
    const IPv4Address& destAddr = ipv4Header->getDestAddress();
    ASSERT(destAddr.isMulticast());
    ASSERT(!destAddr.isLinkLocalMulticast());

    EV_INFO << "Forwarding multicast datagram `" << packet->getName() << "' with dest=" << destAddr << "\n";

    numMulticast++;

    const IPv4MulticastRoute *route = rt->findBestMatchingMulticastRoute(srcAddr, destAddr);
    if (!route) {
        EV_WARN << "Multicast route does not exist, try to add.\n";
        // TODO: no need to emit fromIE when tags will be used in place of control infos
        emit(NF_IPv4_NEW_MULTICAST, ipv4Header.get(), const_cast<InterfaceEntry *>(fromIE));

        // read new record
        route = rt->findBestMatchingMulticastRoute(srcAddr, destAddr);

        if (!route) {
            EV_ERROR << "No route, packet dropped.\n";
            numUnroutable++;
            PacketDropDetails details;
            details.setReason(NO_ROUTE_FOUND);
            emit(packetDropSignal, packet, &details);
            delete packet;
            return;
        }
    }

    if (route->getInInterface() && fromIE != route->getInInterface()->getInterface()) {
        EV_ERROR << "Did not arrive on input interface, packet dropped.\n";
        // TODO: no need to emit fromIE when tags will be used in place of control infos
        emit(NF_IPv4_DATA_ON_NONRPF, ipv4Header.get(), const_cast<InterfaceEntry *>(fromIE));
        numDropped++;
        PacketDropDetails details;
        emit(packetDropSignal, packet, &details);
        delete packet;
    }
    // backward compatible: no parent means shortest path interface to source (RPB routing)
    else if (!route->getInInterface() && fromIE != getShortestPathInterfaceToSource(ipv4Header)) {
        EV_ERROR << "Did not arrive on shortest path, packet dropped.\n";
        numDropped++;
        PacketDropDetails details;
        emit(packetDropSignal, packet, &details);
        delete packet;
    }
    else {
        // TODO: no need to emit fromIE when tags will be used in place of control infos
        emit(NF_IPv4_DATA_ON_RPF, ipv4Header.get(), const_cast<InterfaceEntry *>(fromIE));    // forwarding hook

        numForwarded++;
        // copy original datagram for multiple destinations
        for (unsigned int i = 0; i < route->getNumOutInterfaces(); i++) {
            IPv4MulticastRoute::OutInterface *outInterface = route->getOutInterface(i);
            const InterfaceEntry *destIE = outInterface->getInterface();
            if (destIE != fromIE && outInterface->isEnabled()) {
                int ttlThreshold = destIE->ipv4Data()->getMulticastTtlThreshold();
                if (ipv4Header->getTimeToLive() <= ttlThreshold)
                    EV_WARN << "Not forwarding to " << destIE->getInterfaceName() << " (ttl treshold reached)\n";
                else if (outInterface->isLeaf() && !destIE->ipv4Data()->hasMulticastListener(destAddr))
                    EV_WARN << "Not forwarding to " << destIE->getInterfaceName() << " (no listeners)\n";
                else {
                    EV_DETAIL << "Forwarding to " << destIE->getInterfaceName() << "\n";
                    auto packetCopy = packet->dup();
                    packetCopy->ensureTag<InterfaceReq>()->setInterfaceId(destIE->getInterfaceId());
                    packetCopy->ensureTag<NextHopAddressReq>()->setNextHopAddress(destAddr);
                    fragmentPostRouting(packetCopy);
                }
            }
        }

        // TODO: no need to emit fromIE when tags will be use, d in place of control infos
        emit(NF_IPv4_MDATA_REGISTER, packet, const_cast<InterfaceEntry *>(fromIE));    // postRouting hook

        // only copies sent, delete original packet
        delete packet;
    }
}

void IPv4::reassembleAndDeliver(Packet *packet)
{
    EV_INFO << "Delivering " << packet << " locally.\n";

    const auto& ipv4Header = packet->peekHeader<Ipv4Header>();
    if (ipv4Header->getSrcAddress().isUnspecified())
        EV_WARN << "Received datagram '" << packet->getName() << "' without source address filled in\n";

    // reassemble the packet (if fragmented)
    if (ipv4Header->getFragmentOffset() != 0 || ipv4Header->getMoreFragments()) {
        EV_DETAIL << "Datagram fragment: offset=" << ipv4Header->getFragmentOffset()
                  << ", MORE=" << (ipv4Header->getMoreFragments() ? "true" : "false") << ".\n";

        // erase timed out fragments in fragmentation buffer; check every 10 seconds max
        if (simTime() >= lastCheckTime + 10) {
            lastCheckTime = simTime();
            fragbuf.purgeStaleFragments(icmp, simTime() - fragmentTimeoutTime);
        }

        packet = fragbuf.addFragment(packet, simTime());
        if (!packet) {
            EV_DETAIL << "No complete datagram yet.\n";
            return;
        }
        EV_DETAIL << "This fragment completes the datagram.\n";
    }

    if (datagramLocalInHook(packet) == INetfilter::IHook::ACCEPT)
        reassembleAndDeliverFinish(packet);
}

void IPv4::reassembleAndDeliverFinish(Packet *packet)
{
    auto ipv4HeaderPosition = packet->getHeaderPopOffset();
    const auto& ipv4Header = packet->peekHeader<Ipv4Header>();
    int protocol = ipv4Header->getProtocolId();
    decapsulate(packet);
    auto lowerBound = protocolIdToSocketDescriptors.lower_bound(protocol);
    auto upperBound = protocolIdToSocketDescriptors.upper_bound(protocol);
    bool hasSocket = lowerBound != upperBound;
    for (auto it = lowerBound; it != upperBound; it++) {
        cPacket *packetCopy = packet->dup();
        packetCopy->ensureTag<SocketInd>()->setSocketId(it->second->socketId);
        emit(LayeredProtocolBase::packetSentToUpperSignal, packetCopy);
        send(packetCopy, "transportOut");
    }
    if (mapping.findOutputGateForProtocol(protocol) >= 0) {
        emit(LayeredProtocolBase::packetSentToUpperSignal, packet);
        send(packet, "transportOut");
        numLocalDeliver++;
    }
    else if (hasSocket) {
        delete packet;
    }
    else {
        EV_ERROR << "Transport protocol ID=" << protocol << " not connected, discarding packet\n";
        packet->setHeaderPopOffset(ipv4HeaderPosition);
        const InterfaceEntry* fromIE = getSourceInterface(packet);
        sendIcmpError(packet, fromIE ? fromIE->getInterfaceId() : -1, ICMP_DESTINATION_UNREACHABLE, ICMP_DU_PROTOCOL_UNREACHABLE);
    }
}

void IPv4::decapsulate(Packet *packet)
{
    // decapsulate transport packet
    const auto& ipv4Header = packet->popHeader<Ipv4Header>();

    // create and fill in control info
    packet->ensureTag<DscpInd>()->setDifferentiatedServicesCodePoint(ipv4Header->getDiffServCodePoint());
    packet->ensureTag<EcnInd>()->setExplicitCongestionNotification(ipv4Header->getExplicitCongestionNotification());

    // original IPv4 datagram might be needed in upper layers to send back ICMP error message

    auto transportProtocol = ProtocolGroup::ipprotocol.getProtocol(ipv4Header->getProtocolId());
    packet->ensureTag<PacketProtocolTag>()->setProtocol(transportProtocol);
    packet->ensureTag<DispatchProtocolReq>()->setProtocol(transportProtocol);
    auto l3AddressInd = packet->ensureTag<L3AddressInd>();
    l3AddressInd->setSrcAddress(ipv4Header->getSrcAddress());
    l3AddressInd->setDestAddress(ipv4Header->getDestAddress());
    packet->ensureTag<HopLimitInd>()->setHopLimit(ipv4Header->getTimeToLive());
}

void IPv4::fragmentPostRouting(Packet *packet)
{
    const InterfaceEntry *destIE = ift->getInterfaceById(packet->getMandatoryTag<InterfaceReq>()->getInterfaceId());
    // fill in source address
    if (packet->peekHeader<Ipv4Header>()->getSrcAddress().isUnspecified()) {
        auto ipv4Header = removeNetworkProtocolHeader<Ipv4Header>(packet);
        ipv4Header->setSrcAddress(destIE->ipv4Data()->getIPAddress());
        insertNetworkProtocolHeader(packet, Protocol::ipv4, ipv4Header);
    }
    if (datagramPostRoutingHook(packet) == INetfilter::IHook::ACCEPT)
        fragmentAndSend(packet);
}

void IPv4::fragmentAndSend(Packet *packet)
{
    const InterfaceEntry *destIE = ift->getInterfaceById(packet->getMandatoryTag<InterfaceReq>()->getInterfaceId());
    IPv4Address nextHopAddr = getNextHop(packet);
    if (nextHopAddr.isUnspecified()) {
        IPv4InterfaceData *ipv4Data = destIE->ipv4Data();
        const auto& ipv4Header = packet->peekHeader<Ipv4Header>();
        IPv4Address destAddress = ipv4Header->getDestAddress();
        if (IPv4Address::maskedAddrAreEqual(destAddress, ipv4Data->getIPAddress(), ipv4Data->getNetmask()))
            nextHopAddr = destAddress;
        else if (useProxyARP) {
            nextHopAddr = destAddress;
            EV_WARN << "no next-hop address, using destination address " << nextHopAddr << " (proxy ARP)\n";
        }
        else
            throw cRuntimeError(packet, "Cannot send datagram on broadcast interface: no next-hop address and Proxy ARP is disabled");
        packet->ensureTag<NextHopAddressReq>()->setNextHopAddress(nextHopAddr);
    }

    if (crcMode == CRC_COMPUTED) {
        packet->removePoppedHeaders();
        const auto& ipv4Header = removeNetworkProtocolHeader<Ipv4Header>(packet);
        ipv4Header->setCrc(0);
        MemoryOutputStream ipv4HeaderStream;
        Chunk::serialize(ipv4HeaderStream, ipv4Header);
        auto ipv4HeaderBytes = ipv4HeaderStream.getData();
        auto bufferLength = ipv4HeaderBytes.size();
        auto buffer = new uint8_t[bufferLength];
        // 1. fill in the data
        std::copy(ipv4HeaderBytes.begin(), ipv4HeaderBytes.end(), (uint8_t *)buffer);
        // 2. compute the CRC
        uint16_t crc = inet::serializer::TCPIPchecksum::checksum(buffer, bufferLength);
        delete [] buffer;
        ipv4Header->setCrc(crc);
        insertNetworkProtocolHeader(packet, Protocol::ipv4, ipv4Header);
    }

    const auto& ipv4Header = packet->peekHeader<Ipv4Header>();

    // hop counter check
    if (ipv4Header->getTimeToLive() <= 0) {
        // drop datagram, destruction responsibility in ICMP
        PacketDropDetails details;
        details.setReason(HOP_LIMIT_REACHED);
        emit(packetDropSignal, packet, &details);
        EV_WARN << "datagram TTL reached zero, sending ICMP_TIME_EXCEEDED\n";
        sendIcmpError(packet, -1    /*TODO*/, ICMP_TIME_EXCEEDED, 0);
        numDropped++;
        return;
    }

    int mtu = destIE->getMTU();

    // send datagram straight out if it doesn't require fragmentation (note: mtu==0 means infinite mtu)
    if (mtu == 0 || packet->getByteLength() <= mtu) {
        sendDatagramToOutput(packet);
        return;
    }

    // if "don't fragment" bit is set, throw datagram away and send ICMP error message
    if (ipv4Header->getDontFragment()) {
        PacketDropDetails details;
        emit(packetDropSignal, packet, &details);
        EV_WARN << "datagram larger than MTU and don't fragment bit set, sending ICMP_DESTINATION_UNREACHABLE\n";
        sendIcmpError(packet, -1    /*TODO*/, ICMP_DESTINATION_UNREACHABLE,
                ICMP_DU_FRAGMENTATION_NEEDED);
        numDropped++;
        return;
    }

    // FIXME some IP options should not be copied into each fragment, check their COPY bit
    int headerLength = ipv4Header->getHeaderLength();
    int payloadLength = B(packet->getDataLength()).get() - headerLength;
    int fragmentLength = ((mtu - headerLength) / 8) * 8;    // payload only (without header)
    int offsetBase = ipv4Header->getFragmentOffset();
    if (fragmentLength <= 0)
        throw cRuntimeError("Cannot fragment datagram: MTU=%d too small for header size (%d bytes)", mtu, headerLength); // exception and not ICMP because this is likely a simulation configuration error, not something one wants to simulate

    int noOfFragments = (payloadLength + fragmentLength - 1) / fragmentLength;
    EV_DETAIL << "Breaking datagram into " << noOfFragments << " fragments\n";

    // create and send fragments
    std::string fragMsgName = packet->getName();
    fragMsgName += "-frag-";

    int offset = 0;
    while (offset < payloadLength) {
        bool lastFragment = (offset + fragmentLength >= payloadLength);
        // length equal to fragmentLength, except for last fragment;
        int thisFragmentLength = lastFragment ? payloadLength - offset : fragmentLength;

        std::string curFragName = fragMsgName + std::to_string(offset);
        if (lastFragment)
            curFragName += "-last";
        Packet *fragment = new Packet(curFragName.c_str());     //TODO add offset or index to fragment name

        //copy Tags from packet to fragment     //FIXME optimizing needed
        {
            Packet *tmp = packet->dup();
            fragment->transferTagsFrom(tmp);
            delete tmp;
        }

        ASSERT(fragment->getByteLength() == 0);
        const auto& fraghdr = makeShared<Ipv4Header>(*ipv4Header.get()->dup());
        const auto& fragData = packet->peekDataAt(B(headerLength + offset), B(thisFragmentLength));
        ASSERT(B(fragData->getChunkLength()).get() == thisFragmentLength);
        fragment->append(fragData);

        // "more fragments" bit is unchanged in the last fragment, otherwise true
        if (!lastFragment)
            fraghdr->setMoreFragments(true);

        fraghdr->setFragmentOffset(offsetBase + offset);
        fraghdr->setTotalLengthField(headerLength + thisFragmentLength);


        fraghdr->markImmutable();
        fragment->prepend(fraghdr);
        ASSERT(fragment->getByteLength() == headerLength + thisFragmentLength);
        sendDatagramToOutput(fragment);
        offset += thisFragmentLength;
    }

    delete packet;
}

void IPv4::encapsulate(Packet *transportPacket)
{
    const auto& ipv4Header = makeShared<Ipv4Header>();

    auto l3AddressReq = transportPacket->removeMandatoryTag<L3AddressReq>();
    IPv4Address src = l3AddressReq->getSrcAddress().toIPv4();
    IPv4Address dest = l3AddressReq->getDestAddress().toIPv4();
    delete l3AddressReq;

    ipv4Header->setProtocolId(ProtocolGroup::ipprotocol.getProtocolNumber(transportPacket->getMandatoryTag<PacketProtocolTag>()->getProtocol()));

    auto hopLimitReq = transportPacket->removeTag<HopLimitReq>();
    short ttl = (hopLimitReq != nullptr) ? hopLimitReq->getHopLimit() : -1;
    delete hopLimitReq;
    bool dontFragment = false;
    if (auto dontFragmentReq = transportPacket->removeTag<FragmentationReq>()) {
        dontFragment = dontFragmentReq->getDontFragment();
        delete dontFragmentReq;
    }

    // set source and destination address
    ipv4Header->setDestAddress(dest);

    // when source address was given, use it; otherwise it'll get the address
    // of the outgoing interface after routing
    if (!src.isUnspecified()) {
        // if interface parameter does not match existing interface, do not send datagram
        if (rt->getInterfaceByAddress(src) == nullptr)
            throw cRuntimeError("Wrong source address %s in (%s)%s: no interface with such address",
                    src.str().c_str(), transportPacket->getClassName(), transportPacket->getFullName());

        ipv4Header->setSrcAddress(src);
    }

    // set other fields
    if (DscpReq *dscpReq = transportPacket->removeTag<DscpReq>()) {
        ipv4Header->setDiffServCodePoint(dscpReq->getDifferentiatedServicesCodePoint());
        delete dscpReq;
    }
    if (EcnReq *ecnReq = transportPacket->removeTag<EcnReq>()) {
        ipv4Header->setExplicitCongestionNotification(ecnReq->getExplicitCongestionNotification());
        delete ecnReq;
    }

    ipv4Header->setIdentification(curFragmentId++);
    ipv4Header->setMoreFragments(false);
    ipv4Header->setDontFragment(dontFragment);
    ipv4Header->setFragmentOffset(0);

    if (ttl != -1) {
        ASSERT(ttl > 0);
    }
    else if (ipv4Header->getDestAddress().isLinkLocalMulticast())
        ttl = 1;
    else if (ipv4Header->getDestAddress().isMulticast())
        ttl = defaultMCTimeToLive;
    else
        ttl = defaultTimeToLive;
    ipv4Header->setTimeToLive(ttl);
    ipv4Header->setTotalLengthField(B(ipv4Header->getChunkLength()).get() + transportPacket->getByteLength());
    ipv4Header->setCrcMode(crcMode);
    ipv4Header->setCrc(0);
    switch (crcMode) {
        case CRC_DECLARED_CORRECT:
            // if the CRC mode is declared to be correct, then set the CRC to an easily recognizable value
            ipv4Header->setCrc(0xC00D);
            break;
        case CRC_DECLARED_INCORRECT:
            // if the CRC mode is declared to be incorrect, then set the CRC to an easily recognizable value
            ipv4Header->setCrc(0xBAAD);
            break;
        case CRC_COMPUTED: {
            ipv4Header->setCrc(0);
            // crc will be calculated in fragmentAndSend()
            break;
        }
        default:
            throw cRuntimeError("Unknown CRC mode");
    }
    insertNetworkProtocolHeader(transportPacket, Protocol::ipv4, ipv4Header);
    // setting IPv4 options is currently not supported
}

void IPv4::sendDatagramToOutput(Packet *packet)
{
    const InterfaceEntry *ie = ift->getInterfaceById(packet->getMandatoryTag<InterfaceReq>()->getInterfaceId());
    auto nextHopAddressReq = packet->removeMandatoryTag<NextHopAddressReq>();
    IPv4Address nextHopAddr = nextHopAddressReq->getNextHopAddress().toIPv4();
    delete nextHopAddressReq;
    if (!ie->isBroadcast() || ie->getMacAddress().isUnspecified()) // we can't do ARP
        sendPacketToNIC(packet);
    else {
        MACAddress nextHopMacAddr = resolveNextHopMacAddress(packet, nextHopAddr, ie);
        if (nextHopMacAddr.isUnspecified()) {
            EV_INFO << "Pending " << packet << " to ARP resolution.\n";
            pendingPackets[nextHopAddr].insert(packet);
        }
        else {
            ASSERT2(pendingPackets.find(nextHopAddr) == pendingPackets.end(), "IPv4-ARP error: nextHopAddr found in ARP table, but IPv4 queue for nextHopAddr not empty");
            packet->ensureTag<MacAddressReq>()->setDestAddress(nextHopMacAddr);
            sendPacketToNIC(packet);
        }
    }
}

void IPv4::arpResolutionCompleted(IARP::Notification *entry)
{
    if (entry->l3Address.getType() != L3Address::IPv4)
        return;
    auto it = pendingPackets.find(entry->l3Address.toIPv4());
    if (it != pendingPackets.end()) {
        cPacketQueue& packetQueue = it->second;
        EV << "ARP resolution completed for " << entry->l3Address << ". Sending " << packetQueue.getLength()
           << " waiting packets from the queue\n";

        while (!packetQueue.isEmpty()) {
            Packet *packet = check_and_cast<Packet *>(packetQueue.pop());
            EV << "Sending out queued packet " << packet << "\n";
            packet->ensureTag<InterfaceReq>()->setInterfaceId(entry->ie->getInterfaceId());
            packet->ensureTag<MacAddressReq>()->setDestAddress(entry->macAddress);
            sendPacketToNIC(packet);
        }
        pendingPackets.erase(it);
    }
}

void IPv4::arpResolutionTimedOut(IARP::Notification *entry)
{
    if (entry->l3Address.getType() != L3Address::IPv4)
        return;
    auto it = pendingPackets.find(entry->l3Address.toIPv4());
    if (it != pendingPackets.end()) {
        cPacketQueue& packetQueue = it->second;
        EV << "ARP resolution failed for " << entry->l3Address << ",  dropping " << packetQueue.getLength() << " packets\n";
        for (int i = 0; i < packetQueue.getLength(); i++) {
            auto packet = packetQueue.get(i);
            PacketDropDetails details;
            details.setReason(ADDRESS_RESOLUTION_FAILED);
            emit(packetDropSignal, packet, &details);
        }
        packetQueue.clear();
        pendingPackets.erase(it);
    }
}

MACAddress IPv4::resolveNextHopMacAddress(cPacket *packet, IPv4Address nextHopAddr, const InterfaceEntry *destIE)
{
    if (nextHopAddr.isLimitedBroadcastAddress() || nextHopAddr == destIE->ipv4Data()->getNetworkBroadcastAddress()) {
        EV_DETAIL << "destination address is broadcast, sending packet to broadcast MAC address\n";
        return MACAddress::BROADCAST_ADDRESS;
    }

    if (nextHopAddr.isMulticast()) {
        MACAddress macAddr = MACAddress::makeMulticastAddress(nextHopAddr);
        EV_DETAIL << "destination address is multicast, sending packet to MAC address " << macAddr << "\n";
        return macAddr;
    }

    return arp->resolveL3Address(nextHopAddr, destIE);
}

void IPv4::sendPacketToNIC(Packet *packet)
{
    EV_INFO << "Sending " << packet << " to output interface = " << ift->getInterfaceById(packet->getMandatoryTag<InterfaceReq>()->getInterfaceId())->getInterfaceName() << ".\n";
    packet->ensureTag<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
    packet->ensureTag<DispatchProtocolInd>()->setProtocol(&Protocol::ipv4);
    delete packet->removeTag<DispatchProtocolReq>();
    ASSERT(packet->getTag<InterfaceReq>() != nullptr);
    send(packet, "queueOut");
}

// NetFilter:

void IPv4::registerHook(int priority, INetfilter::IHook *hook)
{
    Enter_Method("registerHook()");
    NetfilterBase::registerHook(priority, hook);
}

void IPv4::unregisterHook(INetfilter::IHook *hook)
{
    Enter_Method("unregisterHook()");
    NetfilterBase::unregisterHook(hook);
}

void IPv4::dropQueuedDatagram(const Packet *packet)
{
    Enter_Method("dropQueuedDatagram()");
    for (auto iter = queuedDatagramsForHooks.begin(); iter != queuedDatagramsForHooks.end(); iter++) {
        if (iter->packet == packet) {
            delete packet;
            queuedDatagramsForHooks.erase(iter);
            return;
        }
    }
}

void IPv4::reinjectQueuedDatagram(const Packet *packet)
{
    Enter_Method("reinjectDatagram()");
    for (auto iter = queuedDatagramsForHooks.begin(); iter != queuedDatagramsForHooks.end(); iter++) {
        if (iter->packet == packet) {
            auto *qPacket = iter->packet;
            take(qPacket);
            switch (iter->hookType) {
                case INetfilter::IHook::LOCALOUT:
                    datagramLocalOut(qPacket);
                    break;

                case INetfilter::IHook::PREROUTING:
                    preroutingFinish(qPacket);
                    break;

                case INetfilter::IHook::POSTROUTING:
                    fragmentAndSend(qPacket);
                    break;

                case INetfilter::IHook::LOCALIN:
                    reassembleAndDeliverFinish(qPacket);
                    break;

                case INetfilter::IHook::FORWARD:
                    routeUnicastPacketFinish(qPacket);
                    break;

                default:
                    throw cRuntimeError("Unknown hook ID: %d", (int)(iter->hookType));
                    break;
            }
            queuedDatagramsForHooks.erase(iter);
            return;
        }
    }
}

INetfilter::IHook::Result IPv4::datagramPreRoutingHook(Packet *packet)
{
    for (auto & elem : hooks) {
        IHook::Result r = elem.second->datagramPreRoutingHook(packet);
        switch (r) {
            case INetfilter::IHook::ACCEPT:
                break;    // continue iteration

            case INetfilter::IHook::DROP:
                delete packet;
                return r;

            case INetfilter::IHook::QUEUE:
                queuedDatagramsForHooks.push_back(QueuedDatagramForHook(packet, INetfilter::IHook::PREROUTING));
                return r;

            case INetfilter::IHook::STOLEN:
                return r;

            default:
                throw cRuntimeError("Unknown Hook::Result value: %d", (int)r);
        }
    }
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result IPv4::datagramForwardHook(Packet *packet)
{
    for (auto & elem : hooks) {
        IHook::Result r = elem.second->datagramForwardHook(packet);
        switch (r) {
            case INetfilter::IHook::ACCEPT:
                break;    // continue iteration

            case INetfilter::IHook::DROP:
                delete packet;
                return r;

            case INetfilter::IHook::QUEUE:
                queuedDatagramsForHooks.push_back(QueuedDatagramForHook(packet, INetfilter::IHook::FORWARD));
                return r;

            case INetfilter::IHook::STOLEN:
                return r;

            default:
                throw cRuntimeError("Unknown Hook::Result value: %d", (int)r);
        }
    }
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result IPv4::datagramPostRoutingHook(Packet *packet)
{
    for (auto & elem : hooks) {
        IHook::Result r = elem.second->datagramPostRoutingHook(packet);
        switch (r) {
            case INetfilter::IHook::ACCEPT:
                break;    // continue iteration

            case INetfilter::IHook::DROP:
                delete packet;
                return r;

            case INetfilter::IHook::QUEUE:
                queuedDatagramsForHooks.push_back(QueuedDatagramForHook(packet, INetfilter::IHook::POSTROUTING));
                return r;

            case INetfilter::IHook::STOLEN:
                return r;

            default:
                throw cRuntimeError("Unknown Hook::Result value: %d", (int)r);
        }
    }
    return INetfilter::IHook::ACCEPT;
}

bool IPv4::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_NETWORK_LAYER)
            start();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_NETWORK_LAYER)
            stop();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH)
            stop();
    }
    return true;
}

void IPv4::start()
{
    ASSERT(queue.isEmpty());
    isUp = true;
}

void IPv4::stop()
{
    isUp = false;
    flush();
}

void IPv4::flush()
{
    delete cancelService();
    EV_DEBUG << "IPv4::flush(): packets in queue: " << queue.str() << endl;
    queue.clear();

    EV_DEBUG << "IPv4::flush(): pending packets:\n";
    for (auto & elem : pendingPackets) {
        EV_DEBUG << "IPv4::flush():    " << elem.first << ": " << elem.second.str() << endl;
        elem.second.clear();
    }
    pendingPackets.clear();

    EV_DEBUG << "IPv4::flush(): packets in hooks: " << queuedDatagramsForHooks.size() << endl;
    for (auto & elem : queuedDatagramsForHooks) {
        delete elem.packet;
    }
    queuedDatagramsForHooks.clear();
}

bool IPv4::isNodeUp()
{
    NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

INetfilter::IHook::Result IPv4::datagramLocalInHook(Packet *packet)
{
    for (auto & elem : hooks) {
        IHook::Result r = elem.second->datagramLocalInHook(packet);
        switch (r) {
            case INetfilter::IHook::ACCEPT:
                break;    // continue iteration

            case INetfilter::IHook::DROP:
                delete packet;
                return r;

            case INetfilter::IHook::QUEUE: {
                if (packet->getOwner() != this)
                    throw cRuntimeError("Model error: netfilter hook changed the owner of queued datagram '%s'", packet->getFullName());
                queuedDatagramsForHooks.push_back(QueuedDatagramForHook(packet, INetfilter::IHook::LOCALIN));
                return r;
            }

            case INetfilter::IHook::STOLEN:
                return r;

            default:
                throw cRuntimeError("Unknown Hook::Result value: %d", (int)r);
        }
    }
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result IPv4::datagramLocalOutHook(Packet *packet)
{
    for (auto & elem : hooks) {
        IHook::Result r = elem.second->datagramLocalOutHook(packet);
        switch (r) {
            case INetfilter::IHook::ACCEPT:
                break;    // continue iteration

            case INetfilter::IHook::DROP:
                delete packet;
                return r;

            case INetfilter::IHook::QUEUE:
                queuedDatagramsForHooks.push_back(QueuedDatagramForHook(packet, INetfilter::IHook::LOCALOUT));
                return r;

            case INetfilter::IHook::STOLEN:
                return r;

            default:
                throw cRuntimeError("Unknown Hook::Result value: %d", (int)r);
        }
    }
    return INetfilter::IHook::ACCEPT;
}

void IPv4::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();

    if (signalID == IARP::completedARPResolutionSignal) {
        arpResolutionCompleted(check_and_cast<IARP::Notification *>(obj));
    }
    if (signalID == IARP::failedARPResolutionSignal) {
        arpResolutionTimedOut(check_and_cast<IARP::Notification *>(obj));
    }
}

void IPv4::sendIcmpError(Packet *origPacket, int inputInterfaceId, ICMPType type, ICMPCode code)
{
    icmp->sendErrorMessage(origPacket, inputInterfaceId, type, code);
}

} // namespace inet

