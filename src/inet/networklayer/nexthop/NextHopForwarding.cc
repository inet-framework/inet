//
// Copyright (C) 2012 Opensim Ltd.
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

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/common/NextHopAddressTag_m.h"
#include "inet/networklayer/contract/L3SocketCommand_m.h"
#include "inet/networklayer/nexthop/NextHopForwarding.h"
#include "inet/networklayer/nexthop/NextHopForwardingHeader_m.h"
#include "inet/networklayer/nexthop/NextHopInterfaceData.h"
#include "inet/networklayer/nexthop/NextHopRoute.h"
#include "inet/networklayer/nexthop/NextHopRoutingTable.h"

namespace inet {

Define_Module(NextHopForwarding);

NextHopForwarding::NextHopForwarding() :
        interfaceTable(nullptr),
        routingTable(nullptr),
        arp(nullptr),
        defaultHopLimit(-1),
        numLocalDeliver(0),
        numDropped(0),
        numUnroutable(0),
        numForwarded(0)
{
}

NextHopForwarding::~NextHopForwarding()
{
    for (auto it : socketIdToSocketDescriptor)
        delete it.second;
    for (auto & elem : queuedDatagramsForHooks) {
        delete elem.datagram;
    }
    queuedDatagramsForHooks.clear();
}

void NextHopForwarding::initialize(int stage)
{
    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        routingTable = getModuleFromPar<NextHopRoutingTable>(par("routingTableModule"), this);
        arp = getModuleFromPar<IArp>(par("arpModule"), this);

        defaultHopLimit = par("hopLimit");
        numLocalDeliver = numDropped = numUnroutable = numForwarded = 0;

        WATCH(numLocalDeliver);
        WATCH(numDropped);
        WATCH(numUnroutable);
        WATCH(numForwarded);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        registerService(Protocol::nextHopForwarding, gate("transportIn"), gate("transportOut"));
        registerProtocol(Protocol::nextHopForwarding, gate("queueOut"), gate("queueIn"));
    }
}

void NextHopForwarding::handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
}

void NextHopForwarding::handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    if (!strcmp("transportOut", gate->getBaseName()))
        upperProtocols.insert(&protocol);
}

void NextHopForwarding::refreshDisplay() const
{
    OperationalBase::refreshDisplay();

    char buf[80] = "";
    if (numForwarded > 0)
        sprintf(buf + strlen(buf), "fwd:%d ", numForwarded);
    if (numLocalDeliver > 0)
        sprintf(buf + strlen(buf), "up:%d ", numLocalDeliver);
    if (numDropped > 0)
        sprintf(buf + strlen(buf), "DROP:%d ", numDropped);
    if (numUnroutable > 0)
        sprintf(buf + strlen(buf), "UNROUTABLE:%d ", numUnroutable);
    getDisplayString().setTagArg("t", 0, buf);
}

void NextHopForwarding::handleMessageWhenUp(cMessage *msg)
{
    if (msg->arrivedOn("transportIn")) {    //TODO packet->getArrivalGate()->getBaseId() == transportInGateBaseId
        if (auto request = dynamic_cast<Request *>(msg))
            handleCommand(request);
        else
            handlePacketFromHL(check_and_cast<Packet*>(msg));
    }
    else if (msg->arrivedOn("queueIn")) {    // from network
        EV_INFO << "Received " << msg << " from network.\n";
        handlePacketFromNetwork(check_and_cast<Packet*>(msg));
    }
    else
        throw cRuntimeError("message arrived on unknown gate '%s'", msg->getArrivalGate()->getName());
}

