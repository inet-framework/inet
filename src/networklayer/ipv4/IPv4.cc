//
// Copyright (C) 2004 Andras Varga
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

#include "IPv4.h"

#include "ARPPacket_m.h"
#include "ICMPMessage_m.h"
#include "InterfaceTableAccess.h"
#include "IPv4ControlInfo.h"
#include "IPv4Datagram.h"
#include "IPv4InterfaceData.h"
#include "IRoutingTable.h"


Define_Module(IPv4);


void IPv4::initialize()
{
    QueueBase::initialize();

    ift = InterfaceTableAccess().get();
    rt = RoutingTableAccess().get();

    queueOutGate = gate("queueOut");

    defaultTimeToLive = par("timeToLive");
    defaultMCTimeToLive = par("multicastTimeToLive");
    fragmentTimeoutTime = par("fragmentTimeout");
    forceBroadcast = par("forceBroadcast");
    mapping.parseProtocolMapping(par("protocolMapping"));

    curFragmentId = 0;
    lastCheckTime = 0;
    fragbuf.init(icmpAccess.get());

    numMulticast = numLocalDeliver = numDropped = numUnroutable = numForwarded = 0;

    WATCH(numMulticast);
    WATCH(numLocalDeliver);
    WATCH(numDropped);
    WATCH(numUnroutable);
    WATCH(numForwarded);

    // by default no MANET routing
    manetRouting=false;

#ifdef WITH_MANET
    // test for the presence of MANET routing
    // check if there is a protocol -> gate mapping
    int gateindex = mapping.getOutputGateForProtocol(IP_PROT_MANET);
    if (gateSize("transportOut")-1<gateindex)
           return;

    // check if that gate is connected at all
    cGate *manetgate = gate("transportOut", gateindex)->getPathEndGate();
    if (manetgate==NULL)
           return;
    cModule *destmod = manetgate->getOwnerModule();
    if (destmod==NULL)
           return;

    // manet routing will be turned on ONLY for routing protocols which has the @reactive property set
    // this prevents performance loss with other protocols that use pro active routing and do not need
    // assistance from the IPv4 component
    cProperties *props = destmod->getProperties();
    manetRouting = props && props->getAsBool("reactive");
#endif
}

void IPv4::updateDisplayString()
{
    char buf[80] = "";
    if (numForwarded>0) sprintf(buf+strlen(buf), "fwd:%d ", numForwarded);
    if (numLocalDeliver>0) sprintf(buf+strlen(buf), "up:%d ", numLocalDeliver);
    if (numMulticast>0) sprintf(buf+strlen(buf), "mcast:%d ", numMulticast);
    if (numDropped>0) sprintf(buf+strlen(buf), "DROP:%d ", numDropped);
    if (numUnroutable>0) sprintf(buf+strlen(buf), "UNROUTABLE:%d ", numUnroutable);
    getDisplayString().setTagArg("t",0,buf);
}

void IPv4::endService(cPacket *msg)
{
    if (msg->getArrivalGate()->isName("transportIn"))
    {
        handleMessageFromHL( msg );
    }
    else if (dynamic_cast<ARPPacket *>(msg))
    {
        // dispatch ARP packets to ARP
        handleARP((ARPPacket *)msg);
    }
    else
    {
        IPv4Datagram *dgram = check_and_cast<IPv4Datagram *>(msg);
        handlePacketFromNetwork(dgram);
    }

    if (ev.isGUI())
        updateDisplayString();
}

InterfaceEntry *IPv4::getSourceInterfaceFrom(cPacket *msg)
{
    cGate *g = msg->getArrivalGate();
    return g ? ift->getInterfaceByNetworkLayerGateIndex(g->getIndex()) : NULL;
}

