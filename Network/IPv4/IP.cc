//
// Copyright (C) 2004 Andras Varga
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
#include <stdlib.h>
#include <string.h>

#include "IP.h"
#include "IPDatagram.h"
#include "IPControlInfo_m.h"
#include "ICMPMessage_m.h"

Define_Module(IP);


void IP::initialize()
{
    QueueBase::initialize();

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
    displayString().setTagArg("t",0,buf);
}

void IP::endService(cMessage *msg)
{
    if (msg->arrivalGate()->isName("transportIn"))
    {
        handleMessageFromHL(msg);
    }
    else
    {
        IPDatagram *dgram = check_and_cast<IPDatagram *>(msg);
        handlePacketFromNetwork(dgram);
    }

    if (ev.isGUI())
        updateDisplayString();
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
        double relativeHeaderLength = datagram->headerLength() / (double)datagram->length()/8;
        if (dblrand() <= relativeHeaderLength)
        {
            ev << "bit error found, sending ICMP_PARAMETER_PROBLEM\n";
            icmpAccess.get()->sendErrorMessage(datagram, ICMP_PARAMETER_PROBLEM, 0);
            return;
        }
    }

    // hop counter decrement
    datagram->setTimeToLive (datagram->timeToLive()-1);

    // routepacket
    routePacket(datagram);
}

void IP::handleMessageFromHL(cMessage *msg)
{
    IPDatagram *datagram = encapsulate(msg);
    routePacket(datagram);
}

void IP::routePacket(IPDatagram *datagram)
{
    // TBD add option handling code here

    IPAddress destAddress = datagram->destAddress();

    ev << "Packet destination address is: " << destAddress << ", ";

    // multicast check
    RoutingTable *rt = routingTableAccess.get();
    if (destAddress.isMulticast())
    {
        ev << "sending to multicast\n";
        numMulticast++;
        handleMulticastPacket(datagram);
        return;
    }

    // check for local delivery
    // FIXME I don't think that local loopback (127.0.0.1) works correctly... (Andras)
    if (rt->localDeliver(destAddress))
    {
        ev << "sending to localDeliver\n";
        numLocalDeliver++;
        localDeliver(datagram);
        return;
    }

    // if datagram arrived from input gate and IP_FORWARD is off, delete datagram
    int inputPort = datagram->arrivalGate() ? datagram->arrivalGate()->index() : -1;
    if (inputPort!=-1 && !rt->ipForward())
    {
        ev << "forwarding off, dropping packet\n";
        numDropped++;
        delete datagram;
        return;
    }

    // error handling: destination address does not exist in routing table:
    // notify ICMP, throw packet away and continue
    int outputPort = rt->outputPortNo(destAddress);
    if (outputPort==-1)
    {
        //FIXME what to do if src=0.0.0.0, i.e. we're still in the src node?? sure don't send icmp to ourselves? (Andras)
        ev << "unroutable, sending ICMP_DESTINATION_UNREACHABLE\n";
        numUnroutable++;
        icmpAccess.get()->sendErrorMessage(datagram, ICMP_DESTINATION_UNREACHABLE, 0);
        return;
    }

    // set datagram source address if not yet set
    if (datagram->srcAddress().isNull())
        datagram->setSrcAddress(rt->interfaceByPortNo(outputPort)->inetAddr);

    // default: send datagram to fragmentation
    ev << "output port is " << outputPort << "\n";
    numForwarded++;

    //
    // "Fragmentation" and "IPOutput"
    //
    fragmentAndSend(datagram, outputPort);
}