void NextHopForwarding::handleCommand(Request *request)
{
    if (auto *command = dynamic_cast<L3SocketBindCommand *>(request->getControlInfo())) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        SocketDescriptor *descriptor = new SocketDescriptor(socketId, command->getProtocol()->getId(), command->getLocalAddress());
        socketIdToSocketDescriptor[socketId] = descriptor;
        delete request;
    }
    else if (auto *command = dynamic_cast<L3SocketConnectCommand *>(request->getControlInfo())) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        if (socketIdToSocketDescriptor.find(socketId) == socketIdToSocketDescriptor.end())
            throw cRuntimeError("L3Socket: should use bind() before connect()");
        socketIdToSocketDescriptor[socketId]->remoteAddress = command->getRemoteAddress();
        delete request;
    }
    else if (dynamic_cast<L3SocketCloseCommand *>(request->getControlInfo()) != nullptr) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        auto it = socketIdToSocketDescriptor.find(socketId);
        if (it != socketIdToSocketDescriptor.end()) {
            delete it->second;
            socketIdToSocketDescriptor.erase(it);
            auto indication = new Indication("closed", L3_I_SOCKET_CLOSED);
            auto ctrl = new L3SocketClosedIndication();
            indication->setControlInfo(ctrl);
            indication->addTag<SocketInd>()->setSocketId(socketId);
            send(indication, "transportOut");
        }
        delete request;
    }
    else if (dynamic_cast<L3SocketDestroyCommand *>(request->getControlInfo()) != nullptr) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        auto it = socketIdToSocketDescriptor.find(socketId);
        if (it != socketIdToSocketDescriptor.end()) {
            delete it->second;
            socketIdToSocketDescriptor.erase(it);
        }
        delete request;
    }
    else
        throw cRuntimeError("Invalid command: (%s)%s", request->getClassName(), request->getName());
}

const InterfaceEntry *NextHopForwarding::getSourceInterfaceFrom(Packet *packet)
{
    const auto& interfaceInd = packet->findTag<InterfaceInd>();
    return interfaceInd != nullptr ? interfaceTable->getInterfaceById(interfaceInd->getInterfaceId()) : nullptr;
}

void NextHopForwarding::handlePacketFromNetwork(Packet *packet)
{
    if (packet->hasBitError()) {
        //TODO emit packetDropped signal
        EV_WARN << "CRC error found, drop packet\n";
        PacketDropDetails details;
        details.setReason(INCORRECTLY_RECEIVED);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }

    const auto& header = packet->peekAtFront<NextHopForwardingHeader>();
    packet->addTagIfAbsent<NetworkProtocolInd>()->setProtocol(&Protocol::nextHopForwarding);
    packet->addTagIfAbsent<NetworkProtocolInd>()->setNetworkProtocolHeader(header);
    B totalLength = header->getChunkLength() + header->getPayloadLengthField();
    if (totalLength > packet->getDataLength()) {
        EV_WARN << "length error found, drop packet\n";
        PacketDropDetails details;
        details.setReason(INCORRECTLY_RECEIVED);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }

    // remove lower layer paddings:
    if (totalLength < packet->getDataLength()) {
        packet->setBackOffset(packet->getFrontOffset() + totalLength);
    }

    const InterfaceEntry *inIE = interfaceTable->getInterfaceById(packet->getTag<InterfaceInd>()->getInterfaceId());
    const InterfaceEntry *destIE = nullptr;

    EV_DETAIL << "Received datagram `" << packet->getName() << "' with dest=" << header->getDestAddr() << " from " << header->getSrcAddr() << " in interface" << inIE->getInterfaceName() << "\n";

    if (datagramPreRoutingHook(packet) != IHook::ACCEPT)
        return;

    inIE = interfaceTable->getInterfaceById(packet->getTag<InterfaceInd>()->getInterfaceId());
    const auto& destIeTag = packet->findTag<InterfaceReq>();
    destIE = destIeTag ? interfaceTable->getInterfaceById(destIeTag->getInterfaceId()) : nullptr;
    const auto& nextHopTag = packet->findTag<NextHopAddressReq>();
    L3Address nextHop = (nextHopTag) ? nextHopTag->getNextHopAddress() : L3Address();
    datagramPreRouting(packet, inIE, destIE, nextHop);
}

