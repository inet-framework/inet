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


#include <omnetpp.h>

#include "IPv6.h"

#include "InterfaceTableAccess.h"
#include "RoutingTable6Access.h"
#include "ICMPv6Access.h"
#include "IPv6NeighbourDiscoveryAccess.h"

#ifdef WITH_xMIPv6
#include "IPv6TunnelingAccess.h"
#endif /* WITH_xMIPv6 */

#include "IPv6ControlInfo.h"
#include "IPv6NDMessage_m.h"
#include "Ieee802Ctrl_m.h"
#include "ICMPv6Message_m.h"

#ifdef WITH_xMIPv6
#include "MobilityHeader.h"
#endif /* WITH_xMIPv6 */

#include "IPv6ExtensionHeaders.h"
#include "IPv6InterfaceData.h"

#define FRAGMENT_TIMEOUT 60   // 60 sec, from IPv6 RFC


Define_Module(IPv6);

void IPv6::initialize()
{
    QueueBase::initialize();

    ift = InterfaceTableAccess().get();
    rt = RoutingTable6Access().get();
    nd = IPv6NeighbourDiscoveryAccess().get();
    icmp = ICMPv6Access().get();

#ifdef WITH_xMIPv6
    tunneling = IPv6TunnelingAccess().get();
#endif /* WITH_xMIPv6 */

    mapping.parseProtocolMapping(par("protocolMapping"));

    curFragmentId = 0;
    lastCheckTime = 0;
    fragbuf.init(icmp);

    numMulticast = numLocalDeliver = numDropped = numUnroutable = numForwarded = 0;

    WATCH(numMulticast);
    WATCH(numLocalDeliver);
    WATCH(numDropped);
    WATCH(numUnroutable);
    WATCH(numForwarded);
}

void IPv6::updateDisplayString()
{
    char buf[80] = "";
    if (numForwarded>0) sprintf(buf+strlen(buf), "fwd:%d ", numForwarded);
    if (numLocalDeliver>0) sprintf(buf+strlen(buf), "up:%d ", numLocalDeliver);
    if (numMulticast>0) sprintf(buf+strlen(buf), "mcast:%d ", numMulticast);
    if (numDropped>0) sprintf(buf+strlen(buf), "DROP:%d ", numDropped);
    if (numUnroutable>0) sprintf(buf+strlen(buf), "UNROUTABLE:%d ", numUnroutable);
    getDisplayString().setTagArg("t", 0, buf);
}

void IPv6::endService(cPacket *msg)
{
#ifdef WITH_xMIPv6
    // 28.09.07 - CB
    // support for rescheduling datagrams which are supposed to be sent over
    // a tentative address.
    if (msg->isSelfMessage())
    {
        ScheduledDatagram* sDgram = check_and_cast<ScheduledDatagram*>(msg);

        // take care of datagram which was supposed to be sent over a tentative address
        if (sDgram->ie->ipv6Data()->isTentativeAddress(sDgram->datagram->getSrcAddress()))
        {
            // address is still tentative - enqueue again
            queue.insert(sDgram);
        }
        else
        {
            // address is not tentative anymore - send out datagram
            numForwarded++;
            sendDatagramToOutput(sDgram->datagram, sDgram->ie, sDgram->macAddr);
            delete sDgram;
        }
    }
    else
#endif /* WITH_xMIPv6 */

    if (msg->getArrivalGate()->isName("transportIn")
#ifdef WITH_xMIPv6
            || (msg->getArrivalGate()->isName("upperTunnelingIn")) // for tunneling support-CB
#endif /* WITH_xMIPv6 */
            || (msg->getArrivalGate()->isName("ndIn") && dynamic_cast<IPv6NDMessage*>(msg))
            || (msg->getArrivalGate()->isName("icmpIn") && dynamic_cast<ICMPv6Message*>(msg)) //Added this for ICMP msgs from ICMP module-WEI
#ifdef WITH_xMIPv6
            || (msg->getArrivalGate()->isName("xMIPv6In") && dynamic_cast<MobilityHeader*>(msg)) // Zarrar
#endif /* WITH_xMIPv6 */
       )
    {
#ifndef WITH_xMIPv6
        // packet from upper layers or ND: encapsulate and send out
#else /* WITH_xMIPv6 */
        // packet from upper layers, tunnel link-layer output or ND: encapsulate and send out
#endif /* WITH_xMIPv6 */

        handleMessageFromHL( msg );
    }
    else
    {
        // datagram from network or from ND: localDeliver and/or route
        IPv6Datagram *dgram = check_and_cast<IPv6Datagram *>(msg);
        handleDatagramFromNetwork(dgram);
    }

    if (ev.isGUI())
        updateDisplayString();
}

