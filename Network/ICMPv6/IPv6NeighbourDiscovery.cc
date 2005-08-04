/**
 * Copyright (C) 2005 Andras Varga
 * Copyright (C) 2005 Wei Yang, Ng
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "IPv6NeighbourDiscovery.h"


#define MK_RESOLVE_TENTATIVE_ADDRESS 0
#define MK_SEND_PERIODIC_RTRADV 1
#define MK_SEND_SOL_RTRADV 2
#define MK_INITIATE_RTRDIS 3
#define MK_DAD_TIMEOUT 4
#define MK_RD_TIMEOUT 5
#define MK_SEND_NBSOL_PROBE 6

Define_Module(IPv6NeighbourDiscovery);

void IPv6NeighbourDiscovery::initialize(int stage)
{
    // We have to wait until the 3rd stage (stage 2) with scheduling messages,
    // because interface registration and IPv6 configuration takes places
    // in the first two stages.
    if (stage==3)
    {
        ift = InterfaceTableAccess().get();
        rt6 = RoutingTable6Access().get();
        pendingQueue.setName("pendingQueue");

        for (int i=0; i < ift->numInterfaces(); i++)
        {
            InterfaceEntry *ie = ift->interfaceAt(i);

            if (ie->ipv6()->advSendAdvertisements())
            {
                createRATimer(ie);
            }
            if (ie->outputPort() != -1)
            {
                cMessage *msg = new cMessage("resolveTentativeAddr", MK_RESOLVE_TENTATIVE_ADDRESS);
                msg->setContextPointer(ie);
                scheduleAt(uniform(0,1), msg);//Random node bootup time
            }
        }
    }
}

void IPv6NeighbourDiscovery::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        ev << "Self message received!\n";
        if (msg->kind()==MK_SEND_PERIODIC_RTRADV)
        {
            ev << "Sending periodic RA\n";
            sendPeriodicRA(msg);
        }
        else if (msg->kind()==MK_SEND_SOL_RTRADV)
        {
            ev << "Sending solicited RA\n";
            sendSolicitedRA(msg);
        }
        else if (msg->kind()==MK_RESOLVE_TENTATIVE_ADDRESS)
        {
            ev << "Resolving Tentative Address\n";
            resolveTentativeAddress(msg);
        }
        else if (msg->kind()==MK_DAD_TIMEOUT)
        {
            ev << "DAD Timeout message received\n";
            processDADTimeout(msg);
        }
        else if (msg->kind()==MK_RD_TIMEOUT)
        {
            ev << "Router Discovery message received\n";
            processRDTimeout(msg);
        }
        else if (msg->kind()==MK_INITIATE_RTRDIS)
        {
            ev << "initiate router discovery.\n";
            initiateRouterDiscovery(msg);
        }
        else
            error("Unrecognized Timer");//stops sim w/ error msg.
    }
    else if (dynamic_cast<ICMPv6Message *>(msg))
    {
        //This information will serve as input parameters to various processors.
        IPv6ControlInfo *ctrlInfo
            = check_and_cast<IPv6ControlInfo*>(msg->removeControlInfo());
        ICMPv6Message *ndMsg = (ICMPv6Message *)msg;
        processNDMessage(ndMsg, ctrlInfo);
    }
    else // not ND message
    {
        IPv6Datagram *datagram = (IPv6Datagram *)msg;
        processIPv6Datagram(datagram);
    }
}

void IPv6NeighbourDiscovery::processNDMessage(ICMPv6Message *msg,
    IPv6ControlInfo *ctrlInfo)
{

    if (dynamic_cast<IPv6RouterSolicitation *>(msg))
    {
        IPv6RouterSolicitation *rs = (IPv6RouterSolicitation *)msg;
        processRSPacket(rs, ctrlInfo);
    }
    else if (dynamic_cast<IPv6RouterAdvertisement *>(msg))
    {
        IPv6RouterAdvertisement *ra = (IPv6RouterAdvertisement *)msg;
        processRAPacket(ra, ctrlInfo);
    }
    else if (dynamic_cast<IPv6NeighbourSolicitation *>(msg))
    {
        IPv6NeighbourSolicitation *ns = (IPv6NeighbourSolicitation *)msg;
        processNSPacket(ns, ctrlInfo);
    }
    else if (dynamic_cast<IPv6NeighbourAdvertisement *>(msg))
    {
        IPv6NeighbourAdvertisement *na = (IPv6NeighbourAdvertisement *)msg;
        processNAPacket(na, ctrlInfo);
    }
    else if (dynamic_cast<IPv6Redirect *>(msg))
    {
        IPv6Redirect *redirect = (IPv6Redirect *)msg;
        processRedirectPacket(redirect, ctrlInfo);
    }
    else
    {
        error("Unrecognized ND message!");
    }
}

void IPv6NeighbourDiscovery::finish()
{
}

void IPv6NeighbourDiscovery::processIPv6Datagram(IPv6Datagram *msg)
{
    EV << "Packet " << msg << " arrived from IPv6 module.\n";
    IPv6Address dgDestAddr = msg->destAddress();
    IPv6Address dgSrcAddr = msg->srcAddress();

    //If a datagram has arrived here, it is either because there is no entry for
    //this dest addr in the dest cache or the dest's entry in the neighbour cache
    //is stale.
    int nextHopID;
    ev << "Determining Next Hop" << endl;
    IPv6Address nextHopAddr = determineNextHop(dgDestAddr, nextHopID);
    ev << "Next Hop Address is: " << nextHopAddr << " on interface: " << nextHopID << endl;

    Neighbour *neighbour = neighbourCache.lookup(nextHopAddr, nextHopID);
    if (neighbour==NULL)
    {
        ev << "No Entry exists in the Neighbour Cache.\n";
        ev << "Initiating Address Resolution for:" << nextHopAddr
           << " on Interface:" << nextHopID << endl;
        initiateAddressResolution(dgSrcAddr, nextHopAddr, nextHopID);

        //Next step is to queue up the packet for this entry.
        ev << "Add packet to entry's queue until Address Resolution is complete.\n";
        pendingQueue.insert(msg);
    }
    else if (neighbour->reachabilityState == IPv6NeighbourCache::INCOMPLETE)
    {
        ev << "INCOMPLETE entry exists in Neighbour Cache.\n";
        ev << "Address Resolution already initiated\n";
        ev << "Add packet to queue until Address Resolution is complete.\n";
        pendingQueue.insert(msg);
    }
    else
    {
        /*Neighbor Unreachability Detection is performed only for neighbors to
          which unicast packets are sent; it is not used when sending to
          multicast addresses.*/
        if (dgDestAddr.isMulticast())
            error("ERROR: Datagram dest address is a Multicast address!");

        //if reachableTime has exceeded for this neighbour, in other words
        //entry is STALE
        if (simTime() > neighbour->reachabilityExpires)
            initiateNeighbourUnreachabilityDetection(neighbour);
    }
}

IPv6NeighbourDiscovery::AdvIfEntry *IPv6NeighbourDiscovery::fetchAdvIfEntry(InterfaceEntry *ie)
{
   for (AdvIfList::iterator it=advIfList.begin(); it!=advIfList.end(); it++)
   {
       AdvIfEntry *advIfEntry = (*it);
       if (advIfEntry->interfaceId == ie->interfaceId())
       {
           return advIfEntry;
       }
   }
   return NULL;
}

IPv6NeighbourDiscovery::RDEntry *IPv6NeighbourDiscovery::fetchRDEntry(InterfaceEntry *ie)
{
   for (RDList::iterator it=rdList.begin(); it!=rdList.end(); it++)
   {
       RDEntry *rdEntry = (*it);
       if (rdEntry->interfaceId == ie->interfaceId())
       {
           return rdEntry;
       }
   }
   return NULL;
}

const MACAddress& IPv6NeighbourDiscovery::resolveNeighbour(const IPv6Address& nextHop, int interfaceId)
{
    Enter_Method("resolveNeighbor(%s,if=%d)", nextHop.str().c_str(), interfaceId);

    Neighbour *neighbour = neighbourCache.lookup(nextHop, interfaceId);
    if (!neighbour || neighbour->reachabilityState==IPv6NeighbourCache::INCOMPLETE)
        return MACAddress::UNSPECIFIED_ADDRESS;
    return neighbour->macAddress;

    // FIXME above code is just temporary, complete according to the doc in the header file!!!!
        /**
         * Public method, to be invoked from the IPv6 module to determine
         * link-layer address and the output interface of the next hop.
         *
         * If the neighbor cache does not contain this address or it's in the
         * state INCOMPLETE, this method will return the NULL address, and the
         * IPv6 module should then send the datagram here to IPv6NeighbourDiscovery
         * where it will be stored until neighbour resolution completes.
         *
         * If the neighbour cache entry is STALE (or REACHABLE but more than
         * reachableTime elapsed since reachability was last confirmed),
         * the link-layer address is still returned and IPv6 can send the
         * datagram, but simultaneously, this call should trigger the Neighbour
         * Unreachability Detection procedure to start in the
         * IPv6NeighbourDiscovery module.
         */

}