void IPv4::handlePacketFromNetwork(IPv4Datagram *datagram)
{
    //
    // "Prerouting"
    //

    // check for header biterror
    if (datagram->hasBitError())
    {
        // probability of bit error in header = size of header / size of total message
        // (ignore bit error if in payload)
        double relativeHeaderLength = datagram->getHeaderLength() / (double)datagram->getByteLength();
        if (dblrand() <= relativeHeaderLength)
        {
            EV << "bit error found, sending ICMP_PARAMETER_PROBLEM\n";
            icmpAccess.get()->sendErrorMessage(datagram, ICMP_PARAMETER_PROBLEM, 0);
            return;
        }
    }

    // remove control info
    if (datagram->getTransportProtocol()!=IP_PROT_DSR && datagram->getTransportProtocol()!=IP_PROT_MANET)
    {
        delete datagram->removeControlInfo();
    }
    else if (datagram->getMoreFragments())
        delete datagram->removeControlInfo(); // delete all control message except the last

    // hop counter decrement; FIXME but not if it will be locally delivered
    datagram->setTimeToLive(datagram->getTimeToLive()-1);

    // route packet
    if (!datagram->getDestAddress().isMulticast())
        routePacket(datagram, NULL, false, NULL);
    else
        routeMulticastPacket(datagram, NULL, getSourceInterfaceFrom(datagram));
}

void IPv4::handleARP(ARPPacket *msg)
{
    // FIXME hasBitError() check  missing!

    // delete old control info
    delete msg->removeControlInfo();

    // dispatch ARP packets to ARP and let it know the gate index it arrived on
    InterfaceEntry *fromIE = getSourceInterfaceFrom(msg);
    ASSERT(fromIE);

    IPv4RoutingDecision *routingDecision = new IPv4RoutingDecision();
    routingDecision->setInterfaceId(fromIE->getInterfaceId());
    msg->setControlInfo(routingDecision);

    send(msg, queueOutGate);
}

void IPv4::handleReceivedICMP(ICMPMessage *msg)
{
    switch (msg->getType())
    {
        case ICMP_REDIRECT: // TODO implement redirect handling
        case ICMP_DESTINATION_UNREACHABLE:
        case ICMP_TIME_EXCEEDED:
        case ICMP_PARAMETER_PROBLEM: {
            // ICMP errors are delivered to the appropriate higher layer protocol
            IPv4Datagram *bogusPacket = check_and_cast<IPv4Datagram *>(msg->getEncapsulatedPacket());
            int protocol = bogusPacket->getTransportProtocol();
            int gateindex = mapping.getOutputGateForProtocol(protocol);
            send(msg, "transportOut", gateindex);
            break;
        }
        default: {
            // all others are delivered to ICMP: ICMP_ECHO_REQUEST, ICMP_ECHO_REPLY,
            // ICMP_TIMESTAMP_REQUEST, ICMP_TIMESTAMP_REPLY, etc.
            int gateindex = mapping.getOutputGateForProtocol(IP_PROT_ICMP);
            send(msg, "transportOut", gateindex);
        }
    }
}

void IPv4::handleMessageFromHL(cPacket *msg)
{
    // if no interface exists, do not send datagram
    if (ift->getNumInterfaces() == 0)
    {
        EV << "No interfaces exist, dropping packet\n";
        numDropped++;
        delete msg;
        return;
    }

    // encapsulate and send
    InterfaceEntry *destIE=NULL; // will be filled in by encapsulate() or dsrFillDestIE
    IPv4Address nextHopAddress;
    IPv4Address *nextHopAddressPrt=NULL;

    // if HL send a Ipdatagram routing the packet
    if (dynamic_cast<IPv4Datagram *>(msg))
    {
        IPv4Datagram *datagram = check_and_cast  <IPv4Datagram *>(msg);
        // Dsr routing, Dsr is a HL protocol and send IPv4Datagram
        dsrFillDestIE(datagram, destIE, nextHopAddress);
        if (!nextHopAddress.isUnspecified())
            nextHopAddressPrt=&nextHopAddress;
        if (!datagram->getDestAddress().isMulticast())
            routePacket(datagram, destIE, true,nextHopAddressPrt);
        else
            routeMulticastPacket(datagram, destIE, NULL);
        return;
    }
    // encapsulate and send

    // encapsulate and send
    IPv4ControlInfo *controlInfo = check_and_cast<IPv4ControlInfo*>(msg->removeControlInfo());
    IPv4Datagram *datagram = encapsulate(msg, destIE,controlInfo);
    nextHopAddress = controlInfo->getNextHopAddr();
    delete controlInfo;
    if (!nextHopAddress.isUnspecified())
        nextHopAddressPrt=&nextHopAddress;

    // route packet
    if (!datagram->getDestAddress().isMulticast())
        routePacket(datagram, destIE, true, nextHopAddressPrt);
    else
        routeMulticastPacket(datagram, destIE, NULL);
}

