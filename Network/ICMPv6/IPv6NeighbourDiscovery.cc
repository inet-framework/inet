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
    IPv6Address nextHopAddr = determineNextHop(dgDestAddr, nextHopID);

    Neighbour *neighbour = neighbourCache.lookup(nextHopAddr, nextHopID);
    if (neighbour==NULL)
    {
        ev << "No Entry exists in the Neighbour Cache.\n";
        ev << "Initiating Address Resolution for: " << nextHopAddr << "\n";
        initiateAddressResolution(nextHopAddr, nextHopID);

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
        bubble("Next-Hop Neighbour not found.");
    }
    else
    {
        bubble("Next-Hop Neighbour found.");
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

void IPv6NeighbourDiscovery::initiateAddressResolution(const IPv6Address& neighbourAddr,
    int ifID)
{
    //RFC2461: Section 7.2.2
    //When a node has a unicast packet to send to a neighbor, but does not
    //know the neighbor's link-layer address, it performs address
    //resolution.  For multicast-capable interfaces this entails creating a
    //Neighbor Cache entry in the INCOMPLETE state.

    //and transmitting a Neighbor Solicitation message targeted at the
    //neighbor.  The solicitation is sent to the solicited-node multicast
    //address corresponding to the target address.
    ev << "Preparing to send NS to solicited-node multicast group\n";
    ev << "on selected route interface\n";
    //Create an all node multicast address to be used in the NS control info
    IPv6Address nsDestAddr = IPv6Address::ALL_NODES_2;//Use this?
    IPv6Address nsTargetAddr = neighbourAddr.formSolicitedNodeMulticastAddress();//Or this for both?

    if (ifID == -1)
    {
        /*If the source address of the packet prompting the solicitation is the
        same as one of the addresses assigned to the outgoing interface, that
        address SHOULD be placed in the IP Source Address of the outgoing
        solicitation.  Otherwise, any one of the addresses assigned to the
        interface should be used.*/
        ev << "No specific interface ID was given. Sending NS on all available\n";
        ev << "interfaces\n";
        for (int i=0;i < ift->numInterfaces(); i++)
        {
            InterfaceEntry *ie = ift->interfaceAt(i);
            if (ie->outputPort() != -1)//we don't use the loopback interface
            {
                ev << "Creating an INCOMPLETE entry in the neighbour cache.\n";
                Neighbour *newNeighbour
                    = neighbourCache.addNeighbour(neighbourAddr, i);
                createAndSendNSPacket(nsTargetAddr, nsTargetAddr, ie);
            }
        }
    }
    else
    {
        //Sending NS on specified interface.
        InterfaceEntry *ie = ift->interfaceAt(ifID);
        createAndSendNSPacket(nsTargetAddr, nsTargetAddr, ie);
    }
}

void IPv6NeighbourDiscovery::sendPacketToIPv6Module(cMessage *msg, const IPv6Address& destAddr,
    const IPv6Address& srcAddr, int inputGateIndex)
{
    // add IPv6 control info with NS packet
    IPv6ControlInfo *controlInfo = new IPv6ControlInfo();
    controlInfo->setProtocol(IP_PROT_IPv6_ICMP);
    controlInfo->setDestAddr(destAddr);
    controlInfo->setSrcAddr(srcAddr);
    controlInfo->setHopLimit(255);//FIXME:should 255 be default?
    controlInfo->setInputGateIndex(inputGateIndex);
    msg->setControlInfo(controlInfo);

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
    createAndSendNSPacket(tentativeAddr, destAddr, ie);
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
        createAndSendNSPacket(dadEntry->address, destAddr, ie);
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
        myIPv6Address = IPv6Address();//set my IPv6 address to unspecified.
    IPv6Address destAddr = IPv6Address("FF02::2");
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
    InterfaceEntry *ie = (InterfaceEntry *)msg->contextPointer();
    delete msg;
    //RFC2461: Section 6.3.7
    /*When an interface becomes enabled, a host may be unwilling to wait for the
    next unsolicited Router Advertisement to locate default routers or learn
    prefixes.  To obtain Router Advertisements quickly, a host SHOULD transmit up
    to MAX_RTR_SOLICITATIONS Router Solicitation messages each separated by at
    least RTR_SOLICITATION_INTERVAL seconds.(Therefore this should be invoked
    at the beginning of the simulation-WEI)*/
    RDEntry *rdEntry = new RDEntry();
    rdEntry->interfaceId = ie->interfaceId();
    rdEntry->numRSSent = 0;
    createAndSendRSPacket(ie);
    rdEntry->numRSSent++;
    //Create and schedule a message for retransmission to this module
    cMessage *rdTimeoutMsg = new cMessage("processRDTimeout", MK_RD_TIMEOUT);
    rdTimeoutMsg->setContextPointer(rdEntry); //Andras
    scheduleAt(simTime()+ie->ipv6()->_rtrSolicitationInterval(), rdTimeoutMsg);
}

void IPv6NeighbourDiscovery::processRDTimeout(cMessage *msg)
{
    RDEntry *rdEntry = (RDEntry *)msg->contextPointer();
    InterfaceEntry *ie = (InterfaceEntry *)ift->interfaceAt(rdEntry->interfaceId);
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
    /*if (rsCtrlInfo->hopLimit() != 255)
    {
        ev << "Hop limit is not 255! RS validation failed!\n";
        result = false;
    }*///FIXME: uncomment this out in future!
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
    IPv6Address sourceAddr = ie->ipv6()->preferredAddress();
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
        //ra->setSourceLinkLayerAddress();
        ra->setMTU(ie->ipv6()->advLinkMTU());

        //we have to set parameters to its constructor and fill in its blanks.
        //First we extract ONLY on-link prefixes from RoutingTable6.
        for (int i = 0; i < rt6->numRoutes(); i++)
        {
            IPv6Route *ipv6Route = rt6->route(i);
            if (ipv6Route->src() == IPv6Route::OWNADVPREFIX)
            {
                //prefixInfo->setPrefixLength();
                //prefixInfo->setOnlinkFlag();
                //prefixInfo->setAutoAddressConfFlag();
                //prefixInfo->setValidLifetime();
                //prefixInfo->setPreferredLifetime();
                //prefixInfo->setPrefix();
                //not link-local prefixes.HAHA anything that is FE80::/10
                // Have a look at routing table 6! the actual information is in routelist!
                //ra->setPrefixInformation(prefixInfo);
            }
        }
        sendPacketToIPv6Module(ra, destAddr, sourceAddr, inputGateIndex);
        return ra;
    }
}

