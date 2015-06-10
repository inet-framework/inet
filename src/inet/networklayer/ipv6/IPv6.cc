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

#include "inet/common/INETDefs.h"

#include "inet/networklayer/ipv6/IPv6.h"

#include "inet/networklayer/common/IPSocket.h"

#include "inet/networklayer/contract/ipv6/IPv6ControlInfo.h"
#include "inet/networklayer/icmpv6/IPv6NDMessage_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/networklayer/icmpv6/ICMPv6Message_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/common/ModuleAccess.h"

#ifdef WITH_xMIPv6
#include "inet/networklayer/xmipv6/MobilityHeader.h"
#endif /* WITH_xMIPv6 */

#include "inet/networklayer/ipv6/IPv6ExtensionHeaders.h"
#include "inet/networklayer/ipv6/IPv6InterfaceData.h"

#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

#define FRAGMENT_TIMEOUT    60   // 60 sec, from IPv6 RFC

Define_Module(IPv6);

IPv6::IPv6() :
        curFragmentId(0)
{
}

IPv6::~IPv6()
{
}

#ifdef WITH_xMIPv6
IPv6::ScheduledDatagram::ScheduledDatagram(IPv6Datagram *datagram, const InterfaceEntry *ie, MACAddress macAddr, bool fromHL) :
        datagram(datagram),
        ie(ie),
        macAddr(macAddr),
        fromHL(fromHL)
{
}

IPv6::ScheduledDatagram::~ScheduledDatagram()
{
    delete datagram;
}
#endif /* WITH_xMIPv6 */

void IPv6::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        QueueBase::initialize();

        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        rt = getModuleFromPar<IPv6RoutingTable>(par("routingTableModule"), this);
        nd = getModuleFromPar<IPv6NeighbourDiscovery>(par("ipv6NeighbourDiscoveryModule"), this);
        icmp = getModuleFromPar<ICMPv6>(par("icmpv6Module"), this);
        tunneling = getModuleFromPar<IPv6Tunneling>(par("ipv6TunnelingModule"), this);

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
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}

void IPv6::updateDisplayString()
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

void IPv6::handleMessage(cMessage *msg)
{
    if (dynamic_cast<RegisterTransportProtocolCommand *>(msg)) {
        RegisterTransportProtocolCommand *command = static_cast<RegisterTransportProtocolCommand *>(msg);
        if (msg->getArrivalGate()->isName("transportIn")) {
            mapping.addProtocolMapping(command->getProtocol(), msg->getArrivalGate()->getIndex());
        }
        else
            throw cRuntimeError("RegisterTransportProtocolCommand %d arrived invalid gate '%s'", command->getProtocol(), msg->getArrivalGate()->getFullName());
        delete msg;
    }
    else
        QueueBase::handleMessage(msg);
}

void IPv6::endService(cPacket *msg)
{
#ifdef WITH_xMIPv6
    // 28.09.07 - CB
    // support for rescheduling datagrams which are supposed to be sent over
    // a tentative address.
    if (msg->isSelfMessage()) {
        ScheduledDatagram *sDgram = check_and_cast<ScheduledDatagram *>(msg);

        // take care of datagram which was supposed to be sent over a tentative address
        if (sDgram->getIE()->ipv6Data()->isTentativeAddress(sDgram->getSrcAddress())) {
            // address is still tentative - enqueue again
            queue.insert(sDgram);
        }
        else {
            // address is not tentative anymore - send out datagram
            numForwarded++;
            fragmentAndSend(sDgram->removeDatagram(), sDgram->getIE(), sDgram->getMACAddress(), sDgram->getFromHL());
            delete sDgram;
        }
    }
    else
#endif /* WITH_xMIPv6 */

    if (msg->getArrivalGate()->isName("transportIn")
        || (msg->getArrivalGate()->isName("ndIn") && dynamic_cast<IPv6NDMessage *>(msg))
        || (msg->getArrivalGate()->isName("upperTunnelingIn"))    // for tunneling support-CB
#ifdef WITH_xMIPv6
        || (msg->getArrivalGate()->isName("xMIPv6In") && dynamic_cast<MobilityHeader *>(msg))    // Zarrar
#endif /* WITH_xMIPv6 */
        )
    {
        // packet from upper layers, tunnel link-layer output or ND: encapsulate and send out
        handleMessageFromHL(msg);
    }
    else if (msg->getArrivalGate()->isName("ndIn") && dynamic_cast<IPv6Datagram *>(msg)) {
        IPv6Datagram *datagram = static_cast<IPv6Datagram *>(msg);
        IPv6NDControlInfo *ctrl = check_and_cast<IPv6NDControlInfo *>(msg->removeControlInfo());
        bool fromHL = ctrl->getFromHL();
        IPv6Address nextHop = ctrl->getNextHop();
        int interfaceId = ctrl->getInterfaceId();
        delete ctrl;
        resolveMACAddressAndSendPacket(datagram, interfaceId, nextHop, fromHL);
    }
    else {
        // datagram from network or from ND: localDeliver and/or route
        IPv6Datagram *datagram = check_and_cast<IPv6Datagram *>(msg);
        bool fromHL = false;
        if (datagram->getArrivalGate()->isName("ndIn")) {
            IPv6NDControlInfo *ctrl = check_and_cast<IPv6NDControlInfo *>(msg->removeControlInfo());
            fromHL = ctrl->getFromHL();
            IPv6Address nextHop = ctrl->getNextHop();
            int interfaceId = ctrl->getInterfaceId();
            delete ctrl;
            resolveMACAddressAndSendPacket(datagram, interfaceId, nextHop, fromHL);
        }

        // Do not handle header biterrors, because
        // 1. IPv6 header does not contain checksum for the header fields, each field is
        //    validated when they are processed.
        // 2. The Ethernet or PPP frame is dropped by the link-layer if there is a transmission error.
        ASSERT(!datagram->hasBitError());

        const InterfaceEntry *fromIE = getSourceInterfaceFrom(datagram);
        const InterfaceEntry *destIE = nullptr;
        L3Address nextHop(IPv6Address::UNSPECIFIED_ADDRESS);
        if (fromHL) {
            // remove control info
            delete datagram->removeControlInfo();
            if (datagramLocalOutHook(datagram, destIE, nextHop) == INetfilter::IHook::ACCEPT)
                datagramLocalOut(datagram, destIE, nextHop.toIPv6());
        }
        else {
            if (datagramPreRoutingHook(datagram, fromIE, destIE, nextHop) == INetfilter::IHook::ACCEPT)
                preroutingFinish(datagram, fromIE, destIE, nextHop.toIPv6());
        }
    }

    if (hasGUI())
        updateDisplayString();
}