void IPv4::routePacket(IPv4Datagram *datagram, InterfaceEntry *destIE, bool fromHL, IPv4Address* nextHopAddrPtr)
{
    // TBD add option handling code here

    if (datagram->getOptionCode()==IPOPTION_STRICT_SOURCE_ROUTING || datagram->getOptionCode()==IPOPTION_LOOSE_SOURCE_ROUTING)
    {
        IPv4SourceRoutingOption rtOpt = datagram->getSourceRoutingOption();
        if (rtOpt.getNextAddressPtr()<rtOpt.getLastAddressPtr())
        {
            IPv4Address nextRouteAddress = rtOpt.getRecordAddress(rtOpt.getNextAddressPtr()/4);class IInterfaceTable;
            rtOpt.setNextAddressPtr(rtOpt.getNextAddressPtr()+4);
            datagram->setSrcAddress(rt->getRouterId());
            datagram->setDestAddress(nextRouteAddress);
        }
    }

    IPv4Address destAddr = datagram->getDestAddress();

    EV << "Routing datagram `" << datagram->getName() << "' with dest=" << destAddr << ": ";

#ifdef WITH_MANET
    controlMessageToManetRouting(MANET_ROUTE_UPDATE, datagram);
#endif

    // check for local delivery
    if (rt->isLocalAddress(destAddr))
    {
        EV << "local delivery\n";
        if (datagram->getSrcAddress().isUnspecified())
            datagram->setSrcAddress(destAddr); // allows two apps on the same host to communicate
        numLocalDeliver++;
        reassembleAndDeliver(datagram);
        return;
    }
    // JcM Fix: broadcast limited address 255.255.255.255 or network broadcast, i.e. 192.168.0.255/24
    if (destAddr == IPv4Address::ALLONES_ADDRESS || rt->isLocalBroadcastAddress(destAddr))
    {
        // check if local
        if (!fromHL)
        {
            EV << "limited broadcast received \n";
            if (datagram->getSrcAddress().isUnspecified())
                datagram->setSrcAddress(destAddr); // allows two apps on the same host to communicate
            numLocalDeliver++;
            reassembleAndDeliver(datagram);
        }
        else
        { // send limited broadcast packet
            if (destIE!=NULL)
            {
                if (datagram->getSrcAddress().isUnspecified())
                     datagram->setSrcAddress(destIE->ipv4Data()->getIPAddress());
                fragmentAndSend(datagram, destIE, IPv4Address::ALLONES_ADDRESS);
            }
            else if (destIE!=NULL && forceBroadcast)
            {
                for (int i = 0;i<ift->getNumInterfaces();i++)
                {
                    InterfaceEntry *ie = ift->getInterface(i);
                    if (!ie->isLoopback())
                    {
                        IPv4Datagram * dataAux = datagram->dup();
                        if (dataAux->getSrcAddress().isUnspecified())
                             dataAux->setSrcAddress(ie->ipv4Data()->getIPAddress());
                        fragmentAndSend(datagram->dup(), ie, IPv4Address::ALLONES_ADDRESS);
                    }
                }
                delete datagram;
            }
            else
            {
                numDropped++;
                delete datagram;
            }
        }
        return;
    }

    // if datagram arrived from input gate and IP_FORWARD is off, delete datagram
    if (!fromHL && !rt->isIPForwardingEnabled())
    {
        EV << "forwarding off, dropping packet\n";
        numDropped++;
        delete datagram;
        return;
    }

    IPv4Address nextHopAddr;
    // if output port was explicitly requested, use that, otherwise use IPv4 routing
    if (destIE)
    {
        EV << "using manually specified output interface " << destIE->getName() << "\n";
        // and nextHopAddr remains unspecified
        if (manetRouting  && fromHL && nextHopAddrPtr)
           nextHopAddr = *nextHopAddrPtr;  // Manet DSR routing explicit route
        // special case ICMP reply
        else if (destIE->isBroadcast())
        {
            // if the interface is broadcast we must search the next hop
            const IPv4Route *re = rt->findBestMatchingRoute(destAddr);
            if (re!=NULL && re->getSource()== IPv4Route::MANET  && re->getHost()!=destAddr)
                re=NULL;
            if (re && destIE == re->getInterface())
                nextHopAddr = re->getGateway();
        }
    }
    else
    {
        // use IPv4 routing (lookup in routing table)
        const IPv4Route *re = rt->findBestMatchingRoute(destAddr);

        if (re!=NULL && re->getSource()== IPv4Route::MANET)
        {
           // special case the address must agree
           if (re->getHost()!=destAddr)
               re=NULL;
        }

        // error handling: destination address does not exist in routing table:
        // notify ICMP, throw packet away and continue
        if (re==NULL)
        {
#ifdef WITH_MANET
            if (manetRouting)
            {
               controlMessageToManetRouting(MANET_ROUTE_NOROUTE,datagram);
               return;
            }
#endif
            EV << "unroutable, sending ICMP_DESTINATION_UNREACHABLE\n";
            numUnroutable++;
            icmpAccess.get()->sendErrorMessage(datagram, ICMP_DESTINATION_UNREACHABLE, 0);
            return;
        }

        // extract interface and next-hop address from routing table entry
        destIE = re->getInterface();
        nextHopAddr = re->getGateway();
    }

    // set datagram source address if not yet set
    if (datagram->getSrcAddress().isUnspecified())
        datagram->setSrcAddress(destIE->ipv4Data()->getIPAddress());

    // default: send datagram to fragmentation
    EV << "output interface is " << destIE->getName() << ", next-hop address: " << nextHopAddr << "\n";
    numForwarded++;

    //
    // fragment and send the packet
    //
    if (datagram->getTransportProtocol()==IP_PROT_MANET)
    {
#ifdef WITH_MANET
       //  check control Info
       if (datagram->getControlInfo())
             delete datagram->removeControlInfo();
#else
       throw cRuntimeError(this, "MANET protocol packet received, but MANET routing support is not available.");
#endif
    }

    fragmentAndSend(datagram, destIE, nextHopAddr);
}