void NextHopForwarding::handlePacketFromHL(Packet *packet)
{
    // if no interface exists, do not send datagram
    if (interfaceTable->getNumInterfaces() == 0) {
        EV_INFO << "No interfaces exist, dropping packet\n";
        PacketDropDetails details;
        details.setReason(NO_INTERFACE_FOUND);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }

    // encapsulate and send
    const InterfaceEntry *destIE;    // will be filled in by encapsulate()
    encapsulate(packet, destIE);

    L3Address nextHop;
    if (datagramLocalOutHook(packet) != IHook::ACCEPT)
        return;

    const auto& destIeTag = packet->findTag<InterfaceReq>();
    destIE = destIeTag ? interfaceTable->getInterfaceById(destIeTag->getInterfaceId()) : nullptr;
    const auto& nextHopTag = packet->findTag<NextHopAddressReq>();
    nextHop = (nextHopTag) ? nextHopTag->getNextHopAddress() : L3Address();

    datagramLocalOut(packet, destIE, nextHop);
}

void NextHopForwarding::routePacket(Packet *datagram, const InterfaceEntry *destIE, const L3Address& requestedNextHop, bool fromHL)
{
    // TBD add option handling code here

    auto header = datagram->peekAtFront<NextHopForwardingHeader>();
    L3Address destAddr = header->getDestinationAddress();

    EV_INFO << "Routing datagram `" << datagram->getName() << "' with dest=" << destAddr << ": ";

    // check for local delivery
    if (routingTable->isLocalAddress(destAddr)) {
        EV_INFO << "local delivery\n";
        if (fromHL && header->getSourceAddress().isUnspecified()) {
            datagram->trimFront();
            const auto& newHeader = removeNetworkProtocolHeader<NextHopForwardingHeader>(datagram);
            newHeader->setSourceAddress(destAddr); // allows two apps on the same host to communicate
            insertNetworkProtocolHeader(datagram, Protocol::nextHopForwarding, newHeader);
            header = newHeader;
        }
        numLocalDeliver++;

        if (datagramLocalInHook(datagram) != INetfilter::IHook::ACCEPT)
            return;

        sendDatagramToHL(datagram);
        return;
    }

    // if datagram arrived from input gate and forwarding is off, delete datagram
    if (!fromHL && !routingTable->isForwardingEnabled()) {
        EV_INFO << "forwarding off, dropping packet\n";
        numDropped++;
        delete datagram;
        return;
    }

    if (!fromHL) {
        datagram->trim();
    }

    // if output port was explicitly requested, use that, otherwise use NextHopForwarding routing
    // TODO: see Ipv4, using destIE here leaves nextHope unspecified
    L3Address nextHop;
    if (destIE && !requestedNextHop.isUnspecified()) {
        EV_DETAIL << "using manually specified output interface " << destIE->getInterfaceName() << "\n";
        nextHop = requestedNextHop;
    }
    else {
        // use NextHopForwarding routing (lookup in routing table)
        const NextHopRoute *re = routingTable->findBestMatchingRoute(destAddr);

        // error handling: destination address does not exist in routing table:
        // throw packet away and continue
        if (re == nullptr) {
            EV_INFO << "unroutable, discarding packet\n";
            numUnroutable++;
            PacketDropDetails details;
            details.setReason(NO_ROUTE_FOUND);
            emit(packetDroppedSignal, datagram, &details);
            delete datagram;
            return;
        }

        // extract interface and next-hop address from routing table entry
        destIE = re->getInterface();
        nextHop = re->getNextHopAsGeneric();
    }

    if (!fromHL) {
        datagram->trimFront();
        const auto& newHeader = removeNetworkProtocolHeader<NextHopForwardingHeader>(datagram);
        newHeader->setHopLimit(header->getHopLimit() - 1);
        insertNetworkProtocolHeader(datagram, Protocol::nextHopForwarding, newHeader);
        header = newHeader;
    }

    // set datagram source address if not yet set
    if (header->getSourceAddress().isUnspecified()) {
        datagram->trimFront();
        const auto& newHeader = removeNetworkProtocolHeader<NextHopForwardingHeader>(datagram);
        newHeader->setSourceAddress(destIE->getProtocolData<NextHopInterfaceData>()->getAddress());
        insertNetworkProtocolHeader(datagram, Protocol::nextHopForwarding, newHeader);
        header = newHeader;
    }

    // default: send datagram to fragmentation
    EV_INFO << "output interface is " << destIE->getInterfaceName() << ", next-hop address: " << nextHop << "\n";
    numForwarded++;

    sendDatagramToOutput(datagram, destIE, nextHop);
}