void IPv6NeighbourDiscovery::reachabilityConfirmed(const IPv6Address& neighbour, int interfaceId)
{
    Enter_Method("reachabilityConfirmed(%s,if=%d)", neighbour.str().c_str(), interfaceId);
    // TODO (see header file for description)
}



const IPv6Address& IPv6NeighbourDiscovery::determineNextHop(
    const IPv6Address& destAddr, int& outIfID)
{
    IPv6Address nextHopAddr;
    nextHopAddr = rt6->lookupDestCache(destAddr, outIfID);
    if (outIfID == -1)
    {
        ev << "Next Hop Address not found in Destination Cache.\n";
        ev << "Performing longest prefix matching.";
        //Perform longest prefix matching on the Routing Table to determine route
        const IPv6Route *route = rt6->doLongestPrefixMatch(nextHopAddr);
        if (route != NULL)
        {
            ev << "Longest Prefix Matching has found a route.\n";
            nextHopAddr = route->nextHop();
            outIfID = route->interfaceID();
        }
        else
        {
            ev << "Longest Prefix Matching has found NO route.\n";
            ev << "Selecting a Default Router.\n";
            //TODO: select a default router.Need to implement address resolution
            //and receiving/processing of RA and RS first!
            nextHopAddr = selectDefaultRouter(outIfID);
        }
        ev << "Next-Hop Neighbour not found." << endl;
    }
    else
    {
        ev << "Next-Hop Neighbour found. Updating Destination Cache" << endl;
        rt6->updateDestCache(destAddr, nextHopAddr, outIfID);
    }

    if (outIfID == -1)
    {
        ev << "No next hop route has been found. Set dest addr as next hop neighbour\n";
        nextHopAddr = destAddr;
        //Note that outIfID is still -1 here, we will have to send neighbour
        //solicitations on all available interfaces when performing address
        //resolution.
        for (int i=0;i < ift->numInterfaces(); i++)
            rt6->updateDestCache(destAddr, nextHopAddr, i);
    }
    return nextHopAddr;
}

void IPv6NeighbourDiscovery::initiateNeighbourUnreachabilityDetection(Neighbour *neighbour)
{
    //FIXME: TO be completed.
    /*When a path to a neighbor appears to be failing, the specific
    recovery procedure depends on how the neighbor is being used.  If the
    neighbor is the ultimate destination, for example, address resolution
    should be performed again.  If the neighbor is a router, however,
    attempting to switch to another router would be appropriate.  The
    specific recovery that takes place is covered under next-hop
    determination; Neighbor Unreachability Detection signals the need for
    next-hop determination by deleting a Neighbor Cache entry.*/
    cMessage *neighbourSolProbe = new cMessage("Neighbour_Sol_Probe",
        MK_SEND_NBSOL_PROBE);
}

const IPv6Address& IPv6NeighbourDiscovery::selectDefaultRouter(int& outIfID)
{
    /*
    The algorithm for selecting a router depends in part on whether or
   not a router is known to be reachable.  The exact details of how a
   node keeps track of a neighbor's reachability state are covered in
   Section 7.3.  The algorithm for selecting a default router is invoked
   during next-hop determination when no Destination Cache entry exists
   for an off-link destination or when communication through an existing
   router appears to be failing.  Under normal conditions, a router
   would be selected the first time traffic is sent to a destination,

   with subsequent traffic for that destination using the same router as
   indicated in the Destination Cache modulo any changes to the
   Destination Cache caused by Redirect messages.

   The policy for selecting routers from the Default Router List is as
   follows:

     1) Routers that are reachable or probably reachable (i.e., in any
        state other than INCOMPLETE) SHOULD be preferred over routers
        whose reachability is unknown or suspect (i.e., in the
        INCOMPLETE state, or for which no Neighbor Cache entry exists).
        An implementation may choose to always return the same router or
        cycle through the router list in a round-robin fashion as long
        as it always returns a reachable or a probably reachable router
        when one is available.

     2) When no routers on the list are known to be reachable or
        probably reachable, routers SHOULD be selected in a round-robin
        fashion, so that subsequent requests for a default router do not
        return the same router until all other routers have been
        selected.

        Cycling through the router list in this case ensures that all
        available routers are actively probed by the Neighbor
        Unreachability Detection algorithm.  A request for a default
        router is made in conjunction with the sending of a packet to a
        router, and the selected router will be probed for reachability
        as a side effect.

     3) If the Default Router List is empty, assume that all
        destinations are on-link as specified in Section 5.2.*/
    outIfID = -1;//nothing found yet
    return IPv6Address();
}

void IPv6NeighbourDiscovery::initiateAddressResolution(const IPv6Address& dgSrcAddr,
    const IPv6Address& neighbourAddr, int ifID)
{
    InterfaceEntry *ie = ift->interfaceAt(ifID);

    //RFC2461: Section 7.2.2
    //When a node has a unicast packet to send to a neighbor, but does not
    //know the neighbor's link-layer address, it performs address
    //resolution.  For multicast-capable interfaces this entails creating a
    //Neighbor Cache entry in the INCOMPLETE state
    ev << "Creating an INCOMPLETE entry in the neighbour cache.\n";
    neighbourCache.addNeighbour(neighbourAddr, ifID);
    
    //and transmitting a Neighbor Solicitation message targeted at the
    //neighbor.  The solicitation is sent to the solicited-node multicast
    //address "corresponding to"(or "derived from") the target address.
    //(in this case, the target address is the address we are trying to resolve)
    ev << "Preparing to send NS to solicited-node multicast group\n";
    ev << "on the next hop interface\n";
    IPv6Address nsDestAddr = neighbourAddr.formSolicitedNodeMulticastAddress();//for NS datagram
    IPv6Address nsTargetAddr = neighbourAddr;//for the field within the NS
    IPv6Address nsSrcAddr;

    /*If the source address of the packet prompting the solicitation is the
    same as one of the addresses assigned to the outgoing interface,*/
    if (ie->ipv6()->hasAddress(dgSrcAddr))
        /*that address SHOULD be placed in the IP Source Address of the outgoing
        solicitation.*/
        nsSrcAddr = dgSrcAddr;
    else
        /*Otherwise, any one of the addresses assigned to the interface
        should be used.*/
        nsSrcAddr = ie->ipv6()->preferredAddress();
    ASSERT(ifID != -1);
    //Sending NS on specified interface.
    createAndSendNSPacket(nsTargetAddr, nsDestAddr, nsSrcAddr, ie);
}

void IPv6NeighbourDiscovery::sendPacketToIPv6Module(cMessage *msg, const IPv6Address& destAddr,
    const IPv6Address& srcAddr, int inputGateIndex)
{
    IPv6ControlInfo *controlInfo = new IPv6ControlInfo();
    controlInfo->setProtocol(IP_PROT_IPv6_ICMP);
    controlInfo->setDestAddr(destAddr);
    controlInfo->setSrcAddr(srcAddr);
    controlInfo->setHopLimit(255);//FIXME:should 255 be default?
    controlInfo->setInputGateIndex(inputGateIndex);
    msg->setControlInfo(controlInfo);

    send(msg,"toIPv6");
}

void IPv6NeighbourDiscovery::sendDelayedPacketToIPv6Module(cMessage *msg,
    const IPv6Address& destAddr, const IPv6Address& srcAddr, int inputGateIndex,
    simtime_t delay)
{
    IPv6ControlInfo *controlInfo = new IPv6ControlInfo();
    controlInfo->setProtocol(IP_PROT_IPv6_ICMP);
    controlInfo->setDestAddr(destAddr);
    controlInfo->setSrcAddr(srcAddr);
    controlInfo->setHopLimit(255);//FIXME:should 255 be default?
    controlInfo->setInputGateIndex(inputGateIndex);
    msg->setControlInfo(controlInfo);

    sendDelayed(msg, delay, "toIPv6");
}

void IPv6NeighbourDiscovery::sendQueuedPacketToIPv6Module()
{
    cMessage *msg = (cMessage *)pendingQueue.pop();
    send(msg,"toIPv6");
}

void IPv6NeighbourDiscovery::resolveTentativeAddress(cMessage *timerMsg)
{
    InterfaceEntry *ie = (InterfaceEntry *)timerMsg->contextPointer();
    IPv6Address linkLocalAddr = ie->ipv6()->linkLocalAddress();

    if (linkLocalAddr.isUnspecified())
    {
        //if no link local address exists for this interface, we assign one to it.
        bubble("No link local address exists. Forming one");
        linkLocalAddr = IPv6Address().formLinkLocalAddress(ie->interfaceToken());
        ie->ipv6()->assignAddress(linkLocalAddr, true, 0, 0);
    }

    if (ie->ipv6()->isTentativeAddress(linkLocalAddr))
    {
        //if the link local address for this interface is tentative, we initiate
        //DAD.
        bubble("Link Local Address is tentative. Initiate DAD");
        initiateDAD(linkLocalAddr, ie);
    }
    delete timerMsg;
}

