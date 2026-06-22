//
// Copyright (C) 2005 OpenSim Ltd.
// Copyright (C) 2005 Wei Yang, Ng
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv6/Ipv6.h"

#include "inet/common/INETUtils.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Message.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/networklayer/common/EcnTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/Icmpv6ErrorTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/common/NextHopAddressTag_m.h"
#include "inet/networklayer/common/TosTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv6/Ipv6SocketCommand_m.h"
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#include "inet/networklayer/ipv6/Ipv6ExtHeaderIndexTag_m.h"
#include "inet/networklayer/icmpv6/Ipv6NdMessage_m.h"
#include "inet/networklayer/ipv6/Ipv6ExtHeaderTag_m.h"
#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6MulticastRoute.h"
#include "inet/networklayer/ipv6/Mipv6InterfaceData.h"


namespace inet {

#define FRAGMENT_TIMEOUT    60   // 60 sec, from Ipv6 RFC


Define_Module(Ipv6);

Ipv6::Ipv6() :
    curFragmentId(0)
{
}

Ipv6::~Ipv6()
{
    for (auto it : socketIdToSocketDescriptor)
        delete it.second;

    for (auto& elem : queuedDatagramsForHooks) {
        delete elem.packet;
    }

    for (auto *sDgram : pendingDadQueue)
        delete sDgram;
}

Ipv6::ScheduledDatagram::ScheduledDatagram(Packet *packet, const Ipv6Header *ipv6Header, const NetworkInterface *ie, MacAddress macAddr, bool fromHL) :
    packet(packet),
    ipv6Header(ipv6Header),
    ie(ie),
    macAddr(macAddr),
    fromHL(fromHL)
{
}

Ipv6::ScheduledDatagram::~ScheduledDatagram()
{
    delete packet;
}

void Ipv6::initialize(int stage)
{
    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);
        nd.reference(this, "ipv6NeighbourDiscoveryModule", true);
        icmp.reference(this, "icmpv6Module", true);

        curFragmentId = 0;
        lastCheckTime = SIMTIME_ZERO;
        sendRedirects = par("sendRedirects");
        fragbuf.init(icmp);

        // NetFilter:
        hooks.clear();
        queuedDatagramsForHooks.clear();

        numMulticast = numLocalDeliver = numDropped = numUnroutable = numForwarded = 0;

        WATCH(curFragmentId);
        WATCH(lastCheckTime);
        WATCH(numMulticast);
        WATCH(numLocalDeliver);
        WATCH(numDropped);
        WATCH(numUnroutable);
        WATCH(numForwarded);
        WATCH_EXPR("ipv6StatusText", getIpv6StatusText());
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        registerService(Protocol::ipv6, gate("transportIn"), gate("transportOut"));
        registerProtocol(Protocol::ipv6, gate("queueOut"), gate("queueIn"));

        // Subscribe to DAD signals from the NeighbourDiscovery module
        nd->subscribe(Ipv6NeighbourDiscovery::dadCompletedSignal, this);
        nd->subscribe(Ipv6NeighbourDiscovery::dadFailedSignal, this);
    }
}

std::string Ipv6::getIpv6StatusText() const
{
    std::string buf;
    if (numForwarded > 0)
        buf += "fwd:" + std::to_string(numForwarded) + " ";
    if (numLocalDeliver > 0)
        buf += "up:" + std::to_string(numLocalDeliver) + " ";
    if (numMulticast > 0)
        buf += "mcast:" + std::to_string(numMulticast) + " ";
    if (numDropped > 0)
        buf += "DROP:" + std::to_string(numDropped) + " ";
    if (numUnroutable > 0)
        buf += "UNROUTABLE:" + std::to_string(numUnroutable) + " ";
    return buf;
}

void Ipv6::handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
}

void Ipv6::handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    if (gate->isName("transportOut"))
        upperProtocols.insert(&protocol);
}

void Ipv6::handleRequest(Request *request)
{
    auto ctrl = request->getControlInfo();
    if (ctrl == nullptr)
        throw cRuntimeError("Request '%s' arrived without controlinfo", request->getName());
    if (auto *command = dynamic_cast<Ipv6SocketBindCommand *>(ctrl)) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        SocketDescriptor *descriptor = new SocketDescriptor(socketId, command->getProtocol()->getId(), command->getLocalAddress());
        socketIdToSocketDescriptor[socketId] = descriptor;
        delete request;
    }
    else if (auto *command = dynamic_cast<Ipv6SocketConnectCommand *>(ctrl)) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        if (!containsKey(socketIdToSocketDescriptor, socketId))
            throw cRuntimeError("Ipv6Socket: should use bind() before connect()");
        socketIdToSocketDescriptor[socketId]->remoteAddress = command->getRemoteAddress();
        delete request;
    }
    else if (dynamic_cast<Ipv6SocketCloseCommand *>(ctrl) != nullptr) {
        int socketId = 0;
        request->getTag<SocketReq>()->getSocketId();
        auto it = socketIdToSocketDescriptor.find(socketId);
        if (it != socketIdToSocketDescriptor.end()) {
            delete it->second;
            socketIdToSocketDescriptor.erase(it);
            auto indication = new Indication("closed", IPv6_I_SOCKET_CLOSED);
            auto ctrl = new Ipv6SocketClosedIndication();
            indication->setControlInfo(ctrl);
            indication->addTag<SocketInd>()->setSocketId(socketId);
            send(indication, "transportOut");
        }
        delete request;
    }
    else if (dynamic_cast<Ipv6SocketDestroyCommand *>(ctrl) != nullptr) {
        int socketId = 0;
        request->getTag<SocketReq>()->getSocketId();
        auto it = socketIdToSocketDescriptor.find(socketId);
        if (it != socketIdToSocketDescriptor.end()) {
            delete it->second;
            socketIdToSocketDescriptor.erase(it);
        }
        delete request;
    }
    else
        throw cRuntimeError("Unknown command: '%s' with %s", request->getName(), ctrl->getClassName());
}

void Ipv6::handleMessageWhenUp(cMessage *msg)
{
    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();

    if (auto indication = dynamic_cast<Indication *>(msg))
        handleIndication(indication);
    else if (auto request = dynamic_cast<Request *>(msg))
        handleRequest(request);
    else if (msg->getArrivalGate()->isName("transportIn")
             || (msg->arrivedOn("ndIn") && tags.getTag<PacketProtocolTag>()->getProtocol() == &Protocol::icmpv6)
             )
    {
        // packet from upper layers or ND: encapsulate and send out
        handleMessageFromHL(check_and_cast<Packet *>(msg));
    }
    else if (msg->arrivedOn("ndIn") && tags.getTag<PacketProtocolTag>()->getProtocol() == &Protocol::ipv6) {
        auto packet = check_and_cast<Packet *>(msg);
        Ipv6NdControlInfo *ctrl = check_and_cast<Ipv6NdControlInfo *>(msg->removeControlInfo());
        bool fromHL = ctrl->getFromHL();
        Ipv6Address nextHop = ctrl->getNextHop();
        int interfaceId = ctrl->getInterfaceId();
        delete ctrl;
        resolveMACAddressAndSendPacket(packet, interfaceId, nextHop, fromHL);
    }
    else {
        // datagram from network or from ND: localDeliver and/or route
        auto packet = check_and_cast<Packet *>(msg);
        auto ipv6Header = packet->peekAtFront<Ipv6Header>();
        packet->addTagIfAbsent<NetworkProtocolInd>()->setProtocol(&Protocol::ipv6);
        packet->addTagIfAbsent<NetworkProtocolInd>()->setNetworkProtocolHeader(ipv6Header);
        bool fromHL = false;
        if (packet->getArrivalGate()->isName("ndIn")) {
            Ipv6NdControlInfo *ctrl = check_and_cast<Ipv6NdControlInfo *>(msg->removeControlInfo());
            fromHL = ctrl->getFromHL();
            Ipv6Address nextHop = ctrl->getNextHop();
            int interfaceId = ctrl->getInterfaceId();
            delete ctrl;
            resolveMACAddressAndSendPacket(packet, interfaceId, nextHop, fromHL);
        }
        else
            emit(packetReceivedFromLowerSignal, msg);

        // Do not handle header biterrors, because
        // 1. Ipv6 header does not contain checksum for the header fields, each field is
        //    validated when they are processed.
        // 2. The Ethernet or PPP frame is dropped by the link-layer if there is a transmission error.
        ASSERT(!packet->hasBitError());

        const NetworkInterface *fromIE = getSourceInterfaceFrom(packet);
        const NetworkInterface *destIE = nullptr;
        if (fromHL) {
            // remove control info
            delete packet->removeControlInfo();
            // a netfilter hook (e.g. a MANET routing protocol) may set the next hop
            if (datagramLocalOutHook(packet) == INetfilter::IHook::ACCEPT)
                datagramLocalOut(packet, destIE, getNextHop(packet));
        }
        else {
            // a received datagram carries no meaningful egress interface request; drop
            // any stale one from the sender so it cannot misdirect routing here
            packet->removeTagIfPresent<InterfaceReq>();
            // a netfilter hook (e.g. a MANET routing protocol) may set the next hop
            if (datagramPreRoutingHook(packet) == INetfilter::IHook::ACCEPT)
                preroutingFinish(packet, fromIE, destIE, getNextHop(packet));
        }
    }
}

