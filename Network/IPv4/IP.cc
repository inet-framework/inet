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


//  Cleanup and rewrite: Andras Varga, 2004

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
    IPForward = par("IPForward").boolValue();
    defaultTimeToLive = par("timeToLive");
    defaultMCTimeToLive = par("multicastTimeToLive");

    curFragmentId = 0;

    numMulticast = numLocalDeliver = numDropped = numUnroutable = numForwarded = 0;

    WATCH(IPForward);
    WATCH(numMulticast);
    WATCH(numLocalDeliver);
    WATCH(numDropped);
    WATCH(numUnroutable);
    WATCH(numForwarded);
}

void IP::handleMessage(cMessage *msg)
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
        double relativeHeaderLength = datagram->headerLength() / (double)datagram->length()/8;
        if (dblrand() <= relativeHeaderLength)
        {
            icmpAccess.get()->sendErrorMessage(datagram, ICMP_PARAMETER_PROBLEM, 0);
            return;
        }
        // FIXME ignore bit error if in payload?
    }

    // hop counter decrement
    datagram->setTimeToLive (datagram->timeToLive()-1);

    //
    // "Routing"
    //
    IPRoutingDecision *routingDecision = check_and_cast<IPRoutingDecision *>(datagram->controlInfo());

    // FIXME add option handling code here!

    IPAddress destAddress = datagram->destAddress();

    ev << "Packet destination address is: " << destAddress << ", ";

    //  multicast check
    RoutingTable *rt = routingTableAccess.get();
    if (destAddress.isMulticast())
    {
        ev << "sending to multicast\n";
        numMulticast++;
        handleMulticastPacket(datagram);
        return;
    }

    // check for local delivery
    if (rt->localDeliver(destAddress))
    {
        ev << "sending to localDeliver\n";
        numLocalDeliver++;
        localDeliver(datagram);
        return;
    }

    // if datagram arrived from input gate and IP_FORWARD is off, delete datagram
    if (routingDecision->inputPort()!=-1 && !IPForward)
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
        ev << "unroutable, sending ICMP_DESTINATION_UNREACHABLE\n";
        numUnroutable++;
        icmpAccess.get()->sendErrorMessage(datagram, ICMP_DESTINATION_UNREACHABLE, 0);
        return;
    }

    // default: send datagram to fragmentation
    routingDecision->setOutputPort(outputPort);
    //FIXME todo: routingDecision->setNextHopAddr(nextHopAddr);

    ev << "output port is " << outputPort << "\n";
    numForwarded++;

    //
    // "Fragmentation" and "IPOutput"
    //
    fragmentAndSend(datagram);
}

void IP::handleMessageFromHL(cMessage *msg)
{
    IPDatagram *dgram = encapsulate(msg);
    //FIXME...
    //send(dgram, "routingOut");
}