void IPv4::routeMulticastPacket(IPv4Datagram *datagram, InterfaceEntry *destIE, InterfaceEntry *fromIE)
{
    IPv4Address destAddr = datagram->getDestAddress();
    EV << "Routing multicast datagram `" << datagram->getName() << "' with dest=" << destAddr << "\n";

    numMulticast++;

    // DVMRP: process datagram only if sent locally or arrived on the shortest
    // route (provided routing table already contains srcAddr); otherwise
    // discard and continue.
    InterfaceEntry *shortestPathIE = rt->getInterfaceForDestAddr(datagram->getSrcAddress());
    if (fromIE!=NULL && shortestPathIE!=NULL && fromIE!=shortestPathIE)
    {
        // FIXME count dropped
        EV << "Packet dropped.\n";
        delete datagram;
        return;
    }

    // if received from the network...
    if (fromIE!=NULL)
    {
        // check for local delivery
        if (rt->isLocalMulticastAddress(destAddr))
        {
            IPv4Datagram *datagramCopy = (IPv4Datagram *) datagram->dup();

            // FIXME code from the MPLS model: set packet dest address to routerId (???)
            datagramCopy->setDestAddress(rt->getRouterId());

            reassembleAndDeliver(datagramCopy);
        }

        // don't forward if IP forwarding is off
        if (!rt->isIPForwardingEnabled())
        {
            delete datagram;
            return;
        }

        // don't forward if dest address is link-scope
        if (destAddr.isLinkLocalMulticast())
        {
            delete datagram;
            return;
        }

    }

    // routed explicitly via IP_MULTICAST_IF
    if (destIE!=NULL)
    {
        ASSERT(datagram->getDestAddress().isMulticast());

        EV << "multicast packet explicitly routed via output interface " << destIE->getName() << endl;

        // set datagram source address if not yet set
        if (datagram->getSrcAddress().isUnspecified())
            datagram->setSrcAddress(destIE->ipv4Data()->getIPAddress());

        // send
        fragmentAndSend(datagram, destIE, datagram->getDestAddress());

        return;
    }

    // now: routing
    MulticastRoutes routes = rt->getMulticastRoutesFor(destAddr);
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
            InterfaceEntry *destIE = routes[i].interf;

            // don't forward to input port
            if (destIE && destIE!=fromIE)
            {
                IPv4Datagram *datagramCopy = (IPv4Datagram *) datagram->dup();

                // set datagram source address if not yet set
                if (datagramCopy->getSrcAddress().isUnspecified())
                    datagramCopy->setSrcAddress(destIE->ipv4Data()->getIPAddress());

                // send
                IPv4Address nextHopAddr = routes[i].gateway;
                fragmentAndSend(datagramCopy, destIE, nextHopAddr);
            }
        }

        // only copies sent, delete original datagram
        delete datagram;
    }
}
#ifndef NEWFRAGMENT
void IPv4::reassembleAndDeliver(IPv4Datagram *datagram)
{
    // reassemble the packet (if fragmented)
    if (datagram->getFragmentOffset()!=0 || datagram->getMoreFragments())
    {
        EV << "Datagram fragment: offset=" << datagram->getFragmentOffset()
           << ", MORE=" << (datagram->getMoreFragments() ? "true" : "false") << ".\n";

        // erase timed out fragments in fragmentation buffer; check every 10 seconds max
        if (simTime() >= lastCheckTime + 10)
        {
            lastCheckTime = simTime();
            fragbuf.purgeStaleFragments(simTime()-fragmentTimeoutTime);
        }

        datagram = fragbuf.addFragment(datagram, simTime());
        if (!datagram)
        {
            EV << "No complete datagram yet.\n";
            return;
        }
        EV << "This fragment completes the datagram.\n";
    }

    // decapsulate and send on appropriate output gate
    int protocol = datagram->getTransportProtocol();
    cPacket *packet=NULL;
    if (protocol!=IP_PROT_DSR)
    {
        packet = decapsulateIP(datagram);
    }

    if (protocol==IP_PROT_ICMP)
    {
        // incoming ICMP packets are handled specially
        handleReceivedICMP(check_and_cast<ICMPMessage *>(packet));
    }
    else if (protocol==IP_PROT_IP)
    {
        // tunnelled IP packets are handled separately
        send(packet, "preRoutingOut");
    }
    else if (protocol==IP_PROT_DSR)
    {
#ifdef WITH_MANET
        // If the protocol is Dsr Send directely the datagram to manet routing
        controlMessageToManetRouting(MANET_ROUTE_NOROUTE,datagram);
#else
        throw cRuntimeError(this, "DSR protocol packet received, but MANET routing support is not available.");
#endif
    }
    else
    {
        // JcM Fix: check if the transportOut port are connected, otherwise
        // discard the packet
        int gateindex = mapping.getOutputGateForProtocol(protocol);

        if (gate("transportOut",gateindex)->isPathOK()) {
        send(packet, "transportOut", gateindex);
        } else {
            EV << "L3 Protocol not connected. discarding packet" << endl;
            delete(packet);
    }
}
}
#endif
cPacket *IPv4::decapsulateIP(IPv4Datagram *datagram)
{
    // decapsulate transport packet
    InterfaceEntry *fromIE = getSourceInterfaceFrom(datagram);
    cPacket *packet = datagram->decapsulate();

    // create and fill in control info
    IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
    controlInfo->setProtocol(datagram->getTransportProtocol());
    controlInfo->setSrcAddr(datagram->getSrcAddress());
    controlInfo->setDestAddr(datagram->getDestAddress());
    controlInfo->setDiffServCodePoint(datagram->getDiffServCodePoint());
    controlInfo->setInterfaceId(fromIE ? fromIE->getInterfaceId() : -1);
    controlInfo->setTimeToLive(datagram->getTimeToLive());

    // original IPv4 datagram might be needed in upper layers to send back ICMP error message
    controlInfo->setOrigDatagram(datagram);

    // attach control info
    packet->setControlInfo(controlInfo);

    return packet;
}