void Ipv6::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (signalID == Ipv6NeighbourDiscovery::dadCompletedSignal) {
        // DAD completed: send out all queued datagrams whose source address is no longer tentative
        for (auto it = pendingDadQueue.begin(); it != pendingDadQueue.end(); ) {
            ScheduledDatagram *sDgram = *it;
            if (!sDgram->getIE()->getProtocolData<Ipv6InterfaceData>()->isTentativeAddress(sDgram->getSrcAddress())) {
                it = pendingDadQueue.erase(it);
                numForwarded++;
                fragmentPostRouting(sDgram->removeDatagram(), sDgram->getIE(), sDgram->getMacAddress(), sDgram->getFromHL());
                delete sDgram;
            }
            else {
                ++it;
            }
        }
    }
    else if (signalID == Ipv6NeighbourDiscovery::dadFailedSignal) {
        // DAD failed: drop all queued datagrams whose source address was removed
        for (auto it = pendingDadQueue.begin(); it != pendingDadQueue.end(); ) {
            ScheduledDatagram *sDgram = *it;
            if (!sDgram->getIE()->getProtocolData<Ipv6InterfaceData>()->hasAddress(sDgram->getSrcAddress())) {
                EV_WARN << "Dropping queued datagram -- source address " << sDgram->getSrcAddress() << " DAD failed\n";
                it = pendingDadQueue.erase(it);
                delete sDgram;
                numDropped++;
            }
            else {
                ++it;
            }
        }
    }
}

void Ipv6::handleIndication(Indication *indication)
{
    if (indication->findTag<Icmpv6ErrorInd>())
        handleIcmpErrorIndication(indication);
    else
        throw cRuntimeError("Unknown Indication arrived: %s", indication->getName());
}

void Ipv6::handleIcmpErrorIndication(Indication *indication)
{
    auto& errorInd = indication->getTagForUpdate<Icmpv6ErrorInd>();
    Packet *originalPacket = errorInd->getOriginalPacketForUpdate();

    // Pop the quoted IPv6 header and convert it to tags on the original packet
    const auto& ipv6Header = originalPacket->popAtFront<Ipv6Header>();

    auto& l3Ind = originalPacket->addTagIfAbsent<L3AddressInd>();
    l3Ind->setSrcAddress(ipv6Header->getSrcAddress());
    l3Ind->setDestAddress(ipv6Header->getDestAddress());
    originalPacket->addTagIfAbsent<HopLimitInd>()->setHopLimit(ipv6Header->getHopLimit());

    // Dispatch the same Indication to the appropriate transport protocol.
    // SP_INDICATION routes via protocolToGateIndex to the transport module's ipIn gate.
    // The offending packet may begin with an IPv6 extension header (e.g. a Type-2
    // Routing Header inserted by route optimization), which is not a registered
    // transport protocol; in that case the error cannot be delivered to a transport,
    // so discard it -- the protocol recovers on its own (e.g. via retransmission).
    auto protocol = ProtocolGroup::getIpProtocolGroup()->findProtocol(ipv6Header->getProtocolId());
    if (protocol == nullptr) {
        EV_WARN << "ICMPv6 error indication for a packet whose leading header ("
                << ipv6Header->getProtocolId() << ") is not a registered transport protocol "
                << "(e.g. an IPv6 extension header); discarding\n";
        delete indication;
        return;
    }
    auto& dispatchReq = indication->addTagIfAbsent<DispatchProtocolReq>();
    dispatchReq->setProtocol(protocol);
    dispatchReq->setServicePrimitive(SP_INDICATION);

    EV_INFO << "Forwarding ICMPv6 error indication to transport protocol " << protocol->getName() << "\n";
    send(indication, "transportOut");
}

NetworkInterface *Ipv6::getSourceInterfaceFrom(Packet *packet)
{
    const auto& interfaceInd = packet->findTag<InterfaceInd>();
    return interfaceInd != nullptr ? ift->getInterfaceById(interfaceInd->getInterfaceId()) : nullptr;
}

Ipv6Address Ipv6::getNextHop(Packet *packet)
{
    const auto& tag = packet->findTag<NextHopAddressReq>();
    return tag != nullptr ? tag->getNextHopAddress().toIpv6() : Ipv6Address::UNSPECIFIED_ADDRESS;
}

void Ipv6::preroutingFinish(Packet *packet, const NetworkInterface *fromIE, const NetworkInterface *destIE, Ipv6Address nextHopAddr)
{
    const auto& ipv6Header = packet->peekAtFront<Ipv6Header>();
    Ipv6Address destAddr = ipv6Header->getDestAddress();
    // remove control info
    delete packet->removeControlInfo();

    // routepacket
    if (!destAddr.isMulticast())
        routePacket(packet, destIE, fromIE, nextHopAddr, false);
    else
        routeMulticastPacket(packet, destIE, fromIE, false);
}