void NextHopForwarding::routeMulticastPacket(Packet *datagram, const InterfaceEntry *destIE, const InterfaceEntry *fromIE)
{
    const auto& header = datagram->peekAtFront<NextHopForwardingHeader>();
    L3Address destAddr = header->getDestinationAddress();
    // if received from the network...
    if (fromIE != nullptr) {
        //TODO: decrement hopLimit before forward frame to another host

        // check for local delivery
        if (routingTable->isLocalMulticastAddress(destAddr))
            sendDatagramToHL(datagram);
//
//        // don't forward if NextHopForwarding forwarding is off
//        if (!rt->isForwardingEnabled())
//        {
//            delete datagram;
//            return;
//        }
//
//        // don't forward if dest address is link-scope
//        if (destAddr.isLinkLocalMulticast())
//        {
//            delete datagram;
//            return;
//        }
    }
    else {
        //TODO
        for (int i = 0; i < interfaceTable->getNumInterfaces(); ++i) {
            const InterfaceEntry *destIE = interfaceTable->getInterface(i);
            if (!destIE->isLoopback())
                sendDatagramToOutput(datagram->dup(), destIE, header->getDestinationAddress());
        }
        delete datagram;
    }

//    Address destAddr = datagram->getDestinationAddress();
//    EV << "Routing multicast datagram `" << datagram->getName() << "' with dest=" << destAddr << "\n";
//
//    numMulticast++;
//
//    // DVMRP: process datagram only if sent locally or arrived on the shortest
//    // route (provided routing table already contains srcAddr); otherwise
//    // discard and continue.
//    const InterfaceEntry *shortestPathIE = rt->getInterfaceForDestinationAddr(datagram->getSourceAddress());
//    if (fromIE!=nullptr && shortestPathIE!=nullptr && fromIE!=shortestPathIE)
//    {
//        // FIXME count dropped
//        EV << "Packet dropped.\n";
//        delete datagram;
//        return;
//    }
//
//    // if received from the network...
//    if (fromIE!=nullptr)
//    {
//        // check for local delivery
//        if (rt->isLocalMulticastAddress(destAddr))
//        {
//            NextHopDatagram *datagramCopy = datagram->dup();
//
//            // FIXME code from the MPLS model: set packet dest address to routerId (???)
//            datagramCopy->setDestinationAddress(rt->getRouterId());
//
//            reassembleAndDeliver(datagramCopy);
//        }
//
//        // don't forward if NextHopForwarding forwarding is off
//        if (!rt->isForwardingEnabled())
//        {
//            delete datagram;
//            return;
//        }
//
//        // don't forward if dest address is link-scope
//        if (destAddr.isLinkLocalMulticast())
//        {
//            delete datagram;
//            return;
//        }
//
//    }
//
//    // routed explicitly via multicast interface
//    if (destIE!=nullptr)
//    {
//        ASSERT(datagram->getDestinationAddress().isMulticast());
//
//        EV << "multicast packet explicitly routed via output interface " << destIE->getName() << endl;
//
//        // set datagram source address if not yet set
//        if (datagram->getSourceAddress().isUnspecified())
//            datagram->setSourceAddress(destIE->getProtocolData<Ipv4InterfaceData>()->getGenericAddress());
//
//        // send
//        sendDatagramToOutput(datagram, destIE, datagram->getDestinationAddress());
//
//        return;
//    }
//
//    // now: routing
//    MulticastRoutes routes = rt->getMulticastRoutesFor(destAddr);
//    if (routes.size()==0)
//    {
//        // no destination: delete datagram
//        delete datagram;
//    }
//    else
//    {
//        // copy original datagram for multiple destinations
//        for (unsigned int i=0; i<routes.size(); i++)
//        {
//            const InterfaceEntry *destIE = routes[i].interf;
//
//            // don't forward to input port
//            if (destIE && destIE!=fromIE)
//            {
//                NextHopDatagram *datagramCopy = datagram->dup();
//
//                // set datagram source address if not yet set
//                if (datagramCopy->getSourceAddress().isUnspecified())
//                    datagramCopy->setSourceAddress(destIE->getProtocolData<Ipv4InterfaceData>()->getGenericAddress());
//
//                // send
//                Address nextHop = routes[i].gateway;
//                sendDatagramToOutput(datagramCopy, destIE, nextHop);
//            }
//        }
//
//        // only copies sent, delete original datagram
//        delete datagram;
//    }
}

