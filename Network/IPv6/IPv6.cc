//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2005 Wei Yang, Ng
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


#include <omnetpp.h>
#include "IPv6.h"
#include "InterfaceTableAccess.h"
#include "RoutingTable6Access.h"
#include "ICMPv6Access.h"
#include "IPv6NeighbourDiscoveryAccess.h"
#include "IPv6ControlInfo_m.h"
#include "IPv6NDMessage_m.h"
#include "Ieee802Ctrl_m.h"


#define FRAGMENT_TIMEOUT 60   // 60 sec, from IPv6 RFC


Define_Module(IPv6);

void IPv6::initialize()
{
    QueueBase::initialize();

    ift = InterfaceTableAccess().get();
    rt = RoutingTable6Access().get();
    nd = IPv6NeighbourDiscoveryAccess().get();
    icmp = ICMPv6Access().get();

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
    displayString().setTagArg("t",0,buf);
}

void IPv6::endService(cMessage *msg)
{
    if (msg->arrivalGate()->isName("transportIn") ||
       (msg->arrivalGate()->isName("ndIn") && dynamic_cast<IPv6NDMessage*>(msg)) ||
       (msg->arrivalGate()->isName("icmpIn") && dynamic_cast<ICMPv6Message*>(msg)))//Added this for ICMP msgs from ICMP module-WEI
    {
        // packet from upper layers or ND: encapsulate and send out
        handleMessageFromHL(msg);
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

void IPv6::handleDatagramFromNetwork(IPv6Datagram *datagram)
{
    // check for header biterror
    if (datagram->hasBitError())
    {
        ev << "bit error\n";return; // revise!
/*FIXME revise
        // probability of bit error in header = size of header / size of total message
        // (ignore bit error if in payload)
        double relativeHeaderLength = datagram->headerLength() / (double)datagram->byteLength();
        if (dblrand() <= relativeHeaderLength)
        {
            ev << "bit error found, sending ICMP_PARAMETER_PROBLEM\n";
            icmp->sendErrorMessage(datagram, ICMP_PARAMETER_PROBLEM, 0);
            return;
        }
*/
    }

    // remove control info
    delete datagram->removeControlInfo();

    // routepacket
    if (!datagram->destAddress().isMulticast())
        routePacket(datagram);
    else
        routeMulticastPacket(datagram);
}

void IPv6::handleMessageFromHL(cMessage *msg)
{
    // if no interface exists, do not send datagram
    if (ift->numInterfaces() == 0)
    {
        ev << "No interfaces exist, dropping packet\n";
        delete msg;
        return;
    }

    IPv6ControlInfo *controlInfo = check_and_cast<IPv6ControlInfo*>(msg->controlInfo());
    int optOutputGateIndex = controlInfo->inputGateIndex(); // FIXME looks a bit silly

    // encapsulate upper-layer packet into IPv6Datagram
    IPv6Datagram *datagram = encapsulate(msg);

    // possibly fragment (in IPv6, only the source node does that), then route it
    fragmentAndRoute(datagram, optOutputGateIndex);
}

void IPv6::fragmentAndRoute(IPv6Datagram *datagram, int optOutputGateIndex)
{
/*
FIXME implement fragmentation here.
   1. determine output interface
   2. compare packet size with interface MTU
   3. if bigger, do fragmentation
         int mtu = ift->interfaceByPortNo(outputGateIndex)->mtu();
*/
    ev << "fragmentation not implemented yet\n";

    // route packet
    if (optOutputGateIndex!=-1)
        sendToGateIndex(datagram, optOutputGateIndex);
    else if (!datagram->destAddress().isMulticast())
        routePacket(datagram);
    else
        routeMulticastPacket(datagram);
}

void IPv6::sendToGateIndex(IPv6Datagram *datagram, int outputGateIndex)
{
    sendDatagramToOutput(datagram, outputGateIndex, MACAddress::BROADCAST_ADDRESS); // FIXME what MAC address to use?
}

void IPv6::routePacket(IPv6Datagram *datagram)
{
    // TBD add option handling code here
    IPv6Address destAddress = datagram->destAddress();

    ev << "Routing datagram `" << datagram->name() << "' with dest=" << destAddress << ": ";

    // local delivery of unicast packets
    if (rt->localDeliver(destAddress))
    {
        ev << "local delivery\n";
        numLocalDeliver++;
        localDeliver(datagram);
        return;
    }

    int inputGateIndex = datagram->arrivalGate() ? datagram->arrivalGate()->index() : -1;
    ev << "Input Gate Index: " << inputGateIndex << endl;
    if (inputGateIndex!=-1)
    {
        // if datagram arrived from input gate and IP forwarding is off, delete datagram
        //yes but datagrams from the ND module is getting dropped too!-WEI
        //so we add a 2nd condition
        // FIXME rewrite code so that condition is cleaner --Andras
        //if (!rt->isRouter())
        if (!rt->isRouter() && !(datagram->arrivalGate()->isName("ndIn")))
        {
            ev << "forwarding is off, dropping packet\n";
            numDropped++;
            delete datagram;
            return;
        }

        // don't forward link-local addresses or weaker
        if (destAddress.isLinkLocal() || destAddress.isLoopback())
        {
            ev << "dest address is link-local (or weaker) scope, doesn't get forwarded\n";
            delete datagram;
            return;
        }

        // hop counter decrement: only if datagram arrived from network, and will be
        // sent out to the network (hoplimit check will be done just before sending
        // out datagram)
        // TBD: in IPv4, arrange TTL check like this
        datagram->setHopLimit(datagram->hopLimit()-1);
    }

    // routing
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
                ev << "unroutable, sending ICMPv6_DESTINATION_UNREACHABLE\n";
                numUnroutable++;
                icmp->sendErrorMessage(datagram, ICMPv6_DESTINATION_UNREACHABLE, 0); // FIXME check ICMP 'code'
            }
            else // host
            {
                ev << "no match in routing table, passing datagram to Neighbour Discovery module for default router selection\n";
                send(datagram, "ndOut");
            }
            return;
        }
        interfaceId = route->interfaceID();
        nextHop = route->nextHop();
        if (nextHop.isUnspecified())
            nextHop = destAddress;  // next hop is the host itself

        // add result into destination cache
        rt->updateDestCache(destAddress, nextHop, interfaceId);
    }
    ev << "next hop for " << destAddress << " is " << nextHop << ", interface " << interfaceId << "\n";
    ASSERT(!nextHop.isUnspecified() && interfaceId!=-1);

    MACAddress macAddr = nd->resolveNeighbour(nextHop, interfaceId);
    if (macAddr.isUnspecified())
    {
        ev << "no link-layer address for next hop yet, passing datagram to Neighbour Discovery module\n";
        send(datagram, "ndOut");
        return;
    }
    ev << "link-layer address: " << macAddr << "\n";

    // set datagram source address if not yet set
    if (datagram->srcAddress().isUnspecified())
    {
        const IPv6Address& srcAddr = ift->interfaceAt(interfaceId)->ipv6()->preferredAddress();
        ASSERT(!srcAddr.isUnspecified()); // FIXME what if we don't have an address yet?
        datagram->setSrcAddress(srcAddr);
    }

    // send out datagram
    numForwarded++;
    int outputGateIndex = ift->interfaceAt(interfaceId)->outputPort();
    sendDatagramToOutput(datagram, outputGateIndex, macAddr);
}