void Ipv6::handleMessageFromHL(Packet *packet)
{
    emit(packetReceivedFromUpperSignal, packet);

    // if no interface exists, do not send datagram
    if (ift->getNumInterfaces() == 0) {
        EV_WARN << "No interfaces exist, dropping packet\n";
        PacketDropDetails details;
        details.setReason(NO_INTERFACE_FOUND);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }

    const auto& ifTag = packet->findTag<InterfaceReq>();
    const NetworkInterface *destIE = ifTag ? ift->getInterfaceById(ifTag->getInterfaceId()) : nullptr;

    // when source address was given, use it; otherwise it'll get the address
    // of the outgoing interface after routing
    Ipv6Address src = packet->getTag<L3AddressReq>()->getSrcAddress().toIpv6();
    if (!src.isUnspecified()) {
        // if interface parameter does not match existing interface, do not send datagram
        if (rt->getInterfaceByAddress(src) == nullptr) {
            EV_WARN << "Encapsulation failed - Loss of interface with source address " << src << ", dropping packet." << endl;
            PacketDropDetails details;
            details.setReason(NO_INTERFACE_FOUND);
            emit(packetDroppedSignal, packet, &details);
            delete packet;
            return;
        }
    }

    // encapsulate upper-layer packet into IPv6Datagram
    encapsulate(packet);

    auto ipv6Header = packet->peekAtFront<Ipv6Header>();
    Ipv6Address destAddress = ipv6Header->getDestAddress();

    // check for local delivery
    if (!destAddress.isMulticast() && rt->isLocalAddress(destAddress)) {
        EV_INFO << "local delivery\n";
        if (ipv6Header->getSrcAddress().isUnspecified()) {
            // allows two apps on the same host to communicate
            packet->trim();
            ipv6Header = nullptr;
            const auto& newIpv6Header = removeNetworkProtocolHeader<Ipv6Header>(packet);
            newIpv6Header->setSrcAddress(destAddress);
            insertNetworkProtocolHeader(packet, Protocol::ipv6, newIpv6Header);
            ipv6Header = newIpv6Header;
        }

        if (destIE && !destIE->isLoopback()) {
            EV_INFO << "datagram destination address is local, ignoring destination interface specified in the control info\n";
            destIE = nullptr;
        }
        if (!destIE)
            destIE = ift->findFirstLoopbackInterface();
        ASSERT(destIE);
    }
    // a netfilter hook (e.g. a MANET routing protocol) may set the next hop
    if (datagramLocalOutHook(packet) == INetfilter::IHook::ACCEPT)
        datagramLocalOut(packet, destIE, getNextHop(packet));
}

void Ipv6::datagramLocalOut(Packet *packet, const NetworkInterface *destIE, Ipv6Address requestedNextHopAddress)
{
    const auto& ipv6Header = packet->peekAtFront<Ipv6Header>();
    // route packet
    if (destIE != nullptr) {
        if (!ipv6Header->getDestAddress().isMulticast())
            fragmentPostRouting(packet, destIE, MacAddress::BROADCAST_ADDRESS, true);
        else
            fragmentPostRouting(packet, destIE, ipv6Header->getDestAddress().mapToMulticastMacAddress(), true);
    }
    else if (!ipv6Header->getDestAddress().isMulticast())
        routePacket(packet, destIE, nullptr, requestedNextHopAddress, true);
    else
        routeMulticastPacket(packet, destIE, nullptr, true);
}

void Ipv6::routePacket(Packet *packet, const NetworkInterface *destIE, const NetworkInterface *fromIE, Ipv6Address requestedNextHopAddress, bool fromHL)
{
    auto ipv6Header = packet->peekAtFront<Ipv6Header>();
    // TODO add option handling code here
    Ipv6Address destAddress = ipv6Header->getDestAddress();

    EV_INFO << "Routing datagram `" << ipv6Header->getName() << "' with dest=" << destAddress << ", requested nexthop is " << requestedNextHopAddress << " on " << (destIE ? destIE->getFullName() : "unspec") << " interface: \n";

    // local delivery of unicast packets
    if (rt->isLocalAddress(destAddress)) {
        if (fromHL)
            throw cRuntimeError("model error: local unicast packet arrived from HL, but handleMessageFromHL() not detected it");
        EV_INFO << "local delivery\n";

        numLocalDeliver++;
        localDeliver(packet, fromIE);
        return;
    }

    if (!fromHL) {
        // if datagram arrived from input gate and IP forwarding is off, delete datagram
        // yes but datagrams from the ND module is getting dropped too!-WEI
        // so we add a 2nd condition
        // FIXME rewrite code so that condition is cleaner --Andras
        //if (!rt->isRouter())
        if (!rt->isRouter() && !(packet->getArrivalGate()->isName("ndIn"))) {
            EV_INFO << "forwarding is off, dropping packet\n";
            numDropped++;
            delete packet;
            return;
        }

        // don't forward link-local addresses or weaker
        if (destAddress.isLinkLocal() || destAddress.isLoopback()) {
            EV_INFO << "dest address is link-local (or weaker) scope, doesn't get forwarded\n";
            delete packet;
            return;
        }

        // hop counter decrement: only if datagram arrived from network, and will be
        // sent out to the network (hoplimit check will be done just before sending
        // out datagram)
        // TODO: in Ipv4, arrange TTL check like this
        packet->trim();
        ipv6Header = nullptr;
        const auto& newIpv6Header = removeNetworkProtocolHeader<Ipv6Header>(packet);
        newIpv6Header->setHopLimit(newIpv6Header->getHopLimit() - 1);
        insertNetworkProtocolHeader(packet, Protocol::ipv6, newIpv6Header);
        ipv6Header = newIpv6Header;
    }

    // routing
    int interfaceId = -1;
    Ipv6Address nextHop(requestedNextHopAddress);

    if (interfaceId == -1 && destIE != nullptr)
        interfaceId = destIE->getInterfaceId(); // output interface from a control-info request

    if (interfaceId == -1) {
        // a netfilter hook (pre-routing or local-out) may have requested an output
        // interface for this datagram, e.g. a tunnel interface
        const auto& ifReq = packet->findTag<InterfaceReq>();
        if (ifReq != nullptr)
            interfaceId = ifReq->getInterfaceId();
    }

    // A point-to-point output interface (e.g. a tunnel interface) has no next hop to
    // resolve, but resolveMACAddressAndSendPacket() requires one to be specified, so
    // use the destination as the nominal next hop (not used to address the link).
    if (interfaceId != -1 && nextHop.isUnspecified()) {
        NetworkInterface *ie = ift->getInterfaceById(interfaceId);
        if (ie != nullptr && ie->isPointToPoint())
            nextHop = destAddress;
    }

    if (interfaceId == -1)
        if (!determineOutputInterface(destAddress, nextHop, interfaceId, packet, fromHL))
            // no interface found; sent to ND or to ICMP for error processing
//            throw cRuntimeError("No interface found!");//return;
            return;
    // don't raise error if sent to ND or ICMP!

    // RFC 4861 Section 8.2: a router sends a Redirect if it is forwarding a
    // packet back out on the same interface it arrived on, to a different next-hop.
    // Disabled (sendRedirects=false) on wireless multi-hop/ad-hoc networks, where the
    // next hop is not directly reachable by the source and redirects cause a storm.
    if (sendRedirects && rt->isRouter() && !fromHL && fromIE && interfaceId == fromIE->getInterfaceId()) {
        Ipv6Address pktSrcAddr = ipv6Header->getSrcAddress();
        if (nextHop != pktSrcAddr && !pktSrcAddr.isUnspecified() && !pktSrcAddr.isMulticast()) {
            NetworkInterface *outIe = ift->getInterfaceById(interfaceId);
            nd->sendRedirect(packet, nextHop, destAddress, outIe);
        }
    }

    resolveMACAddressAndSendPacket(packet, interfaceId, nextHop, fromHL);
}