void NextHopForwarding::decapsulate(Packet *packet)
{
    // decapsulate transport packet
    const InterfaceEntry *fromIE = getSourceInterfaceFrom(packet);
    const auto& header = packet->popAtFront<NextHopForwardingHeader>();

    // create and fill in control info
    if (fromIE) {
        auto ifTag = packet->addTagIfAbsent<InterfaceInd>();
        ifTag->setInterfaceId(fromIE->getInterfaceId());
    }

    ASSERT(header->getPayloadLengthField() <= packet->getDataLength());
    // drop padding
    packet->setBackOffset(packet->getFrontOffset() + header->getPayloadLengthField());

    // attach control info
    auto payloadProtocol = header->getProtocol();
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(payloadProtocol);
    auto networkProtocolInd = packet->addTagIfAbsent<NetworkProtocolInd>();
    networkProtocolInd->setProtocol(&Protocol::nextHopForwarding);
    networkProtocolInd->setNetworkProtocolHeader(header);
    auto l3AddressInd = packet->addTagIfAbsent<L3AddressInd>();
    l3AddressInd->setSrcAddress(header->getSourceAddress());
    l3AddressInd->setDestAddress(header->getDestinationAddress());
    packet->addTagIfAbsent<HopLimitInd>()->setHopLimit(header->getHopLimit());
}

void NextHopForwarding::encapsulate(Packet *transportPacket, const InterfaceEntry *& destIE)
{
    auto header = makeShared<NextHopForwardingHeader>();
    header->setChunkLength(B(par("headerLength")));
    auto& l3AddressReq = transportPacket->removeTag<L3AddressReq>();
    L3Address src = l3AddressReq->getSrcAddress();
    L3Address dest = l3AddressReq->getDestAddress();

    header->setProtocol(transportPacket->getTag<PacketProtocolTag>()->getProtocol());

    auto& hopLimitReq = transportPacket->removeTagIfPresent<HopLimitReq>();
    short ttl = (hopLimitReq != nullptr) ? hopLimitReq->getHopLimit() : -1;

    // set source and destination address
    header->setDestinationAddress(dest);

    // multicast interface option, but allow interface selection for unicast packets as well
    const auto& ifTag = transportPacket->findTag<InterfaceReq>();
    destIE = ifTag ? interfaceTable->getInterfaceById(ifTag->getInterfaceId()) : nullptr;


    // when source address was given, use it; otherwise it'll get the address
    // of the outgoing interface after routing
    if (!src.isUnspecified()) {
        // if interface parameter does not match existing interface, do not send datagram
        if (routingTable->getInterfaceByAddress(src) == nullptr)
            throw cRuntimeError("Wrong source address %s in (%s)%s: no interface with such address",
                    src.str().c_str(), transportPacket->getClassName(), transportPacket->getFullName());
        header->setSourceAddress(src);
    }

    // set other fields
    if (ttl != -1) {
        ASSERT(ttl > 0);
    }
    else if (false) //TODO: datagram->getDestinationAddress().isLinkLocalMulticast())
        ttl = 1;
    else
        ttl = defaultHopLimit;

    header->setHopLimit(ttl);

    // setting NextHopForwarding options is currently not supported

    delete transportPacket->removeControlInfo();
    header->setPayloadLengthField(transportPacket->getDataLength());

    insertNetworkProtocolHeader(transportPacket, Protocol::nextHopForwarding, header);
}