#ifndef NEWFRAGMENT
void IPv4::fragmentAndSend(IPv4Datagram *datagram, InterfaceEntry *ie, IPv4Address nextHopAddr)
{
    int mtu = ie->getMTU();

    // check if datagram does not require fragmentation
    if (datagram->getByteLength() <= mtu)
    {
        sendDatagramToOutput(datagram, ie, nextHopAddr);
        return;
    }

    int headerLength = datagram->getHeaderLength();
    int payload = datagram->getByteLength() - headerLength;

    int noOfFragments =
        int(ceil((float(payload)/mtu) /
        (1-float(headerLength)/mtu) ) ); // FIXME ???

    // if "don't fragment" bit is set, throw datagram away and send ICMP error message
    if (datagram->getDontFragment() && noOfFragments>1)
    {
        EV << "datagram larger than MTU and don't fragment bit set, sending ICMP_DESTINATION_UNREACHABLE\n";
        icmpAccess.get()->sendErrorMessage(datagram, ICMP_DESTINATION_UNREACHABLE,
                                                     ICMP_FRAGMENTATION_ERROR_CODE);
        return;
    }

    // create and send fragments
    EV << "Breaking datagram into " << noOfFragments << " fragments\n";
    std::string fragMsgName = datagram->getName();
    fragMsgName += "-frag";

    // FIXME revise this!
    for (int i=0; i<noOfFragments; i++)
    {
        // FIXME is it ok that full encapsulated packet travels in every datagram fragment?
        // should better travel in the last fragment only. Cf. with reassembly code!
        IPv4Datagram *fragment = (IPv4Datagram *) datagram->dup();
        fragment->setName(fragMsgName.c_str());

        // total_length equal to mtu, except for last fragment;
        // "more fragments" bit is unchanged in the last fragment, otherwise true
        if (i != noOfFragments-1)
        {
            fragment->setMoreFragments(true);
            fragment->setByteLength(mtu);
        }
        else
        {
            // size of last fragment
            int bytes = datagram->getByteLength() - (noOfFragments-1) * (mtu - datagram->getHeaderLength());
            fragment->setByteLength(bytes);
        }
        fragment->setFragmentOffset( i*(mtu - datagram->getHeaderLength()) );

        sendDatagramToOutput(fragment, ie, nextHopAddr);
    }

    delete datagram;
}
#endif