void Ipv6::resolveMACAddressAndSendPacket(Packet *packet, int interfaceId, Ipv6Address nextHop, bool fromHL)
{
    const auto& ipv6Header = packet->peekAtFront<Ipv6Header>();
    NetworkInterface *ie = ift->getInterfaceById(interfaceId);
    ASSERT(ie != nullptr);
    ASSERT(!nextHop.isUnspecified());
    Ipv6Address destAddress = ipv6Header->getDestAddress();
    EV_INFO << "next hop for " << destAddress << " is " << nextHop << ", interface " << ie->getInterfaceName() << "\n";

    if (rt->isMobileNode()) {
        // if the source address is the HoA and we have a CoA then drop the packet
        // (address is topologically incorrect!). Skipped for interfaces without
        // MIPv6 data (e.g. a tunnel interface, which legitimately carries
        // HoA-sourced inner packets toward the home agent).
        auto mipv6Data = ie->findProtocolData<Mipv6InterfaceData>();
        auto ipv6Data = ie->findProtocolData<Ipv6InterfaceData>();
        if (mipv6Data && ipv6Data
            && ipv6Header->getSrcAddress() == mipv6Data->getMNHomeAddress()
            && !ipv6Data->getGlobalAddress(Ipv6InterfaceData::CoA).isUnspecified())
        {
            EV_WARN << "Using HoA instead of CoA... dropping datagram" << endl;
            delete packet;
            numDropped++;
            return;
        }
    }

    MacAddress macAddr = nd->resolveNeighbour(nextHop, interfaceId); // might initiate NUD
    if (macAddr.isUnspecified()) {
        if (!ie->isPointToPoint()) {
            EV_INFO << "no link-layer address for next hop yet, passing datagram to Neighbour Discovery module\n";
            Ipv6NdControlInfo *ctrl = new Ipv6NdControlInfo();
            ctrl->setFromHL(fromHL);
            ctrl->setNextHop(nextHop);
            ctrl->setInterfaceId(interfaceId);
            packet->cMessage::setControlInfo(ctrl);
            send(packet, "ndOut");
            return;
        }
    }
    else
        EV_DETAIL << "link-layer address: " << macAddr << "\n";

    // send out datagram
    numForwarded++;
    fragmentPostRouting(packet, ie, macAddr, fromHL);
}

void Ipv6::routeMulticastPacket(Packet *packet, const NetworkInterface *destIE, const NetworkInterface *fromIE, bool fromHL)
{
    auto ipv6Header = packet->peekAtFront<Ipv6Header>();
    Ipv6Address srcAddr = ipv6Header->getSrcAddress();
    Ipv6Address destAddr = ipv6Header->getDestAddress();
    ASSERT(destAddr.isMulticast());

    EV_INFO << "destination address " << destAddr << " is multicast, doing multicast routing\n";
    numMulticast++;

    // Locally-originated multicast without a pinned output interface: send it onto
    // the local link(s). When the upper layer pins a destIE, datagramLocalOut() sends
    // the packet out directly and never reaches this method, so fromIE/destIE are null
    // here. A source host typically has a single non-loopback interface, so this just
    // puts the datagram on its link; multi-homed senders emit on each multicast link.
    if (fromHL) {
        ASSERT(fromIE == nullptr);
        EV_INFO << "sending locally-originated multicast datagram out on local interfaces\n";
        for (int i = 0; i < ift->getNumInterfaces(); i++) {
            NetworkInterface *ie = ift->getInterface(i);
            if (!ie->isLoopback() && ie->isMulticast())
                fragmentPostRouting(packet->dup(), ie, destAddr.mapToMulticastMacAddress(), true);
        }
        delete packet;
        return;
    }

    // arrived from the network
    ASSERT(fromIE != nullptr);

    // Deliver locally if we belong to the group. isLocalAddress() covers the
    // preassigned addresses (all-nodes/all-routers/solicited-node) used by ND, while
    // the incoming interface's membership covers application groups joined on it
    // (e.g. a routing protocol's LL-MANET-Routers group) -- mirroring Ipv4.
    // Additionally, multicast routers must receive MLD messages (ICMPv6 types 130-132)
    // regardless of group membership, mirroring the IPv4 rule that multicast routers
    // always receive IGMP datagrams (see Ipv4.cc: IP_PROT_IGMP special case).
    auto fromIeIpv6Data = fromIE->findProtocolData<Ipv6InterfaceData>();
    if (rt->isLocalAddress(destAddr) ||
        (fromIeIpv6Data && fromIeIpv6Data->isMemberOfMulticastGroup(destAddr)) ||
        (rt->isMulticastForwardingEnabled() && ipv6Header->getProtocolId() == IP_PROT_IPv6_ICMP))
    {
        EV_INFO << "local delivery of multicast packet\n";
        numLocalDeliver++;
        localDeliver(packet->dup(), fromIE);
    }

    // forward only if forwarding is enabled on this node
    if (!rt->isRouter()) {
        EV_INFO << "forwarding is off\n";
        delete packet;
        return;
    }

    // link-local (or smaller) scope multicast is never forwarded off-link
    if (destAddr.getMulticastScope() <= 2) {
        EV_INFO << "multicast dest address is link-local (or smaller) scope, not forwarding\n";
        delete packet;
        return;
    }

    // find the multicast route for (source, group); if there is none, give a multicast
    // routing protocol (e.g. PIM) a chance to create one, then look up again
    Ipv6MulticastRoute *route = rt->findBestMatchingMulticastRoute(srcAddr, destAddr);
    if (!route) {
        EV_WARN << "Multicast route does not exist, trying to add.\n";
        emit(ipv6NewMulticastSignal, ipv6Header.get(), const_cast<NetworkInterface *>(fromIE));
        route = rt->findBestMatchingMulticastRoute(srcAddr, destAddr);
        if (!route) {
            EV_ERROR << "No multicast route, packet dropped.\n";
            numUnroutable++;
            PacketDropDetails details;
            details.setReason(NO_ROUTE_FOUND);
            emit(packetDroppedSignal, packet, &details);
            delete packet;
            return;
        }
    }

    // Reverse Path Forwarding: the datagram must arrive on the route's parent
    // (upstream) interface; otherwise drop it (and notify, e.g. for PIM assert handling)
    if (route->getInInterface() && fromIE != route->getInInterface()->getInterface()) {
        EV_ERROR << "Multicast datagram did not arrive on the RPF interface, packet dropped.\n";
        emit(ipv6DataOnNonrpfSignal, ipv6Header.get(), const_cast<NetworkInterface *>(fromIE));
        numDropped++;
        PacketDropDetails details;
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }
    emit(ipv6DataOnRpfSignal, ipv6Header.get(), const_cast<NetworkInterface *>(fromIE)); // forwarding hook

    // decrement the hop limit before forwarding
    packet->trim();
    ipv6Header = nullptr;
    auto newIpv6Header = removeNetworkProtocolHeader<Ipv6Header>(packet);
    newIpv6Header->setHopLimit(newIpv6Header->getHopLimit() - 1);
    insertNetworkProtocolHeader(packet, Protocol::ipv6, newIpv6Header);
    ipv6Header = newIpv6Header;

    if (ipv6Header->getHopLimit() <= 0) {
        // RFC 4443: no ICMPv6 error is generated for a multicast packet
        EV_INFO << "multicast datagram hopLimit reached zero, dropping\n";
        numDropped++;
        PacketDropDetails details;
        details.setReason(HOP_LIMIT_REACHED);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }

    // forward a copy onto each downstream (child) interface of the route
    numForwarded++;
    for (unsigned int i = 0; i < route->getNumOutInterfaces(); i++) {
        Ipv6MulticastRoute::OutInterface *outInterface = route->getOutInterface(i);
        const NetworkInterface *outIE = outInterface->getInterface();
        if (outIE != fromIE) {
            // TRPB: on a leaf interface, forward only if there are group listeners
            if (outInterface->isLeaf() && !outIE->getProtocolData<Ipv6InterfaceData>()->hasMulticastListener(destAddr))
                EV_DETAIL << "Not forwarding to " << outIE->getInterfaceName() << " (leaf interface with no listeners)\n";
            else {
                EV_DETAIL << "Forwarding to " << outIE->getInterfaceName() << "\n";
                fragmentPostRouting(packet->dup(), outIE, destAddr.mapToMulticastMacAddress(), false);
            }
        }
    }

    emit(ipv6MdataRegisterSignal, packet, const_cast<NetworkInterface *>(fromIE)); // postRouting hook
    delete packet;
}