void IPv6::routeMulticastPacket(IPv6Datagram *datagram)
{
    const IPv6Address& destAddr = datagram->destAddress();

    ev << "destination address " << destAddr << " is multicast, doing multicast routing\n";
    numMulticast++;

    // if received from the network...
    int inputGateIndex = datagram->arrivalGate() ? datagram->arrivalGate()->index() : -1;
    if (inputGateIndex!=-1)
    {
        // deliver locally
        if (rt->localDeliver(destAddr))
        {
            ev << "local delivery of multicast packet\n";
            numLocalDeliver++;
            localDeliver((IPv6Datagram *)datagram->dup());
        }

        // if datagram arrived from input gate and IP forwarding is off, delete datagram
        if (!rt->isRouter())
        {
            ev << "forwarding is off\n";
            delete datagram;
            return;
        }

        // make sure scope of multicast address is large enough to be forwarded to other links
        if (destAddr.multicastScope()<=2)
        {
            ev << "multicast dest address is link-local (or smaller) scope\n";
            delete datagram;
            return;
        }

        // hop counter decrement: only if datagram arrived from network, and will be
        // sent out to the network (hoplimit check will be done just before sending
        // out datagram)
        // TBD: in IPv4, arrange TTL check like this
        datagram->setHopLimit(datagram->hopLimit()-1);
    }

    // for now, we just send it out on every interface except on which it came. FIXME better!!!
    ev << "sending out datagram on every interface (except incoming one)\n";
    for (int i=0; i<ift->numInterfaces(); i++)
    {
        int outputGateIndex = ift->interfaceAt(i)->outputPort();
        if (inputGateIndex!=outputGateIndex)
            sendDatagramToOutput((IPv6Datagram *)datagram->dup(), outputGateIndex, MACAddress::BROADCAST_ADDRESS);
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
    int inputGateIndex = datagram->arrivalGate() ? datagram->arrivalGate()->index() : -1;
    int shortestPathInputGateIndex = rt->outputGateIndexNo(datagram->srcAddress());
    if (inputGateIndex!=-1 && shortestPathInputGateIndex!=-1 && inputGateIndex!=shortestPathInputGateIndex)
    {
        // FIXME count dropped
        ev << "Packet dropped.\n";
        delete datagram;
        return;
    }

    // check for local delivery
    IPv6Address destAddress = datagram->destAddress();
    if (rt->multicastLocalDeliver(destAddress))
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

    MulticastRoutes routes = rt->multicastRoutesFor(destAddress);
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
                if (datagramCopy->srcAddress().isUnspecified())
                    datagramCopy->setSrcAddress(ift->interfaceByPortNo(outputGateIndex)->ipv6()->inetAddress());

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
    if (datagram->fragmentOffset()!=0 || datagram->moreFragments())
    {
        ev << "Datagram fragment: offset=" << datagram->fragmentOffset()
           << ", MORE=" << (datagram->moreFragments() ? "true" : "false") << ".\n";

        // erase timed out fragments in fragmentation buffer; check every 10 seconds max
        if (simTime() >= lastCheckTime + 10)
        {
            lastCheckTime = simTime();
            fragbuf.purgeStaleFragments(simTime()-FRAGMENT_TIMEOUT);
        }

        datagram = fragbuf.addFragment(datagram, simTime());
        if (!datagram)
        {
            ev << "No complete datagram yet.\n";
            return;
        }
        ev << "This fragment completes the datagram.\n";
    }
*/
    // decapsulate and send on appropriate output gate
    int protocol = datagram->transportProtocol();
    cMessage *packet = decapsulate(datagram);

    if (protocol==IP_PROT_IPv6_ICMP && dynamic_cast<IPv6NDMessage*>(packet))
    {
        ev << "Neigbour Discovery packet: passing it to ND module\n";
        send(packet, "ndOut");
    }
    else if (protocol==IP_PROT_IPv6_ICMP && dynamic_cast<ICMPv6Message*>(packet))
    {
        ev << "ICMPv6 packet: passing it to ICMPv6 module\n";
        send(packet, "icmpOut");
    }//Added by WEI to forward ICMPv6 msgs to ICMPv6 module.
    else if (protocol==IP_PROT_IP || protocol==IP_PROT_IPv6)
    {
        ev << "Tunnelled IP datagram\n";
        // FIXME handle tunnelling
        error("tunnelling not yet implemented");
    }
    else
    {
        int gateindex = mapping.outputGateForProtocol(protocol);
        ev << "Protocol " << protocol << ", passing up on gate " << gateindex << "\n";
        //TODO: Indication of forward progress
        send(packet, "transportOut", gateindex);
    }
}

cMessage *IPv6::decapsulate(IPv6Datagram *datagram)
{
    cMessage *packet = datagram->decapsulate();

    IPv6ControlInfo *controlInfo = new IPv6ControlInfo();
    controlInfo->setProtocol(datagram->transportProtocol());
    controlInfo->setSrcAddr(datagram->srcAddress());
    controlInfo->setDestAddr(datagram->destAddress());
    controlInfo->setHopLimit(datagram->hopLimit());
    int inputGateIndex = datagram->arrivalGate() ? datagram->arrivalGate()->index() : -1;
    controlInfo->setInputGateIndex(inputGateIndex);
    packet->setControlInfo(controlInfo);
    delete datagram;

    return packet;
}

IPv6Datagram *IPv6::encapsulate(cMessage *transportPacket)
{
    IPv6ControlInfo *controlInfo = check_and_cast<IPv6ControlInfo*>(transportPacket->removeControlInfo());

    IPv6Datagram *datagram = new IPv6Datagram(transportPacket->name());
    datagram->setByteLength(datagram->calculateHeaderByteLength());
    datagram->encapsulate(transportPacket);

    // set source and destination address
    IPv6Address dest = controlInfo->destAddr();
    datagram->setDestAddress(dest);

    IPv6Address src = controlInfo->srcAddr();

    // when source address was given, use it; otherwise it'll get the address
    // of the outgoing interface after routing
    if (!src.isUnspecified())
    {
        // if interface parameter does not match existing interface, do not send datagram
        if (rt->interfaceByAddress(src)==NULL)
            opp_error("Wrong source address %s in (%s)%s: no interface with such address",
                      src.str().c_str(), transportPacket->className(), transportPacket->fullName());
        datagram->setSrcAddress(src);
    }

    // set other fields
    datagram->setHopLimit(controlInfo->hopLimit()>0 ? controlInfo->hopLimit() : 32); //FIXME use iface hop limit instead of 32?
    datagram->setTransportProtocol(controlInfo->protocol());
    delete controlInfo;

    // setting IP options is currently not supported

    return datagram;
}

void IPv6::sendDatagramToOutput(IPv6Datagram *datagram, int outputGateIndex, const MACAddress& macAddr)
{
    // hop counter check
    if (datagram->hopLimit() <= 0)
    {
        // drop datagram, destruction responsibility in ICMP
        ev << "datagram hopLimit reached zero, sending ICMPv6_TIME_EXCEEDED\n";
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
    send(datagram, "queueOut", outputGateIndex);
}