InterfaceEntry *IPv6::getSourceInterfaceFrom(cPacket *msg)
{
    cGate *g = msg->getArrivalGate();
    return g ? ift->getInterfaceByNetworkLayerGateIndex(g->getIndex()) : nullptr;
}

void IPv6::preroutingFinish(IPv6Datagram *datagram, const InterfaceEntry *fromIE, const InterfaceEntry *destIE, IPv6Address nextHopAddr)
{
    IPv6Address& destAddr = datagram->getDestAddress();
    // remove control info
    delete datagram->removeControlInfo();

    // routepacket
    if (!destAddr.isMulticast())
        routePacket(datagram, destIE, nextHopAddr, false);
    else
        routeMulticastPacket(datagram, destIE, fromIE, false);
}

void IPv6::handleMessageFromHL(cPacket *msg)
{
    // if no interface exists, do not send datagram
    if (ift->getNumInterfaces() == 0) {
        EV_WARN << "No interfaces exist, dropping packet\n";
        delete msg;
        return;
    }

    IPv6ControlInfo *controlInfo = check_and_cast<IPv6ControlInfo *>(msg->removeControlInfo());
    // encapsulate upper-layer packet into IPv6Datagram
    // IPV6_MULTICAST_IF option, but allow interface selection for unicast packets as well
    const InterfaceEntry *destIE = ift->getInterfaceById(controlInfo->getInterfaceId());
    IPv6Datagram *datagram = encapsulate(msg, controlInfo);
    delete controlInfo;

#ifdef WITH_xMIPv6
    if (datagram == nullptr) {
        EV_WARN << "Encapsulation failed - dropping packet." << endl;
        delete msg;
        return;
    }
#endif /* WITH_xMIPv6 */

    IPv6Address destAddress = datagram->getDestAddress();

    // check for local delivery
    if (!destAddress.isMulticast() && rt->isLocalAddress(destAddress)) {
        EV_INFO << "local delivery\n";
        if (datagram->getSrcAddress().isUnspecified())
            datagram->setSrcAddress(destAddress); // allows two apps on the same host to communicate

        if (destIE && !destIE->isLoopback()) {
            EV_INFO << "datagram destination address is local, ignoring destination interface specified in the control info\n";
            destIE = nullptr;
        }
        if (!destIE)
            destIE = ift->getFirstLoopbackInterface();
        ASSERT(destIE);
    }
    L3Address nextHopAddr(IPv6Address::UNSPECIFIED_ADDRESS);
    if (datagramLocalOutHook(datagram, destIE, nextHopAddr) == INetfilter::IHook::ACCEPT)
        datagramLocalOut(datagram, destIE, nextHopAddr.toIPv6());
}

void IPv6::datagramLocalOut(IPv6Datagram *datagram, const InterfaceEntry *destIE, IPv6Address requestedNextHopAddress)
{
    // route packet
    if (destIE != nullptr)
        fragmentAndSend(datagram, destIE, MACAddress::BROADCAST_ADDRESS, true); // FIXME what MAC address to use?
    else if (!datagram->getDestAddress().isMulticast())
        routePacket(datagram, destIE, requestedNextHopAddress, true);
    else
        routeMulticastPacket(datagram, destIE, nullptr, true);
}