void Ipv6::localDeliver(Packet *packet, const NetworkInterface *fromIE)
{
    auto ipv6Header = packet->peekAtFront<Ipv6Header>();

    // Defragmentation: look for Ipv6FragmentHeader chunk after the base header
    // Scan extension header chunks to find the fragment header
    const Ipv6FragmentHeader *fh = nullptr;
    {
        b offset = ipv6Header->getChunkLength();
        IpProtocolId nextHdr = ipv6Header->getProtocolId();
        while (isIpv6ExtensionHeader(nextHdr)) {
            auto extHdr = peekIpv6ExtensionHeaderAt(packet, offset, nextHdr);
            if (nextHdr == IP_PROT_IPv6EXT_FRAGMENT) {
                auto fragHdr = dynamicPtrCast<const Ipv6FragmentHeader>(extHdr);
                if (fragHdr) {
                    fh = fragHdr.get();
                    break;
                }
            }
            nextHdr = extHdr->getNextHeaderProtocol();
            offset += extHdr->getChunkLength();
        }
    }
    if (fh) {
        EV_DETAIL << "Datagram fragment: offset=" << fh->getFragmentOffset()
                  << ", MORE=" << (fh->getMoreFragments() ? "true" : "false") << ".\n";

        // erase timed out fragments in fragmentation buffer; check every 10 seconds max
        if (simTime() >= lastCheckTime + 10) {
            lastCheckTime = simTime();
            fragbuf.purgeStaleFragments(simTime() - FRAGMENT_TIMEOUT);
        }

        packet = fragbuf.addFragment(packet, ipv6Header.get(), fh, simTime());
        if (!packet) {
            EV_DETAIL << "No complete datagram yet.\n";
            return;
        }
        EV_DETAIL << "This fragment completes the datagram.\n";
        // Re-peek header from the reassembled packet (old reference is stale)
        ipv6Header = packet->peekAtFront<Ipv6Header>();
    }

    // check for extension headers
    if (!processExtensionHeaders(packet)) {
        // ext. header processing not yet finished
        // datagram was sent to another module or dropped
        // -> interrupt local delivery process
        return;
    }

    auto origPacket = packet->dup();
    // Determine actual transport protocol by walking extension header chain
    IpProtocolId finalProtoId = ipv6Header->getProtocolId();
    {
        b off = ipv6Header->getChunkLength();
        while (isIpv6ExtensionHeader(finalProtoId)) {
            auto extHdr = peekIpv6ExtensionHeaderAt(packet, off, finalProtoId);
            finalProtoId = extHdr->getNextHeaderProtocol();
            off += extHdr->getChunkLength();
        }
    }
    const Protocol *protocol = ProtocolGroup::getIpProtocolGroup()->findProtocol(finalProtoId);
    auto remoteAddress(ipv6Header->getSrcAddress());
    auto localAddress(ipv6Header->getDestAddress());
    decapsulate(packet);
    bool hasSocket = false;
    for (const auto& elem : socketIdToSocketDescriptor) {
        if (elem.second->protocolId == protocol->getId()
            && (elem.second->localAddress.isUnspecified() || elem.second->localAddress == localAddress)
            && (elem.second->remoteAddress.isUnspecified() || elem.second->remoteAddress == remoteAddress))
        {
            auto *packetCopy = packet->dup();
            packetCopy->setKind(IPv6_I_DATA);
            packetCopy->addTagIfAbsent<SocketInd>()->setSocketId(elem.second->socketId);
            EV_INFO << "Passing up to socket " << elem.second->socketId << "\n";
            emit(packetSentToUpperSignal, packetCopy);
            send(packetCopy, "transportOut");
            hasSocket = true;
        }
    }

    if (protocol == &Protocol::icmpv6) {
        handleReceivedIcmp(packet);
    } // Added by WEI to forward ICMPv6 msgs to ICMPv6 module.
    else if (protocol == &Protocol::ipv6) {
        EV_INFO << "Tunnelled IP datagram\n";
        // Generic IPv6-in-IPv6 decapsulation (RFC 2473): the decapsulated inner
        // datagram is re-processed as if received from the network, so it is routed
        // or forwarded -- and seen by the netfilter pre-routing hooks -- normally.
        // The L3AddressInd left by decapsulate() carries the tunnel (outer) source.
        packet->removeTagIfPresent<InterfaceReq>();
        auto verdict = datagramPreRoutingHook(packet);
        if (verdict == INetfilter::IHook::ACCEPT)
            preroutingFinish(packet, fromIE, nullptr, Ipv6Address::UNSPECIFIED_ADDRESS);
        else if (verdict == INetfilter::IHook::DROP)
            delete packet;
    }
    else if (contains(upperProtocols, protocol)) {
        EV_INFO << "Passing up to protocol " << *protocol << "\n";
        emit(packetSentToUpperSignal, packet);
        send(packet, "transportOut");
    }
    else if (!hasSocket) {
        // send ICMP Destination Unreacheable error: protocol unavailable
        EV_INFO << "Transport layer gate not connected - dropping packet!\n";
        sendIcmpError(origPacket, ICMPv6_PARAMETER_PROBLEM, UNRECOGNIZED_NEXT_HDR_TYPE);
        origPacket = nullptr; // for not delete
        delete packet; // delete decapsulated packet
    }
    else {
        // the packet was delivered to one or more sockets above (a copy per socket) and
        // is not also handled by a registered upper protocol, so the original is now
        // redundant and must be freed (otherwise it leaks).
        delete packet;
    }
    delete origPacket;
}

void Ipv6::handleReceivedIcmp(Packet *msg)
{
    const auto& icmpHeader = msg->peekAtFront<Icmpv6Header>();
    if (dynamicPtrCast<const Ipv6NdMessage>(icmpHeader) != nullptr) {
        EV_INFO << "Neighbour Discovery packet: passing it to ND module\n";
        send(msg, "ndOut");
    }
    else {
        EV_INFO << "ICMPv6 packet: passing it to ICMPv6 module\n";
        send(msg, "transportOut");
    }
}

void Ipv6::decapsulate(Packet *packet)
{
    // save the front offset so upper layers can restore the original IP datagram
    auto networkHeaderFrontOffset = packet->getFrontOffset();

    // decapsulate transport packet
    auto ipv6Header = packet->popAtFront<Ipv6Header>();

    B payloadLength = ipv6Header->getPayloadLength();
    if (payloadLength != B(0)) { // payloadLength == 0 occured with Jumbo payload
        ASSERT(payloadLength <= packet->getDataLength());
        // drop padding behind the payload:
        if (payloadLength < packet->getDataLength())
            packet->setBackOffset(packet->getFrontOffset() + payloadLength);
    }

    // Pop extension header chunks and find the actual transport protocol
    IpProtocolId nextHdr = ipv6Header->getProtocolId();
    while (isIpv6ExtensionHeader(nextHdr)) {
        auto extHdr = peekIpv6ExtensionHeaderAt(packet, b(0), nextHdr);
        packet->popAtFront(extHdr->getChunkLength());
        nextHdr = extHdr->getNextHeaderProtocol();
    }
    auto payloadProtocol = ProtocolGroup::getIpProtocolGroup()->findProtocol(nextHdr);

    // create and fill in control info
    packet->addTagIfAbsent<TosInd>()->setTos(ipv6Header->getTrafficClass());
    packet->addTagIfAbsent<DscpInd>()->setDifferentiatedServicesCodePoint(ipv6Header->getDscp());
    packet->addTagIfAbsent<EcnInd>()->setExplicitCongestionNotification(ipv6Header->getEcn());
    packet->addTagIfAbsent<DispatchProtocolInd>()->setProtocol(&Protocol::ipv6);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(payloadProtocol);
    auto networkProtocolInd = packet->addTagIfAbsent<NetworkProtocolInd>();
    networkProtocolInd->setProtocol(&Protocol::ipv6);
    networkProtocolInd->setNetworkProtocolHeader(ipv6Header);
    networkProtocolInd->setNetworkHeaderFrontOffset(networkHeaderFrontOffset);
    auto l3AddressInd = packet->addTagIfAbsent<L3AddressInd>();
    l3AddressInd->setSrcAddress(ipv6Header->getSrcAddress());
    l3AddressInd->setDestAddress(ipv6Header->getDestAddress());
    packet->addTagIfAbsent<HopLimitInd>()->setHopLimit(ipv6Header->getHopLimit());
}