void NextHopForwarding::sendDatagramToHL(Packet *packet)
{
    const auto& header = packet->peekAtFront<NextHopForwardingHeader>();
    const Protocol *protocol = header->getProtocol();
    decapsulate(packet);
    // deliver to sockets
    auto localAddress(header->getDestAddr());
    auto remoteAddress(header->getSrcAddr());
    bool hasSocket = false;
    for (const auto &elem: socketIdToSocketDescriptor) {
        if (elem.second->protocolId == protocol->getId()
                && (elem.second->localAddress.isUnspecified() || elem.second->localAddress == localAddress)
                && (elem.second->remoteAddress.isUnspecified() || elem.second->remoteAddress == remoteAddress)) {
            auto *packetCopy = packet->dup();
            packetCopy->setKind(L3_I_DATA);
            packetCopy->addTagIfAbsent<SocketInd>()->setSocketId(elem.second->socketId);
            EV_INFO << "Passing up to socket " << elem.second->socketId << "\n";
            emit(packetSentToUpperSignal, packetCopy);
            send(packetCopy, "transportOut");
            hasSocket = true;
        }
    }
    if (upperProtocols.find(protocol) != upperProtocols.end()) {
        EV_INFO << "Passing up to protocol " << *protocol << "\n";
        emit(packetSentToUpperSignal, packet);
        send(packet, "transportOut");
        numLocalDeliver++;
    }
    else {
        if (!hasSocket) {
            EV_ERROR << "Transport protocol '" << protocol->getName() << "' not connected, discarding packet\n";
            //TODO send an ICMP error: protocol unreachable
            // sendToIcmp(datagram, inputInterfaceId, ICMP_DESTINATION_UNREACHABLE, ICMP_DU_PROTOCOL_UNREACHABLE);
        }
        delete packet;
    }
}

void NextHopForwarding::sendDatagramToOutput(Packet *datagram, const InterfaceEntry *ie, L3Address nextHop)
{
    delete datagram->removeControlInfo();

    if (datagram->getByteLength() > ie->getMtu())
        throw cRuntimeError("datagram too large"); //TODO refine

    const auto& header = datagram->peekAtFront<NextHopForwardingHeader>();
    // hop counter check
    if (header->getHopLimit() <= 0) {
        EV_INFO << "datagram hopLimit reached zero, discarding\n";
        delete datagram;    //TODO stats counter???
        return;
    }

    if (!ie->isBroadcast()) {
        EV_DETAIL << "output interface " << ie->getInterfaceName() << " is not broadcast, skipping ARP\n";
        //Peer to peer interface, no broadcast, no MACAddress. // packet->addTagIfAbsent<MACAddressReq>()->setDestinationAddress(macAddress);
        datagram->removeTagIfPresent<DispatchProtocolReq>();
        datagram->addTagIfAbsent<InterfaceReq>()->setInterfaceId(ie->getInterfaceId());
        datagram->addTagIfAbsent<DispatchProtocolInd>()->setProtocol(&Protocol::nextHopForwarding);
        datagram->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::nextHopForwarding);
        send(datagram, "queueOut");
        return;
    }

    // determine what address to look up in ARP cache
    if (nextHop.isUnspecified()) {
        nextHop = header->getDestinationAddress();
        EV_WARN << "no next-hop address, using destination address " << nextHop << " (proxy ARP)\n";
    }

    // send out datagram to NIC, with control info attached
    MacAddress nextHopMAC = arp->resolveL3Address(nextHop, ie);
    if (nextHopMAC == MacAddress::UNSPECIFIED_ADDRESS) {
        throw cRuntimeError("ARP couldn't resolve the '%s' address", nextHop.str().c_str());
    }
    else {
        // add control info with MAC address
        datagram->addTagIfAbsent<MacAddressReq>()->setDestAddress(nextHopMAC);
        datagram->removeTagIfPresent<DispatchProtocolReq>();
        datagram->addTagIfAbsent<InterfaceReq>()->setInterfaceId(ie->getInterfaceId());
        datagram->addTagIfAbsent<DispatchProtocolInd>()->setProtocol(&Protocol::nextHopForwarding);
        datagram->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::nextHopForwarding);

        // send out
        send(datagram, "queueOut");
    }
}