void IPv6NeighbourDiscovery::initiateDAD(const IPv6Address& tentativeAddr,
    InterfaceEntry *ie)
{
    DADEntry *dadEntry = new DADEntry();
    dadEntry->interfaceId = ie->interfaceId();
    dadEntry->address = tentativeAddr;
    dadEntry->numNSSent = 0;
    dadList.insert(dadEntry);
    /*
    RFC2462: Section 5.4.2
    To check an address, a node sends DupAddrDetectTransmits Neighbor
    Solicitations, each separated by RetransTimer milliseconds. The
    solicitation's Target Address is set to the address being checked,
    the IP source is set to the unspecified address and the IP
    destination is set to the solicited-node multicast address of the
    target address.*/
    IPv6Address destAddr = tentativeAddr.formSolicitedNodeMulticastAddress();
    //Send a NS
    createAndSendNSPacket(tentativeAddr, destAddr, IPv6Address::UNSPECIFIED_ADDRESS, ie);
    dadEntry->numNSSent++;

    cMessage *msg = new cMessage("dadTimeout", MK_DAD_TIMEOUT);
    msg->setContextPointer(dadEntry);
    scheduleAt(simTime()+ie->ipv6()->retransTimer(), msg);
}

void IPv6NeighbourDiscovery::processDADTimeout(cMessage *msg)
{
    DADEntry *dadEntry = (DADEntry *)msg->contextPointer();
    InterfaceEntry *ie = (InterfaceEntry *)ift->interfaceAt(dadEntry->interfaceId);
    IPv6Address tentativeAddr = dadEntry->address;
    //Here, we need to check how many DAD messages for the interface entry were
    //sent vs. DupAddrDetectTransmits
    ev << "numOfDADMessagesSent is: " << dadEntry->numNSSent << endl;
    ev << "dupAddrDetectTrans is: " << ie->ipv6()->dupAddrDetectTransmits() << endl;
    if (dadEntry->numNSSent < ie->ipv6()->dupAddrDetectTransmits())
    {
        bubble("Sending another DAD NS message.");
        IPv6Address destAddr = tentativeAddr.formSolicitedNodeMulticastAddress();
        createAndSendNSPacket(dadEntry->address, destAddr, IPv6Address::UNSPECIFIED_ADDRESS, ie);
        dadEntry->numNSSent++;
        //Reuse the received msg
        scheduleAt(simTime()+ie->ipv6()->retransTimer(), msg);
    }
    else
    {
        bubble("Max number of DAD messages for interface sent. Address is unique.");
        ie->ipv6()->permanentlyAssign(tentativeAddr);
        dadList.erase(dadEntry);
        ev << "delete dadEntry and msg\n";
        delete dadEntry;
        delete msg;
        /*RFC 2461: Section 6.3.7 2nd Paragraph
        Before a host sends an initial solicitation, it SHOULD delay the
        transmission for a random amount of time between 0 and
        MAX_RTR_SOLICITATION_DELAY.  This serves to alleviate congestion when
        many hosts start up on a link at the same time, such as might happen
        after recovery from a power failure.*/
        //FIXME: Placing these operations here means fast router solicitation is
        //not adopted. Will relocate.
        if (ie->ipv6()->advSendAdvertisements() == false)
        {
            ev << "creating router discovery message timer\n";
            cMessage *rtrDisMsg = new cMessage("initiateRTRDIS",MK_INITIATE_RTRDIS);
            rtrDisMsg->setContextPointer(ie);
            simtime_t interval = uniform(0,ie->ipv6()->_maxRtrSolicitationDelay());//random delay
            scheduleAt(simTime()+interval, rtrDisMsg);
        }
    }
}

IPv6RouterSolicitation *IPv6NeighbourDiscovery::createAndSendRSPacket(InterfaceEntry *ie)
{
    ASSERT(ie->ipv6()->advSendAdvertisements() == false);
    //RFC 2461: Section 6.3.7 Sending Router Solicitations
    //A host sends Router Solicitations to the All-Routers multicast address. The
    //IP source address is set to either one of the interface's unicast addresses
    //or the unspecified address.
    IPv6Address myIPv6Address = ie->ipv6()->preferredAddress();
    if (myIPv6Address.isUnspecified())
        myIPv6Address = ie->ipv6()->linkLocalAddress();//so we use the link local address instead
    if (ie->ipv6()->isTentativeAddress(myIPv6Address))
        myIPv6Address = IPv6Address::UNSPECIFIED_ADDRESS;//set my IPv6 address to unspecified.
    IPv6Address destAddr = IPv6Address::ALL_ROUTERS_2;//all_routers multicast
    IPv6RouterSolicitation *rs = new IPv6RouterSolicitation("RSpacket");
    rs->setType(ICMPv6_ROUTER_SOL);

    //The Source Link-Layer Address option SHOULD be set to the host's link-layer
    //address, if the IP source address is not the unspecified address.
    if (!myIPv6Address.isUnspecified())
        rs->setSourceLinkLayerAddress(ie->macAddress());

    //Construct a Router Solicitation message
    sendPacketToIPv6Module(rs, destAddr, myIPv6Address, ie->outputPort());
    return rs;
}

void IPv6NeighbourDiscovery::initiateRouterDiscovery(cMessage *msg)
{
    ev << "Initiating Router Discovery" << endl;
    InterfaceEntry *ie = (InterfaceEntry *)msg->contextPointer();
    delete msg;
    //RFC2461: Section 6.3.7
    /*When an interface becomes enabled, a host may be unwilling to wait for the
    next unsolicited Router Advertisement to locate default routers or learn
    prefixes.  To obtain Router Advertisements quickly, a host SHOULD transmit up
    to MAX_RTR_SOLICITATIONS Router Solicitation messages each separated by at
    least RTR_SOLICITATION_INTERVAL seconds.(FIXME:Therefore this should be invoked
    at the beginning of the simulation-WEI)*/
    RDEntry *rdEntry = new RDEntry();
    rdEntry->interfaceId = ie->interfaceId();
    rdEntry->numRSSent = 0;
    createAndSendRSPacket(ie);
    rdEntry->numRSSent++;

    //Create and schedule a message for retransmission to this module
    cMessage *rdTimeoutMsg = new cMessage("processRDTimeout", MK_RD_TIMEOUT);
    rdTimeoutMsg->setContextPointer(ie); //Andras
    rdEntry->timeoutMsg = rdTimeoutMsg;
    rdList.insert(rdEntry);
    /*Before a host sends an initial solicitation, it SHOULD delay the
    transmission for a random amount of time between 0 and
    MAX_RTR_SOLICITATION_DELAY.  This serves to alleviate congestion when
    many hosts start up on a link at the same time, such as might happen
    after recovery from a power failure.  If a host has already performed
    a random delay since the interface became (re)enabled (e.g., as part
    of Duplicate Address Detection [ADDRCONF]) there is no need to delay
    again before sending the first Router Solicitation message.*/
    //simtime_t rndInterval = uniform(0, ie->ipv6()->_maxRtrSolicitationDelay());
    scheduleAt(simTime()+ie->ipv6()->_rtrSolicitationInterval(), rdTimeoutMsg);
}

void IPv6NeighbourDiscovery::cancelRouterDiscovery(InterfaceEntry *ie)
{
    //Next we retrieve the rdEntry with the Interface Entry.
    RDEntry *rdEntry = fetchRDEntry(ie);
    if (rdEntry != NULL)
    {
        ev << "rdEntry is not NULL, RD cancelled!" << endl;
        cancelEvent(rdEntry->timeoutMsg);
        rdList.erase(rdEntry);
        delete rdEntry;
    }
    else
        ev << "rdEntry is NULL, not cancelling RD!" << endl;
}

void IPv6NeighbourDiscovery::processRDTimeout(cMessage *msg)
{
    InterfaceEntry *ie = (InterfaceEntry *)msg->contextPointer();
    RDEntry *rdEntry = fetchRDEntry(ie);
    if (rdEntry->numRSSent < ie->ipv6()->_maxRtrSolicitations())
    {
        bubble("Sending another RS message.");
        createAndSendRSPacket(ie);
        rdEntry->numRSSent++;
        //Need to find out if this is the last RS we are sending out.
        if (rdEntry->numRSSent == ie->ipv6()->_maxRtrSolicitations())
            scheduleAt(simTime()+ie->ipv6()->_maxRtrSolicitationDelay(), msg);
        else
            scheduleAt(simTime()+ie->ipv6()->_rtrSolicitationInterval(), msg);
    }
    else
    {
        //RFC 2461, Section 6.3.7
        /*If a host sends MAX_RTR_SOLICITATIONS solicitations, and receives no Router
        Advertisements after having waited MAX_RTR_SOLICITATION_DELAY seconds after
        sending the last solicitation, the host concludes that there are no routers
        on the link for the purpose of [ADDRCONF]. However, the host continues to
        receive and process Router Advertisements messages in the event that routers
        appear on the link.*/
        bubble("Max number of RS messages sent");
        ev << "No RA messages were received. Assume no routers are on-link";
        delete rdEntry;
        delete msg;
    }
}