void Ipv6::encapsulate(Packet *transportPacket)
{
    auto ipv6Header = makeShared<Ipv6Header>(); // TODO transportPacket->getName());

    auto& addresses = transportPacket->removeTag<L3AddressReq>();
    Ipv6Address src = addresses->getSrcAddress().toIpv6();
    Ipv6Address dest = addresses->getDestAddress().toIpv6();

    auto hopLimitReq = transportPacket->removeTagIfPresent<HopLimitReq>();
    short ttl = (hopLimitReq != nullptr) ? hopLimitReq->getHopLimit() : -1;

    ipv6Header->setPayloadLength(transportPacket->getDataLength());

    // set source and destination address
    ipv6Header->setDestAddress(dest);
    ipv6Header->setSrcAddress(src);

    // set other fields
    if (auto& tosReq = transportPacket->removeTagIfPresent<TosReq>()) {
        ipv6Header->setTrafficClass(tosReq->getTos());
        if (transportPacket->findTag<DscpReq>())
            throw cRuntimeError("TosReq and DscpReq found together");
        if (transportPacket->findTag<EcnReq>())
            throw cRuntimeError("TosReq and EcnReq found together");
    }
    if (auto& dscpReq = transportPacket->removeTagIfPresent<DscpReq>())
        ipv6Header->setDscp(dscpReq->getDifferentiatedServicesCodePoint());
    if (auto& ecnReq = transportPacket->removeTagIfPresent<EcnReq>())
        ipv6Header->setEcn(ecnReq->getExplicitCongestionNotification());

    ipv6Header->setHopLimit(ttl != -1 ? ttl : IPv6_DEFAULT_ADVCURHOPLIMIT);
    ASSERT(ipv6Header->getHopLimit() > 0);
    ipv6Header->setProtocolId(static_cast<IpProtocolId>(ProtocolGroup::getIpProtocolGroup()->getProtocolNumber(transportPacket->getTag<PacketProtocolTag>()->getProtocol())));

    // #### Move extension headers from tag to packet as separate chunks
    auto extHeadersTag = transportPacket->removeTagIfPresent<Ipv6ExtHeaderReq>();
    std::vector<Ipv6ExtensionHeader *> extHeaders;
    while (extHeadersTag && 0 < extHeadersTag->getExtensionHeaderArraySize()) {
        extHeaders.push_back(extHeadersTag->removeFirstExtensionHeader());
    }
    // Sort extension headers according to RFC 2460 §4.1 order
    std::stable_sort(extHeaders.begin(), extHeaders.end(),
        [](const Ipv6ExtensionHeader *a, const Ipv6ExtensionHeader *b) {
            return a->getOrder() < b->getOrder();
        });

    // Chain the Next Header fields
    IpProtocolId transportProto = ipv6Header->getProtocolId();
    if (!extHeaders.empty()) {
        ipv6Header->setProtocolId(static_cast<IpProtocolId>(extHeaders.front()->getExtensionType()));
        for (size_t i = 0; i < extHeaders.size() - 1; i++)
            extHeaders[i]->setNextHeaderProtocol(static_cast<IpProtocolId>(extHeaders[i + 1]->getExtensionType()));
        extHeaders.back()->setNextHeaderProtocol(transportProto);
    }

    transportPacket->trimFront();
    // Insert extension headers (in reverse order, each at front, so they end up in correct order)
    for (int i = (int)extHeaders.size() - 1; i >= 0; i--)
        transportPacket->insertAtFront(Ptr<Ipv6ExtensionHeader>(extHeaders[i]->dup()));
    insertNetworkProtocolHeader(transportPacket, Protocol::ipv6, ipv6Header);
    // Clean up owned ext headers from tag (originals)
    for (auto *eh : extHeaders)
        delete eh;
    // setting IP options is currently not supported
}

void Ipv6::fragmentPostRouting(Packet *packet, const NetworkInterface *ie, const MacAddress& nextHopAddr, bool fromHL)
{
//    const NetworkInterface *destIE = ift->getInterfaceById(packet->getTag<InterfaceReq>()->getInterfaceId());
    auto ipv6Header = packet->peekAtFront<Ipv6Header>();
    // ensure source address is filled
    if (fromHL && ipv6Header->getSrcAddress().isUnspecified() &&
        !ipv6Header->getDestAddress().isSolicitedNodeMulticastAddress())
    {
        // source address can be unspecified during DAD
        const Ipv6Address& srcAddr = ie->getProtocolData<Ipv6InterfaceData>()->getPreferredAddress();
        ASSERT(!srcAddr.isUnspecified()); // FIXME what if we don't have an address yet?

        // TODO factor out
        ipv6Header = nullptr;
        auto newIpv6Header = removeNetworkProtocolHeader<Ipv6Header>(packet);
        newIpv6Header->setSrcAddress(srcAddr);
        insertNetworkProtocolHeader(packet, Protocol::ipv6, newIpv6Header);
        ipv6Header = newIpv6Header;

        // RFC 4862: a tentative source address (DAD still in progress) must not be
        // used, so defer the datagram until DAD completes. Under RFC 4429 Optimistic
        // DAD the address may be used right away, so the deferral is skipped.
        if (ie->getProtocolData<Ipv6InterfaceData>()->isTentativeAddress(srcAddr)
                && !ie->getProtocolData<Ipv6InterfaceData>()->isOptimisticDad()) {
            EV_INFO << "Source address is tentative - enqueueing datagram for later resubmission." << endl;
            ScheduledDatagram *sDgram = new ScheduledDatagram(packet, ipv6Header.get(), ie, nextHopAddr, fromHL);
            take(sDgram);
            pendingDadQueue.push_back(sDgram);
            return;
        }
    }
    const NetworkInterface *fromIe = fromHL ? nullptr : ift->getInterfaceById(packet->getTag<InterfaceInd>()->getInterfaceId());
    L3Address nextHopAddr_(nextHopAddr);
    if (datagramPostRoutingHook(packet, fromIe, ie, nextHopAddr_) == INetfilter::IHook::ACCEPT) {
        fragmentAndSend(packet, ie, nextHopAddr_.toMac(), fromHL);
    }
}