IPv4Datagram *IPv4::encapsulate(cPacket *transportPacket, InterfaceEntry *&destIE)
{
    IPv4ControlInfo *controlInfo = check_and_cast<IPv4ControlInfo*>(transportPacket->removeControlInfo());
    IPv4Datagram *datagram = encapsulate(transportPacket, destIE, controlInfo);
    delete controlInfo;
    return datagram;
}

IPv4Datagram *IPv4::encapsulate(cPacket *transportPacket, InterfaceEntry *&destIE, IPv4ControlInfo *controlInfo)
{
    IPv4Datagram *datagram = createIPv4Datagram(transportPacket->getName());
    datagram->setByteLength(IP_HEADER_BYTES);
    datagram->encapsulate(transportPacket);

    // set source and destination address
    IPv4Address dest = controlInfo->getDestAddr();
    datagram->setDestAddress(dest);

    // IP_MULTICAST_IF option, but allow interface selection for unicast packets as well
    destIE = ift->getInterfaceById(controlInfo->getInterfaceId());

    IPv4Address src = controlInfo->getSrcAddr();

    // when source address was given, use it; otherwise it'll get the address
    // of the outgoing interface after routing
    if (!src.isUnspecified())
    {
        // if interface parameter does not match existing interface, do not send datagram
        if (rt->getInterfaceByAddress(src)==NULL)
            throw cRuntimeError(this, "Wrong source address %s in (%s)%s: no interface with such address",
                      src.str().c_str(), transportPacket->getClassName(), transportPacket->getFullName());

        datagram->setSrcAddress(src);
    }

    // set other fields
    datagram->setDiffServCodePoint(controlInfo->getDiffServCodePoint());

    datagram->setIdentification(curFragmentId++);
    datagram->setMoreFragments(false);
    datagram->setDontFragment (controlInfo->getDontFragment());
    datagram->setFragmentOffset(0);

    short ttl;
    if (controlInfo->getTimeToLive() > 0)
        ttl = controlInfo->getTimeToLive();
    else if (datagram->getDestAddress().isLinkLocalMulticast())
        ttl = 1;
    else if (datagram->getDestAddress().isMulticast())
        ttl = defaultMCTimeToLive;
    else
        ttl = defaultTimeToLive;

    datagram->setTimeToLive(ttl);
    datagram->setTransportProtocol(controlInfo->getProtocol());

    // setting IPv4 options is currently not supported

    return datagram;
}