void IPv6NeighbourDiscovery::processRSPacket(IPv6RouterSolicitation *rs,
    IPv6ControlInfo *rsCtrlInfo)
{
    if (validateRSPacket(rs, rsCtrlInfo) == false) return;
    //Find out which interface the RS message arrived on.
    InterfaceEntry *ie = ift->interfaceByPortNo(rsCtrlInfo->inputGateIndex());
    AdvIfEntry *advIfEntry = fetchAdvIfEntry(ie);

    //RFC 2461: Section 6.2.6
    //A host MUST silently discard any received Router Solicitation messages.
    if (ie->ipv6()->advSendAdvertisements())
    {
        ev << "This is an advertising interface, processing RS\n";

        if (validateRSPacket(rs, rsCtrlInfo) == false) return;
        ev << "RS message validated\n";

        //First we extract RS specific information from the received message
        MACAddress macAddr = rs->sourceLinkLayerAddress();
        ev << "MAC Address extracted\n";
        delete rs;

        /*A router MAY choose to unicast the response directly to the soliciting
        host's address (if the solicitation's source address is not the unspecified
        address), but the usual case is to multicast the response to the
        all-nodes group. In the latter case, the interface's interval timer is
        reset to a new random value, as if an unsolicited advertisement had just
        been sent(see Section 6.2.4).*/

        /*In all cases, Router Advertisements sent in response to a Router
        Solicitation MUST be delayed by a random time between 0 and
        MAX_RA_DELAY_TIME seconds. (If a single advertisement is sent in
        response to multiple solicitations, the delay is relative to the
        first solicitation.)  In addition, consecutive Router Advertisements
        sent to the all-nodes multicast address MUST be rate limited to no
        more than one advertisement every MIN_DELAY_BETWEEN_RAS seconds.*/

        /*A router might process Router Solicitations as follows:
        - Upon receipt of a Router Solicitation, compute a random delay
        within the range 0 through MAX_RA_DELAY_TIME. If the computed
        value corresponds to a time later than the time the next multicast
        Router Advertisement is scheduled to be sent, ignore the random
        delay and send the advertisement at the already-scheduled time.*/
        cMessage *msg = new cMessage("sendSolicitedRA", MK_SEND_SOL_RTRADV);
        msg->setContextPointer(ie);
        simtime_t interval = uniform(0,ie->ipv6()->_maxRADelayTime());

        if (interval < advIfEntry->nextScheduledRATime)
        {
            simtime_t nextScheduledTime;
            nextScheduledTime = simTime()+interval;
            scheduleAt(nextScheduledTime, msg);
            advIfEntry->nextScheduledRATime = nextScheduledTime;
        }
        //else we ignore the generate interval and send it at the next scheduled time.

        //We need to keep a log here each time an RA is sent. Not implemented yet.
        //Assume the first course of action.
        /*- If the router sent a multicast Router Advertisement (solicited or
        unsolicited) within the last MIN_DELAY_BETWEEN_RAS seconds,
        schedule the advertisement to be sent at a time corresponding to
        MIN_DELAY_BETWEEN_RAS plus the random value after the previous
        advertisement was sent. This ensures that the multicast Router
        Advertisements are rate limited.

        - Otherwise, schedule the sending of a Router Advertisement at the
        time given by the random value.*/
    }
    else
    {
        ev << "This interface is a host, discarding RA message\n";
        delete rs;
    }
}

bool IPv6NeighbourDiscovery::validateRSPacket(IPv6RouterSolicitation *rs,
    IPv6ControlInfo *rsCtrlInfo)
{
    bool result = true;
    /*6.1.1.  Validation of Router Solicitation Messages
    A router MUST silently discard any received Router Solicitation
    messages that do not satisfy all of the following validity checks:

    - The IP Hop Limit field has a value of 255, i.e., the packet
    could not possibly have been forwarded by a router.*/
    if (rsCtrlInfo->hopLimit() != 255)
    {
        ev << "Hop limit is not 255! RS validation failed!\n";
        result = false;
    }
    //- ICMP Code is 0.
    if (rsCtrlInfo->protocol() != IP_PROT_IPv6_ICMP)
    {
        ev << "ICMP Code is not 0! RS validation failed!\n";
        result = false;
    }
    //- If the IP source address is the unspecified address, there is no
    //source link-layer address option in the message.
    if (rsCtrlInfo->srcAddr().isUnspecified())
    {
        ev << "IP source address is unspecified\n";
        if (rs->sourceLinkLayerAddress().isUnspecified() == false)
        {
            ev << " but source link layer address is provided. RS validation failed!\n";
        }
    }
    return result;
}

IPv6RouterAdvertisement *IPv6NeighbourDiscovery::createAndSendRAPacket(
    const IPv6Address& destAddr, InterfaceEntry *ie)
{
    ev << "Create and send RA invoked!\n";
    //Must use link-local addr. See: RFC2461 Section 6.1.2
    IPv6Address sourceAddr = ie->ipv6()->linkLocalAddress();
    int inputGateIndex = ie->outputPort();
    //This operation includes all options, regardless whether it is solicited or unsolicited.
    if (ie->ipv6()->advSendAdvertisements())//if this is an advertising interface
    {
        //Construct a Router Advertisment message
        IPv6RouterAdvertisement *ra = new IPv6RouterAdvertisement("RApacket");
        ra->setType(ICMPv6_ROUTER_AD);

        //RFC 2461: Section 6.2.3 Router Advertisment Message Content
        /*A router sends periodic as well as solicited Router Advertisements out
        its advertising interfaces.  Outgoing Router Advertisements are filled
        with the following values consistent with the message format given in
        Section 4.2:*/
        
        //- In the Router Lifetime field: the interface's configured AdvDefaultLifetime.
        ra->setRouterLifetime(ie->ipv6()->advDefaultLifetime());

        //- In the M and O flags: the interface's configured AdvManagedFlag and
        //AdvOtherConfigFlag, respectively.  See [ADDRCONF].
        ra->setManagedAddrConfFlag(ie->ipv6()->advManagedFlag());
        ra->setOtherStatefulConfFlag(ie->ipv6()->advOtherConfigFlag());
        
        //- In the Cur Hop Limit field: the interface's configured CurHopLimit.*shouldn't it be advCurHopLimit???
        ra->setCurHopLimit(ie->ipv6()->advCurHopLimit());

        //- In the Reachable Time field: the interface's configured AdvReachableTime.
        ra->setReachableTime(ie->ipv6()->advReachableTime());
        
        //- In the Retrans Timer field: the interface's configured AdvRetransTimer.
        ra->setRetransTimer(ie->ipv6()->advRetransTimer());

        //Possible Option
        ra->setSourceLinkLayerAddress(ie->macAddress());
        ra->setMTU(ie->ipv6()->advLinkMTU());

        //Add all Advertising Prefixes to the RA
        int numAdvPrefixes = ie->ipv6()->numAdvPrefixes();
        ev << "Number of Adv Prefixes: " << numAdvPrefixes << endl;
        ra->setPrefixInformationArraySize(numAdvPrefixes);
        for (int i = 0; i < numAdvPrefixes; i++)
        {
            IPv6InterfaceData::AdvPrefix advPrefix = ie->ipv6()->advPrefix(i);
            IPv6NDPrefixInformation prefixInfo;
            prefixInfo.setPrefixLength(advPrefix.prefixLength);
            prefixInfo.setOnlinkFlag(advPrefix.advOnLinkFlag);
            prefixInfo.setAutoAddressConfFlag(advPrefix.advAutonomousFlag);
            prefixInfo.setValidLifetime(advPrefix.advValidLifetime);
            prefixInfo.setPreferredLifetime(advPrefix.advPreferredLifetime);
            prefixInfo.setPrefix(advPrefix.prefix);
            ra->setPrefixInformation(i, prefixInfo);
        }
        sendPacketToIPv6Module(ra, destAddr, sourceAddr, inputGateIndex);
        return ra;
    }
}

void IPv6NeighbourDiscovery::processRAPacket(IPv6RouterAdvertisement *ra,
    IPv6ControlInfo *raCtrlInfo)
{
    int inputGateIndex = raCtrlInfo->inputGateIndex();
    InterfaceEntry *ie = ift->interfaceByPortNo(inputGateIndex);

    if (ie->ipv6()->advSendAdvertisements())
    {
        ev << "Interface is an advertising interface, dropping RA message.\n";
        delete ra;
        return;
    }
    else
    {
        if (validateRAPacket(ra, raCtrlInfo) == false)
        {
            delete ra;
            return;
        }
        cancelRouterDiscovery(ie);
        ev << "Interface is a host, processing RA.\n";
        //RFC2461: Section 6.3.4
        //On receipt of a valid Router Advertisement, a host extracts the source
        //address of the packet and does the following:
        updateRouterList(ra, raCtrlInfo);

        //We need to unpack information from the RA message
        uint32 curHopLimit = ra->curHopLimit();
        bool managedAddrConfFlag = ra->managedAddrConfFlag();//M bit/flag
        bool otherStatefulConfFlag = ra->otherStatefulConfFlag();//O bit/flag
        unsigned short routerLifetime = ra->routerLifetime();//AdvDefaultLifetime
        uint32 reachableTime = ra->reachableTime();//AdvReachableTime
        uint32 retransTimer = ra->retransTimer();//AdvRetransTimer
        //Possible options
        MACAddress macAddress = ra->sourceLinkLayerAddress();
        uint mtu = ra->MTU();
        for (int i = 0; i < ra->prefixInformationArraySize(); i++)
        {
            IPv6NDPrefixInformation& prefixInfo = ra->prefixInformation(i);

            if (prefixInfo.autoAddressConfFlag() == true)//If auto addr conf is set
                processRAPrefixInfoForAutoConf(prefixInfo, ie);//We process prefix Info and form an addr
                
        }
    }
    delete raCtrlInfo;
    delete ra;
}