void Ipv6::fragmentAndSend(Packet *packet, const NetworkInterface *ie, const MacAddress& nextHopAddr, bool fromHL)
{
    auto ipv6Header = packet->peekAtFront<Ipv6Header>();
    // hop counter check
    if (ipv6Header->getHopLimit() <= 0) {
        // drop datagram, destruction responsibility in ICMP
        EV_INFO << "datagram hopLimit reached zero, sending ICMPv6_TIME_EXCEEDED\n";
        sendIcmpError(packet, ICMPv6_TIME_EXCEEDED, 0); // FIXME check icmp 'code'
        return;
    }

    int mtu = ie->getMtu();

    // check if datagram does not require fragmentation
    if (packet->getDataLength() <= B(mtu)) {
        sendDatagramToOutput(packet, ie, nextHopAddr);
        return;
    }

    // routed datagrams are not fragmented
    if (!fromHL) {
        // FIXME check for multicast datagrams, how many ICMP error should be sent
        sendIcmpError(packet, ICMPv6_PACKET_TOO_BIG, 0); // TODO set MTU
        return;
    }

    // create and send fragments
    // Pop the base header (extension header chunks, if any, remain in the packet data)
    ipv6Header = packet->popAtFront<Ipv6Header>();

    // Calculate unfragmentable header length: base header + extension headers up to (but not including) the first
    // fragmentable point. For simplicity, the unfragmentable part is the base header only (40 bytes).
    // TODO: properly split unfragmentable/fragmentable extension headers
    B headerLength = IPv6_HEADER_BYTES;
    B payloadLength = packet->getDataLength();
    B fragmentLength = ((B(mtu) - headerLength - IPv6_FRAGMENT_HEADER_LENGTH) / 8) * 8;
    ASSERT(fragmentLength > B(0));

    int noOfFragments = (payloadLength + fragmentLength - B(1)).get<B>() / fragmentLength.get<B>();
    EV_INFO << "Breaking datagram into " << noOfFragments << " fragments\n";
    std::string fragMsgName = packet->getName();
    fragMsgName += "-frag-";

    unsigned int identification = curFragmentId++;
    // The base header's protocolId currently points to the first ext header (or transport).
    // We need to insert a Fragment Header into the chain.
    IpProtocolId origNextHdr = ipv6Header->getProtocolId();

    for (B offset = B(0); offset < payloadLength; offset += fragmentLength) {
        bool lastFragment = (offset + fragmentLength >= payloadLength);
        B thisFragmentLength = lastFragment ? payloadLength - offset : fragmentLength;

        std::string curFragName = fragMsgName + std::to_string(offset.get());
        if (lastFragment)
            curFragName += "-last";
        Packet *fragPk = new Packet(curFragName.c_str());

        // Create fragment header chunk
        auto fh = makeShared<Ipv6FragmentHeader>();
        fh->setIdentification(identification);
        fh->setFragmentOffset(offset.get());
        fh->setMoreFragments(!lastFragment);
        fh->setNextHeaderProtocol(origNextHdr);

        // Base header points to Fragment Header
        const auto& fragBaseHdr = staticPtrCast<Ipv6Header>(ipv6Header->dupShared());
        fragBaseHdr->setProtocolId(IP_PROT_IPv6EXT_FRAGMENT);

        fragPk->insertAtFront(fh);
        fragPk->insertAtFront(fragBaseHdr);
        fragPk->insertAtBack(packet->peekDataAt(offset, thisFragmentLength));

        ASSERT(fragPk->getDataLength() == headerLength + IPv6_FRAGMENT_HEADER_LENGTH + thisFragmentLength);

        sendDatagramToOutput(fragPk, ie, nextHopAddr);
    }

    delete packet;
}

void Ipv6::sendDatagramToOutput(Packet *packet, const NetworkInterface *destIE, const MacAddress& macAddr)
{
    packet->addTagIfAbsent<MacAddressReq>()->setDestAddress(macAddr);
    packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(destIE->getInterfaceId());
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv6);
    packet->addTagIfAbsent<DispatchProtocolInd>()->setProtocol(&Protocol::ipv6);
    auto protocol = destIE->getProtocol();
    if (protocol != nullptr)
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
    else
        packet->removeTagIfPresent<DispatchProtocolReq>();
    send(packet, "queueOut");
}

bool Ipv6::determineOutputInterface(const Ipv6Address& destAddress, Ipv6Address& nextHop,
        int& interfaceId, Packet *packet, bool fromHL)
{
    // try destination cache
    nextHop = rt->lookupDestCache(destAddress, interfaceId);

    if (interfaceId == -1) {
        // address not in destination cache: do longest prefix match in routing table
        EV_INFO << "do longest prefix match in routing table" << endl;
        const Ipv6Route *route = rt->doLongestPrefixMatch(destAddress);
        EV_INFO << "finished longest prefix match in routing table" << endl;
        if (!route) {
            if (rt->isRouter()) {
                EV_INFO << "unroutable, sending ICMPv6_DESTINATION_UNREACHABLE\n";
                numUnroutable++;
                sendIcmpError(packet, ICMPv6_DESTINATION_UNREACHABLE, 0); // FIXME check ICMP 'code'
            }
            else { // host
                EV_INFO << "no match in routing table, passing datagram to Neighbour Discovery module for default router selection\n";
                Ipv6NdControlInfo *ctrl = new Ipv6NdControlInfo();
                ctrl->setFromHL(fromHL);
                ctrl->setNextHop(nextHop);
                ctrl->setInterfaceId(interfaceId);
                packet->cMessage::setControlInfo(ctrl);
                send(packet, "ndOut");
            }
            return false;
        }
        interfaceId = route->getInterface() ? route->getInterface()->getInterfaceId() : -1;
        nextHop = route->getNextHop();
        if (nextHop.isUnspecified())
            nextHop = destAddress; // next hop is the host itself

        // add result into destination cache
        rt->updateDestCache(destAddress, nextHop, interfaceId, route->getExpiryTime());
    }

    return true;
}

bool Ipv6::processExtensionHeaders(Packet *packet)
{
    const auto& ipv6Header = packet->peekAtFront<Ipv6Header>();
    b offset = ipv6Header->getChunkLength();
    IpProtocolId nextHdr = ipv6Header->getProtocolId();

    while (isIpv6ExtensionHeader(nextHdr)) {
        auto extHdr = peekIpv6ExtensionHeaderAt(packet, offset, nextHdr);
        switch (nextHdr) {
            case IP_PROT_IPv6EXT_HOP: {
                auto hopHdr = dynamicPtrCast<const Ipv6HopByHopOptionsHeader>(extHdr);
                if (hopHdr) {
                    const TlvOptions& opts = hopHdr->getTlvOptions();
                    for (size_t j = 0; j < opts.getTlvOptionArraySize(); j++) {
                        const TlvOptionBase *opt = opts.getTlvOption(j);
                        int optType = opt->getType();
                        if (optType == IPv6TLVOPTION_NOP1 || optType == IPv6TLVOPTION_NOPN)
                            continue;
                        auto it = hopByHopOptionHandlers.find(optType);
                        if (it != hopByHopOptionHandlers.end()) {
                            if (!it->second->processTlvOption(packet, hopHdr.get(), opt))
                                return false;
                        }
                        else if (!handleUnrecognizedTlvOption(packet, optType))
                            return false;
                    }
                }
                break;
            }
            case IP_PROT_IPv6EXT_ROUTING: {
                auto rh = dynamicPtrCast<const Ipv6RoutingHeader>(extHdr);
                if (rh) {
                    EV_DETAIL << "Routing Header with type=" << rh->getRoutingType() << endl;
                    if (rh->getSegmentsLeft() == 0) {
                        EV_INFO << "Ignoring routing header with segmentsLeft=0" << endl;
                        break;
                    }
                    auto it = routingHeaderHandlers.find(rh->getRoutingType());
                    if (it != routingHeaderHandlers.end()) {
                        if (!it->second->processExtensionHeader(packet, rh.get()))
                            return false;
                    }
                    else {
                        throw cRuntimeError("No handler registered for routing header type %d", (int)rh->getRoutingType());
                    }
                }
                break;
            }
            case IP_PROT_IPv6EXT_FRAGMENT:
                // handled by localDeliver() reassembly before this function is called
                break;
            case IP_PROT_IPv6EXT_DEST: {
                auto destOptsHdr = dynamicPtrCast<const Ipv6DestinationOptionsHeader>(extHdr);
                if (destOptsHdr) {
                    const TlvOptions& opts = destOptsHdr->getTlvOptions();
                    for (size_t j = 0; j < opts.getTlvOptionArraySize(); j++) {
                        const TlvOptionBase *opt = opts.getTlvOption(j);
                        int optType = opt->getType();
                        if (optType == IPv6TLVOPTION_NOP1 || optType == IPv6TLVOPTION_NOPN)
                            continue;
                        auto it = destOptionHandlers.find(optType);
                        if (it != destOptionHandlers.end()) {
                            if (!it->second->processTlvOption(packet, destOptsHdr.get(), opt))
                                return false;
                        }
                        else if (!handleUnrecognizedTlvOption(packet, optType))
                            return false;
                    }
                }
                break;
            }
            default:
                EV_INFO << "Ignoring unknown extension header type " << nextHdr << endl;
                break;
        }
        nextHdr = extHdr->getNextHeaderProtocol();
        offset += extHdr->getChunkLength();
    }

    return true;
}