void IP::handleMulticastPacket(IPDatagram *datagram)
{
    // FIXME multicast-->tunneling link (present in original IPSuite) missing from here

    RoutingTable *rt = routingTableAccess.get();

    // FIXME We should probably handle if IGMP message comes from localIn.
    // IGMP is not implemented.
    IPRoutingDecision *controlInfo = check_and_cast<IPRoutingDecision *>(datagram->controlInfo());
    int inputPort = controlInfo->inputPort();

    IPAddress destAddress = datagram->destAddress();

    // DVMRP: process datagram only if sent locally or arrived on
    // the shortest route; otherwise discard and continue.
    if (controlInfo->inputPort()!=-1 && controlInfo->inputPort()!=rt->outputPortNo(datagram->srcAddress()))
    {
        // FIXME count dropped
        delete datagram;
        return;
    }

    // check for local delivery
    if (rt->multicastLocalDeliver(destAddress))
    {
        IPDatagram *datagramCopy = (IPDatagram *) datagram->dup();
        // FIXME control info will NOT be present in duplicate packet!
// BCH Andras -- code from UTS MPLS model  FIXME!!!!!!!!!!!!!!!!!!!!!!!!
        // find "local_addr" module parameter among our parents, and assign it to packet
        cModule *curmod = this;
        for (curmod = parentModule(); curmod != NULL; curmod = curmod->parentModule())
        {
            if (curmod->hasPar("local_addr"))
            {
                datagramCopy->setDestAddress(IPAddress(curmod->par("local_addr").stringValue()));
                break;
            }
        }
// ECH
        localDeliver(datagramCopy);
    }

    // forward datagram only if IPForward is true or sent locally
    if (inputPort!=-1 && !IPForward)
    {
        delete datagram;
        return;
    }

    int numDest = rt->numMulticastDestinations(destAddress);
    if (numDest == 0)
    {
        // no destination: delete datagram
        delete datagram;
    }
    else
    {
        // copy original datagram for multiple destinations
        for (int i=0; i<numDest; i++)
        {
            int outputPort = rt->multicastOutputPortNo(destAddress, i);

            // don't forward to input port
            if (outputPort>=0 && outputPort!=inputPort)
            {
                IPDatagram *datagramCopy = (IPDatagram *) datagram->dup();

                // add a copy of the control info (OMNeT++ doesn't copy it)
                IPRoutingDecision *newControlInfo = new IPRoutingDecision();
                newControlInfo->setOutputPort(outputPort);
                //newControlInfo->setNextHopAddr(...); FIXME TODO
                datagramCopy->setControlInfo(newControlInfo);

                fragmentAndSend(datagramCopy);
            }
        }

        // only copies sent, delete original datagram
        delete datagram;
    }
}

void IP::localDeliver(IPDatagram *datagram)
{
    // FIXME defragment, etc...

    // FIXME select transport gate index and send there
    send(datagram, "transportOut", 0);
}

void IP::fragmentAndSend(IPDatagram *datagram)
{
    IPRoutingDecision *controlInfo = check_and_cast<IPRoutingDecision *>(datagram->controlInfo());
    int outputPort = controlInfo->outputPort();

    RoutingTable *rt = routingTableAccess.get();
    int mtu = rt->getInterfaceByIndex(outputPort)->mtu;

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

    // ev << "No of Fragments: " << noOfFragments << endl;

    // if "don't fragment" bit is set, throw datagram away and send ICMP error message
    if (datagram->dontFragment() && noOfFragments>1)
    {
        icmpAccess.get()->sendErrorMessage(datagram, ICMP_DESTINATION_UNREACHABLE,
                                                     ICMP_FRAGMENTATION_ERROR_CODE);
        return;
    }

    // create and send fragments
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
    if (!src.isNull())
    {
        // if interface parameter does not match existing interface, do not send datagram
        if (rt->findInterfaceByAddress(src) == -1)
            opp_error("Wrong source address %s in (%s)%s: no interface with such address",
                      src.str().c_str(), transportPacket->className(), transportPacket->fullName());
        datagram->setSrcAddress(src);
    }
    else
    {
        // otherwise, just use the first
        datagram->setSrcAddress(rt->getInterfaceByIndex(0)->inetAddr);
    }

    // set other fields
    datagram->setDiffServCodePoint(controlInfo->diffServCodePoint());

    datagram->setFragmentId(curFragmentId++);
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

    // add blank RoutingDecision info
    IPRoutingDecision *routingDecision = new IPRoutingDecision();
    datagram->setControlInfo(routingDecision);

    // setting IP options is currently not supported

    return datagram;
}

void IP::sendDatagramToOutput(IPDatagram *datagram, int outputPort)
{
    // hop counter check
    if (datagram->timeToLive() <= 0)
    {
        // drop datagram, destruction responsibility in ICMP
        icmpAccess.get()->sendErrorMessage(datagram, ICMP_TIME_EXCEEDED, 0);
        return;
    }

    int numOfPorts = 8; //FIXME
    if (outputPort >= numOfPorts)
        error("Illegal output port %d", outputPort);

    send(datagram, "queueOut", outputPort);
}