void IPv6NeighbourDiscovery::updateRouterList(IPv6RouterAdvertisement *ra,
    IPv6ControlInfo *raCtrlInfo)
{
    IPv6Address raSrcAddr = raCtrlInfo->srcAddr();
    InterfaceEntry *ie = ift->interfaceByPortNo(raCtrlInfo->inputGateIndex());
    int ifID = ie->interfaceId();
    MACAddress macAddress = ra->sourceLinkLayerAddress();
    /*- If the address is not already present in the host's Default Router List,
    and the advertisement's Router Lifetime is non-zero, create a new entry in
    the list, and initialize its invalidation timer value from the advertisement's
    Router Lifetime field.*/
    Neighbour *neighbour = neighbourCache.lookup(raSrcAddr, ifID);
    if (neighbour == NULL)
    {
        if (macAddress.isUnspecified())
            neighbourCache.addRouter(raSrcAddr, ifID, 0);//FIXME!
        else
            neighbourCache.addRouter(raSrcAddr, ifID, macAddress, 0);//FIXME!
    }
    else
    {
        /*- If the address is already present in the host's Default Router
        List as a result of a previously-received advertisement, reset
        its invalidation timer to the Router Lifetime value in the
        newly-received advertisement.*/

        /*- If the address is already present in the host's Default Router
        List and the received Router Lifetime value is zero, immediately
        time-out the entry as specified in Section 6.3.5.*/
    }
}

void IPv6NeighbourDiscovery::processRAPrefixInfoForAutoConf(
    IPv6NDPrefixInformation& prefixInfo, InterfaceEntry *ie)
{
    ev << "Processing Prefix Info for auto-configuration.\n";
    IPv6Address prefix = prefixInfo.prefix();
    uint prefixLength = prefixInfo.prefixLength();
    simtime_t preferredLifetime = prefixInfo.preferredLifetime();
    simtime_t validLifetime = prefixInfo.validLifetime();

    //RFC 2461: Section 5.5.3
    //First condition tested, the autonomous flag is already set
    
    //b) If the prefix is the link-local prefix, silently ignore the Prefix
    //Information option.
    if (prefixInfo.prefix().isLinkLocal() == true)
    {
        ev << "Prefix is link-local, ignore Prefix Information Option\n";
        return;
    }

    //c) If the preferred lifetime is greater than the valid lifetime, silently
    //ignore the Prefix Information option. A node MAY wish to log a system
    //management error in this case.
    if (preferredLifetime > validLifetime)
    {
        ev << "Preferred lifetime is greater than valid lifetime, ignore Prefix Information\n";
        return;
    }

    bool isPrefixAssignedToInterface = false;
    for (int i = 0; i < ie->ipv6()->numAddresses(); i++)
    {
        if (ie->ipv6()->address(i).matches(prefix, prefixLength) == true)
            isPrefixAssignedToInterface = true;
    }
    /*d) If the prefix advertised does not match the prefix of an address already
         in the list, and the Valid Lifetime is not 0, form an address (and add
         it to the list) by combining the advertised prefix with the link’s
         interface identifier as follows:*/
    if (isPrefixAssignedToInterface == false && validLifetime != 0)
    {
        IPv6Address linkLocalAddress = ie->ipv6()->linkLocalAddress();
        ASSERT(linkLocalAddress.isUnspecified() == false);
        IPv6Address newAddr = linkLocalAddress.setPrefix(prefix, prefixLength);
        //FIXME: for now we leave the newly formed address as not tentative,
        //according to Greg, we have to always perform DAD for a newly formed address.
        ev << "Assigning new address to: " << ie->name() << endl;
        ie->ipv6()->assignAddress(newAddr, false, validLifetime, preferredLifetime);
    }
    
    //FIXME: this is the simplified version.
    /*e) If the advertised prefix matches the prefix of an autoconfigured
       address (i.e., one obtained via stateless or stateful address
       autoconfiguration) in the list of addresses associated with the
       interface, the specific action to perform depends on the Valid
       Lifetime in the received advertisement and the Lifetime
       associated with the previously autoconfigured address (which we
       call StoredLifetime in the discussion that follows):

       1) If the received Lifetime is greater than 2 hours or greater
          than StoredLifetime, update the stored Lifetime of the
          corresponding address.

       2) If the StoredLifetime is less than or equal to 2 hours and the
          received Lifetime is less than or equal to StoredLifetime,
          ignore the prefix, unless the Router Advertisement from which

          this Prefix Information option was obtained has been
          authenticated (e.g., via IPSec [RFC2402]). If the Router
          Advertisment was authenticated, the StoredLifetime should be
          set to the Lifetime in the received option.

       3) Otherwise, reset the stored Lifetime in the corresponding
          address to two hours.*/
    
}

void IPv6NeighbourDiscovery::createRATimer(InterfaceEntry *ie)
{
    cMessage *msg = new cMessage("sendPeriodicRA", MK_SEND_PERIODIC_RTRADV);
    msg->setContextPointer(ie);
    AdvIfEntry *advIfEntry = new AdvIfEntry();
    advIfEntry->interfaceId = ie->interfaceId();
    advIfEntry->numRASent = 0;
    simtime_t interval
        = uniform(ie->ipv6()->minRtrAdvInterval(),ie->ipv6()->maxRtrAdvInterval());
    advIfEntry->raTimeoutMsg = msg;
    
    simtime_t nextScheduledTime = simTime() + interval;
    advIfEntry->nextScheduledRATime = nextScheduledTime;
    advIfList.insert(advIfEntry);
    ev << "Interval: " << interval << endl;
    ev << "Next scheduled time: " << nextScheduledTime << endl;
    //now we schedule the msg for whatever time that was derived
    scheduleAt(nextScheduledTime, msg);
}

void IPv6NeighbourDiscovery::resetRATimer(InterfaceEntry *ie)
{//Not used yet but could be useful later on.-WEI
    //Iterate through all RA timers within the Neighbour Discovery module.
/*
    for (RATimerList::iterator it=raTimerList.begin(); it != raTimerList.end(); it++)
    {
        cMessage *msg = (*it);
        InterfaceEntry *msgIE = (InterfaceEntry *)msg->contextPointer();
        //Find the timer that matches the given Interface Entry.
        if (msgIE->outputPort() == ie->outputPort())
        {
            ev << "Resetting RA timer for port: " << ie->outputPort();
            cancelEvent(msg);//Cancel the next scheduled msg.
            simtime_t interval
                = uniform(ie->ipv6()->minRtrAdvInterval(),ie->ipv6()->maxRtrAdvInterval());
            scheduleAt(simTime()+interval, msg);
        }
    }
*/
}

void IPv6NeighbourDiscovery::sendPeriodicRA(cMessage *msg)
{
    InterfaceEntry *ie = (InterfaceEntry *)msg->contextPointer();
    AdvIfEntry *advIfEntry = fetchAdvIfEntry(ie);
    IPv6Address destAddr = IPv6Address("FF02::1");
    createAndSendRAPacket(destAddr, ie);
    advIfEntry->numRASent++;

    //RFC 2461, Section 6.2.4
    /*Whenever a multicast advertisement is sent from an interface, the timer is
    reset to a uniformly-distributed random value between the interface's
    configured MinRtrAdvInterval and MaxRtrAdvInterval; expiration of the timer
    causes the next advertisement to be sent and a new random value to be chosen.*/
    simtime_t interval
        = uniform(ie->ipv6()->minRtrAdvInterval(),ie->ipv6()->maxRtrAdvInterval());

    /*For the first few advertisements (up to MAX_INITIAL_RTR_ADVERTISEMENTS)
    sent from an interface when it becomes an advertising interface, if the
    randomly chosen interval is greater than MAX_INITIAL_RTR_ADVERT_INTERVAL, the
    timer SHOULD be set to MAX_INITIAL_RTR_ADVERT_INTERVAL instead.  Using a
    smaller interval for the initial advertisements increases the likelihood of
    a router being discovered quickly when it first becomes available, in the
    presence of possible packet loss.*/

    simtime_t nextScheduledTime;
    ev << "Num RA sent is: " << advIfEntry->numRASent << endl;
    ev << "maxInitialRtrAdvertisements is: " << ie->ipv6()->_maxInitialRtrAdvertisements() << endl;
    if(advIfEntry->numRASent <= ie->ipv6()->_maxInitialRtrAdvertisements())
    {
        if (interval > ie->ipv6()->_maxInitialRtrAdvertInterval())
        {
            nextScheduledTime = simTime() + ie->ipv6()->_maxInitialRtrAdvertInterval();
            ev << "Sending initial RA but interval is too long. Using default value." << endl;
        }
        else
        {
            nextScheduledTime = simTime() + interval;
            ev << "Sending initial RA. Using randomly generated interval." << endl;
        }
    }
    ev << "Next scheduled time: " << nextScheduledTime;
    advIfEntry->nextScheduledRATime = nextScheduledTime;
    ASSERT(nextScheduledTime > simTime());
    scheduleAt(nextScheduledTime, msg);
}