void NextHopForwarding::datagramPreRouting(Packet *datagram, const InterfaceEntry *inIE, const InterfaceEntry *destIE, const L3Address& nextHop)
{
    const auto& header = datagram->peekAtFront<NextHopForwardingHeader>();
    // route packet
    if (!header->getDestinationAddress().isMulticast())
        routePacket(datagram, destIE, nextHop, false);
    else
        routeMulticastPacket(datagram, destIE, inIE);
}

void NextHopForwarding::datagramLocalIn(Packet *packet, const InterfaceEntry *inIE)
{
    sendDatagramToHL(packet);
}

void NextHopForwarding::datagramLocalOut(Packet *datagram, const InterfaceEntry *destIE, const L3Address& nextHop)
{
    const auto& header = datagram->peekAtFront<NextHopForwardingHeader>();
    // route packet
    if (!header->getDestinationAddress().isMulticast())
        routePacket(datagram, destIE, nextHop, true);
    else
        routeMulticastPacket(datagram, destIE, nullptr);
}

void NextHopForwarding::registerHook(int priority, IHook *hook)
{
    Enter_Method("registerHook()");
    NetfilterBase::registerHook(priority, hook);
}

void NextHopForwarding::unregisterHook(IHook *hook)
{
    Enter_Method("unregisterHook()");
    NetfilterBase::unregisterHook(hook);
}

void NextHopForwarding::dropQueuedDatagram(const Packet *datagram)
{
    Enter_Method("dropDatagram()");
    for (auto iter = queuedDatagramsForHooks.begin(); iter != queuedDatagramsForHooks.end(); iter++) {
        if (iter->datagram == datagram) {
            delete datagram;
            queuedDatagramsForHooks.erase(iter);
            return;
        }
    }
}

void NextHopForwarding::reinjectQueuedDatagram(const Packet *datagram)
{
    Enter_Method("reinjectDatagram()");
    for (auto iter = queuedDatagramsForHooks.begin(); iter != queuedDatagramsForHooks.end(); iter++) {
        if (iter->datagram == datagram) {
            Packet *datagram = iter->datagram;
            const InterfaceEntry *inIE = iter->inIE;
            const InterfaceEntry *outIE = iter->outIE;
            const L3Address& nextHop = iter->nextHop;
            INetfilter::IHook::Type hookType = iter->hookType;
            switch (hookType) {
                case INetfilter::IHook::PREROUTING:
                    datagramPreRouting(datagram, inIE, outIE, nextHop);
                    break;

                case INetfilter::IHook::LOCALIN:
                    datagramLocalIn(datagram, inIE);
                    break;

                case INetfilter::IHook::LOCALOUT:
                    datagramLocalOut(datagram, outIE, nextHop);
                    break;

                default:
                    throw cRuntimeError("Re-injection of datagram queued for this hook not implemented");
                    break;
            }
            queuedDatagramsForHooks.erase(iter);
            return;
        }
    }
}

INetfilter::IHook::Result NextHopForwarding::datagramPreRoutingHook(Packet *datagram)
{
    for (auto & elem : hooks) {
        IHook::Result r = elem.second->datagramPreRoutingHook(datagram);
        switch (r) {
            case IHook::ACCEPT:
                break;    // continue iteration

            case IHook::DROP:
                delete datagram;
                return r;

            case IHook::QUEUE:
                if (datagram->getOwner() != this)
                    throw cRuntimeError("Model error: netfilter hook changed the owner of queued datagram '%s'", datagram->getFullName());
                queuedDatagramsForHooks.push_back(QueuedDatagramForHook(datagram, INetfilter::IHook::PREROUTING));
                return r;

            case IHook::STOLEN:
                return r;

            default:
                throw cRuntimeError("Unknown Hook::Result value: %d", (int)r);
        }
    }
    return IHook::ACCEPT;
}