void IPv6::routePacket(IPv6Datagram *datagram, const InterfaceEntry *destIE, IPv6Address requestedNextHopAddress, bool fromHL)
{
    // TBD add option handling code here
    IPv6Address destAddress = datagram->getDestAddress();

    EV_INFO << "Routing datagram `" << datagram->getName() << "' with dest=" << destAddress << ", requested nexthop is " << requestedNextHopAddress << " on " << (destIE ? destIE->getFullName() : "unspec") << " interface: \n";

    // local delivery of unicast packets
    if (rt->isLocalAddress(destAddress)) {
        if (fromHL)
            throw cRuntimeError("model error: local unicast packet arrived from HL, but handleMessageFromHL() not detected it");
        EV_INFO << "local delivery\n";

        numLocalDeliver++;
        localDeliver(datagram);
        return;
    }

    if (!fromHL) {
        // if datagram arrived from input gate and IP forwarding is off, delete datagram
        //yes but datagrams from the ND module is getting dropped too!-WEI
        //so we add a 2nd condition
        // FIXME rewrite code so that condition is cleaner --Andras
        //if (!rt->isRouter())
        if (!rt->isRouter() && !(datagram->getArrivalGate()->isName("ndIn"))) {
            EV_INFO << "forwarding is off, dropping packet\n";
            numDropped++;
            delete datagram;
            return;
        }

        // don't forward link-local addresses or weaker
        if (destAddress.isLinkLocal() || destAddress.isLoopback()) {
            EV_INFO << "dest address is link-local (or weaker) scope, doesn't get forwarded\n";
            delete datagram;
            return;
        }

        // hop counter decrement: only if datagram arrived from network, and will be
        // sent out to the network (hoplimit check will be done just before sending
        // out datagram)
        // TBD: in IPv4, arrange TTL check like this
        datagram->setHopLimit(datagram->getHopLimit() - 1);
    }

    // routing
    int interfaceId = -1;
    IPv6Address nextHop(requestedNextHopAddress);

#ifdef WITH_xMIPv6
    // tunneling support - CB
    // check if destination is covered by tunnel lists
    if ((datagram->getTransportProtocol() != IP_PROT_IPv6) &&    // if datagram was already tunneled, don't tunnel again
        (datagram->getExtensionHeaderArraySize() == 0) &&    // we do not already have extension headers - FIXME: check for RH2 existence
        ((rt->isMobileNode() && rt->isHomeAddress(datagram->getSrcAddress())) ||    // for MNs: only if source address is a HoA // 27.08.07 - CB
         rt->isHomeAgent() ||    // but always check for tunnel if node is a HA
         !rt->isMobileNode()    // or if it is a correspondent or non-MIP node
        )
        )
    {
        if (datagram->getTransportProtocol() == IP_PROT_IPv6EXT_MOB)
            // in case of mobility header we can only search for "real" tunnels
            // as T2RH or HoA Opt. are not allowed with these messages
            interfaceId = tunneling->getVIfIndexForDest(destAddress, IPv6Tunneling::NORMAL); // 10.06.08 - CB
        //getVIfIndexForDestForXSplitTunnel(destAddress);
        else
            // otherwise we can search for everything
            interfaceId = tunneling->getVIfIndexForDest(destAddress);
    }
#else // ifdef WITH_xMIPv6
      // FIXME this is not the same as the code above (when WITH_xMIPv6 is defined),
      // so tunneling examples could not work with xMIPv6
    interfaceId = tunneling->getVIfIndexForDest(destAddress, IPv6Tunneling::NORMAL);
#endif /* WITH_xMIPv6 */

    if (interfaceId == -1 && destIE != nullptr)
        interfaceId = destIE->getInterfaceId();         // set interfaceId to destIE when not tunneling

    if (interfaceId > ift->getBiggestInterfaceId()) {
        // a virtual tunnel interface provides a path to the destination: do tunneling
        EV_INFO << "tunneling: src addr=" << datagram->getSrcAddress() << ", dest addr=" << destAddress << std::endl;
        send(datagram, "lowerTunnelingOut");
        return;
    }

    if (interfaceId == -1)
        if (!determineOutputInterface(destAddress, nextHop, interfaceId, datagram, fromHL))
            // no interface found; sent to ND or to ICMP for error processing
            //throw cRuntimeError("No interface found!");//return;
            return;
    // don't raise error if sent to ND or ICMP!

    resolveMACAddressAndSendPacket(datagram, interfaceId, nextHop, fromHL);
}

void IPv6::resolveMACAddressAndSendPacket(IPv6Datagram *datagram, int interfaceId, IPv6Address nextHop, bool fromHL)
{
    InterfaceEntry *ie = ift->getInterfaceById(interfaceId);
    ASSERT(ie != nullptr);
    ASSERT(!nextHop.isUnspecified());
    IPv6Address destAddress = datagram->getDestAddress();
    EV_INFO << "next hop for " << destAddress << " is " << nextHop << ", interface " << ie->getName() << "\n";

#ifdef WITH_xMIPv6
    if (rt->isMobileNode()) {
        // if the source address is the HoA and we have a CoA then drop the packet
        // (address is topologically incorrect!)
        if (datagram->getSrcAddress() == ie->ipv6Data()->getMNHomeAddress()
            && !ie->ipv6Data()->getGlobalAddress(IPv6InterfaceData::CoA).isUnspecified())
        {
            EV_WARN << "Using HoA instead of CoA... dropping datagram" << endl;
            delete datagram;
            numDropped++;
            return;
        }
    }
#endif /* WITH_xMIPv6 */

    MACAddress macAddr = nd->resolveNeighbour(nextHop, interfaceId);    // might initiate NUD
    if (macAddr.isUnspecified()) {
        if (!ie->isPointToPoint()) {
            EV_INFO << "no link-layer address for next hop yet, passing datagram to Neighbour Discovery module\n";
            IPv6NDControlInfo *ctrl = new IPv6NDControlInfo();
            ctrl->setFromHL(fromHL);
            ctrl->setNextHop(nextHop);
            ctrl->setInterfaceId(interfaceId);
            datagram->setControlInfo(ctrl);
            send(datagram, "ndOut");
            return;
        }
    }
    else
        EV_DETAIL << "link-layer address: " << macAddr << "\n";

    // send out datagram
    numForwarded++;
    fragmentAndSend(datagram, ie, macAddr, fromHL);
}