IPv4Datagram *IPv4::createIPv4Datagram(const char *name)
{
    return new IPv4Datagram(name);
}

void IPv4::sendDatagramToOutput(IPv4Datagram *datagram, InterfaceEntry *ie, IPv4Address nextHopAddr)
{
    // hop counter check
    if (datagram->getTimeToLive() <= 0)
    {
        // drop datagram, destruction responsibility in ICMP
        EV << "datagram TTL reached zero, sending ICMP_TIME_EXCEEDED\n";
        icmpAccess.get()->sendErrorMessage(datagram, ICMP_TIME_EXCEEDED, 0);
        return;
    }

    // send out datagram to ARP, with control info attached
    IPv4RoutingDecision *routingDecision = new IPv4RoutingDecision();
    routingDecision->setInterfaceId(ie->getInterfaceId());
    routingDecision->setNextHopAddr(nextHopAddr);
    datagram->setControlInfo(routingDecision);

    send(datagram, queueOutGate);

}

void IPv4::dsrFillDestIE(IPv4Datagram *datagram, InterfaceEntry *&destIE,IPv4Address &nextHopAddress)
{

    nextHopAddress= IPv4Address::UNSPECIFIED_ADDRESS;

    if (datagram->getTransportProtocol()!=IP_PROT_DSR)
        return; // Not Dsr packet

    IPv4ControlInfo *controlInfo = check_and_cast<IPv4ControlInfo*>(datagram->removeControlInfo());
    if (controlInfo==NULL)
        return; // Not contolInfo

    destIE = ift->getInterfaceById(controlInfo->getInterfaceId());

    nextHopAddress  = controlInfo->getNextHopAddr();
    delete controlInfo;

}

#ifdef WITH_MANET
void IPv4::controlMessageToManetRouting(int code,IPv4Datagram *datagram)
{
    ControlManetRouting *control;
    if (!manetRouting)
            return;

    if (datagram->getTransportProtocol()==IP_PROT_DSR)
    {
        if (code==MANET_ROUTE_NOROUTE)
        {
            int gateindex = mapping.getOutputGateForProtocol(IP_PROT_MANET);
            send(datagram, "transportOut", gateindex);
        }
        return; // Dsr don't use update code, the Dsr datagram is the update.
    }


    control = new ControlManetRouting();
    control->setOptionCode(code);

    switch (code)
    {
    case MANET_ROUTE_UPDATE:
        control->setSrcAddress(datagram->getSrcAddress());
        control->setDestAddress(datagram->getDestAddress());
        break;
    case MANET_ROUTE_NOROUTE:
        control->setSrcAddress(datagram->getSrcAddress());
        control->setDestAddress(datagram->getDestAddress());
        control->encapsulate(datagram);
        break;
    default:
        delete control;
        return;
    }
    int gateindex = mapping.getOutputGateForProtocol(IP_PROT_MANET);
    send(control, "transportOut", gateindex);
}
#endif