InterfaceEntry *IPv6::getSourceInterfaceFrom(cPacket *msg)
{
    cGate *g = msg->getArrivalGate();
    return g ? ift->getInterfaceByNetworkLayerGateIndex(g->getIndex()) : NULL;
}

void IPv6::handleDatagramFromNetwork(IPv6Datagram *datagram)
{
    // check for header biterror
    if (datagram->hasBitError())
    {
        EV << "bit error\n"; return; // revise!
/*FIXME revise
        // probability of bit error in header = size of header / size of total message
        // (ignore bit error if in payload)
        double relativeHeaderLength = datagram->getHeaderLength() / (double)datagram->getByteLength();
        if (dblrand() <= relativeHeaderLength)
        {
            EV << "bit error found, sending ICMP_PARAMETER_PROBLEM\n";
            icmp->sendErrorMessage(datagram, ICMP_PARAMETER_PROBLEM, 0);
            return;
        }
*/
    }

    // remove control info
    delete datagram->removeControlInfo();

    // routepacket
    if (!datagram->getDestAddress().isMulticast())
        routePacket(datagram, NULL, false);
    else
        routeMulticastPacket(datagram, NULL, getSourceInterfaceFrom(datagram));
}

void IPv6::handleMessageFromHL(cPacket *msg)
{
    // if no interface exists, do not send datagram
    if (ift->getNumInterfaces() == 0)
    {
        EV << "No interfaces exist, dropping packet\n";
        delete msg;
        return;
    }

    // encapsulate upper-layer packet into IPv6Datagram
    InterfaceEntry *destIE; // to be filled in by encapsulate()
    IPv6Datagram *datagram = encapsulate(msg, destIE);

#ifdef WITH_xMIPv6
    if (datagram == NULL)
    {
        EV << "Encapsulation failed - dropping packet." << endl;
        delete msg;
        return;
    }
#endif /* WITH_xMIPv6 */

    // possibly fragment (in IPv6, only the source node does that), then route it
    fragmentAndRoute(datagram, destIE);
}

void IPv6::fragmentAndRoute(IPv6Datagram *datagram, InterfaceEntry *destIE)
{
/*
FIXME implement fragmentation here.
   1. determine output interface
   2. compare packet size with interface MTU
   3. if bigger, do fragmentation
         int mtu = ift->interfaceByPortNo(outputGateIndex)->getMTU();
*/
    EV << "fragmentation not implemented yet\n";

    // route packet
    if (destIE != NULL)
        sendDatagramToOutput(datagram, destIE, MACAddress::BROADCAST_ADDRESS); // FIXME what MAC address to use?
    else if (!datagram->getDestAddress().isMulticast())
        routePacket(datagram, destIE, true);
    else
        routeMulticastPacket(datagram, destIE, NULL);
}