void IPv6::routeMulticastPacket(IPv6Datagram *datagram, const InterfaceEntry *destIE, const InterfaceEntry *fromIE, bool fromHL)
{
    const IPv6Address& destAddr = datagram->getDestAddress();

    EV_INFO << "destination address " << destAddr << " is multicast, doing multicast routing\n";
    numMulticast++;

    // if received from the network...
    if (fromIE != nullptr) {
        ASSERT(!fromHL);
        // deliver locally
        if (rt->isLocalAddress(destAddr)) {
            EV_INFO << "local delivery of multicast packet\n";
            numLocalDeliver++;
            localDeliver(datagram->dup());
        }

        // if datagram arrived from input gate and IP forwarding is off, delete datagram
        if (!rt->isRouter()) {
            EV_INFO << "forwarding is off\n";
            delete datagram;
            return;
        }

        // make sure scope of multicast address is large enough to be forwarded to other links
        if (destAddr.getMulticastScope() <= 2) {
            EV_INFO << "multicast dest address is link-local (or smaller) scope\n";
            delete datagram;
            return;
        }

        // hop counter decrement: only if datagram arrived from network, and will be
        // sent out to the network (hoplimit check will be done just before sending
        // out datagram)
        // TBD: in IPv4, arrange TTL check like this
        datagram->setHopLimit(datagram->getHopLimit() - 1);
    }

    // for now, we just send it out on every interface except on which it came. FIXME better!!!
    EV_INFO << "sending out datagram on every interface (except incoming one)\n";
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        InterfaceEntry *ie = ift->getInterface(i);
        if (fromIE != ie && !ie->isLoopback())
            fragmentAndSend((IPv6Datagram *)datagram->dup(), ie, MACAddress::BROADCAST_ADDRESS, fromHL);
    }
    delete datagram;

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
    IPv6Address destAddress = datagram->getDestAddress();
    if (rt->isLocalMulticastAddress(destAddress))
    {
        IPv6Datagram *datagramCopy = (IPv6Datagram *) datagram->dup();

        // FIXME code from the MPLS model: set packet dest address to routerId (???)
        datagramCopy->setDestAddress(rt->getRouterId());

        localDeliver(datagramCopy);
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
                IPv6Datagram *datagramCopy = (IPv6Datagram *) datagram->dup();

                // set datagram source address if not yet set
                if (datagramCopy->getSrcAddress().isUnspecified())
                    datagramCopy->setSrcAddress(ift->interfaceByPortNo(outputGateIndex)->ipv6Data()->getIPAddress());

                // send
                IPv6Address nextHopAddr = routes[i].gateway;
                fragmentAndSend(datagramCopy, outputGateIndex, macAddr, fromHL);
            }
        }

        // only copies sent, delete original datagram
        delete datagram;
    }
 */
}