void IPv6NeighbourDiscovery::sendSolicitedRA(cMessage *msg)
{
    ev << "Send Solicited RA invoked!\n";
    InterfaceEntry *ie = (InterfaceEntry *)msg->contextPointer();
    IPv6Address destAddr = IPv6Address("FF02::1");
    ev << "Testing condition!\n";
    createAndSendRAPacket(destAddr, ie);
    delete msg;
}

bool IPv6NeighbourDiscovery::validateRAPacket(IPv6RouterAdvertisement *ra,
    IPv6ControlInfo *raCtrlInfo)
{
    bool result = true;

    //RFC 2461: Section 6.1.2 Validation of Router Advertisement Messages
    /*A node MUST silently discard any received Router Advertisement
    messages that do not satisfy all of the following validity checks:*/
    raCtrlInfo->srcAddr();
    //- IP Source Address is a link-local address.  Routers must use
    //  their link-local address as the source for Router Advertisement
    //  and Redirect messages so that hosts can uniquely identify
    //  routers.
    if (raCtrlInfo->srcAddr().isLinkLocal() == false)
    {
        ev << "RA source address is not link-local. RA validation failed!\n";
        result = false;
    }

    //- The IP Hop Limit field has a value of 255, i.e., the packet
    //  could not possibly have been forwarded by a router.
    if (raCtrlInfo->hopLimit() != 255)
    {
        ev << "Hop limit is not 255! RA validation failed!\n";
        result = false;
    }

    //- ICMP Code is 0.
    if (raCtrlInfo->protocol() != IP_PROT_IPv6_ICMP)
    {
        ev << "ICMP Code is not 0! RA validation failed!\n";
        result = false;
    }

    return result;
}

IPv6NeighbourSolicitation *IPv6NeighbourDiscovery::createAndSendNSPacket(
    const IPv6Address& nsTargetAddr, const IPv6Address& dgDestAddr,
    const IPv6Address& dgSrcAddr, InterfaceEntry *ie)
{
    MACAddress myMacAddr = ie->macAddress();
    int inputGateIndex = ie->outputPort();

    //Construct a Neighbour Solicitation message
    IPv6NeighbourSolicitation *ns = new IPv6NeighbourSolicitation("NSpacket");
    ns->setType(ICMPv6_NEIGHBOUR_SOL);

    //Neighbour Solicitation Specific Information
    ns->setTargetAddress(nsTargetAddr);

    /*If the solicitation is being sent to a solicited-node multicast
    address, the sender MUST include its link-layer address (if it has
    one) as a Source Link-Layer Address option.*/
    if (dgDestAddr.matches(IPv6Address("FF02::1:FF00:0"),104))
        ns->setSourceLinkLayerAddress(myMacAddr);

    sendPacketToIPv6Module(ns, dgDestAddr, dgSrcAddr, inputGateIndex);
    return ns;
}

void IPv6NeighbourDiscovery::processNSPacket(IPv6NeighbourSolicitation *ns,
    IPv6ControlInfo *nsCtrlInfo)
{
    //Control Information
    int nsInputGate = nsCtrlInfo->inputGateIndex();

    IPv6Address nsTargetAddr = ns->targetAddress();
    InterfaceEntry *ie = ift->interfaceByPortNo(nsInputGate);

    //RFC 2461:Section 7.2.3
    //If target address is not a valid "unicast" or anycast address assigned to the
    //receiving interface, we should silently discard the packet.
    if (validateNSPacket(ns, nsCtrlInfo) == false
        || ie->ipv6()->hasAddress(nsTargetAddr) == false)
    {
        bubble("NS validation failed\n");
        delete nsCtrlInfo;
        delete ns;
        return;
    }
    bubble("NS validation passed.\n");
    if (ie->ipv6()->isTentativeAddress(nsTargetAddr))
    {
        //If the Target Address is tentative, the Neighbor Solicitation should
        //be processed as described in [ADDRCONF].
        ev << "Process NS for Tentative target address.\n";
        processNSForTentativeAddress(ns, nsCtrlInfo);
    }
    else
    {
        //Otherwise, the following description applies.
        ev << "Process NS for Non-Tentative target address.\n";
        processNSForNonTentativeAddress(ns, nsCtrlInfo, ie);
    }
    delete nsCtrlInfo;
    delete ns;
}

bool IPv6NeighbourDiscovery::validateNSPacket(IPv6NeighbourSolicitation *ns,
    IPv6ControlInfo *nsCtrlInfo)
{
    bool result = true;
    /*RFC 2461:7.1.1. Validation of Neighbor Solicitations(some checks are omitted)
    A node MUST silently discard any received Neighbor Solicitation
    messages that do not satisfy all of the following validity checks:*/
    //- The IP Hop Limit field has a value of 255, i.e., the packet
    //could not possibly have been forwarded by a router.
    if (nsCtrlInfo->hopLimit() != 255)
    {
        ev << "Hop limit is not 255! NS validation failed!\n";
        result = false;
    }
    //- Target Address is not a multicast address.
    if (ns->targetAddress().isMulticast() == true)
    {
        ev << "Target address is a multicast address! NS validation failed!\n";
        result = false;
    }
    //- If the IP source address is the unspecified address,
    if (nsCtrlInfo->srcAddr().isUnspecified())
    {
        ev << "Source Address is unspecified\n";
        //the IP destination address is a solicited-node multicast address.
        if (nsCtrlInfo->destAddr().matches(IPv6Address::SOLICITED_NODE_PREFIX,104) == false)
        {
            ev << " but IP dest address is not a solicited-node multicast address! NS validation failed!\n";
            result = false;
        }
        //there is no source link-layer address option in the message.
        else if (ns->sourceLinkLayerAddress().isUnspecified() == false)
        {
            ev << " but Source link-layer address is not empty! NS validation failed!\n";
            result = false;
        }
        else
            ev << "NS Validation Passed\n";
    }

    return result;
}

void IPv6NeighbourDiscovery::processNSForTentativeAddress(IPv6NeighbourSolicitation *ns,
    IPv6ControlInfo *nsCtrlInfo)
{
    //Control Information
    IPv6Address nsSrcAddr = nsCtrlInfo->srcAddr();
    IPv6Address nsDestAddr = nsCtrlInfo->destAddr();
    int inputGateIndex = nsCtrlInfo->inputGateIndex();

    ASSERT(nsSrcAddr.isUnicast() || nsSrcAddr.isUnspecified());
    //solicitation is processed as described in RFC2462:section 5.4.3

    if (nsSrcAddr.isUnspecified())
    {
        ev << "Source Address is UNSPECIFIED. Sender is performing DAD\n";
        //Sender performing Duplicate Address Detection
        if (rt6->localDeliver(nsSrcAddr))
            ev << "NS comes from myself. Ignoring NS\n";
        else
            ev << "NS comes from another node. Address is duplicate!\n";
            error("Duplicate Address Detected! Manual Attention Required!");
    }
    else if (nsSrcAddr.isUnicast())
    {
        //Sender performing address resolution
        ev << "Sender is performing Address Resolution\n";
        ev << "Target Address is tentative. Ignoring NS.\n";
    }
}

void IPv6NeighbourDiscovery::processNSForNonTentativeAddress(IPv6NeighbourSolicitation *ns,
    IPv6ControlInfo *nsCtrlInfo, InterfaceEntry *ie)
{
    //Neighbour Solicitation Information
    MACAddress nsMacAddr = ns->sourceLinkLayerAddress();

    int ifID = ie->interfaceId();

    //target addr is not tentative addr
    //solicitation processed as described in RFC2461:section 7.2.3
    if (nsCtrlInfo->srcAddr().isUnspecified())
    {
        ev << "Address is duplicate! Inform Sender of duplicate address!\n";
        sendSolicitedNA(ns, nsCtrlInfo, ie);
    }
    else
    {
        processNSWithSpecifiedSrcAddr(ns, nsCtrlInfo, ie);
    }
}

void IPv6NeighbourDiscovery::processNSWithSpecifiedSrcAddr(IPv6NeighbourSolicitation *ns,
    IPv6ControlInfo *nsCtrlInfo, InterfaceEntry *ie)
{
    /*If the Source Address is not the unspecified address and, on link layers
    that have addresses, the solicitation includes a Source Link-Layer Address
    option, then the recipient SHOULD create or update the Neighbor Cache entry
    for the IP Source Address of the solicitation.*/

    //Neighbour Solicitation Information
    MACAddress nsMacAddr = ns->sourceLinkLayerAddress();

    int ifID = ie->interfaceId();
    ASSERT(!nsMacAddr.isUnspecified());

    //Look for the Neighbour Cache Entry
    Neighbour *entry = neighbourCache.lookup(nsCtrlInfo->srcAddr(), ifID);

    if (entry == NULL)
    {
        /*If an entry does not already exist, the node SHOULD create a new one
        and set its reachability state to STALE as specified in Section 7.3.3.*/
        ev << "Neighbour Entry not found. Create a new Neighbour Cache Entry.\n";
        neighbourCache.addNeighbour(nsCtrlInfo->srcAddr(), ifID, nsMacAddr);
    }
    else
    {
        /*If an entry already exists, and the cached link-layer address differs from
        the one in the received Source Link-Layer option,*/
        if (entry->macAddress.equals(nsMacAddr) == false)
        {
            //the cached address should be replaced by the received address
            entry->macAddress = nsMacAddr;
            //and the entry's reachability state MUST be set to STALE.
            entry->reachabilityState = IPv6NeighbourCache::STALE;
        }
    }
    /*After any updates to the Neighbor Cache, the node sends a Neighbor
    Advertisement response as described in the next section.*/
    sendSolicitedNA(ns, nsCtrlInfo, ie);
}