void IPv6::routePacket(IPv6Datagram *datagram, InterfaceEntry *destIE, bool fromHL)
{
    // TBD add option handling code here
    IPv6Address destAddress = datagram->getDestAddress();

#ifndef WITH_xMIPv6
    EV << "Routing datagram `" << datagram->getName() << "' with dest=" << destAddress << ": ";
#else /* WITH_xMIPv6 */
    EV << "Routing datagram '" << datagram->getName() << "' with dest=" << destAddress << ":\n";
#endif /* WITH_xMIPv6 */

    // local delivery of unicast packets
    if (rt->isLocalAddress(destAddress))
    {
        EV << "local delivery\n";

        if (datagram->getSrcAddress().isUnspecified())
            datagram->setSrcAddress(destAddress); // allows two apps on the same host to communicate

        numLocalDeliver++;
        localDeliver(datagram);
        return;
    }

    if (!fromHL)
    {
        // if datagram arrived from input gate and IP forwarding is off, delete datagram
        //yes but datagrams from the ND module is getting dropped too!-WEI
        //so we add a 2nd condition
        // FIXME rewrite code so that condition is cleaner --Andras
        //if (!rt->isRouter())
        if (!rt->isRouter() && !(datagram->getArrivalGate()->isName("ndIn")))
        {
            EV << "forwarding is off, dropping packet\n";
            numDropped++;
            delete datagram;
            return;
        }

        // don't forward link-local addresses or weaker
        if (destAddress.isLinkLocal() || destAddress.isLoopback())
        {
            EV << "dest address is link-local (or weaker) scope, doesn't get forwarded\n";
            delete datagram;
            return;
        }

        // hop counter decrement: only if datagram arrived from network, and will be
        // sent out to the network (hoplimit check will be done just before sending
        // out datagram)
        // TBD: in IPv4, arrange TTL check like this
        datagram->setHopLimit(datagram->getHopLimit()-1);
    }

    // routing
#ifndef WITH_xMIPv6
    // first try destination cache
    int interfaceId;
    IPv6Address nextHop = rt->lookupDestCache(destAddress, interfaceId);
    if (interfaceId==-1)
    {
        // address not in destination cache: do longest prefix match in routing table
        const IPv6Route *route = rt->doLongestPrefixMatch(destAddress);
        if (!route)
        {
            if (rt->isRouter())
            {
                EV << "unroutable, sending ICMPv6_DESTINATION_UNREACHABLE\n";
                numUnroutable++;
                icmp->sendErrorMessage(datagram, ICMPv6_DESTINATION_UNREACHABLE, 0); // FIXME check ICMP 'code'
            }
            else // host
            {
                EV << "no match in routing table, passing datagram to Neighbour Discovery module for default router selection\n";
                send(datagram, "ndOut");
            }
            return;
        }
        interfaceId = route->getInterfaceId();
        nextHop = route->getNextHop();
        if (nextHop.isUnspecified())
            nextHop = destAddress;  // next hop is the host itself

        // add result into destination cache
        rt->updateDestCache(destAddress, nextHop, interfaceId);
    }
#else /* WITH_xMIPv6 */
    int interfaceId = -1;
    IPv6Address nextHop;

    // restructured code from below due for mobility - CB

    // tunneling support - CB
    // check if destination is covered by tunnel lists
    if ((datagram->getTransportProtocol() != IP_PROT_IPv6) && // if datagram was already tunneled, don't tunnel again
         (datagram->getExtensionHeaderArraySize() == 0) && // we do not already have extension headers - FIXME: check for RH2 existence
          ((rt->isMobileNode() && rt->isHomeAddress( datagram->getSrcAddress())) || // for MNs: only if source address is a HoA // 27.08.07 - CB
             rt->isHomeAgent() || // but always check for tunnel if node is a HA
             !rt->isMobileNode() // or if it is a correspondent or non-MIP node
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
    //else
        //interfaceId = -1;

    if (interfaceId > ift->getNumInterfaces())
    {
        // a virtual tunnel interface provides a path to the destination: do tunneling
        EV << "tunneling: src addr=" << datagram->getSrcAddress() << ", dest addr=" << destAddress << std::endl;
        //EV << "sending datagram to encapsulation..." << endl;
        send(datagram, "lowerTunnelingOut");
        return;
    }

    if (interfaceId == -1)
        if ( !determineOutputInterface(destAddress, nextHop, interfaceId, datagram) )
            // no interface found; sent to ND or to ICMP for error processing
            //opp_error("No interface found!");//return;
            return; // don't raise error if sent to ND or ICMP!
#endif /* WITH_xMIPv6 */

    InterfaceEntry *ie = ift->getInterfaceById(interfaceId);
    ASSERT(ie!=NULL);
    EV << "next hop for " << destAddress << " is " << nextHop << ", interface " << ie->getName() << "\n";

#ifndef WITH_xMIPv6
    ASSERT(!nextHop.isUnspecified());
#else /* WITH_xMIPv6 */
    ASSERT(!nextHop.isUnspecified() && ie!=NULL);
#endif /* WITH_xMIPv6 */

#ifdef WITH_xMIPv6
     if (rt->isMobileNode())
     {
          // if the source address is the HoA and we have a CoA then drop the packet
          // (address is topologically incorrect!)
         if (datagram->getSrcAddress() == ie->ipv6Data()->getMNHomeAddress()
                 && !ie->ipv6Data()->getGlobalAddress(IPv6InterfaceData::CoA).isUnspecified())
         {
              EV << "Using HoA instead of CoA... dropping datagram" << endl;
             delete datagram;
             numDropped++;
             return;
         }
     }
#endif /* WITH_xMIPv6 */

    MACAddress macAddr = nd->resolveNeighbour(nextHop, interfaceId);
    if (macAddr.isUnspecified())
    {
        EV << "no link-layer address for next hop yet, passing datagram to Neighbour Discovery module\n";
        send(datagram, "ndOut");
        return;
    }
    EV << "link-layer address: " << macAddr << "\n";

    // set datagram source address if not yet set
    if (datagram->getSrcAddress().isUnspecified())
    {
        const IPv6Address& srcAddr = ie->ipv6Data()->getPreferredAddress();
        ASSERT(!srcAddr.isUnspecified()); // FIXME what if we don't have an address yet?
        datagram->setSrcAddress(srcAddr);

#ifdef WITH_xMIPv6
        // if the datagram has a tentative address as source we have to reschedule it
        // as it can not be sent before the address' tentative status is cleared - CB
        if (ie->ipv6Data()->isTentativeAddress(srcAddr))
        {
            EV << "Source address is tentative - enqueueing datagram for later resubmission." << endl;
            ScheduledDatagram* sDgram = new ScheduledDatagram();
            sDgram->datagram = datagram;
            sDgram->ie = ie;
            sDgram->macAddr = macAddr;
            queue.insert(sDgram);
            return;
        }
#endif /* WITH_xMIPv6 */

    }

    // send out datagram
    numForwarded++;
    sendDatagramToOutput(datagram, ie, macAddr);
}

void IPv6::routeMulticastPacket(IPv6Datagram *datagram, InterfaceEntry *destIE, InterfaceEntry *fromIE)
{
    const IPv6Address& destAddr = datagram->getDestAddress();

    EV << "destination address " << destAddr << " is multicast, doing multicast routing\n";
    numMulticast++;

    // if received from the network...
    if (fromIE != NULL)
    {
        // deliver locally
        if (rt->isLocalAddress(destAddr))
        {
            EV << "local delivery of multicast packet\n";
            numLocalDeliver++;
            localDeliver((IPv6Datagram *)datagram->dup());
        }

        // if datagram arrived from input gate and IP forwarding is off, delete datagram
        if (!rt->isRouter())
        {
            EV << "forwarding is off\n";
            delete datagram;
            return;
        }

        // make sure scope of multicast address is large enough to be forwarded to other links
        if (destAddr.getMulticastScope()<=2)
        {
            EV << "multicast dest address is link-local (or smaller) scope\n";
            delete datagram;
            return;
        }

        // hop counter decrement: only if datagram arrived from network, and will be
        // sent out to the network (hoplimit check will be done just before sending
        // out datagram)
        // TBD: in IPv4, arrange TTL check like this
        datagram->setHopLimit(datagram->getHopLimit()-1);
    }

    // for now, we just send it out on every interface except on which it came. FIXME better!!!
    EV << "sending out datagram on every interface (except incoming one)\n";
    for (int i=0; i < ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (fromIE != ie)
            sendDatagramToOutput((IPv6Datagram *)datagram->dup(), ie, MACAddress::BROADCAST_ADDRESS);
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
                sendDatagramToOutput(datagramCopy, outputGateIndex, macAddr);
            }
        }

        // only copies sent, delete original datagram
        delete datagram;
    }
*/
}