void IPv6NeighbourDiscovery::processRAPacket(IPv6RouterAdvertisement *ra,
    IPv6ControlInfo *raCtrlInfo)
{

    int inputGateIndex = raCtrlInfo->inputGateIndex();
    delete raCtrlInfo;
    InterfaceEntry *ie = ift->interfaceByPortNo(inputGateIndex);

    if (ie->ipv6()->advSendAdvertisements())
    {
        ev << "Interface is a router, dropping RA message.";
        delete ra;
        return;
    }
    else
    {
        //RFC2461: Section 6.3.4
        //On receipt of a valid Router Advertisement, a host extracts the source
        //address of the packet and does the following:
        ev << "Interface is a host, processing RA.";
        //First we need to extract information from the RA message
        uint32 curHopLimit = ra->curHopLimit();
        bool managedAddrConfFlag = ra->managedAddrConfFlag();
        bool otherStatefulConfFlag = ra->otherStatefulConfFlag();
        unsigned short routerLifetime = ra->routerLifetime();
        uint32 reachableTime = ra->reachableTime();
        uint32 retransTimer = ra->retransTimer();
    }
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
    IPv6Address destAddr = IPv6Address("FF02::2");
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
    InterfaceEntry *ie = (InterfaceEntry *)msg->contextPointer();
    IPv6Address destAddr = IPv6Address("FF02::2");
    createAndSendRAPacket(destAddr, ie);
    delete msg;
}

bool IPv6NeighbourDiscovery::validateRAPacket(IPv6RouterAdvertisement *ra,
    IPv6ControlInfo *raCtrlInfo)
{
    bool result = true;
    return result;
}

IPv6NeighbourSolicitation *IPv6NeighbourDiscovery::createAndSendNSPacket(
    const IPv6Address& nsTargetAddr, const IPv6Address& destAddr,
    InterfaceEntry *ie)
{
    IPv6Address myIPv6Addr = ie->ipv6()->preferredAddress();
    MACAddress myMacAddr = ie->macAddress();
    int inputGateIndex = ie->outputPort();

    //if myIPv6Addr is tentative, I would set the datagram's src address to unspecified.
    if (ie->ipv6()->isTentativeAddress(myIPv6Addr))
        myIPv6Addr = IPv6Address();

    //Construct a Neighbour Solicitation message
    IPv6NeighbourSolicitation *ns = new IPv6NeighbourSolicitation("NSpacket");
    ns->setType(ICMPv6_NEIGHBOUR_SOL);

    //Neighbour Solicitation Specific Information
    ns->setTargetAddress(nsTargetAddr);

    /*If the solicitation is being sent to a solicited-node multicast
    address, the sender MUST include its link-layer address (if it has
    one) as a Source Link-Layer Address option.*/
    if (destAddr.matches(IPv6Address("FF02::1:FF00:0"),104))
        ns->setSourceLinkLayerAddress(myMacAddr);

    sendPacketToIPv6Module(ns, destAddr, myIPv6Addr, inputGateIndex);
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
        delete ns;
    }
    else
    {
        ev << "Process NS for Non-Tentative target address.\n";
        processNSForNonTentativeAddress(ns, nsCtrlInfo, ie);
    }
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
/*    if (nsCtrlInfo->hopLimit() != 255)
    {
        ev << "Hop limit is not 255! NS validation failed!\n";
        result = false;
    }*///FIXME: uncomment this out in future!
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
        if (ns->sourceLinkLayerAddress().isUnspecified() == false)
        {
            ev << " but Source link-layer address is not empty! NS validation failed!\n";
            result = false;
        }
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
    delete nsCtrlInfo;

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
    IPv6ControlInfo *ctrlInfo, InterfaceEntry *ie)
{
    //Control Information
    IPv6Address nsDestAddr = ctrlInfo->destAddr();
    IPv6Address nsSrcAddr = ctrlInfo->srcAddr();

    //Neighbour Solicitation Information
    MACAddress nsMacAddr = ns->sourceLinkLayerAddress();

    int ifID = ie->interfaceId();

    //target addr is not tentative addr
    //solicitation processed as described in RFC2461:section 7.2.3
    if (nsSrcAddr.isUnspecified())
    {
        ev << "Address is duplicate! Inform Sender of duplicate address!\n";
        //Send NA packet. Use MULTICAST address as destAddr.
        createAndSendNAPacket(ns, nsSrcAddr, nsDestAddr, ie);
        delete ctrlInfo;
    }
    else
    {
        processNSWithSpecifiedSrcAddr(ns, ctrlInfo, ie);
    }
    delete ns;
}