void IPv6::localDeliver(IPv6Datagram *datagram)
{
    // Defragmentation. skip defragmentation if datagram is not fragmented
    IPv6FragmentHeader *fh = dynamic_cast<IPv6FragmentHeader *>(datagram->findExtensionHeaderByType(IP_PROT_IPv6EXT_FRAGMENT));
    if (fh) {
        EV_DETAIL << "Datagram fragment: offset=" << fh->getFragmentOffset()
                  << ", MORE=" << (fh->getMoreFragments() ? "true" : "false") << ".\n";

        // erase timed out fragments in fragmentation buffer; check every 10 seconds max
        if (simTime() >= lastCheckTime + 10) {
            lastCheckTime = simTime();
            fragbuf.purgeStaleFragments(simTime() - FRAGMENT_TIMEOUT);
        }

        datagram = fragbuf.addFragment(datagram, fh, simTime());
        if (!datagram) {
            EV_DETAIL << "No complete datagram yet.\n";
            return;
        }
        EV_DETAIL << "This fragment completes the datagram.\n";
    }

#ifdef WITH_xMIPv6
    // #### 29.08.07 - CB
    // check for extension headers
    if (!processExtensionHeaders(datagram)) {
        // ext. header processing not yet finished
        // datagram was sent to another module or dropped
        // -> interrupt local delivery process
        return;
    }
    // #### end CB
#endif /* WITH_xMIPv6 */

    // decapsulate and send on appropriate output gate
    int protocol = datagram->getTransportProtocol();
    cPacket *packet = decapsulate(datagram);


    if (protocol == IP_PROT_IPv6_ICMP && dynamic_cast<IPv6NDMessage *>(packet)) {
        EV_INFO << "Neigbour Discovery packet: passing it to ND module\n";
        send(packet, "ndOut");
        packet = nullptr;
    }
#ifdef WITH_xMIPv6
    else if (protocol == IP_PROT_IPv6EXT_MOB && dynamic_cast<MobilityHeader *>(packet)) {
        // added check for MIPv6 support to prevent nodes w/o the
        // xMIP module from processing related messages, 4.9.07 - CB
        if (rt->hasMIPv6Support()) {
            EV_INFO << "MIPv6 packet: passing it to xMIPv6 module\n";
            send(check_and_cast<MobilityHeader *>(packet), "xMIPv6Out");
            packet = nullptr;
        }
        else {
            // update 12.9.07 - CB
            /*RFC3775, 11.3.5
               Any node that does not recognize the Mobility header will return an
               ICMP Parameter Problem, Code 1, message to the sender of the packet*/
            EV_INFO << "No MIPv6 support on this node!\n";
            IPv6ControlInfo *ctrlInfo = check_and_cast<IPv6ControlInfo *>(packet->removeControlInfo());
            icmp->sendErrorMessage(packet, ctrlInfo, ICMPv6_PARAMETER_PROBLEM, UNRECOGNIZED_NEXT_HDR_TYPE);
            packet = nullptr;
        }
    }
#endif /* WITH_xMIPv6 */
    else if (protocol == IP_PROT_IPv6_ICMP) {
        handleReceivedICMP(check_and_cast<ICMPv6Message *>(packet));
        packet = nullptr;
    }    //Added by WEI to forward ICMPv6 msgs to ICMPv6 module.
    else if (protocol == IP_PROT_IP || protocol == IP_PROT_IPv6) {
        EV_INFO << "Tunnelled IP datagram\n";
        send(packet, "upperTunnelingOut");
        packet = nullptr;
    }
    else {
        int gateindex = mapping.findOutputGateForProtocol(protocol);
        // check if the transportOut port are connected, otherwise discard the packet
        if (gateindex >= 0) {
            cGate *outGate = gate("transportOut", gateindex);
            if (outGate->isPathOK()) {
                EV_INFO << "Protocol " << protocol << ", passing up on gate " << gateindex << "\n";
                //TODO: Indication of forward progress
                send(packet, outGate);
                packet = nullptr;
                return;
            }
        }

        // TODO send ICMP Destination Unreacheable error
        EV_INFO << "Transport layer gate not connected - dropping packet!\n";
        IPv6ControlInfo *ctrlInfo = check_and_cast<IPv6ControlInfo *>(packet->removeControlInfo());
        icmp->sendErrorMessage(packet, ctrlInfo, ICMPv6_PARAMETER_PROBLEM, UNRECOGNIZED_NEXT_HDR_TYPE);
        packet = nullptr;
    }
    ASSERT(packet == nullptr);
}

void IPv6::handleReceivedICMP(ICMPv6Message *msg)
{
    int type = msg->getType();
    if (type < 128) {
        // ICMP errors are delivered to the appropriate higher layer protocols
        EV_INFO << "ICMPv6 packet: passing it to higher layer\n";
        IPv6Datagram *bogusPacket = check_and_cast<IPv6Datagram *>(msg->getEncapsulatedPacket());
        int protocol = bogusPacket->getTransportProtocol();
        int gateindex = mapping.getOutputGateForProtocol(protocol);
        send(msg, "transportOut", gateindex);
    }
    else {
        // all others are delivered to ICMP:
        // ICMPv6_ECHO_REQUEST, ICMPv6_ECHO_REPLY, ICMPv6_MLD_QUERY, ICMPv6_MLD_REPORT,
        // ICMPv6_MLD_DONE, ICMPv6_ROUTER_SOL, ICMPv6_ROUTER_AD, ICMPv6_NEIGHBOUR_SOL,
        // ICMPv6_NEIGHBOUR_AD, ICMPv6_MLDv2_REPORT
        EV_INFO << "ICMPv6 packet: passing it to ICMPv6 module\n";
        int gateindex = mapping.getOutputGateForProtocol(IP_PROT_IPv6_ICMP);
        send(msg, "transportOut", gateindex);
    }
}

cPacket *IPv6::decapsulate(IPv6Datagram *datagram)
{
    // decapsulate transport packet
    InterfaceEntry *fromIE = getSourceInterfaceFrom(datagram);
    cPacket *packet = datagram->decapsulate();

    // create and fill in control info
    IPv6ControlInfo *controlInfo = new IPv6ControlInfo();
    controlInfo->setProtocol(datagram->getTransportProtocol());
    controlInfo->setSrcAddr(datagram->getSrcAddress());
    controlInfo->setDestAddr(datagram->getDestAddress());
    controlInfo->setTrafficClass(datagram->getTrafficClass());
    controlInfo->setHopLimit(datagram->getHopLimit());
    controlInfo->setInterfaceId(fromIE ? fromIE->getInterfaceId() : -1);

    // original IP datagram might be needed in upper layers to send back ICMP error message
    controlInfo->setOrigDatagram(datagram);

    // attach control info
    packet->setControlInfo(controlInfo);

    return packet;
}

