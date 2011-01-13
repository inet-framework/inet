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


#include <omnetpp.h>
#include <stdlib.h>
#include <string.h>

#include "IP.h"
#include "IPDatagram.h"
#include "IPControlInfo.h"
#include "ICMPMessage_m.h"
#include "IPv4InterfaceData.h"
#include "ARPPacket_m.h"

Define_Module(IP);


void IP::initialize()
{
    QueueBase::initialize();

    ift = InterfaceTableAccess().get();
    rt = RoutingTableAccess().get();

    queueOutGate = gate("queueOut");

    defaultTimeToLive = par("timeToLive");
    defaultMCTimeToLive = par("multicastTimeToLive");
    fragmentTimeoutTime = par("fragmentTimeout");
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
}

void IP::updateDisplayString()
{
    char buf[80] = "";
    if (numForwarded>0) sprintf(buf+strlen(buf), "fwd:%d ", numForwarded);
    if (numLocalDeliver>0) sprintf(buf+strlen(buf), "up:%d ", numLocalDeliver);
    if (numMulticast>0) sprintf(buf+strlen(buf), "mcast:%d ", numMulticast);
    if (numDropped>0) sprintf(buf+strlen(buf), "DROP:%d ", numDropped);
    if (numUnroutable>0) sprintf(buf+strlen(buf), "UNROUTABLE:%d ", numUnroutable);
    getDisplayString().setTagArg("t",0,buf);
}

void IP::endService(cPacket *msg)
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
        IPDatagram *dgram = check_and_cast<IPDatagram *>(msg);
        handlePacketFromNetwork(dgram);
    }

    if (ev.isGUI())
        updateDisplayString();
}

InterfaceEntry *IP::getSourceInterfaceFrom(cPacket *msg)
{
    cGate *g = msg->getArrivalGate();
    return g ? ift->getInterfaceByNetworkLayerGateIndex(g->getIndex()) : NULL;
}

void IP::handlePacketFromNetwork(IPDatagram *datagram)
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
    delete datagram->removeControlInfo();

    // hop counter decrement; FIXME but not if it will be locally delivered
    datagram->setTimeToLive(datagram->getTimeToLive()-1);

    // route packet
    if (!datagram->getDestAddress().isMulticast())
        routePacket(datagram, NULL, false);
    else
        routeMulticastPacket(datagram, NULL, getSourceInterfaceFrom(datagram));
}

void IP::handleARP(ARPPacket *msg)
{
    // FIXME hasBitError() check  missing!

    // delete old control info
    delete msg->removeControlInfo();

    // dispatch ARP packets to ARP and let it know the gate index it arrived on
    InterfaceEntry *fromIE = getSourceInterfaceFrom(msg);
    ASSERT(fromIE);

    IPRoutingDecision *routingDecision = new IPRoutingDecision();
    routingDecision->setInterfaceId(fromIE->getInterfaceId());
    msg->setControlInfo(routingDecision);

    send(msg, queueOutGate);
}