void IPv6NeighbourDiscovery::processNSWithSpecifiedSrcAddr(IPv6NeighbourSolicitation *ns,
    IPv6ControlInfo *ctrlInfo, InterfaceEntry *ie)
{
    //Control Information
    IPv6Address nsDestAddr = ctrlInfo->destAddr();
    IPv6Address nsSrcAddr = ctrlInfo->srcAddr();
    delete ctrlInfo;

    //Neighbour Solicitation Information
    MACAddress nsMacAddr = ns->sourceLinkLayerAddress();

    int ifID = ie->interfaceId();
    ASSERT(!nsMacAddr.isUnspecified() && !nsDestAddr.isMulticast());

    //Look for the Neighbour Cache Entry
    Neighbour *entry = neighbourCache.lookup(nsSrcAddr,ifID);

    if (entry == NULL)
    {
        //Create and initialize a neighbour entry with isRouter=false,
        //MAC address and state=STALE.
        ev << "Neighbour Entry not found. Create a new Neighbour Cache Entry.\n";
        neighbourCache.addNeighbour(nsSrcAddr, ifID, nsMacAddr);
        createAndSendNAPacket(ns, nsSrcAddr, nsDestAddr, ie);
        //TODO: if targetAddr is anycast address
        //  sender should delay sending response between 0
        //and MAX_ANYCAST_DELAY_TIME secs
        /*
        if (ns->targetAddress().isGlobal())
        {
            simtime_t interval
                = uniform(ie->ipv6()->minRtrAdvInt(),ie->ipv6()->maxRtrAdvInt());
        }*/
        //sendPacketToIPv6Module(na);
    }
    else
    {
        if (entry->macAddress.equals(nsMacAddr) == false)
            entry->reachabilityState = IPv6NeighbourCache::STALE;
            createAndSendNAPacket(ns, nsSrcAddr, nsDestAddr, ie);
        //TODO: if targetAddr is anycast address
        //  sender should delay sending response between 0
        //and MAX_ANYCAST_DELAY_TIME secs
        //sendPacketToIPv6Module(na);
    }
}

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
    Neighbour *neighbourEntry = neighbourCache.lookup(naTargetAddr, inputGateIndex);

    if (neighbourEntry == NULL)
    {
        ev << "NA received. Target Address not found in Neighbour Cache\n";
        ev << "Dropping NA packet.\n";
        delete na;
        return;
    }

    //Target Address has entry in Neighbour Cache
    ev << "NA received. Target Address found in Neighbour Cache\n";

    //If the target's neighbour Cache entry is in the INCOMPLETE state when
    //the advertisement is received, one of two things happens.
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
    /*
    if (naCtrlInfo->hopLimit() != 255)
    {
        ev << "Hop Limit is not 255! NA validation failed!\n";
        result = false;
    }*///FIXME:To be removed!

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

    //If the link layer has addresses and no Target Link-Layer address option is
    //included, the receiving node SHOULD silently discard the received
    //advertisement. Otherwise, the receiving node performs the following
    //steps:
    if (naMacAddr.isUnspecified())
    {
        ev << "No MAC Address specified in NA. Ignoring NA\n";
        delete na;
    }
    else
    {
        ev << "ND is updating Neighbour Cache Entry.\n";
        //- It records the link-layer address in the neighbour Cache entry.
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

        //TODO:
        //- It sends any packets queued for the neighbour awaiting address
        //  resolution.
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