void IP::handleMulticastPacket(IPDatagram *datagram)
{
    // FIXME multicast-->tunneling link (present in original IPSuite) missing from here
    RoutingTable *rt = routingTableAccess.get();

    // DVMRP: process datagram only if sent locally or arrived on the shortest 
    // route (provided routing table already contains srcAddr); otherwise 
    // discard and continue.
    int inputPort = datagram->arrivalGate() ? datagram->arrivalGate()->index() : -1;
    int shortestPathInputPort = rt->outputPortNo(datagram->srcAddress());
    if (inputPort!=-1 && shortestPathInputPort!=-1 && inputPort!=shortestPathInputPort)
    {
        // FIXME count dropped
        ev << "Packet dropped.\n";
        delete datagram;
        return;
    }

    // check for local delivery
    IPAddress destAddress = datagram->destAddress();
    if (rt->multicastLocalDeliver(destAddress))
    {
        IPDatagram *datagramCopy = (IPDatagram *) datagram->dup();

        // FIXME code from the MPLS model: set packet dest address to routerId (???)
        datagramCopy->setDestAddress(rt->getRouterId());

        localDeliver(datagramCopy);
    }

    // forward datagram only if IP forward is enabled, or sent locally
    if (inputPort!=-1 && !rt->ipForward())
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
            int outputPort = routes[i].interf->outputPort;

            // don't forward to input port
            if (outputPort>=0 && outputPort!=inputPort)
            {
                IPDatagram *datagramCopy = (IPDatagram *) datagram->dup();

                // set datagram source address if not yet set
                if (datagramCopy->srcAddress().isNull())
                    datagramCopy->setSrcAddress(rt->interfaceByPortNo(outputPort)->inetAddr);

                // send
                fragmentAndSend(datagramCopy, outputPort);
            }
        }

        // only copies sent, delete original datagram
        delete datagram;
    }
}

void IP::localDeliver(IPDatagram *datagram)
{
    // Defragmentation. skip defragmentation if datagram is not fragmented
    if (datagram->fragmentOffset()!=0 || datagram->moreFragments())
    {
        // erase timed out fragments in fragmentation buffer; check every 10 seconds max
        if (simTime() >= lastCheckTime + 10)
        {
            lastCheckTime = simTime();
            fragbuf.purgeStaleFragments(simTime()-fragmentTimeoutTime);
        }

        datagram = fragbuf.addFragment(datagram, simTime());
        if (!datagram)
            return;
    }

    // decapsulate and send on appropriate output gate
    int protocol = datagram->transportProtocol();
    cMessage *packet = decapsulateIP(datagram);

    if (protocol==IP_PROT_IP)
    {
        // tunnelled IP packets are handled separately
        send(packet, "preRoutingOut");
    }
    else
    {
        int gateindex = mapping.outputGateForProtocol(protocol);
        send(packet, "transportOut", gateindex);
    }
}

cMessage *IP::decapsulateIP(IPDatagram *datagram)
{
    cMessage *packet = datagram->decapsulate();

    IPControlInfo *controlInfo = new IPControlInfo();
    controlInfo->setProtocol(datagram->transportProtocol());
    controlInfo->setSrcAddr(datagram->srcAddress());
    controlInfo->setDestAddr(datagram->destAddress());
    controlInfo->setDiffServCodePoint(datagram->diffServCodePoint());
    int inputPort = datagram->arrivalGate() ? datagram->arrivalGate()->index() : -1;
    controlInfo->setInputPort(inputPort);
    packet->setControlInfo(controlInfo);
    delete datagram;

    return packet;
}