IPv6Datagram *IPv6::encapsulate(cPacket *transportPacket, IPv6ControlInfo *controlInfo)
{
    IPv6Datagram *datagram = new IPv6Datagram(transportPacket->getName());

    // set source and destination address
    IPv6Address dest = controlInfo->getDestAddr();
    datagram->setDestAddress(dest);

    IPv6Address src = controlInfo->getSrcAddr();

    // when source address was given, use it; otherwise it'll get the address
    // of the outgoing interface after routing
    if (!src.isUnspecified()) {
        // if interface parameter does not match existing interface, do not send datagram
        if (rt->getInterfaceByAddress(src) == nullptr) {
            delete datagram;
            delete controlInfo;
#ifndef WITH_xMIPv6
            throw cRuntimeError("Wrong source address %s in (%s)%s: no interface with such address",
                    src.str().c_str(), transportPacket->getClassName(), transportPacket->getFullName());
#else /* WITH_xMIPv6 */
            return nullptr;
#endif /* WITH_xMIPv6 */
        }
        datagram->setSrcAddress(src);
    }

    // set other fields
    datagram->setTrafficClass(controlInfo->getTrafficClass());
    datagram->setHopLimit(controlInfo->getHopLimit() > 0 ? controlInfo->getHopLimit() : 32);    //FIXME use iface hop limit instead of 32?
    datagram->setTransportProtocol(controlInfo->getProtocol());

    // #### Move extension headers from ctrlInfo to datagram if present
    while (0 < controlInfo->getExtensionHeaderArraySize()) {
        IPv6ExtensionHeader *extHeader = controlInfo->removeFirstExtensionHeader();
        datagram->addExtensionHeader(extHeader);
        // EV << "Move extension header to datagram." << endl;
    }

    datagram->setByteLength(datagram->calculateHeaderByteLength());
    datagram->encapsulate(transportPacket);

    // setting IP options is currently not supported

    return datagram;
}

void IPv6::fragmentAndSend(IPv6Datagram *datagram, const InterfaceEntry *ie, const MACAddress& nextHopAddr, bool fromHL)
{
    // hop counter check
    if (datagram->getHopLimit() <= 0) {
        // drop datagram, destruction responsibility in ICMP
        EV_INFO << "datagram hopLimit reached zero, sending ICMPv6_TIME_EXCEEDED\n";
        icmp->sendErrorMessage(datagram, ICMPv6_TIME_EXCEEDED, 0);    // FIXME check icmp 'code'
        return;
    }

    // ensure source address is filled
    if (fromHL && datagram->getSrcAddress().isUnspecified() &&
        !datagram->getDestAddress().isSolicitedNodeMulticastAddress())
    {
        // source address can be unspecified during DAD
        const IPv6Address& srcAddr = ie->ipv6Data()->getPreferredAddress();
        ASSERT(!srcAddr.isUnspecified());    // FIXME what if we don't have an address yet?
        datagram->setSrcAddress(srcAddr);
    #ifdef WITH_xMIPv6
        // if the datagram has a tentative address as source we have to reschedule it
        // as it can not be sent before the address' tentative status is cleared - CB
        if (ie->ipv6Data()->isTentativeAddress(srcAddr)) {
            EV_INFO << "Source address is tentative - enqueueing datagram for later resubmission." << endl;
            ScheduledDatagram *sDgram = new ScheduledDatagram(datagram, ie, nextHopAddr, fromHL);
            queue.insert(sDgram);
            return;
        }
    #endif /* WITH_xMIPv6 */
    }

    int mtu = ie->getMTU();

    // check if datagram does not require fragmentation
    if (datagram->getByteLength() <= mtu) {
        sendDatagramToOutput(datagram, ie, nextHopAddr);
        return;
    }

    // routed datagrams are not fragmented
    if (!fromHL) {
        // FIXME check for multicast datagrams, how many ICMP error should be sent
        icmp->sendErrorMessage(datagram, ICMPv6_PACKET_TOO_BIG, 0);    // TODO set MTU
        return;
    }

    // create and send fragments
    int headerLength = datagram->calculateUnfragmentableHeaderByteLength();
    int payloadLength = datagram->getByteLength() - headerLength;
    int fragmentLength = ((mtu - headerLength - IPv6_FRAGMENT_HEADER_LENGTH) / 8) * 8;
    ASSERT(fragmentLength > 0);

    int noOfFragments = (payloadLength + fragmentLength - 1) / fragmentLength;
    EV_INFO << "Breaking datagram into " << noOfFragments << " fragments\n";
    std::string fragMsgName = datagram->getName();
    fragMsgName += "-frag";

    unsigned int identification = curFragmentId++;
    cPacket *encapsulatedPacket = datagram->decapsulate();
    for (int offset = 0; offset < payloadLength; offset += fragmentLength) {
        bool lastFragment = (offset + fragmentLength >= payloadLength);
        int thisFragmentLength = lastFragment ? payloadLength - offset : fragmentLength;

        IPv6FragmentHeader *fh = new IPv6FragmentHeader();
        fh->setIdentification(identification);
        fh->setFragmentOffset(offset);
        fh->setMoreFragments(!lastFragment);

        IPv6Datagram *fragment = datagram->dup();
        if (offset == 0)
            fragment->encapsulate(encapsulatedPacket);
        fragment->setName(fragMsgName.c_str());
        fragment->addExtensionHeader(fh);
        fragment->setByteLength(headerLength + fh->getByteLength() + thisFragmentLength);

        sendDatagramToOutput(fragment, ie, nextHopAddr);
    }

    delete datagram;
}

