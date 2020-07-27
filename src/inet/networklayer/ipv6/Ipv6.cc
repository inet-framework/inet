//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2005 Wei Yang, Ng
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

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/INETDefs.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Message.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/networklayer/common/EcnTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/common/TosTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv6/Ipv6SocketCommand_m.h"
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#include "inet/networklayer/icmpv6/Ipv6NdMessage_m.h"
#include "inet/networklayer/ipv6/Ipv6.h"
#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders_m.h"
#include "inet/networklayer/ipv6/Ipv6ExtHeaderTag_m.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"

#ifdef WITH_xMIPv6
#include "inet/networklayer/xmipv6/MobilityHeader_m.h"
#endif /* WITH_xMIPv6 */


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
}

#ifdef WITH_xMIPv6
Ipv6::ScheduledDatagram::ScheduledDatagram(Packet *packet, const Ipv6Header *ipv6Header, const InterfaceEntry *ie, MacAddress macAddr, bool fromHL) :
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
#endif /* WITH_xMIPv6 */

void Ipv6::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        rt = getModuleFromPar<Ipv6RoutingTable>(par("routingTableModule"), this);
        nd = getModuleFromPar<Ipv6NeighbourDiscovery>(par("ipv6NeighbourDiscoveryModule"), this);
        icmp = getModuleFromPar<Icmpv6>(par("icmpv6Module"), this);
        tunneling = getModuleFromPar<Ipv6Tunneling>(par("ipv6TunnelingModule"), this);

        curFragmentId = 0;
        lastCheckTime = SIMTIME_ZERO;
        fragbuf.init(icmp);

        // NetFilter:
        hooks.clear();
        queuedDatagramsForHooks.clear();

        numMulticast = numLocalDeliver = numDropped = numUnroutable = numForwarded = 0;

        WATCH(numMulticast);
        WATCH(numLocalDeliver);
        WATCH(numDropped);
        WATCH(numUnroutable);
        WATCH(numForwarded);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        cModule *node = findContainingNode(this);
        NodeStatus *nodeStatus = node ? check_and_cast_nullable<NodeStatus *>(node->getSubmodule("status")) : nullptr;
        bool isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");

        registerService(Protocol::ipv6, gate("transportIn"), gate("queueIn"));
        registerProtocol(Protocol::ipv6, gate("queueOut"), gate("transportOut"));
    }
}

void Ipv6::handleRegisterService(const Protocol& protocol, cGate *out, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
}