void IPv6::localDeliver(IPv6Datagram *datagram)
{
/* FIXME revise and complete defragmentation
    // Defragmentation. skip defragmentation if datagram is not fragmented
    if (datagram->getFragmentOffset()!=0 || datagram->getMoreFragments())
    {
        EV << "Datagram fragment: offset=" << datagram->getFragmentOffset()
           << ", MORE=" << (datagram->getMoreFragments() ? "true" : "false") << ".\n";

        // erase timed out fragments in fragmentation buffer; check every 10 seconds max
        if (simTime() >= lastCheckTime + 10)
        {
            lastCheckTime = simTime();
            fragbuf.purgeStaleFragments(simTime()-FRAGMENT_TIMEOUT);
        }

        datagram = fragbuf.addFragment(datagram, simTime());
        if (!datagram)
        {
            EV << "No complete datagram yet.\n";
            return;
        }
        EV << "This fragment completes the datagram.\n";
    }
*/

#ifdef WITH_xMIPv6
    // #### 29.08.07 - CB
    // check for extension headers
    if (!processExtensionHeaders(datagram))
    {
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

    if (protocol == IP_PROT_IPv6_ICMP && dynamic_cast<IPv6NDMessage*>(packet))
    {
        EV << "Neigbour Discovery packet: passing it to ND module\n";
        send(packet, "ndOut");
    }
#ifdef WITH_xMIPv6
    else if (protocol == IP_PROT_IPv6EXT_MOB && dynamic_cast<MobilityHeader*>(packet))
    {
        // added check for MIPv6 support to prevent nodes w/o the
        // xMIP module from processing related messages, 4.9.07 - CB
        if (rt->hasMIPv6Support())
        {
            EV << "MIPv6 packet: passing it to xMIPv6 module\n";
            send(check_and_cast<MobilityHeader*>(packet), "xMIPv6Out");
        }
        else
        {
            // update 12.9.07 - CB
            /*RFC3775, 11.3.5
              Any node that does not recognize the Mobility header will return an
              ICMP Parameter Problem, Code 1, message to the sender of the packet*/
            EV << "No MIPv6 support on this node!\n";
            IPv6ControlInfo *ctrlInfo = check_and_cast<IPv6ControlInfo*>(packet->removeControlInfo());
            icmp->sendErrorMessage(packet, ctrlInfo, ICMPv6_PARAMETER_PROBLEM, 1);

            //delete packet; // 13.9.07 - CB, update 21.9.07 - CB
        }
    }
#endif /* WITH_xMIPv6 */
    else if (protocol == IP_PROT_IPv6_ICMP && dynamic_cast<ICMPv6Message*>(packet))
    {
        EV << "ICMPv6 packet: passing it to ICMPv6 module\n";
        send(packet, "icmpOut");
    }//Added by WEI to forward ICMPv6 msgs to ICMPv6 module.
    else if (protocol == IP_PROT_IP || protocol == IP_PROT_IPv6)
    {
        EV << "Tunnelled IP datagram\n";

#ifndef WITH_xMIPv6
        // FIXME handle tunnelling
        error("tunnelling not yet implemented");
#else /* WITH_xMIPv6 */
        send(packet, "upperTunnelingOut");
#endif /* WITH_xMIPv6 */
    }
    else
    {
        int gateindex = mapping.getOutputGateForProtocol(protocol);

#ifndef WITH_xMIPv6
        EV << "Protocol " << protocol << ", passing up on gate " << gateindex << "\n";
        //TODO: Indication of forward progress
        send(packet, "transportOut", gateindex);
#else /* WITH_xMIPv6 */
        // 21.9.07 - CB
        cGate* outGate = gate("transportOut", gateindex);
        if (!outGate->isConnected())
        {
            EV << "Transport layer gate not connected - dropping packet!\n";
            delete packet;
        }
        else
        {
            EV << "Protocol " << protocol << ", passing up on gate " << gateindex << "\n";
            //TODO: Indication of forward progress
            send(packet, outGate);
        }
#endif /* WITH_xMIPv6 */

    }
}

void IPv6::handleReceivedICMP(ICMPv6Message *msg)
{
    switch (msg->getType())
    {
        case ICMPv6_REDIRECT:  // TODO implement redirect handling
        case ICMPv6_DESTINATION_UNREACHABLE:
        case ICMPv6_PACKET_TOO_BIG:
        case ICMPv6_TIME_EXCEEDED:
        case ICMPv6_PARAMETER_PROBLEM: {
            // ICMP errors are delivered to the appropriate higher layer protocols
            IPv6Datagram *bogusPacket = check_and_cast<IPv6Datagram *>(msg->getEncapsulatedPacket());
            int protocol = bogusPacket->getTransportProtocol();
            int gateindex = mapping.getOutputGateForProtocol(protocol);
            send(msg, "transportOut", gateindex);
            break;
        }
        default: {
            // all others are delivered to ICMP:
            // ICMPv6_ECHO_REQUEST, ICMPv6_ECHO_REPLY, ICMPv6_MLD_QUERY, ICMPv6_MLD_REPORT,
            // ICMPv6_MLD_DONE, ICMPv6_ROUTER_SOL, ICMPv6_ROUTER_AD, ICMPv6_NEIGHBOUR_SOL,
            // ICMPv6_NEIGHBOUR_AD, ICMPv6_MLDv2_REPORT
            int gateindex = mapping.getOutputGateForProtocol(IP_PROT_ICMP);
            send(msg, "transportOut", gateindex);
        }
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
    controlInfo->setHopLimit(datagram->getHopLimit());
    controlInfo->setInterfaceId(fromIE ? fromIE->getInterfaceId() : -1);

    // original IP datagram might be needed in upper layers to send back ICMP error message
    controlInfo->setOrigDatagram(datagram);

    // attach control info
    packet->setControlInfo(controlInfo);

    return packet;
}

IPv6Datagram *IPv6::encapsulate(cPacket *transportPacket, InterfaceEntry *&destIE)
{
    IPv6ControlInfo *controlInfo = check_and_cast<IPv6ControlInfo*>(transportPacket->removeControlInfo());

    IPv6Datagram *datagram = new IPv6Datagram(transportPacket->getName());

    // IPV6_MULTICAST_IF option, but allow interface selection for unicast packets as well
    destIE = ift->getInterfaceById(controlInfo->getInterfaceId());

    // set source and destination address
    IPv6Address dest = controlInfo->getDestAddr();
    datagram->setDestAddress(dest);

    IPv6Address src = controlInfo->getSrcAddr();

    // when source address was given, use it; otherwise it'll get the address
    // of the outgoing interface after routing
    if (!src.isUnspecified())
    {
        // if interface parameter does not match existing interface, do not send datagram
        if (rt->getInterfaceByAddress(src)==NULL)
        {

#ifndef WITH_xMIPv6
            throw cRuntimeError(this, "Wrong source address %s in (%s)%s: no interface with such address",
                      src.str().c_str(), transportPacket->getClassName(), transportPacket->getFullName());
#else /* WITH_xMIPv6 */
            // throw cRuntimeError(this, "Wrong source address %s in (%s)%s: no interface with such address",
            //          src.str().c_str(), transportPacket->getClassName(), transportPacket->getFullName());
            delete datagram;
            return NULL;
#endif /* WITH_xMIPv6 */

        }
        datagram->setSrcAddress(src);
    }

    // set other fields
    datagram->setHopLimit(controlInfo->getHopLimit()>0 ? controlInfo->getHopLimit() : 32); //FIXME use iface hop limit instead of 32?
    datagram->setTransportProtocol(controlInfo->getProtocol());

    // #### Move extension headers from ctrlInfo to datagram if present
    while (0 < controlInfo->getExtensionHeaderArraySize())
    {
        IPv6ExtensionHeader* extHeader = controlInfo->removeFirstExtensionHeader();
        datagram->addExtensionHeader(extHeader);
        // EV << "Move extension header to datagram." << endl;
    }

    delete controlInfo;

    datagram->setByteLength(datagram->calculateHeaderByteLength());
    datagram->encapsulate(transportPacket);

    // setting IP options is currently not supported

    return datagram;
}

void IPv6::sendDatagramToOutput(IPv6Datagram *datagram, InterfaceEntry *ie, const MACAddress& macAddr)
{
    // hop counter check
    if (datagram->getHopLimit() <= 0)
    {
        // drop datagram, destruction responsibility in ICMP
        EV << "datagram hopLimit reached zero, sending ICMPv6_TIME_EXCEEDED\n";
        icmp->sendErrorMessage(datagram, ICMPv6_TIME_EXCEEDED, 0); // FIXME check icmp 'code'
        return;
    }

    // in link layer uses MAC addresses (basically, not PPP), add control info
    if (!macAddr.isUnspecified())
    {
        Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
        controlInfo->setDest(macAddr);
        datagram->setControlInfo(controlInfo);
    }

    // send datagram to link layer
    send(datagram, "queueOut", ie->getNetworkLayerGateIndex());
}

#ifdef WITH_xMIPv6
bool IPv6::determineOutputInterface(const IPv6Address& destAddress, IPv6Address& nextHop,
                                    int& interfaceId, IPv6Datagram* datagram)
{
    // try destination cache
    //IPv6Address nextHop = rt->lookupDestCache(destAddress, interfaceId);
    nextHop = rt->lookupDestCache(destAddress, interfaceId);

    if (interfaceId == -1)
    {
        // address not in destination cache: do longest prefix match in routing table
        EV << "do longest prefix match in routing table" << endl;
        const IPv6Route *route = rt->doLongestPrefixMatch(destAddress);
        EV << "finished longest prefix match in routing table" << endl;
        if (!route)
        {
            if (rt->isRouter())
            {
                EV << "unroutable, sending ICMPv6_DESTINATION_UNREACHABLE\n";
                numUnroutable++;
                icmp->sendErrorMessage(datagram, ICMPv6_DESTINATION_UNREACHABLE, 0); // FIXME check ICMP 'code'
            }
            else // host
            {
                EV << "no match in routing table, passing datagram to Neighbour Discovery module for default router selection\n";
                send(datagram, "ndOut");
            }
            return false;
        }
        interfaceId = route->getInterfaceId();
        nextHop = route->getNextHop();
        if (nextHop.isUnspecified())
            nextHop = destAddress;  // next hop is the host itself

        // add result into destination cache
        rt->updateDestCache(destAddress, nextHop, interfaceId);
    }

    return true;
}

bool IPv6::processExtensionHeaders(IPv6Datagram* datagram)
{
    int noExtHeaders = datagram->getExtensionHeaderArraySize();
    EV << noExtHeaders << " extension header(s) for processing..." << endl;

    // walk through all extension headers
    for (int i = 0; i < noExtHeaders; i++)
    {
        IPv6ExtensionHeader* eh = datagram->removeFirstExtensionHeader();

        if (dynamic_cast<IPv6RoutingHeader*>(eh))
        {
            IPv6RoutingHeader* rh = (IPv6RoutingHeader*) (eh);
            EV << "Routing Header with type=" << rh->getRoutingType() << endl;

            // type 2 routing header should be processed by MIPv6 module
            // if no MIP support, ignore the header
            if (rt->hasMIPv6Support() && rh->getRoutingType() == 2)
            {
                // for simplicity, we set a context pointer on the datagram
                datagram->setContextPointer(rh);
                EV << "Sending datagram with RH2 to MIPv6 module" << endl;
                send(datagram, "xMIPv6Out");
                return false;
            }
            else
            {
                delete rh;
                EV << "Ignoring unknown routing header" << endl;
            }
        }
        else if (dynamic_cast<IPv6DestinationOptionsHeader*>(eh))
        {
            //IPv6DestinationOptionsHeader* doh = (IPv6DestinationOptionsHeader*) (eh);
            //EV << "object of type=" << typeid(eh).name() << endl;

            if (rt->hasMIPv6Support() && dynamic_cast<HomeAddressOption*>(eh))
            {
                datagram->setContextPointer(eh);
                EV << "Sending datagram with HoA Option to MIPv6 module" << endl;
                send(datagram, "xMIPv6Out");
                return false;
            }
            else
            {
                delete eh;
                EV << "Ignoring unknown destination options header" << endl;
            }
        }
        else
        {
            delete eh;
            EV << "Ignoring unknown extension header" << endl;
        }
    }

    // we have processed no extension headers -> the IPv6 module can continue
    // working on this datagram
    return true;
}
#endif /* WITH_xMIPv6 */