void IPv6::sendDatagramToOutput(IPv6Datagram *datagram, const InterfaceEntry *destIE, const MACAddress& macAddr)
{
    // if link layer uses MAC addresses (basically, not PPP), add control info
    if (!macAddr.isUnspecified()) {
        Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
        controlInfo->setDest(macAddr);
        controlInfo->setEtherType(ETHERTYPE_IPv6);
        datagram->setControlInfo(controlInfo);
    }

    // send datagram to link layer
    send(datagram, "queueOut", destIE->getNetworkLayerGateIndex());
}

bool IPv6::determineOutputInterface(const IPv6Address& destAddress, IPv6Address& nextHop,
        int& interfaceId, IPv6Datagram *datagram, bool fromHL)
{
    // try destination cache
    nextHop = rt->lookupDestCache(destAddress, interfaceId);

    if (interfaceId == -1) {
        // address not in destination cache: do longest prefix match in routing table
        EV_INFO << "do longest prefix match in routing table" << endl;
        const IPv6Route *route = rt->doLongestPrefixMatch(destAddress);
        EV_INFO << "finished longest prefix match in routing table" << endl;
        if (!route) {
            if (rt->isRouter()) {
                EV_INFO << "unroutable, sending ICMPv6_DESTINATION_UNREACHABLE\n";
                numUnroutable++;
                icmp->sendErrorMessage(datagram, ICMPv6_DESTINATION_UNREACHABLE, 0);    // FIXME check ICMP 'code'
            }
            else {    // host
                EV_INFO << "no match in routing table, passing datagram to Neighbour Discovery module for default router selection\n";
                IPv6NDControlInfo *ctrl = new IPv6NDControlInfo();
                ctrl->setFromHL(fromHL);
                ctrl->setNextHop(nextHop);
                ctrl->setInterfaceId(interfaceId);
                datagram->setControlInfo(ctrl);
                send(datagram, "ndOut");
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
bool IPv6::processExtensionHeaders(IPv6Datagram *datagram)
{
    int noExtHeaders = datagram->getExtensionHeaderArraySize();
    EV_INFO << noExtHeaders << " extension header(s) for processing..." << endl;

    // walk through all extension headers
    for (int i = 0; i < noExtHeaders; i++) {
        IPv6ExtensionHeader *eh = datagram->removeFirstExtensionHeader();

        if (dynamic_cast<IPv6RoutingHeader *>(eh)) {
            IPv6RoutingHeader *rh = (IPv6RoutingHeader *)(eh);
            EV_DETAIL << "Routing Header with type=" << rh->getRoutingType() << endl;

            // type 2 routing header should be processed by MIPv6 module
            // if no MIP support, ignore the header
            if (rt->hasMIPv6Support() && rh->getRoutingType() == 2) {
                // for simplicity, we set a context pointer on the datagram
                datagram->setContextPointer(rh);
                EV_INFO << "Sending datagram with RH2 to MIPv6 module" << endl;
                send(datagram, "xMIPv6Out");
                return false;
            }
            else {
                delete rh;
                EV_INFO << "Ignoring unknown routing header" << endl;
            }
        }
        else if (dynamic_cast<IPv6DestinationOptionsHeader *>(eh)) {
            //IPv6DestinationOptionsHeader* doh = (IPv6DestinationOptionsHeader*) (eh);
            //EV << "object of type=" << typeid(eh).name() << endl;

            if (rt->hasMIPv6Support() && dynamic_cast<HomeAddressOption *>(eh)) {
                datagram->setContextPointer(eh);
                EV_INFO << "Sending datagram with HoA Option to MIPv6 module" << endl;
                send(datagram, "xMIPv6Out");
                return false;
            }
            else {
                delete eh;
                EV_INFO << "Ignoring unknown destination options header" << endl;
            }
        }
        else {
            delete eh;
            EV_INFO << "Ignoring unknown extension header" << endl;
        }
    }

    // we have processed no extension headers -> the IPv6 module can continue
    // working on this datagram
    return true;
}

#endif /* WITH_xMIPv6 */

bool IPv6::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    throw cRuntimeError("Lifecycle operation support not implemented");
}

// NetFilter:
void IPv6::registerHook(int priority, INetfilter::IHook *hook)
{
    Enter_Method("registerHook()");
    hooks.insert(std::pair<int, INetfilter::IHook *>(priority, hook));
}

void IPv6::unregisterHook(int priority, INetfilter::IHook *hook)
{
    Enter_Method("unregisterHook()");
    for (auto iter = hooks.begin(); iter != hooks.end(); iter++) {
        if ((iter->first == priority) && (iter->second == hook)) {
            hooks.erase(iter);
            return;
        }
    }
}

void IPv6::dropQueuedDatagram(const INetworkDatagram *datagram)
{
    Enter_Method("dropQueuedDatagram()");
    for (auto iter = queuedDatagramsForHooks.begin(); iter != queuedDatagramsForHooks.end(); iter++) {
        if (iter->datagram == datagram) {
            delete datagram;
            queuedDatagramsForHooks.erase(iter);
            return;
        }
    }
}

void IPv6::reinjectQueuedDatagram(const INetworkDatagram *datagram)
{
    Enter_Method("reinjectDatagram()");
    for (auto iter = queuedDatagramsForHooks.begin(); iter != queuedDatagramsForHooks.end(); iter++) {
        if (iter->datagram == datagram) {
            IPv6Datagram *datagram = iter->datagram;
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

INetfilter::IHook::Result IPv6::datagramPreRoutingHook(INetworkDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr)
{
    for (auto & elem : hooks) {
        IHook::Result r = elem.second->datagramPreRoutingHook(datagram, inIE, outIE, nextHopAddr);
        switch (r) {
            case INetfilter::IHook::ACCEPT:
                break;    // continue iteration

            case INetfilter::IHook::DROP:
                delete datagram;
                return r;

            case INetfilter::IHook::QUEUE:
                queuedDatagramsForHooks.push_back(QueuedDatagramForHook(dynamic_cast<IPv6Datagram *>(datagram), inIE, outIE, nextHopAddr.toIPv6(), INetfilter::IHook::PREROUTING));
                return r;

            case INetfilter::IHook::STOLEN:
                return r;

            default:
                throw cRuntimeError("Unknown Hook::Result value: %d", (int)r);
        }
    }
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result IPv6::datagramForwardHook(INetworkDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr)
{
    for (auto & elem : hooks) {
        IHook::Result r = elem.second->datagramForwardHook(datagram, inIE, outIE, nextHopAddr);
        switch (r) {
            case INetfilter::IHook::ACCEPT:
                break;    // continue iteration

            case INetfilter::IHook::DROP:
                delete datagram;
                return r;

            case INetfilter::IHook::QUEUE:
                queuedDatagramsForHooks.push_back(QueuedDatagramForHook(dynamic_cast<IPv6Datagram *>(datagram), inIE, outIE, nextHopAddr.toIPv6(), INetfilter::IHook::FORWARD));
                return r;

            case INetfilter::IHook::STOLEN:
                return r;

            default:
                throw cRuntimeError("Unknown Hook::Result value: %d", (int)r);
        }
    }
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result IPv6::datagramPostRoutingHook(INetworkDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr)
{
    for (auto & elem : hooks) {
        IHook::Result r = elem.second->datagramPostRoutingHook(datagram, inIE, outIE, nextHopAddr);
        switch (r) {
            case INetfilter::IHook::ACCEPT:
                break;    // continue iteration

            case INetfilter::IHook::DROP:
                delete datagram;
                return r;

            case INetfilter::IHook::QUEUE:
                queuedDatagramsForHooks.push_back(QueuedDatagramForHook(dynamic_cast<IPv6Datagram *>(datagram), inIE, outIE, nextHopAddr.toIPv6(), INetfilter::IHook::POSTROUTING));
                return r;

            case INetfilter::IHook::STOLEN:
                return r;

            default:
                throw cRuntimeError("Unknown Hook::Result value: %d", (int)r);
        }
    }
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result IPv6::datagramLocalInHook(INetworkDatagram *datagram, const InterfaceEntry *inIE)
{
    for (auto & elem : hooks) {
        IHook::Result r = elem.second->datagramLocalInHook(datagram, inIE);
        switch (r) {
            case INetfilter::IHook::ACCEPT:
                break;    // continue iteration

            case INetfilter::IHook::DROP:
                delete datagram;
                return r;

            case INetfilter::IHook::QUEUE:
                queuedDatagramsForHooks.push_back(QueuedDatagramForHook(dynamic_cast<IPv6Datagram *>(datagram), inIE, nullptr, IPv6Address::UNSPECIFIED_ADDRESS, INetfilter::IHook::LOCALIN));
                return r;

            case INetfilter::IHook::STOLEN:
                return r;

            default:
                throw cRuntimeError("Unknown Hook::Result value: %d", (int)r);
        }
    }
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result IPv6::datagramLocalOutHook(INetworkDatagram *datagram, const InterfaceEntry *& outIE, L3Address& nextHopAddr)
{
    for (auto & elem : hooks) {
        IHook::Result r = elem.second->datagramLocalOutHook(datagram, outIE, nextHopAddr);
        switch (r) {
            case INetfilter::IHook::ACCEPT:
                break;    // continue iteration

            case INetfilter::IHook::DROP:
                delete datagram;
                return r;

            case INetfilter::IHook::QUEUE:
                queuedDatagramsForHooks.push_back(QueuedDatagramForHook(dynamic_cast<IPv6Datagram *>(datagram), nullptr, outIE, nextHopAddr.toIPv6(), INetfilter::IHook::LOCALOUT));
                return r;

            case INetfilter::IHook::STOLEN:
                return r;

            default:
                throw cRuntimeError("Unknown Hook::Result value: %d", (int)r);
        }
    }
    return INetfilter::IHook::ACCEPT;
}

} // namespace inet