void IP::fragmentAndSend(IPDatagram *datagram, int outputPort)
{
    RoutingTable *rt = routingTableAccess.get();
    int mtu = rt->interfaceByPortNo(outputPort)->mtu;

    // check if datagram does not require fragmentation
    if (datagram->length()/8 <= mtu)
    {
        sendDatagramToOutput(datagram, outputPort);
        return;
    }

    int headerLength = datagram->headerLength();
    int payload = datagram->length()/8 - headerLength;

    int noOfFragments =
        int(ceil((float(payload)/mtu) /
        (1-float(headerLength)/mtu) ) ); // FIXME ???

    // if "don't fragment" bit is set, throw datagram away and send ICMP error message
    if (datagram->dontFragment() && noOfFragments>1)
    {
        ev << "datagram larger than MTU and don't fragment bit set, sending ICMP_DESTINATION_UNREACHABLE\n";
        icmpAccess.get()->sendErrorMessage(datagram, ICMP_DESTINATION_UNREACHABLE,
                                                     ICMP_FRAGMENTATION_ERROR_CODE);
        return;
    }

    // create and send fragments
    ev << "Breaking datagram into " << noOfFragments << " fragments\n";
    // FIXME revise this!
    for (int i=0; i<noOfFragments; i++)
    {
        // FIXME is it ok that full encapsulated packet travels in every datagram fragment?
        // should better travel in the last fragment only. Cf. with reassembly code!
        IPDatagram *fragment = (IPDatagram *) datagram->dup();

        // total_length equal to mtu, except for last fragment;
        // "more fragments" bit is unchanged in the last fragment, otherwise true
        if (i != noOfFragments-1)
        {
            fragment->setMoreFragments(true);
            fragment->setLength(8*mtu);
        }
        else
        {
            // size of last fragment
            int bytes = datagram->length()/8 - (noOfFragments-1) * (mtu - datagram->headerLength());
            fragment->setLength(8*bytes);
        }
        fragment->setFragmentOffset( i*(mtu - datagram->headerLength()) );

        sendDatagramToOutput(fragment, outputPort);
    }

    delete datagram;
}


IPDatagram *IP::encapsulate(cMessage *transportPacket)
{
    // if no interface exists, do not send datagram
    RoutingTable *rt = routingTableAccess.get();
    if (rt->numInterfaces() == 0)
    {
        ev << "No interfaces exist, dropping packet\n";
        delete transportPacket;
        return NULL;
    }

    IPControlInfo *controlInfo = check_and_cast<IPControlInfo*>(transportPacket->removeControlInfo());

    IPDatagram *datagram = new IPDatagram(transportPacket->name());
    datagram->encapsulate(transportPacket);

    // set source and destination address
    IPAddress dest = controlInfo->destAddr();
    datagram->setDestAddress(dest);

    IPAddress src = controlInfo->srcAddr();

    // when source address given in Interface Message, use it
    // (otherwise it'll get the address of the outgoing interface after routing)
    if (!src.isNull())
    {
        // if interface parameter does not match existing interface, do not send datagram
        if (rt->interfaceByAddress(src)==NULL)
            opp_error("Wrong source address %s in (%s)%s: no interface with such address",
                      src.str().c_str(), transportPacket->className(), transportPacket->fullName());
        datagram->setSrcAddress(src);
    }

    // set other fields
    datagram->setDiffServCodePoint(controlInfo->diffServCodePoint());

    datagram->setIdentification(curFragmentId++);
    datagram->setMoreFragments(false);
    datagram->setDontFragment (controlInfo->dontFragment());
    datagram->setFragmentOffset(0);

    datagram->setTimeToLive(
           controlInfo->timeToLive() > 0 ?
           controlInfo->timeToLive() :
           (datagram->destAddress().isMulticast() ? defaultMCTimeToLive : defaultTimeToLive)
    );

    datagram->setTransportProtocol(controlInfo->protocol());
    delete controlInfo;

    // setting IP options is currently not supported

    return datagram;
}

void IP::sendDatagramToOutput(IPDatagram *datagram, int outputPort)
{
    // hop counter check
    if (datagram->timeToLive() <= 0)
    {
        // drop datagram, destruction responsibility in ICMP
        ev << "datagram TTL reached zero, sending ICMP_TIME_EXCEEDED\n";
        icmpAccess.get()->sendErrorMessage(datagram, ICMP_TIME_EXCEEDED, 0);
        return;
    }

    int numOfPorts = gate("queueOut")->size();
    if (outputPort >= numOfPorts)
        error("Illegal output port %d", outputPort);

    send(datagram, "queueOut", outputPort);
}