INetfilter::IHook::Result NextHopForwarding::datagramForwardHook(Packet *datagram)
{
    for (auto & elem : hooks) {
        IHook::Result r = elem.second->datagramForwardHook(datagram);
        switch (r) {
            case IHook::ACCEPT:
                break;    // continue iteration

            case IHook::DROP:
                delete datagram;
                return r;

            case IHook::QUEUE:
                queuedDatagramsForHooks.push_back(QueuedDatagramForHook(datagram, INetfilter::IHook::FORWARD));
                return r;

            case IHook::STOLEN:
                return r;

            default:
                throw cRuntimeError("Unknown Hook::Result value: %d", (int)r);
        }
    }
    return IHook::ACCEPT;
}

INetfilter::IHook::Result NextHopForwarding::datagramPostRoutingHook(Packet *datagram)
{
    for (auto & elem : hooks) {
        IHook::Result r = elem.second->datagramPostRoutingHook(datagram);
        switch (r) {
            case IHook::ACCEPT:
                break;    // continue iteration

            case IHook::DROP:
                delete datagram;
                return r;

            case IHook::QUEUE:
                queuedDatagramsForHooks.push_back(QueuedDatagramForHook(datagram, INetfilter::IHook::POSTROUTING));
                return r;

            case IHook::STOLEN:
                return r;

            default:
                throw cRuntimeError("Unknown Hook::Result value: %d", (int)r);
        }
    }
    return IHook::ACCEPT;
}

INetfilter::IHook::Result NextHopForwarding::datagramLocalInHook(Packet *datagram)
{
    L3Address address;
    for (auto & elem : hooks) {
        IHook::Result r = elem.second->datagramLocalInHook(datagram);
        switch (r) {
            case IHook::ACCEPT:
                break;    // continue iteration

            case IHook::DROP:
                delete datagram;
                return r;

            case IHook::QUEUE:
                queuedDatagramsForHooks.push_back(QueuedDatagramForHook(datagram, INetfilter::IHook::LOCALIN));
                return r;

            case IHook::STOLEN:
                return r;

            default:
                throw cRuntimeError("Unknown Hook::Result value: %d", (int)r);
        }
    }
    return IHook::ACCEPT;
}

INetfilter::IHook::Result NextHopForwarding::datagramLocalOutHook(Packet *datagram)
{
    for (auto & elem : hooks) {
        IHook::Result r = elem.second->datagramLocalOutHook(datagram);
        switch (r) {
            case IHook::ACCEPT:
                break;    // continue iteration

            case IHook::DROP:
                delete datagram;
                return r;

            case IHook::QUEUE:
                queuedDatagramsForHooks.push_back(QueuedDatagramForHook(datagram, INetfilter::IHook::LOCALOUT));
                return r;

            case IHook::STOLEN:
                return r;

            default:
                throw cRuntimeError("Unknown Hook::Result value: %d", (int)r);
        }
    }
    return IHook::ACCEPT;
}

void NextHopForwarding::handleStartOperation(LifecycleOperation *operation)
{
    start();
}

void NextHopForwarding::handleStopOperation(LifecycleOperation *operation)
{
    stop();
}

void NextHopForwarding::handleCrashOperation(LifecycleOperation *operation)
{
    stop();
}

void NextHopForwarding::start()
{
}

void NextHopForwarding::stop()
{
    flush();
    for (auto it : socketIdToSocketDescriptor)
        delete it.second;
    socketIdToSocketDescriptor.clear();
}

void NextHopForwarding::flush()
{
    EV_DEBUG << "NextHopForwarding::flush(): packets in hooks: " << queuedDatagramsForHooks.size() << endl;
    for (auto & elem : queuedDatagramsForHooks) {
        delete elem.datagram;
    }
    queuedDatagramsForHooks.clear();

//    fragbuf.flush();
}

} // namespace inet