bool Ipv6::handleUnrecognizedTlvOption(Packet *packet, int optType)
{
    // RFC 8200 Section 4.2: the two highest-order bits of the option type encode
    // the action to take when the option is not recognized.
    switch ((optType >> 6) & 0x3) {
        case 0: // 00: skip over this option and continue processing the header
            EV_INFO << "Skipping unrecognized IPv6 option type " << optType << "\n";
            return true;
        case 1: // 01: discard the packet
            EV_INFO << "Discarding packet with unrecognized IPv6 option type " << optType << "\n";
            numDropped++;
            delete packet;
            return false;
        default: { // 10/11: discard and send an ICMPv6 Parameter Problem
            // For 11 the error is suppressed when the destination is a multicast address.
            bool dstMulticast = packet->peekAtFront<Ipv6Header>()->getDestAddress().isMulticast();
            if (((optType >> 6) & 0x3) == 3 && dstMulticast) {
                numDropped++;
                delete packet;
            }
            else
                sendIcmpError(packet, ICMPv6_PARAMETER_PROBLEM, UNRECOGNIZED_IPV6_OPTION);
            return false;
        }
    }
}

void Ipv6::registerRoutingHeaderHandler(int routingType, IIpv6ExtensionHeaderHandler *handler)
{
    if (routingHeaderHandlers.find(routingType) != routingHeaderHandlers.end())
        throw cRuntimeError("Routing header handler already registered for type %d", routingType);
    routingHeaderHandlers[routingType] = handler;
}

void Ipv6::registerHopByHopOptionHandler(int optionType, IIpv6TlvOptionHandler *handler)
{
    if (hopByHopOptionHandlers.find(optionType) != hopByHopOptionHandlers.end())
        throw cRuntimeError("Hop-by-Hop option handler already registered for type %d", optionType);
    hopByHopOptionHandlers[optionType] = handler;
}

void Ipv6::registerDestinationOptionHandler(int optionType, IIpv6TlvOptionHandler *handler)
{
    if (destOptionHandlers.find(optionType) != destOptionHandlers.end())
        throw cRuntimeError("Destination option handler already registered for type %d", optionType);
    destOptionHandlers[optionType] = handler;
}

// NetFilter:
void Ipv6::registerHook(int priority, INetfilter::IHook *hook)
{
    Enter_Method("registerHook()");
    NetfilterBase::registerHook(priority, hook);
}

void Ipv6::unregisterHook(INetfilter::IHook *hook)
{
    Enter_Method("unregisterHook()");
    NetfilterBase::unregisterHook(hook);
}

void Ipv6::dropQueuedDatagram(const Packet *packet)
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

void Ipv6::reinjectQueuedDatagram(const Packet *packet)
{
    Enter_Method("reinjectDatagram()");
    for (auto iter = queuedDatagramsForHooks.begin(); iter != queuedDatagramsForHooks.end(); iter++) {
        if (iter->packet == packet) {
            Packet *datagram = iter->packet;
            switch (iter->hookType) {
                case INetfilter::IHook::LOCALOUT:
                    datagramLocalOut(datagram, iter->outIE, iter->nextHopAddr);
                    break;

                case INetfilter::IHook::PREROUTING:
                    preroutingFinish(datagram, iter->inIE, iter->outIE, iter->nextHopAddr);
                    break;

                case INetfilter::IHook::POSTROUTING:
//                    fragmentAndSend(datagram, iter->outIE, iter->nextHopAddr);
                    throw cRuntimeError("Re-injection of datagram queued for POSTROUTING hook not implemented");
                    break;

                case INetfilter::IHook::LOCALIN:
//                    reassembleAndDeliverFinish(datagram);
                    throw cRuntimeError("Re-injection of datagram queued for LOCALIN hook not implemented");
                    break;

                case INetfilter::IHook::FORWARD:
                    throw cRuntimeError("Re-injection of datagram queued for FORWARD hook not implemented");
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

INetfilter::IHook::Result Ipv6::datagramPreRoutingHook(Packet *packet)
{
    for (auto& elem : hooks) {
        IHook::Result r = elem.second->datagramPreRoutingHook(packet);
        switch (r) {
            case INetfilter::IHook::ACCEPT:
                break; // continue iteration

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

INetfilter::IHook::Result Ipv6::datagramForwardHook(Packet *packet)
{
    for (auto& elem : hooks) {
        IHook::Result r = elem.second->datagramForwardHook(packet);
        switch (r) {
            case INetfilter::IHook::ACCEPT:
                break; // continue iteration

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

INetfilter::IHook::Result Ipv6::datagramPostRoutingHook(Packet *packet, const NetworkInterface *inIE, const NetworkInterface *& outIE, L3Address& nextHopAddr)
{
    for (auto& elem : hooks) {
        IHook::Result r = elem.second->datagramPostRoutingHook(packet);
        switch (r) {
            case INetfilter::IHook::ACCEPT:
                break; // continue iteration

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

INetfilter::IHook::Result Ipv6::datagramLocalInHook(Packet *packet, const NetworkInterface *inIE)
{
    for (auto& elem : hooks) {
        IHook::Result r = elem.second->datagramLocalInHook(packet);
        switch (r) {
            case INetfilter::IHook::ACCEPT:
                break; // continue iteration

            case INetfilter::IHook::DROP:
                delete packet;
                return r;

            case INetfilter::IHook::QUEUE:
                queuedDatagramsForHooks.push_back(QueuedDatagramForHook(packet, INetfilter::IHook::LOCALIN));
                return r;

            case INetfilter::IHook::STOLEN:
                return r;

            default:
                throw cRuntimeError("Unknown Hook::Result value: %d", (int)r);
        }
    }
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result Ipv6::datagramLocalOutHook(Packet *packet)
{
    for (auto& elem : hooks) {
        IHook::Result r = elem.second->datagramLocalOutHook(packet);
        switch (r) {
            case INetfilter::IHook::ACCEPT:
                break; // continue iteration

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

void Ipv6::sendIcmpError(Packet *packet, Icmpv6Type type, int code)
{
    icmp->sendErrorMessage(packet, type, code);
    delete packet;
}

// lifecycle management

void Ipv6::handleStartOperation(LifecycleOperation *operation)
{
    start();
}

void Ipv6::handleStopOperation(LifecycleOperation *operation)
{
    stop();
}

void Ipv6::handleCrashOperation(LifecycleOperation *operation)
{
    stop();
}

void Ipv6::start()
{
}

void Ipv6::stop()
{
    flush();
    for (auto it : socketIdToSocketDescriptor)
        delete it.second;
    socketIdToSocketDescriptor.clear();
}

void Ipv6::flush()
{
    EV_DEBUG << "Ipv6::flush(): packets in hooks: " << queuedDatagramsForHooks.size() << endl;
    for (auto& elem : queuedDatagramsForHooks)
        delete elem.packet;
    queuedDatagramsForHooks.clear();

    EV_DEBUG << "Ipv6::flush(): pending DAD queue: " << pendingDadQueue.size() << endl;
    for (auto *sDgram : pendingDadQueue)
        delete sDgram;
    pendingDadQueue.clear();

    fragbuf.flush();
}

} // namespace inet