/*we may never need this
IPv6NeighbourAdvertisement *IPv6NeighbourDiscovery::createAndSendNAPacket(
    IPv6NeighbourSolicitation *ns, const IPv6Address& nsSrcAddr,
    const IPv6Address& nsDestAddr, InterfaceEntry *ie)
{
    //Neighbour Solicitation Information
    IPv6Address nsTargetAddr = ns->targetAddress();
    MACAddress nsMacAddr = ns->sourceLinkLayerAddress();

    MACAddress myMacAddr = ie->macAddress();
    IPv6Address myIPv6Addr = ie->ipv6()->preferredAddress();
    int outputPort = ie->outputPort();

    //Construct a Neighbour Advertisment message
    IPv6NeighbourAdvertisement *na = new IPv6NeighbourAdvertisement("NApacket");
    na->setType(ICMPv6_NEIGHBOUR_AD);
    na->setTargetAddress(nsTargetAddr);

    //Logic as defined in section 7.2.4
    if (nsDestAddr.isMulticast())
        na->setTargetLinkLayerAddress(myMacAddr);

    na->setRouterFlag(rt6->isRouter());

    //if nsTargetAddr is unicast or is empty, set override flag to FALSE.
    na->setOverrideFlag(!(nsTargetAddr.isUnicast() || nsMacAddr.isUnspecified()));

    // add IPv6 control info with NA packet
    if (nsSrcAddr.isUnspecified())
    {
        na->setSolicitedFlag(false);
        sendPacketToIPv6Module(na, IPv6Address::ALL_NODES_2, myIPv6Addr, outputPort);
    }
    else
    {
        na->setSolicitedFlag(true);
        sendPacketToIPv6Module(na, nsSrcAddr, myIPv6Addr, outputPort);
    }
    return na;
}*/

void IPv6NeighbourDiscovery::sendSolicitedNA(IPv6NeighbourSolicitation *ns,
    IPv6ControlInfo *nsCtrlInfo, InterfaceEntry *ie)
{
    IPv6NeighbourAdvertisement *na = new IPv6NeighbourAdvertisement("NApacket");
    //RFC 2461: Section 7.2.4
    /*A node sends a Neighbor Advertisement in response to a valid Neighbor
    Solicitation targeting one of the node's assigned addresses.  The
    Target Address of the advertisement is copied from the Target Address
    of the solicitation.*/
    na->setTargetAddress(ns->targetAddress());

    /*If the solicitation's IP Destination Address is not a multicast address,
    the Target Link-Layer Address option MAY be omitted; the neighboring node's
    cached value must already be current in order for the solicitation to have
    been received. If the solicitation's IP Destination Address is a multicast
    address, the Target Link-Layer option MUST be included in the advertisement.*/
    na->setTargetLinkLayerAddress(ie->macAddress());//here, we always include the MAC addr.
    
    /*Furthermore, if the node is a router, it MUST set the Router flag to one;
    otherwise it MUST set the flag to zero.*/
    na->setRouterFlag(rt6->isRouter());

    /*If the Target Address is either an anycast address or a unicast
    address for which the node is providing proxy service, or the Target
    Link-Layer Address option is not included,*/
    if (ns->targetAddress().isUnicast() || ns->sourceLinkLayerAddress().isUnspecified())
        //the Override flag SHOULD be set to zero.
        na->setOverrideFlag(false);
    else
        //Otherwise, the Override flag SHOULD be set to one.
        na->setOverrideFlag(true);
    /*Proper setting of the Override flag ensures that nodes give preference to
    non-proxy advertisements, even when received after proxy advertisements, and
    also ensures that the first advertisement for an anycast address "wins".*/

    IPv6Address naDestAddr;
    //If the source of the solicitation is the unspecified address,
    if(nsCtrlInfo->srcAddr().isUnspecified())
    {
        /*the node MUST set the Solicited flag to zero and multicast the advertisement
        to the all-nodes address.*/
        na->setSolicitedFlag(false);
        naDestAddr = IPv6Address::ALL_NODES_2;
    }
    else
    {
        /*Otherwise, the node MUST set the Solicited flag to one and unicast
        the advertisement to the Source Address of the solicitation.*/
        na->setSolicitedFlag(true);
        naDestAddr = nsCtrlInfo->srcAddr();
    }
    
    /*If the Target Address is an anycast address the sender SHOULD delay sending
    a response for a random time between 0 and MAX_ANYCAST_DELAY_TIME seconds.*/
    /*TODO: More associated complexity for this one. We will have to delay
    sending off the solicitation. Perhaps the self message could have a context
    pointer pointing to a struct with enough info to create and send a NA packet.*/

    /*Because unicast Neighbor Solicitations are not required to include a
    Source Link-Layer Address, it is possible that a node sending a
    solicited Neighbor Advertisement does not have a corresponding link-
    layer address for its neighbor in its Neighbor Cache.  In such
    situations, a node will first have to use Neighbor Discovery to
    determine the link-layer address of its neighbor (i.e, send out a
    multicast Neighbor Solicitation).*/
    IPv6Address myIPv6Addr = ie->ipv6()->preferredAddress();
    sendPacketToIPv6Module(na, naDestAddr, myIPv6Addr, ie->outputPort());
}

void IPv6NeighbourDiscovery::sendUnsolicitedNA(InterfaceEntry *ie)
{
    //RFC 2461
    //Section 7.2.6: Sending Unsolicited Neighbor Advertisements

    /*In some cases a node may be able to determine that its link-layer
    address has changed (e.g., hot-swap of an interface card) and may
    wish to inform its neighbors of the new link-layer address quickly.
    In such cases a node MAY send up to MAX_NEIGHBOR_ADVERTISEMENT
    unsolicited Neighbor Advertisement messages to the all-nodes
    multicast address.  These advertisements MUST be separated by at
    least RetransTimer seconds.

    The Target Address field in the unsolicited advertisement is set to
    an IP address of the interface, and the Target Link-Layer Address
    option is filled with the new link-layer address.  The Solicited flag
    MUST be set to zero, in order to avoid confusing the Neighbor
    Unreachability Detection algorithm.  If the node is a router, it MUST
    set the Router flag to one; otherwise it MUST set it to zero.  The
    Override flag MAY be set to either zero or one.  In either case,
    neighboring nodes will immediately change the state of their Neighbor
    Cache entries for the Target Address to STALE, prompting them to
    verify the path for reachability.  If the Override flag is set to
    one, neighboring nodes will install the new link-layer address in
    their caches.  Otherwise, they will ignore the new link-layer
    address, choosing instead to probe the cached address.

    A node that has multiple IP addresses assigned to an interface MAY
    multicast a separate Neighbor Advertisement for each address.  In
    such a case the node SHOULD introduce a small delay between the
    sending of each advertisement to reduce the probability of the
    advertisements being lost due to congestion.

    A proxy MAY multicast Neighbor Advertisements when its link-layer
    address changes or when it is configured (by system management or
    other mechanisms) to proxy for an address.  If there are multiple
    nodes that are providing proxy services for the same set of addresses
    the proxies SHOULD provide a mechanism that prevents multiple proxies
    from multicasting advertisements for any one address, in order to
    reduce the risk of excessive multicast traffic.

    Also, a node belonging to an anycast address MAY multicast
    unsolicited Neighbor Advertisements for the anycast address when the
    node's link-layer address changes.

    Note that because unsolicited Neighbor Advertisements do not reliably
    update caches in all nodes (the advertisements might not be received
    by all nodes), they should only be viewed as a performance
    optimization to quickly update the caches in most neighbors.  The
    Neighbor Unreachability Detection algorithm ensures that all nodes
    obtain a reachable link-layer address, though the delay may be
    slightly longer.*/
}