void Ipv6::handleRegisterProtocol(const Protocol& protocol, cGate *in, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    if (in->isName("transportIn"))
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
        if (socketIdToSocketDescriptor.find(socketId) == socketIdToSocketDescriptor.end())
            throw cRuntimeError("Ipv6Socket: should use bind() before connect()");
        socketIdToSocketDescriptor[socketId]->remoteAddress = command->getRemoteAddress();
        delete request;
    }
    else if (dynamic_cast<Ipv6SocketCloseCommand *>(ctrl) != nullptr) {
        int socketId = 0; request->getTag<SocketReq>()->getSocketId();
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
        int socketId = 0; request->getTag<SocketReq>()->getSocketId();
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

void Ipv6::refreshDisplay() const
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

void Ipv6::handleMessage(cMessage *msg)
{
    auto& tags = getTags(msg);

#ifdef WITH_xMIPv6
    // 28.09.07 - CB
    // support for rescheduling datagrams which are supposed to be sent over
    // a tentative address.
    if (msg->isSelfMessage()) {
        ScheduledDatagram *sDgram = check_and_cast<ScheduledDatagram *>(msg);

        // take care of datagram which was supposed to be sent over a tentative address
        if (sDgram->getIE()->getProtocolData<Ipv6InterfaceData>()->isTentativeAddress(sDgram->getSrcAddress())) {
            // address is still tentative - enqueue again
            //queue.insert(sDgram);
            scheduleAfter(1.0, sDgram);    //FIXME KLUDGE wait 1s for tentative->permanent. MISSING: timeout for drop or send back icmpv6 error, processing signals from IE, need another msg queue for waiting (similar to Ipv4 ARP)
        }
        else {
            // address is not tentative anymore - send out datagram
            numForwarded++;
            fragmentPostRouting(sDgram->removeDatagram(), sDgram->getIE(), sDgram->getMacAddress(), sDgram->getFromHL());
            delete sDgram;
        }
    }
    else
#endif /* WITH_xMIPv6 */

    if (auto request = dynamic_cast<Request *>(msg))
        handleRequest(request);
    else
    if (msg->getArrivalGate()->isName("transportIn")
        || (msg->arrivedOn("ndIn") && tags.getTag<PacketProtocolTag>()->getProtocol() == &Protocol::icmpv6)
        || (msg->arrivedOn("upperTunnelingIn"))    // for tunneling support-CB
#ifdef WITH_xMIPv6
        || (msg->arrivedOn("xMIPv6In") && tags.getTag<PacketProtocolTag>()->getProtocol() == &Protocol::mobileipv6)
#endif /* WITH_xMIPv6 */
        )
    {
        // packet from upper layers, tunnel link-layer output or ND: encapsulate and send out
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

        // Do not handle header biterrors, because
        // 1. Ipv6 header does not contain checksum for the header fields, each field is
        //    validated when they are processed.
        // 2. The Ethernet or PPP frame is dropped by the link-layer if there is a transmission error.
        ASSERT(!packet->hasBitError());

        const InterfaceEntry *fromIE = getSourceInterfaceFrom(packet);
        const InterfaceEntry *destIE = nullptr;
        L3Address nextHop(Ipv6Address::UNSPECIFIED_ADDRESS);
        if (fromHL) {
            // remove control info
            delete packet->removeControlInfo();
            if (datagramLocalOutHook(packet) == INetfilter::IHook::ACCEPT)
                datagramLocalOut(packet, destIE, nextHop.toIpv6());
        }
        else {
            if (datagramPreRoutingHook(packet) == INetfilter::IHook::ACCEPT)
                preroutingFinish(packet, fromIE, destIE, nextHop.toIpv6());
        }
    }
}

InterfaceEntry *Ipv6::getSourceInterfaceFrom(Packet *packet)
{
    auto interfaceInd = packet->findTag<InterfaceInd>();
    return interfaceInd != nullptr ? ift->getInterfaceById(interfaceInd->getInterfaceId()) : nullptr;
}

void Ipv6::preroutingFinish(Packet *packet, const InterfaceEntry *fromIE, const InterfaceEntry *destIE, Ipv6Address nextHopAddr)
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

void Ipv6::handleMessageFromHL(Packet *msg)
{
    // if no interface exists, do not send datagram
    if (ift->getNumInterfaces() == 0) {
        EV_WARN << "No interfaces exist, dropping packet\n";
        delete msg;
        return;
    }

    auto ifTag = msg->findTag<InterfaceReq>();
    const InterfaceEntry *destIE = ifTag ? ift->getInterfaceById(ifTag->getInterfaceId()) : nullptr;
    auto packet = check_and_cast<Packet *>(msg);

    // when source address was given, use it; otherwise it'll get the address
    // of the outgoing interface after routing
    Ipv6Address src = packet->getTag<L3AddressReq>()->getSrcAddress().toIpv6();
    if (!src.isUnspecified()) {
        // if interface parameter does not match existing interface, do not send datagram
        if (rt->getInterfaceByAddress(src) == nullptr) {
#ifdef WITH_xMIPv6
            EV_WARN << "Encapsulation failed - dropping packet." << endl;
            PacketDropDetails details;
            details.setReason(NO_INTERFACE_FOUND);
            emit(packetDroppedSignal, packet, &details);
            delete packet;
            return;
#else /* WITH_xMIPv6 */
            throw cRuntimeError("Wrong source address %s in (%s)%s: no interface with such address",
                    src.str().c_str(), packet->getClassName(), packet->getFullName());
#endif /* WITH_xMIPv6 */
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
    L3Address nextHopAddr(Ipv6Address::UNSPECIFIED_ADDRESS);
    if (datagramLocalOutHook(packet) == INetfilter::IHook::ACCEPT)
        datagramLocalOut(packet, destIE, nextHopAddr.toIpv6());
}

void Ipv6::datagramLocalOut(Packet *packet, const InterfaceEntry *destIE, Ipv6Address requestedNextHopAddress)
{
    const auto& ipv6Header = packet->peekAtFront<Ipv6Header>();
    // route packet
    if (destIE != nullptr)
        fragmentPostRouting(packet, destIE, MacAddress::BROADCAST_ADDRESS, true); // FIXME what MAC address to use?
    else if (!ipv6Header->getDestAddress().isMulticast())
        routePacket(packet, destIE, nullptr, requestedNextHopAddress, true);
    else
        routeMulticastPacket(packet, destIE, nullptr, true);
}

void Ipv6::routePacket(Packet *packet, const InterfaceEntry *destIE, const InterfaceEntry *fromIE, Ipv6Address requestedNextHopAddress, bool fromHL)
{
    auto ipv6Header = packet->peekAtFront<Ipv6Header>();
    // TBD add option handling code here
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
        //yes but datagrams from the ND module is getting dropped too!-WEI
        //so we add a 2nd condition
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
        // TBD: in Ipv4, arrange TTL check like this
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

#ifdef WITH_xMIPv6
    // tunneling support - CB
    // check if destination is covered by tunnel lists
    if ((ipv6Header->getProtocolId() != IP_PROT_IPv6) &&    // if datagram was already tunneled, don't tunnel again
        (ipv6Header->getExtensionHeaderArraySize() == 0) &&    // we do not already have extension headers - FIXME: check for RH2 existence
        ((rt->isMobileNode() && rt->isHomeAddress(ipv6Header->getSrcAddress())) ||    // for MNs: only if source address is a HoA // 27.08.07 - CB
         rt->isHomeAgent() ||    // but always check for tunnel if node is a HA
         !rt->isMobileNode()    // or if it is a correspondent or non-MIP node
        )
        )
    {
        if (ipv6Header->getProtocolId() == IP_PROT_IPv6EXT_MOB)
            // in case of mobility header we can only search for "real" tunnels
            // as T2RH or HoA Opt. are not allowed with these messages
            interfaceId = tunneling->getVIfIndexForDest(destAddress, Ipv6Tunneling::NORMAL); // 10.06.08 - CB
        //getVIfIndexForDestForXSplitTunnel(destAddress);
        else
            // otherwise we can search for everything
            interfaceId = tunneling->getVIfIndexForDest(destAddress);
    }
#else // ifdef WITH_xMIPv6
      // FIXME this is not the same as the code above (when WITH_xMIPv6 is defined),
      // so tunneling examples could not work with xMIPv6
    interfaceId = tunneling->getVIfIndexForDest(destAddress, Ipv6Tunneling::NORMAL);
#endif /* WITH_xMIPv6 */

    if (interfaceId == -1 && destIE != nullptr)
        interfaceId = destIE->getInterfaceId();         // set interfaceId to destIE when not tunneling

    if (interfaceId > ift->getBiggestInterfaceId()) {
        // a virtual tunnel interface provides a path to the destination: do tunneling
        EV_INFO << "tunneling: src addr=" << ipv6Header->getSrcAddress() << ", dest addr=" << destAddress << std::endl;
        send(packet, "lowerTunnelingOut");
        return;
    }

    if (interfaceId == -1)
        if (!determineOutputInterface(destAddress, nextHop, interfaceId, packet, fromHL))
            // no interface found; sent to ND or to ICMP for error processing
            //throw cRuntimeError("No interface found!");//return;
            return;
    // don't raise error if sent to ND or ICMP!

    resolveMACAddressAndSendPacket(packet, interfaceId, nextHop, fromHL);
}

void Ipv6::resolveMACAddressAndSendPacket(Packet *packet, int interfaceId, Ipv6Address nextHop, bool fromHL)
{
    const auto& ipv6Header = packet->peekAtFront<Ipv6Header>();
    InterfaceEntry *ie = ift->getInterfaceById(interfaceId);
    ASSERT(ie != nullptr);
    ASSERT(!nextHop.isUnspecified());
    Ipv6Address destAddress = ipv6Header->getDestAddress();
    EV_INFO << "next hop for " << destAddress << " is " << nextHop << ", interface " << ie->getInterfaceName() << "\n";

#ifdef WITH_xMIPv6
    if (rt->isMobileNode()) {
        // if the source address is the HoA and we have a CoA then drop the packet
        // (address is topologically incorrect!)
        if (ipv6Header->getSrcAddress() == ie->getProtocolData<Ipv6InterfaceData>()->getMNHomeAddress()
            && !ie->getProtocolData<Ipv6InterfaceData>()->getGlobalAddress(Ipv6InterfaceData::CoA).isUnspecified())
        {
            EV_WARN << "Using HoA instead of CoA... dropping datagram" << endl;
            delete packet;
            numDropped++;
            return;
        }
    }
#endif /* WITH_xMIPv6 */

    MacAddress macAddr = nd->resolveNeighbour(nextHop, interfaceId);    // might initiate NUD
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

void Ipv6::routeMulticastPacket(Packet *packet, const InterfaceEntry *destIE, const InterfaceEntry *fromIE, bool fromHL)
{
    auto ipv6Header = packet->peekAtFront<Ipv6Header>();
    const Ipv6Address& destAddr = ipv6Header->getDestAddress();

    EV_INFO << "destination address " << destAddr << " is multicast, doing multicast routing\n";
    numMulticast++;

    // if received from the network...
    if (fromIE != nullptr) {
        ASSERT(!fromHL);
        // deliver locally
        if (rt->isLocalAddress(destAddr)) {
            EV_INFO << "local delivery of multicast packet\n";
            numLocalDeliver++;
            localDeliver(packet->dup(), fromIE);
        }

        // if ipv6Header arrived from input gate and IP forwarding is off, delete datagram
        if (!rt->isRouter()) {
            EV_INFO << "forwarding is off\n";
            delete packet;
            return;
        }

        // make sure scope of multicast address is large enough to be forwarded to other links
        if (destAddr.getMulticastScope() <= 2) {
            EV_INFO << "multicast dest address is link-local (or smaller) scope\n";
            delete packet;
            return;
        }

        // hop counter decrement: only if datagram arrived from network, and will be
        // sent out to the network (hoplimit check will be done just before sending
        // out datagram)
        packet->trim();
        ipv6Header = nullptr;
        const auto& newIpv6Header = removeNetworkProtocolHeader<Ipv6Header>(packet);
        newIpv6Header->setHopLimit(ipv6Header->getHopLimit() - 1);
        insertNetworkProtocolHeader(packet, Protocol::ipv6, newIpv6Header);
        ipv6Header = newIpv6Header;
    }

    // for now, we just send it out on every interface except on which it came. FIXME better!!!
    EV_INFO << "sending out datagram on every interface (except incoming one)\n";
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        InterfaceEntry *ie = ift->getInterface(i);
        if (fromIE != ie && !ie->isLoopback())
            fragmentPostRouting(packet->dup(), ie, MacAddress::BROADCAST_ADDRESS, fromHL);
    }
    delete packet;

/* FIXME implement handling of multicast

    According to Gopi: "multicast routing table" should map
       srcAddr+multicastDestAddr to a set of next hops (interface+nexthopAddr)
    Where srcAddr is the multicast server, and destAddr sort of narrows it down to a given stream

    // FIXME multicast-->tunneling link (present in original IPSuite) missing from here

    // DVMRP: process datagram only if sent locally or arrived on the shortest
    // route (provided routing table already contains srcAddr); otherwise
    // discard and continue.
    int inputGateIndex = datagram->getArrivalGate() ? datagram->getArrivalGate()->getIndex() : -1;
    int shortestPathInputGateIndex = rt->outputGateIndexNo(datagram->getSrcAddress());
    if (inputGateIndex!=-1 && shortestPathInputGateIndex!=-1 && inputGateIndex!=shortestPathInputGateIndex)
    {
        // FIXME count dropped
        EV << "Packet dropped.\n";
        delete datagram;
        return;
    }

    // check for local delivery
    Ipv6Address destAddress = datagram->getDestAddress();
    if (rt->isLocalMulticastAddress(destAddress))
    {
        IPv6Datagram *datagramCopy = datagram->dup();

        // FIXME code from the Mpls model: set packet dest address to routerId (???)
        datagramCopy->setDestAddress(rt->getRouterId());

        localDeliver(datagramCopy, fromIE);
    }

    // forward datagram only if IP forward is enabled, or sent locally
    if (inputGateIndex!=-1 && !rt->isRouter())
    {
        delete datagram;
        return;
    }

    MulticastRoutes routes = rt->getMulticastRoutesFor(destAddress);
    if (routes.size()==0)
    {
        // no destination: delete datagram
        delete datagram;
    }
    else
    {
        // copy original datagram for multiple destinations
        for (unsigned int i=0; i<routes.size(); i++)
        {
            int outputGateIndex = routes[i].interf->outputGateIndex();

            // don't forward to input port
            if (outputGateIndex>=0 && outputGateIndex!=inputGateIndex)
            {
                IPv6Datagram *datagramCopy = datagram->dup();

                // set datagram source address if not yet set
                if (datagramCopy->getSrcAddress().isUnspecified())
                    datagramCopy->setSrcAddress(ift->interfaceByPortNo(outputGateIndex)->getProtocolData<Ipv6InterfaceData>()->getIPAddress());

                // send
                Ipv6Address nextHopAddr = routes[i].gateway;
                fragmentPostRouting(datagramCopy, outputGateIndex, macAddr, fromHL);
            }
        }

        // only copies sent, delete original datagram
        delete datagram;
    }
 */
}

void Ipv6::localDeliver(Packet *packet, const InterfaceEntry *fromIE)
{
    const auto& ipv6Header = packet->peekAtFront<Ipv6Header>();

    // Defragmentation. skip defragmentation if datagram is not fragmented
    const Ipv6FragmentHeader *fh = dynamic_cast<const Ipv6FragmentHeader *>(ipv6Header->findExtensionHeaderByType(IP_PROT_IPv6EXT_FRAGMENT));
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
    }

#ifdef WITH_xMIPv6
    // #### 29.08.07 - CB
    // check for extension headers
    if (!processExtensionHeaders(packet, ipv6Header.get())) {
        // ext. header processing not yet finished
        // datagram was sent to another module or dropped
        // -> interrupt local delivery process
        return;
    }
    // #### end CB
#endif /* WITH_xMIPv6 */

    auto origPacket = packet->dup();
    const Protocol *protocol = ipv6Header->getProtocol();
    auto remoteAddress(ipv6Header->getSrcAddress());
    auto localAddress(ipv6Header->getDestAddress());
    decapsulate(packet);
    bool hasSocket = false;
    for (const auto &elem: socketIdToSocketDescriptor) {
        if (elem.second->protocolId == protocol->getId()
                && (elem.second->localAddress.isUnspecified() || elem.second->localAddress == localAddress)
                && (elem.second->remoteAddress.isUnspecified() || elem.second->remoteAddress == remoteAddress)) {
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
    }    //Added by WEI to forward ICMPv6 msgs to ICMPv6 module.
#ifdef WITH_xMIPv6
    else if (protocol == &Protocol::mobileipv6) {
        // added check for MIPv6 support to prevent nodes w/o the
        // xMIP module from processing related messages, 4.9.07 - CB
        if (rt->hasMipv6Support()) {
            EV_INFO << "MIPv6 packet: passing it to xMIPv6 module\n";
            send(packet, "xMIPv6Out");
        }
        else {
            // update 12.9.07 - CB
            /*RFC3775, 11.3.5
               Any node that does not recognize the Mobility header will return an
               ICMP Parameter Problem, Code 1, message to the sender of the packet*/
            EV_INFO << "No MIPv6 support on this node!\n";
            sendIcmpError(packet, ICMPv6_PARAMETER_PROBLEM, UNRECOGNIZED_NEXT_HDR_TYPE);
        }
    }
#endif /* WITH_xMIPv6 */
    else if (protocol == &Protocol::ipv4 || protocol == &Protocol::ipv6) {
        EV_INFO << "Tunnelled IP datagram\n";
        send(packet, "upperTunnelingOut");
    }
    else if (upperProtocols.find(protocol) != upperProtocols.end()) {
        EV_INFO << "Passing up to protocol " << *protocol << "\n";
        send(packet, "transportOut");
    }
    else if (!hasSocket) {
        // send ICMP Destination Unreacheable error: protocol unavailable
        EV_INFO << "Transport layer gate not connected - dropping packet!\n";
        sendIcmpError(origPacket, ICMPv6_PARAMETER_PROBLEM, UNRECOGNIZED_NEXT_HDR_TYPE);
        origPacket = nullptr;    // for not delete
        delete packet;          // delete decapsulated packet
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
    // decapsulate transport packet
    auto ipv6Header = packet->popAtFront<Ipv6Header>();

    B payloadLength = ipv6Header->getPayloadLength();
    if (payloadLength != B(0)) {      // payloadLength == 0 occured with Jumbo payload
        ASSERT(payloadLength <= packet->getDataLength());
        // drop padding behind the payload:
        if (payloadLength < packet->getDataLength())
            packet->setBackOffset(packet->getFrontOffset() + payloadLength);
    }

    // create and fill in control info
    packet->addTagIfAbsent<TosInd>()->setTos(ipv6Header->getTrafficClass());
    packet->addTagIfAbsent<DscpInd>()->setDifferentiatedServicesCodePoint(ipv6Header->getDscp());
    packet->addTagIfAbsent<EcnInd>()->setExplicitCongestionNotification(ipv6Header->getEcn());
    packet->addTagIfAbsent<DispatchProtocolInd>()->setProtocol(&Protocol::ipv6);
    auto payloadProtocol = ipv6Header->getProtocol();
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(payloadProtocol);
    auto networkProtocolInd = packet->addTagIfAbsent<NetworkProtocolInd>();
    networkProtocolInd->setProtocol(&Protocol::ipv6);
    networkProtocolInd->setNetworkProtocolHeader(ipv6Header);
    auto l3AddressInd = packet->addTagIfAbsent<L3AddressInd>();
    l3AddressInd->setSrcAddress(ipv6Header->getSrcAddress());
    l3AddressInd->setDestAddress(ipv6Header->getDestAddress());
    packet->addTagIfAbsent<HopLimitInd>()->setHopLimit(ipv6Header->getHopLimit());
}

void Ipv6::encapsulate(Packet *transportPacket)
{
    auto ipv6Header = makeShared<Ipv6Header>(); // TODO: transportPacket->getName());

    L3AddressReq *addresses = transportPacket->removeTag<L3AddressReq>();
    Ipv6Address src = addresses->getSrcAddress().toIpv6();
    Ipv6Address dest = addresses->getDestAddress().toIpv6();
    delete addresses;

    auto hopLimitReq = transportPacket->removeTagIfPresent<HopLimitReq>();
    short ttl = (hopLimitReq != nullptr) ? hopLimitReq->getHopLimit() : -1;
    delete hopLimitReq;

    ipv6Header->setPayloadLength(transportPacket->getDataLength());

    // set source and destination address
    ipv6Header->setDestAddress(dest);
    ipv6Header->setSrcAddress(src);

    // set other fields
    if (TosReq *tosReq = transportPacket->removeTagIfPresent<TosReq>()) {
        ipv6Header->setTrafficClass(tosReq->getTos());
        delete tosReq;
        if (transportPacket->findTag<DscpReq>())
            throw cRuntimeError("TosReq and DscpReq found together");
        if (transportPacket->findTag<EcnReq>())
            throw cRuntimeError("TosReq and EcnReq found together");
    }
    if (DscpReq *dscpReq = transportPacket->removeTagIfPresent<DscpReq>()) {
        ipv6Header->setDscp(dscpReq->getDifferentiatedServicesCodePoint());
        delete dscpReq;
    }
    if (EcnReq *ecnReq = transportPacket->removeTagIfPresent<EcnReq>()) {
        ipv6Header->setEcn(ecnReq->getExplicitCongestionNotification());
        delete ecnReq;
    }

    ipv6Header->setHopLimit(ttl != -1 ? ttl : 32);    //FIXME use iface hop limit instead of 32?
    ASSERT(ipv6Header->getHopLimit() > 0);
    ipv6Header->setProtocolId(static_cast<IpProtocolId>(ProtocolGroup::ipprotocol.getProtocolNumber(transportPacket->getTag<PacketProtocolTag>()->getProtocol())));

    // #### Move extension headers from ctrlInfo to datagram if present
    auto extHeadersTag = transportPacket->removeTagIfPresent<Ipv6ExtHeaderReq>();
    while (extHeadersTag && 0 < extHeadersTag->getExtensionHeaderArraySize()) {
        Ipv6ExtensionHeader *extHeader = extHeadersTag->removeFirstExtensionHeader();
        ipv6Header->addExtensionHeader(extHeader);
        // EV << "Move extension header to datagram." << endl;
    }
    delete extHeadersTag;

    ipv6Header->setChunkLength(B(ipv6Header->calculateHeaderByteLength()));
    transportPacket->trimFront();
    insertNetworkProtocolHeader(transportPacket, Protocol::ipv6, ipv6Header);
    // setting IP options is currently not supported
}

void Ipv6::fragmentPostRouting(Packet *packet, const InterfaceEntry *ie, const MacAddress& nextHopAddr, bool fromHL)
{
//    const InterfaceEntry *destIE = ift->getInterfaceById(packet->getTag<InterfaceReq>()->getInterfaceId());
    auto ipv6Header = packet->peekAtFront<Ipv6Header>();
    // ensure source address is filled
    if (fromHL && ipv6Header->getSrcAddress().isUnspecified() &&
        !ipv6Header->getDestAddress().isSolicitedNodeMulticastAddress())
    {
        // source address can be unspecified during DAD
        const Ipv6Address& srcAddr = ie->getProtocolData<Ipv6InterfaceData>()->getPreferredAddress();
        ASSERT(!srcAddr.isUnspecified());    // FIXME what if we don't have an address yet?

        // TODO: factor out
        ipv6Header = nullptr;
        auto newIpv6Header = removeNetworkProtocolHeader<Ipv6Header>(packet);
        newIpv6Header->setSrcAddress(srcAddr);
        insertNetworkProtocolHeader(packet, Protocol::ipv6, newIpv6Header);
        ipv6Header = newIpv6Header;
    }
    const InterfaceEntry *fromIe = fromHL ? nullptr : ift->getInterfaceById(packet->getTag<InterfaceInd>()->getInterfaceId());
    L3Address nextHopAddr_(nextHopAddr);
    if (datagramPostRoutingHook(packet, fromIe, ie, nextHopAddr_) == INetfilter::IHook::ACCEPT) {
        fragmentAndSend(packet, ie, nextHopAddr_.toMac(), fromHL);
    }
}

void Ipv6::fragmentAndSend(Packet *packet, const InterfaceEntry *ie, const MacAddress& nextHopAddr, bool fromHL)
{
    auto ipv6Header = packet->peekAtFront<Ipv6Header>();
    // hop counter check
    if (ipv6Header->getHopLimit() <= 0) {
        // drop datagram, destruction responsibility in ICMP
        EV_INFO << "datagram hopLimit reached zero, sending ICMPv6_TIME_EXCEEDED\n";
        sendIcmpError(packet, ICMPv6_TIME_EXCEEDED, 0);    // FIXME check icmp 'code'
        return;
    }

    // ensure source address is filled
    if (fromHL && ipv6Header->getSrcAddress().isUnspecified() &&
        !ipv6Header->getDestAddress().isSolicitedNodeMulticastAddress())
    {
        // source address can be unspecified during DAD
        const Ipv6Address& srcAddr = ie->getProtocolData<Ipv6InterfaceData>()->getPreferredAddress();
        ASSERT(!srcAddr.isUnspecified());    // FIXME what if we don't have an address yet?

        // TODO: factor out
        packet->eraseAtFront(ipv6Header->getChunkLength());
        auto ipv6HeaderCopy = staticPtrCast<Ipv6Header>(ipv6Header->dupShared());
        // TODO: dup or mark ipv4Header->markMutableIfExclusivelyOwned();
        ipv6HeaderCopy->setSrcAddress(srcAddr);
        packet->insertAtFront(ipv6HeaderCopy);
        ipv6Header = ipv6HeaderCopy;

    #ifdef WITH_xMIPv6
        // if the datagram has a tentative address as source we have to reschedule it
        // as it can not be sent before the address' tentative status is cleared - CB
        if (ie->getProtocolData<Ipv6InterfaceData>()->isTentativeAddress(srcAddr)) {
            EV_INFO << "Source address is tentative - enqueueing datagram for later resubmission." << endl;
            ScheduledDatagram *sDgram = new ScheduledDatagram(packet, ipv6Header.get(), ie, nextHopAddr, fromHL);
            // queue.insert(sDgram);
            scheduleAfter(1.0, sDgram);    //FIXME KLUDGE wait 1s for tentative->permanent. MISSING: timeout for drop or send back icmpv6 error, processing signals from IE, need another msg queue for waiting (similar to Ipv4 ARP)
            return;
        }
    #endif /* WITH_xMIPv6 */
    }

    int mtu = ie->getMtu();

    // check if datagram does not require fragmentation
    if (packet->getTotalLength() <= B(mtu)) {
        sendDatagramToOutput(packet, ie, nextHopAddr);
        return;
    }

    // routed datagrams are not fragmented
    if (!fromHL) {
        // FIXME check for multicast datagrams, how many ICMP error should be sent
        sendIcmpError(packet, ICMPv6_PACKET_TOO_BIG, 0);    // TODO set MTU
        return;
    }

    // create and send fragments
    ipv6Header = packet->popAtFront<Ipv6Header>();
    B headerLength = ipv6Header->calculateUnfragmentableHeaderByteLength();
    B payloadLength = packet->getDataLength();
    B fragmentLength = ((B(mtu) - headerLength - IPv6_FRAGMENT_HEADER_LENGTH) / 8) * 8;
    ASSERT(fragmentLength > B(0));

    int noOfFragments = B(payloadLength + fragmentLength - B(1)).get() / B(fragmentLength).get();
    EV_INFO << "Breaking datagram into " << noOfFragments << " fragments\n";
    std::string fragMsgName = packet->getName();
    fragMsgName += "-frag-";

    //FIXME is need to remove unfragmentable header extensions? see calculateUnfragmentableHeaderByteLength()

    unsigned int identification = curFragmentId++;
    for (B offset = B(0); offset < payloadLength; offset += fragmentLength) {
        bool lastFragment = (offset + fragmentLength >= payloadLength);
        B thisFragmentLength = lastFragment ? payloadLength - offset : fragmentLength;

        std::string curFragName = fragMsgName + std::to_string(offset.get());
        if (lastFragment)
            curFragName += "-last";
        Packet *fragPk = new Packet(curFragName.c_str());
        const auto& fragHdr = staticPtrCast<Ipv6Header>(ipv6Header->dupShared());
        auto *fh = new Ipv6FragmentHeader();
        fh->setIdentification(identification);
        fh->setFragmentOffset(offset.get());
        fh->setMoreFragments(!lastFragment);
        fragHdr->addExtensionHeader(fh);
        fragHdr->setChunkLength(headerLength + fh->getByteLength());      //FIXME KLUDGE
        fragPk->insertAtFront(fragHdr);
        fragPk->insertAtBack(packet->peekDataAt(offset, thisFragmentLength));

        ASSERT(fragPk->getDataLength() == headerLength + fh->getByteLength() + thisFragmentLength);

        sendDatagramToOutput(fragPk, ie, nextHopAddr);
    }

    delete packet;
}

void Ipv6::sendDatagramToOutput(Packet *packet, const InterfaceEntry *destIE, const MacAddress& macAddr)
{
    packet->addTagIfAbsent<MacAddressReq>()->setDestAddress(macAddr);
    delete packet->removeTagIfPresent<DispatchProtocolReq>();
    packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(destIE->getInterfaceId());
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv6);
    packet->addTagIfAbsent<DispatchProtocolInd>()->setProtocol(&Protocol::ipv6);
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
                sendIcmpError(packet, ICMPv6_DESTINATION_UNREACHABLE, 0);    // FIXME check ICMP 'code'
            }
            else {    // host
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

#ifdef WITH_xMIPv6
bool Ipv6::processExtensionHeaders(Packet *packet, const Ipv6Header *ipv6Header)
{
    int noExtHeaders = ipv6Header->getExtensionHeaderArraySize();
    EV_INFO << noExtHeaders << " extension header(s) for processing..." << endl;

    // walk through all extension headers
    for (int i = 0; i < noExtHeaders; i++) {
        const Ipv6ExtensionHeader *eh = ipv6Header->getExtensionHeader(i);

        if (const Ipv6RoutingHeader *rh = dynamic_cast<const Ipv6RoutingHeader *>(eh)) {
            EV_DETAIL << "Routing Header with type=" << rh->getRoutingType() << endl;

            // type 2 routing header should be processed by MIPv6 module
            // if no MIP support, ignore the header
            if (rt->hasMipv6Support() && rh->getRoutingType() == 2) {
                // for simplicity, we set a context pointer on the datagram
                packet->setContextPointer((void *)rh);
                EV_INFO << "Sending datagram with RH2 to MIPv6 module" << endl;
                send(packet, "xMIPv6Out");
                return false;
            }
            else {
                EV_INFO << "Ignoring unknown routing header" << endl;
            }
        }
        else if (dynamic_cast<const Ipv6DestinationOptionsHeader *>(eh)) {
            //Ipv6DestinationOptionsHeader* doh = (Ipv6DestinationOptionsHeader*) (eh);
            //EV << "object of type=" << typeid(eh).name() << endl;

            if (rt->hasMipv6Support() && dynamic_cast<const HomeAddressOption *>(eh)) {
                packet->setContextPointer((void *)eh);
                EV_INFO << "Sending datagram with HoA Option to MIPv6 module" << endl;
                send(packet, "xMIPv6Out");
                return false;
            }
            else {
//                delete eh;
                EV_INFO << "Ignoring unknown destination options header" << endl;
            }
        }
        else {
//            delete eh;
            EV_INFO << "Ignoring unknown extension header" << endl;
        }
    }

    // we have processed no extension headers -> the Ipv6 module can continue
    // working on this datagram
    return true;
}

#endif /* WITH_xMIPv6 */

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
                    //fragmentAndSend(datagram, iter->outIE, iter->nextHopAddr);
                    throw cRuntimeError("Re-injection of datagram queued for POSTROUTING hook not implemented");
                    break;

                case INetfilter::IHook::LOCALIN:
                    //reassembleAndDeliverFinish(datagram);
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

INetfilter::IHook::Result Ipv6::datagramForwardHook(Packet *packet)
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

INetfilter::IHook::Result Ipv6::datagramPostRoutingHook(Packet *packet, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr)
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

INetfilter::IHook::Result Ipv6::datagramLocalInHook(Packet *packet, const InterfaceEntry *inIE)
{
    for (auto & elem : hooks) {
        IHook::Result r = elem.second->datagramLocalInHook(packet);
        switch (r) {
            case INetfilter::IHook::ACCEPT:
                break;    // continue iteration

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

void Ipv6::sendIcmpError(Packet *packet, Icmpv6Type type, int code)
{
    icmp->sendErrorMessage(packet, type, code);
}

} // namespace inet