void IP::handleReceivedICMP(ICMPMessage *msg)
{
    switch (msg->getType())
    {
        case ICMP_REDIRECT: // TODO implement redirect handling
        case ICMP_DESTINATION_UNREACHABLE:
        case ICMP_TIME_EXCEEDED:
        case ICMP_PARAMETER_PROBLEM: {
            // ICMP errors are delivered to the appropriate higher layer protocol
            IPDatagram *bogusPacket = check_and_cast<IPDatagram *>(msg->getEncapsulatedPacket());
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

void IP::handleMessageFromHL(cPacket *msg)
{
    // if no interface exists, do not send datagram
    if (ift->getNumInterfaces() == 0)
    {
        EV << "No interfaces exist, dropping packet\n";
        delete msg;
        return;
    }

    // encapsulate and send
    InterfaceEntry *destIE; // will be filled in by encapsulate()
    IPDatagram *datagram = encapsulate(msg, destIE);

    // route packet
    if (!datagram->getDestAddress().isMulticast())
        routePacket(datagram, destIE, true);
    else
        routeMulticastPacket(datagram, destIE, NULL);
}

void IP::routePacket(IPDatagram *datagram, InterfaceEntry *destIE, bool fromHL)
{
    // TBD add option handling code here

    IPAddress destAddr = datagram->getDestAddress();

    EV << "Routing datagram `" << datagram->getName() << "' with dest=" << destAddr << ": ";

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

    // if datagram arrived from input gate and IP_FORWARD is off, delete datagram
    if (!fromHL && !rt->isIPForwardingEnabled())
    {
        EV << "forwarding off, dropping packet\n";
        numDropped++;
        delete datagram;
        return;
    }

    IPAddress nextHopAddr;

    // if output port was explicitly requested, use that, otherwise use IP routing
    if (destIE)
    {
        EV << "using manually specified output interface " << destIE->getName() << "\n";
        // and nextHopAddr remains unspecified
    }
    else
    {
        // use IP routing (lookup in routing table)
        const IPRoute *re = rt->findBestMatchingRoute(destAddr);

        // error handling: destination address does not exist in routing table:
        // notify ICMP, throw packet away and continue
        if (re==NULL)
        {
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
    fragmentAndSend(datagram, destIE, nextHopAddr);
}

void IP::routeMulticastPacket(IPDatagram *datagram, InterfaceEntry *destIE, InterfaceEntry *fromIE)
{
    IPAddress destAddr = datagram->getDestAddress();
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
            IPDatagram *datagramCopy = (IPDatagram *) datagram->dup();

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
                IPDatagram *datagramCopy = (IPDatagram *) datagram->dup();

                // set datagram source address if not yet set
                if (datagramCopy->getSrcAddress().isUnspecified())
                    datagramCopy->setSrcAddress(destIE->ipv4Data()->getIPAddress());

                // send
                IPAddress nextHopAddr = routes[i].gateway;
                fragmentAndSend(datagramCopy, destIE, nextHopAddr);
            }
        }

        // only copies sent, delete original datagram
        delete datagram;
    }
}

void IP::reassembleAndDeliver(IPDatagram *datagram)
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
    cPacket *packet = decapsulateIP(datagram);

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
    else
    {
        int gateindex = mapping.getOutputGateForProtocol(protocol);
        send(packet, "transportOut", gateindex);
    }
}

cPacket *IP::decapsulateIP(IPDatagram *datagram)
{
    // decapsulate transport packet
    InterfaceEntry *fromIE = getSourceInterfaceFrom(datagram);
    cPacket *packet = datagram->decapsulate();

    // create and fill in control info
    IPControlInfo *controlInfo = new IPControlInfo();
    controlInfo->setProtocol(datagram->getTransportProtocol());
    controlInfo->setSrcAddr(datagram->getSrcAddress());
    controlInfo->setDestAddr(datagram->getDestAddress());
    controlInfo->setDiffServCodePoint(datagram->getDiffServCodePoint());
    controlInfo->setInterfaceId(fromIE ? fromIE->getInterfaceId() : -1);

    // original IP datagram might be needed in upper layers to send back ICMP error message
    controlInfo->setOrigDatagram(datagram);

    // attach control info
    packet->setControlInfo(controlInfo);

    return packet;
}


void IP::fragmentAndSend(IPDatagram *datagram, InterfaceEntry *ie, IPAddress nextHopAddr)
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
        IPDatagram *fragment = (IPDatagram *) datagram->dup();
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


IPDatagram *IP::encapsulate(cPacket *transportPacket, InterfaceEntry *&destIE)
{
    IPControlInfo *controlInfo = check_and_cast<IPControlInfo*>(transportPacket->removeControlInfo());
    IPDatagram *datagram = encapsulate(transportPacket, destIE, controlInfo);
    delete controlInfo;
    return datagram;
}

IPDatagram *IP::encapsulate(cPacket *transportPacket, InterfaceEntry *&destIE, IPControlInfo *controlInfo)
{
    IPDatagram *datagram = createIPDatagram(transportPacket->getName());
    datagram->setByteLength(IP_HEADER_BYTES);
    datagram->encapsulate(transportPacket);

    // set source and destination address
    IPAddress dest = controlInfo->getDestAddr();
    datagram->setDestAddress(dest);

    // IP_MULTICAST_IF option, but allow interface selection for unicast packets as well
    destIE = ift->getInterfaceById(controlInfo->getInterfaceId());

    IPAddress src = controlInfo->getSrcAddr();

    // when source address was given, use it; otherwise it'll get the address
    // of the outgoing interface after routing
    if (!src.isUnspecified())
    {
        // if interface parameter does not match existing interface, do not send datagram
        if (rt->getInterfaceByAddress(src)==NULL)
            opp_error("Wrong source address %s in (%s)%s: no interface with such address",
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

    // setting IP options is currently not supported

    return datagram;
}

IPDatagram *IP::createIPDatagram(const char *name)
{
    return new IPDatagram(name);
}

void IP::sendDatagramToOutput(IPDatagram *datagram, InterfaceEntry *ie, IPAddress nextHopAddr)
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
    IPRoutingDecision *routingDecision = new IPRoutingDecision();
    routingDecision->setInterfaceId(ie->getInterfaceId());
    routingDecision->setNextHopAddr(nextHopAddr);
    datagram->setControlInfo(routingDecision);

    send(datagram, queueOutGate);
}