void IPv6NeighbourDiscovery::processNAPacket(IPv6NeighbourAdvertisement *na,
    IPv6ControlInfo *naCtrlInfo)
{
    if (validateNAPacket(na, naCtrlInfo) == false)
    {
        delete na;
        return;
    }

    //Control Information
    int inputGateIndex = naCtrlInfo->inputGateIndex();
    delete naCtrlInfo;

    //Neighbour Advertisement Information
    IPv6Address naTargetAddr = na->targetAddress();

    //First, we check if the target address in NA is found in the interface it
    //was received on is tentative.
    InterfaceEntry *ie = ift->interfaceByPortNo(inputGateIndex);
    if (ie->ipv6()->isTentativeAddress(naTargetAddr))
    {
        error("Duplicate Address Detected! Manual attention needed!");
    }
    //Logic as defined in Section 7.2.5
    Neighbour *neighbourEntry = neighbourCache.lookup(naTargetAddr, ie->interfaceId());

    if (neighbourEntry == NULL)
    {
        ev << "NA received. Target Address not found in Neighbour Cache\n";
        ev << "Dropping NA packet.\n";
        delete na;
        return;
    }

    //Target Address has entry in Neighbour Cache
    ev << "NA received. Target Address found in Neighbour Cache\n";

    if (neighbourEntry->reachabilityState == IPv6NeighbourCache::INCOMPLETE)
        processNAForIncompleteNeighbourEntry(na, neighbourEntry);
    else
        processNAForExistingNeighborEntry(na, neighbourEntry);
}

bool IPv6NeighbourDiscovery::validateNAPacket(IPv6NeighbourAdvertisement *na,
    IPv6ControlInfo *naCtrlInfo)
{
    bool result = true;//adopt optimistic approach

    //RFC 2461:7.1.2 Validation of Neighbor Advertisments(some checks are omitted)
    //A node MUST silently discard any received Neighbor Advertisment messages
    //that do not satisfy all of the following validity checks:

    //- The IP Hop Limit field has a value of 255, i.e., the packet
    //  could not possibly have been forwarded by a router.
    if (naCtrlInfo->hopLimit() != 255)
    {
        ev << "Hop Limit is not 255! NA validation failed!\n";
        result = false;
    }

    //- Target Address is not a multicast address.
    if (na->targetAddress().isMulticast() == true)
    {
        ev << "Target Address is a multicast address! NA validation failed!\n";
        result = false;
    }

    //- If the IP Destination Address is a multicast address the Solicited flag
    //  is zero.
    if (naCtrlInfo->destAddr().isMulticast())
    {
        if (na->solicitedFlag() == true)
        {
            ev << "Dest Address is multicast address but solicted flag is 0!\n";
            result = false;
        }
    }

    if (result == true) bubble("NA validation passed.");
    else bubble("NA validation failed.");
    return result;
}

void IPv6NeighbourDiscovery::processNAForIncompleteNeighbourEntry(
                    IPv6NeighbourAdvertisement *na, Neighbour *neighbourEntry)
{
    MACAddress naMacAddr = na->targetLinkLayerAddress();
    bool naRouterFlag = na->routerFlag();
    bool naSolicitedFlag = na->solicitedFlag();

    /*If the target's neighbour Cache entry is in the INCOMPLETE state when the
    advertisement is received, one of two things happens.*/
    if (naMacAddr.isUnspecified())
    {
        /*If the link layer has addresses and no Target Link-Layer address option
        is included, the receiving node SHOULD silently discard the received
        advertisement.*/
        ev << "No MAC Address specified in NA. Ignoring NA\n";
        delete na;
    }
    else
    {
        //Otherwise, the receiving node performs the following steps:
        //- It records the link-layer address in the neighbour Cache entry.
        ev << "ND is updating Neighbour Cache Entry.\n";
        neighbourEntry->macAddress = naMacAddr;

        //- If the advertisement's Solicited flag is set, the state of the
        //  entry is set to REACHABLE, otherwise it is set to STALE.
        if (naSolicitedFlag == true)
            neighbourEntry->reachabilityState = IPv6NeighbourCache::REACHABLE;
        else
            neighbourEntry->reachabilityState = IPv6NeighbourCache::STALE;

        //- It sets the IsRouter flag in the cache entry based on the Router
        //  flag in the received advertisement.
        neighbourEntry->isRouter = naRouterFlag;

        //- It sends any packets queued for the neighbour awaiting address
        //  resolution.
        sendQueuedPacketToIPv6Module();
    }
}

void IPv6NeighbourDiscovery:: processNAForExistingNeighborEntry(
                    IPv6NeighbourAdvertisement *na, Neighbour *neighbourEntry)
{
    bool naRouterFlag = na->routerFlag();
    bool naSolicitedFlag = na->solicitedFlag();
    bool naOverrideFlag = na->overrideFlag();
    MACAddress naMacAddr = na->targetLinkLayerAddress();

    //If the target's Neighbor Cache entry is in any state other than
    //INCOMPLETE when the advertisement is received, processing becomes
    //quite a bit more complex.

    //If the Override flag is clear
    if (naOverrideFlag == false)
    {
        //and the supplied link-layer address differs from that in the
        //cache, then one of two actions takes place:
        if (!(naMacAddr.equals(neighbourEntry->macAddress)))
        {
            //if the state of the entry is REACHABLE, set it to STALE, but do
            //not update the entry in any other way;
            ev << "Override is FALSE but MAC is different. Set state to STALE.\n";
            if (neighbourEntry->reachabilityState == IPv6NeighbourCache::REACHABLE)
                neighbourEntry->reachabilityState = IPv6NeighbourCache::STALE;
            delete na;
        }
        else
        {
            //otherwise, the received advertisement should be ignored and
            //MUST NOT update the cache.
            ev << "Override is FALSE but MAC is same. Ignoring NA\n";
            delete na;
        }
    }
    else
    {
        //If the Override flag is set, both the Override flag is clear(ambiguous)
        //and the supplied link-layer address is the same as that in the cache,
        //or no Target Link-layer address option was supplied,

        //the received advertisement MUST update the Neighbor Cache entry as
        //follows:

        //- The link-layer address in the Target Link-Layer Address option
        //  MUST be inserted in the cache (if one is supplied and is different
        //  than the already recorded address).
        if (!(neighbourEntry->macAddress.isUnspecified()))
        {
            if (!naMacAddr.equals(neighbourEntry->macAddress))
            {
                ev << "Override is TRUE, MAC is different. Updating MAC entry\n";
                neighbourEntry->macAddress = naMacAddr;
        //- If the Solicited flag is set, the state of the entry MUST be set
        //  to REACHABLE.  If the Solicited flag is zero and the link-layer
        //  address was updated with a different address the state MUST be set
        //  to STALE.
                if (naSolicitedFlag == true)
                    neighbourEntry->reachabilityState = IPv6NeighbourCache::REACHABLE;
                else
                    neighbourEntry->reachabilityState = IPv6NeighbourCache::STALE;
            }
            else
            {
                ev << "Override is TRUE, MAC is the same. No update is needed\n";

            }
        }
        else
        {
            ev << "Override is TRUE but no MAC given.\n";
            if (naSolicitedFlag == true)
            {
                ev << "Reachability confirmed.\n";
                neighbourEntry->reachabilityState = IPv6NeighbourCache::REACHABLE;
                //TODO: reset reachabilityTime.
            }
            //else there is no need to update state for unsolicited NA
            //that do not change contents of cache.
        }

        //- The IsRouter flag in the cache entry MUST be set based on the
        //  Router flag in the received advertisement.
        //  This is needed to detect when a node that is used as a router
        //  stops forwarding packets due to being configured as a host.
        if (naRouterFlag != neighbourEntry->isRouter)
        {
            ev << "Router flag has changed, updating changes.\n";
            neighbourEntry->isRouter = naRouterFlag;
            if (naRouterFlag == true)
                ev << "Neighbour is a Router. Updating Default Router List\n";
                //TODO: update Default Router List
            else
                //In those cases where the IsRouter flag changes from
                //TRUE to FALSE as a result of this update, the node
                //MUST remove that router from the Default Router List and
                //update the Destination Cache entries for all destinations
                //using that neighbor as a router as specified in Section 7.3.3.
                ev << "Neighbour is no longer a Router. Update Default Router List\n";
                //TODO: update Default router list and destination cache entry.
        }
        delete na;
    }
}

IPv6Redirect *IPv6NeighbourDiscovery::createAndSendRedirectPacket(InterfaceEntry *ie)
{
    //Construct a Redirect message
    IPv6Redirect *redirect = new IPv6Redirect("redirectMsg");
    redirect->setType(ICMPv6_REDIRECT);

    //Redirect Message Specific Information
    //redirect->setTargetAddress();
    //redirect->setDestinationAddress();

    //Possible Option
    //redirect->setTargetLinkLayerAddress();

    return redirect;
}

void IPv6NeighbourDiscovery::processRedirectPacket(IPv6Redirect *redirect,
    IPv6ControlInfo *ctrlInfo)
{
    //First we need to extract information from the redirect message
    IPv6Address targetAddr = redirect->targetAddress();//Addressed to me
    IPv6Address destAddr = redirect->destinationAddress();//new dest addr

    //Optional
    MACAddress macAddr = redirect->targetLinkLayerAddress();
}

ICMPv6DestUnreachableMsg *IPv6NeighbourDiscovery::createUnreachableMessage(IPv6Address destAddress)
{
    ICMPv6DestUnreachableMsg *icmp = new ICMPv6DestUnreachableMsg("icmp_unreachable");
    icmp->setType(ICMPv6_DESTINATION_UNREACHABLE);
    icmp->setCode(ADDRESS_UNREACHABLE);

    return icmp;
}