#ifdef NEWFRAGMENT
void IPv4::fragmentAndSend(IPv4Datagram *datagram, InterfaceEntry *ie, IPv4Address nextHopAddr)
{
    int mtu = ie->getMTU();

    // check if datagram does not require fragmentation
    if (datagram->getByteLength() <= mtu)
    {
        sendDatagramToOutput(datagram, ie, nextHopAddr);
        return;
    }

    cPacket * payload = datagram->decapsulate();
    int headerLength = datagram->getByteLength();
    int payloadLength = payload->getByteLength();


    int noOfFragments =
        int(ceil((float(payloadLength)/mtu) /
        (1-float(headerLength)/mtu) ) ); // FIXME ???

    // if "don't fragment" bit is set, throw datagram away and send ICMP error message
    if (datagram->getDontFragment() && noOfFragments>1)
    {
        EV << "datagram larger than MTU and don't fragment bit set, sending ICMP_DESTINATION_UNREACHABLE\n";
        datagram->encapsulate(payload);
        icmpAccess.get()->sendErrorMessage(datagram, ICMP_DESTINATION_UNREACHABLE,
                                                     ICMP_FRAGMENTATION_ERROR_CODE);
        return;
    }

    datagram->setTotalPayloadLength(payloadLength);

    // create and send fragments
    EV << "Breaking datagram into " << noOfFragments << " fragments\n";
    std::string fragMsgName = datagram->getName();
    fragMsgName += "-frag";

    // FIXME revise this!
    for (int i=0; i<noOfFragments; i++)
    {
        // FIXME is it ok that full encapsulated packet travels in every datagram fragment?
        // should better travel in the last fragment only. Cf. with reassembly code!
        IPv4Datagram *fragment = (IPv4Datagram *) datagram->dup();
        cPacket *payloadFrag = payload->dup();
        fragment->setName(fragMsgName.c_str());

        // total_length equal to mtu, except for last fragment;
        // "more fragments" bit is unchanged in the last fragment, otherwise true
        if (i != noOfFragments-1)
        {
            fragment->setMoreFragments(true);
            payloadFrag->setByteLength(mtu-headerLength);
            payloadLength = payloadLength -(mtu-headerLength);
        }
        else
        {
            payloadFrag->setByteLength(payloadLength);
        }
        fragment->encapsulate(payloadFrag);
        fragment->setFragmentOffset( i*(mtu - headerLength) );

        sendDatagramToOutput(fragment, ie, nextHopAddr);
    }

    delete datagram;
}


void IPv4::reassembleAndDeliver(IPv4Datagram *datagram)
{

    // reassemble the packet (if fragmented)


    int headerLength;
    if (datagram->getFragmentOffset()!=0 || datagram->getMoreFragments())
    {
        EV << "Datagram fragment: offset=" << datagram->getFragmentOffset()
           << ", MORE=" << (datagram->getMoreFragments() ? "true" : "false") << ".\n";

        // erase timed out fragments in fragmentation buffer; check every 10 seconds max
        if (simTime() >= lastCheckTime + 10)
        {
            lastCheckTime = simTime();
            fragbuf.purgeStaleFragments(simTime()-fragmentTimeoutTime);
        }
        if (!datagram->getMoreFragments())
        {
            int totalLength = datagram->getByteLength();
            headerLength = datagram->getHeaderLength();
            cPacket * payload=datagram->decapsulate();
            datagram->setHeaderLength(datagram->getByteLength());
            payload->setByteLength(datagram->getTotalPayloadLength());
            datagram->encapsulate(payload);
            datagram->setByteLength(totalLength);
        }

        datagram = fragbuf.addFragment(datagram, simTime());
        if (!datagram)
        {
            EV << "No complete datagram yet.\n";
            return;
        }
        datagram->setHeaderLength(headerLength);
        EV << "This fragment completes the datagram.\n";
    }

    int protocol = datagram->getTransportProtocol();
    cPacket *packet=NULL;
    if (protocol!=IP_PROT_DSR)
    {
        packet = decapsulateIP(datagram);
    }

    if (protocol==IP_PROT_ICMP)
    {
        // incoming ICMP packets are handled specially
        handleReceivedICMP(check_and_cast<ICMPMessage *>(packet));
    }
    else if (protocol==IP_PROT_DSR)
    {
        // If the protocol is Dsr Send directely the datagram to manet routing
        controlMessageToManetRouting(MANET_ROUTE_NOROUTE,datagram);
    }
    else if (protocol==IP_PROT_IP)
    {
        // tunnelled IP packets are handled separately
        send(packet, "preRoutingOut");
    }
    else
    {
        // JcM Fix: check if the transportOut port are connected, otherwise
        // discard the packet
        int gateindex = mapping.getOutputGateForProtocol(protocol);

        if (gate("transportOut",gateindex)->isPathOK()) {
            send(packet, "transportOut", gateindex);
        } else {
            EV << "L3 Protocol not connected. discarding packet" << endl;
            delete(packet);
        }
    }
}
#endif

