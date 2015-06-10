/**
 * Copyright (C) 2005 Andras Varga
 * Copyright (C) 2005 Wei Yang, Ng
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "inet/networklayer/icmpv6/IPv6NeighbourDiscovery.h"

#include "inet/networklayer/contract/ipv6/IPv6ControlInfo.h"
#include "inet/networklayer/ipv6/IPv6Datagram.h"
#include "inet/networklayer/ipv6/IPv6InterfaceData.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv6/IPv6RoutingTable.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/icmpv6/ICMPv6.h"

#ifdef WITH_xMIPv6
#include "inet/networklayer/xmipv6/xMIPv6.h"
#endif /* WITH_xMIPv6 */

namespace inet {

#define MK_ASSIGN_LINKLOCAL_ADDRESS    0
#define MK_SEND_PERIODIC_RTRADV        1
#define MK_SEND_SOL_RTRADV             2
#define MK_INITIATE_RTRDIS             3
#define MK_DAD_TIMEOUT                 4
#define MK_RD_TIMEOUT                  5
#define MK_NUD_TIMEOUT                 6
#define MK_AR_TIMEOUT                  7

Define_Module(IPv6NeighbourDiscovery);

simsignal_t IPv6NeighbourDiscovery::startDADSignal = registerSignal("startDAD");

IPv6NeighbourDiscovery::IPv6NeighbourDiscovery()
    : neighbourCache(*this)
{
}

IPv6NeighbourDiscovery::~IPv6NeighbourDiscovery()
{
    // FIXME delete the following data structures, cancelAndDelete timers in them etc.
    // Deleting the data structures my become unnecessary if the lists store the
    // structs themselves and not pointers.

    //   RATimerList raTimerList;
    for (const auto & elem : raTimerList) {
        cancelAndDelete(elem);
        delete (elem);
    }

    //   DADList dadList;
    for (const auto & elem : dadList) {
        cancelAndDelete((elem)->timeoutMsg);
        delete (elem);
    }

    //   RDList rdList;
    for (const auto & elem : rdList) {
        cancelAndDelete((elem)->timeoutMsg);
        delete (elem);
    }

    //   AdvIfList advIfList;
    for (const auto & elem : advIfList) {
        cancelAndDelete((elem)->raTimeoutMsg);
        delete (elem);
    }
}

void IPv6NeighbourDiscovery::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_NETWORK_LAYER) {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
    else if (stage == INITSTAGE_NETWORK_LAYER_3) {
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        rt6 = getModuleFromPar<IPv6RoutingTable>(par("routingTableModule"), this);
        icmpv6 = getModuleFromPar<ICMPv6>(par("icmpv6Module"), this);

#ifdef WITH_xMIPv6
        if (rt6->isMobileNode())
            mipv6 = getModuleFromPar<xMIPv6>(par("xmipv6Module"), this);
#endif /* WITH_xMIPv6 */

        pendingQueue.setName("pendingQueue");

#ifdef WITH_xMIPv6
        //MIPv6Enabled = par("MIPv6Support");    // (Zarrar 14.07.07)
        /*if(rt6->isRouter()) // 12.9.07 - CB
           {
            minRAInterval = par("minIntervalBetweenRAs"); // from the omnetpp.ini file (Zarrar 15.07.07)
            maxRAInterval = par("maxIntervalBetweenRAs"); // from the omnetpp.ini file (Zarrar 15.07.07)
            //WATCH (MIPv6Enabled);    // (Zarrar 14.07.07)
            WATCH(minRAInterval);    // (Zarrar 15.07.07)
            WATCH(maxRAInterval);    // (Zarrar 15.07.07)
           }*/
#endif /* WITH_xMIPv6 */

        for (int i = 0; i < ift->getNumInterfaces(); i++) {
            InterfaceEntry *ie = ift->getInterface(i);

            if (ie->ipv6Data()->getAdvSendAdvertisements() && !(ie->isLoopback())) {
                createRATimer(ie);
            }
        }

        //This simulates random node bootup time. Link local address assignment
        //takes place during this time.
        cMessage *msg = new cMessage("assignLinkLocalAddr", MK_ASSIGN_LINKLOCAL_ADDRESS);

        //We want routers to boot up faster!
        if (rt6->isRouter())
            scheduleAt(simTime() + uniform(0, 0.3), msg); //Random Router bootup time
        else
            scheduleAt(simTime() + uniform(0.4, 1), msg); //Random Host bootup time
    }
}

void IPv6NeighbourDiscovery::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        EV_TRACE << "Self message received!\n";

        if (msg->getKind() == MK_SEND_PERIODIC_RTRADV) {
            EV_INFO << "Sending periodic RA\n";
            sendPeriodicRA(msg);
        }
        else if (msg->getKind() == MK_SEND_SOL_RTRADV) {
            EV_INFO << "Sending solicited RA\n";
            sendSolicitedRA(msg);
        }
        else if (msg->getKind() == MK_ASSIGN_LINKLOCAL_ADDRESS) {
            EV_INFO << "Assigning Link Local Address\n";
            assignLinkLocalAddress(msg);
        }
        else if (msg->getKind() == MK_DAD_TIMEOUT) {
            EV_INFO << "DAD Timeout message received\n";
            processDADTimeout(msg);
        }
        else if (msg->getKind() == MK_RD_TIMEOUT) {
            EV_INFO << "Router Discovery message received\n";
            processRDTimeout(msg);
        }
        else if (msg->getKind() == MK_INITIATE_RTRDIS) {
            EV_INFO << "initiate router discovery.\n";
            initiateRouterDiscovery(msg);
        }
        else if (msg->getKind() == MK_NUD_TIMEOUT) {
            EV_INFO << "NUD Timeout message received\n";
            processNUDTimeout(msg);
        }
        else if (msg->getKind() == MK_AR_TIMEOUT) {
            EV_INFO << "Address Resolution Timeout message received\n";
            processARTimeout(msg);
        }
        else
            throw cRuntimeError("Unrecognized Timer"); //stops sim w/ error msg.
    }
    else if (dynamic_cast<ICMPv6Message *>(msg)) {
        //This information will serve as input parameters to various processors.
        IPv6ControlInfo *ctrlInfo = check_and_cast<IPv6ControlInfo *>(msg->removeControlInfo());
        ICMPv6Message *ndMsg = (ICMPv6Message *)msg;
        processNDMessage(ndMsg, ctrlInfo);
    }
    else if (dynamic_cast<IPv6Datagram *>(msg)) {    // not ND message
        IPv6Datagram *datagram = static_cast<IPv6Datagram *>(msg);
        processIPv6Datagram(datagram);
    }
    else
        throw cRuntimeError("Unknown message type received.\n");
}

void IPv6NeighbourDiscovery::processNDMessage(ICMPv6Message *msg, IPv6ControlInfo *ctrlInfo)
{
    if (dynamic_cast<IPv6RouterSolicitation *>(msg)) {
        IPv6RouterSolicitation *rs = (IPv6RouterSolicitation *)msg;
        processRSPacket(rs, ctrlInfo);
    }
    else if (dynamic_cast<IPv6RouterAdvertisement *>(msg)) {
        IPv6RouterAdvertisement *ra = (IPv6RouterAdvertisement *)msg;
        processRAPacket(ra, ctrlInfo);
    }
    else if (dynamic_cast<IPv6NeighbourSolicitation *>(msg)) {
        IPv6NeighbourSolicitation *ns = (IPv6NeighbourSolicitation *)msg;
        processNSPacket(ns, ctrlInfo);
    }
    else if (dynamic_cast<IPv6NeighbourAdvertisement *>(msg)) {
        IPv6NeighbourAdvertisement *na = (IPv6NeighbourAdvertisement *)msg;
        processNAPacket(na, ctrlInfo);
    }
    else if (dynamic_cast<IPv6Redirect *>(msg)) {
        IPv6Redirect *redirect = (IPv6Redirect *)msg;
        processRedirectPacket(redirect, ctrlInfo);
    }
    else {
        throw cRuntimeError("Unrecognized ND message!");
    }
}

bool IPv6NeighbourDiscovery::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    throw cRuntimeError("Lifecycle operation support not implemented");
}

void IPv6NeighbourDiscovery::finish()
{
}

void IPv6NeighbourDiscovery::processIPv6Datagram(IPv6Datagram *msg)
{
    EV_INFO << "Packet " << msg << " arrived from IPv6 module.\n";

    IPv6NDControlInfo *ctrl = check_and_cast<IPv6NDControlInfo *>(msg->getControlInfo());
    int nextHopIfID = ctrl->getInterfaceId();
    IPv6Address nextHopAddr = ctrl->getNextHop();
    //bool fromHL = ctrl->getFromHL();

    if (nextHopIfID == -1 || nextHopAddr.isUnspecified()) {
        EV_INFO << "Determining Next Hop" << endl;
        nextHopAddr = determineNextHop(msg->getDestAddress(), nextHopIfID);
        ctrl->setInterfaceId(nextHopIfID);
        ctrl->setNextHop(nextHopAddr);
    }

    if (nextHopIfID == -1) {
        //draft-ietf-ipv6-2461bis-04 has omitted on-link assumption.
        //draft-ietf-v6ops-onlinkassumption-03 explains why.
        delete msg->removeControlInfo();
        icmpv6->sendErrorMessage(msg, ICMPv6_DESTINATION_UNREACHABLE, NO_ROUTE_TO_DEST);
        return;
    }

    EV_INFO << "Next Hop Address is: " << nextHopAddr << " on interface: " << nextHopIfID << endl;

    //RFC2461: Section 5.2 Conceptual Sending Algorithm
    //Once the IP address of the next-hop node is known, the sender examines the
    //Neighbor Cache for link-layer information about that neighbor.
    Neighbour *nce = neighbourCache.lookup(nextHopAddr, nextHopIfID);

    if (nce == nullptr) {
        EV_INFO << "No Entry exists in the Neighbour Cache.\n";
        InterfaceEntry *ie = ift->getInterfaceById(nextHopIfID);
        if (ie->isPointToPoint()) {
            //the sender creates one, sets its state to STALE,
            EV_DETAIL << "Creating an STALE entry in the neighbour cache.\n";
            nce = neighbourCache.addNeighbour(nextHopAddr, nextHopIfID, MACAddress::UNSPECIFIED_ADDRESS);
        }
        else {
            //the sender creates one, sets its state to INCOMPLETE,
            EV_DETAIL << "Creating an INCOMPLETE entry in the neighbour cache.\n";
            nce = neighbourCache.addNeighbour(nextHopAddr, nextHopIfID);

            //initiates Address Resolution,
            EV_DETAIL << "Initiating Address Resolution for:" << nextHopAddr
                      << " on Interface:" << nextHopIfID << endl;
            initiateAddressResolution(msg->getSrcAddress(), nce);
        }
    }

    /*
     * A host is capable of sending packets to a destination in all states except INCOMPLETE
     * or when there is no corresponding NC entry. In INCOMPLETE state the data packets are
     * queued pending completion of address resolution.
     */
    switch (nce->reachabilityState) {
    case IPv6NeighbourCache::INCOMPLETE:
        EV_INFO << "Reachability State is INCOMPLETE. Address Resolution already initiated.\n";
        EV_INFO << "Add packet to entry's queue until Address Resolution is complete.\n";
        bubble("Packet added to queue until Address Resolution is complete.");
        nce->pendingPackets.push_back(msg);
        pendingQueue.insert(msg);
        break;
    case IPv6NeighbourCache::STALE:
        EV_INFO << "Reachability State is STALE.\n";
        send(msg, "ipv6Out");
        initiateNeighbourUnreachabilityDetection(nce);
        break;
    case IPv6NeighbourCache::REACHABLE:
        EV_INFO << "Next hop is REACHABLE, sending packet to next-hop address.";
        send(msg, "ipv6Out");
        break;
    case IPv6NeighbourCache::DELAY:
        EV_INFO << "Next hop is in DELAY state, sending packet to next-hop address.";
        send(msg, "ipv6Out");
        break;
    case IPv6NeighbourCache::PROBE:
        EV_INFO << "Next hop is in PROBE state, sending packet to next-hop address.";
        send(msg, "ipv6Out");
        break;
    default:
        throw cRuntimeError("Unknown Neighbour cache entry state.");
        break;
    }
}

IPv6NeighbourDiscovery::AdvIfEntry *IPv6NeighbourDiscovery::fetchAdvIfEntry(InterfaceEntry *ie)
{
    for (auto advIfEntry : advIfList) {
        if (advIfEntry->interfaceId == ie->getInterfaceId()) {
            return advIfEntry;
        }
    }
    return nullptr;
}

IPv6NeighbourDiscovery::RDEntry *IPv6NeighbourDiscovery::fetchRDEntry(InterfaceEntry *ie)
{
    for (auto rdEntry : rdList) {
        
        if (rdEntry->interfaceId == ie->getInterfaceId()) {
            return rdEntry;
        }
    }
    return nullptr;
}

const MACAddress& IPv6NeighbourDiscovery::resolveNeighbour(const IPv6Address& nextHop, int interfaceId)
{
    Enter_Method("resolveNeighbor(%s,if=%d)", nextHop.str().c_str(), interfaceId);

    Neighbour *nce = neighbourCache.lookup(nextHop, interfaceId);
    //InterfaceEntry *ie = ift->getInterfaceById(interfaceId);

    if (!nce || nce->reachabilityState == IPv6NeighbourCache::INCOMPLETE)
        return MACAddress::UNSPECIFIED_ADDRESS;

    if (nce->reachabilityState == IPv6NeighbourCache::STALE) {
        initiateNeighbourUnreachabilityDetection(nce);
    }
    else if (nce->reachabilityState == IPv6NeighbourCache::REACHABLE &&
             simTime() > nce->reachabilityExpires)
    {
        nce->reachabilityState = IPv6NeighbourCache::STALE;
        initiateNeighbourUnreachabilityDetection(nce);
    }
    else if (nce->reachabilityState != IPv6NeighbourCache::REACHABLE) {
        //reachability state must be either in DELAY or PROBE
        ASSERT(nce->reachabilityState == IPv6NeighbourCache::DELAY ||
                nce->reachabilityState == IPv6NeighbourCache::PROBE);
        EV_INFO << "NUD in progress.\n";
    }

    //else the entry is REACHABLE and no further action is required here.
    return nce->macAddress;
}

void IPv6NeighbourDiscovery::reachabilityConfirmed(const IPv6Address& neighbour, int interfaceId)
{
    Enter_Method("reachabilityConfirmed(%s,if=%d)", neighbour.str().c_str(), interfaceId);
    //hmmm... this should only be invoked if a TCP ACK was received and NUD is
    //currently being performed on the neighbour where the TCP ACK was received from.

    Neighbour *nce = neighbourCache.lookup(neighbour, interfaceId);
    if (!nce)
        throw cRuntimeError("Model error: not found in cache");

    cMessage *msg = nce->nudTimeoutEvent;
    if (msg != nullptr) {
        EV_INFO << "NUD in progress. Cancelling NUD Timer\n";
        bubble("Reachability Confirmed via NUD.");
        cancelAndDelete(msg);
        nce->nudTimeoutEvent = nullptr;
    }

    // TODO (see header file for description)
    /*A neighbor is considered reachable if the node has recently received
       a confirmation that packets sent recently to the neighbor were
       received by its IP layer.  Positive confirmation can be gathered in
       two ways: hints from upper layer protocols that indicate a connection
       is making "forward progress", or receipt of a Neighbor Advertisement
       message that is a response to a Neighbor Solicitation message.

       A connection makes "forward progress" if the packets received from a
       remote peer can only be arriving if recent packets sent to that peer
       are actually reaching it.  In TCP, for example, receipt of a (new)
       acknowledgement indicates that previously sent data reached the peer.
       Likewise, the arrival of new (non-duplicate) data indicates that

       earlier acknowledgements are being delivered to the remote peer.  If
       packets are reaching the peer, they must also be reaching the
       sender's next-hop neighbor; thus "forward progress" is a confirmation
       that the next-hop neighbor is reachable.  For off-link destinations,
       forward progress implies that the first-hop router is reachable.
       When available, this upper-layer information SHOULD be used.

       In some cases (e.g., UDP-based protocols and routers forwarding
       packets to hosts) such reachability information may not be readily
       available from upper-layer protocols.  When no hints are available
       and a node is sending packets to a neighbor, the node actively probes
       the neighbor using unicast Neighbor Solicitation messages to verify
       that the forward path is still working.

       The receipt of a solicited Neighbor Advertisement serves as
       reachability confirmation, since advertisements with the Solicited
       flag set to one are sent only in response to a Neighbor Solicitation.
       Receipt of other Neighbor Discovery messages such as Router
       Advertisements and Neighbor Advertisement with the Solicited flag set
       to zero MUST NOT be treated as a reachability confirmation.  Receipt
       of unsolicited messages only confirm the one-way path from the sender
       to the recipient node.  In contrast, Neighbor Unreachability
       Detection requires that a node keep track of the reachability of the
       forward path to a neighbor from the its perspective, not the
       neighbor's perspective.  Note that receipt of a solicited
       advertisement indicates that a path is working in both directions.
       The solicitation must have reached the neighbor, prompting it to
       generate an advertisement.  Likewise, receipt of an advertisement
       indicates that the path from the sender to the recipient is working.
       However, the latter fact is known only to the recipient; the
       advertisement's sender has no direct way of knowing that the
       advertisement it sent actually reached a neighbor.  From the
       perspective of Neighbor Unreachability Detection, only the
       reachability of the forward path is of interest.*/
}

IPv6Address IPv6NeighbourDiscovery::determineNextHop(const IPv6Address& destAddr, int& outIfID)
{
    IPv6Address nextHopAddr;
    simtime_t expiryTime;

    //RFC 2461 Section 5.2
    //Next-hop determination for a given unicast destination operates as follows.

    //The sender performs a longest prefix match against the Prefix List to
    //determine whether the packet's destination is on- or off-link.
    EV_INFO << "Find out if supplied dest addr is on-link or off-link.\n";
    const IPv6Route *route = rt6->doLongestPrefixMatch(destAddr);

    if (route != nullptr) {
        expiryTime = route->getExpiryTime();
        outIfID = route->getInterface() ? route->getInterface()->getInterfaceId() : -1;

        //If the destination is on-link, the next-hop address is the same as the
        //packet's destination address.
        if (route->getNextHop().isUnspecified()) {
            EV_INFO << "Dest is on-link, next-hop addr is same as dest addr.\n";
            nextHopAddr = destAddr;
        }
        else {
            EV_INFO << "A next-hop address was found with the route, dest is off-link\n";
            EV_INFO << "Assume next-hop address as the selected default router.\n";
            nextHopAddr = route->getNextHop();
        }
    }
    else {
        //Otherwise, the sender selects a router from the Default Router List
        //(following the rules described in Section 6.3.6).

        EV_INFO << "No routes were found, Dest addr is off-link.\n";
        nextHopAddr = selectDefaultRouter(outIfID);
        expiryTime = 0;

        if (outIfID == -1)
            EV_INFO << "No Default Routers were found.";
        else
            EV_INFO << "Default router found.\n";
    }

    /*the results of next-hop determination computations are saved in the Destination
       Cache (which also contains updates learned from Redirect messages).*/
    rt6->updateDestCache(destAddr, nextHopAddr, outIfID, expiryTime);
    return nextHopAddr;
}

void IPv6NeighbourDiscovery::initiateNeighbourUnreachabilityDetection(Neighbour *nce)
{
    ASSERT(nce->reachabilityState == IPv6NeighbourCache::STALE);
    ASSERT(nce->nudTimeoutEvent == nullptr);
    const Key *nceKey = nce->nceKey;
    EV_INFO << "Initiating Neighbour Unreachability Detection";
    InterfaceEntry *ie = ift->getInterfaceById(nceKey->interfaceID);
    EV_INFO << "Setting NCE state to DELAY.\n";
    /*The first time a node sends a packet to a neighbor whose entry is
       STALE, the sender changes the state to DELAY*/
    nce->reachabilityState = IPv6NeighbourCache::DELAY;

    /*and sets a timer to expire in DELAY_FIRST_PROBE_TIME seconds.*/
    cMessage *msg = new cMessage("NUDTimeout", MK_NUD_TIMEOUT);
    msg->setContextPointer(nce);
    nce->nudTimeoutEvent = msg;
    scheduleAt(simTime() + ie->ipv6Data()->_getDelayFirstProbeTime(), msg);
}

void IPv6NeighbourDiscovery::processNUDTimeout(cMessage *timeoutMsg)
{
    EV_INFO << "NUD has timed out\n";
    Neighbour *nce = (Neighbour *)timeoutMsg->getContextPointer();

    const Key *nceKey = nce->nceKey;
    if (nceKey == nullptr)
        throw cRuntimeError("The nceKey is nullptr at nce->MAC=%s, isRouter=%d",
                nce->macAddress.str().c_str(), nce->isRouter);

    InterfaceEntry *ie = ift->getInterfaceById(nceKey->interfaceID);

    if (nce->reachabilityState == IPv6NeighbourCache::DELAY) {
        /*If the entry is still in the DELAY state when the timer expires, the
           entry's state changes to PROBE. If reachability confirmation is received,
           the entry's state changes to REACHABLE.*/
        EV_DETAIL << "Neighbour Entry is still in DELAY state.\n";
        EV_DETAIL << "Entering PROBE state. Sending NS probe.\n";
        nce->reachabilityState = IPv6NeighbourCache::PROBE;
        nce->numProbesSent = 0;
    }

    /*If no response is received after waiting RetransTimer milliseconds
       after sending the MAX_UNICAST_SOLICIT solicitations, retransmissions cease
       and the entry SHOULD be deleted. Subsequent traffic to that neighbor will
       recreate the entry and performs address resolution again.*/
    if (nce->numProbesSent == (int)ie->ipv6Data()->_getMaxUnicastSolicit()) {
        EV_DETAIL << "Max number of probes have been sent." << endl;
        EV_DETAIL << "Neighbour is Unreachable, removing NCE." << endl;
        neighbourCache.remove(nceKey->address, nceKey->interfaceID); // remove nce from cache, cancel and delete timeoutMsg;
        return;
    }

    /*Upon entering the PROBE state, a node sends a unicast Neighbor Solicitation
       message to the neighbor using the cached link-layer address.*/
    createAndSendNSPacket(nceKey->address, nceKey->address,
            ie->ipv6Data()->getPreferredAddress(), ie);
    nce->numProbesSent++;
    /*While in the PROBE state, a node retransmits Neighbor Solicitation messages
       every RetransTimer milliseconds until reachability confirmation is obtained.
       Probes are retransmitted even if no additional packets are sent to the
       neighbor.*/
    scheduleAt(simTime() + ie->ipv6Data()->_getRetransTimer(), timeoutMsg);
}

IPv6Address IPv6NeighbourDiscovery::selectDefaultRouter(int& outIfID)
{
    EV_INFO << "Selecting default router...\n";
    //draft-ietf-ipv6-2461bis-04.txt Section 6.3.6
    /*The algorithm for selecting a router depends in part on whether or not a
       router is known to be reachable. The exact details of how a node keeps track
       of a neighbor's reachability state are covered in Section 7.3.  The algorithm
       for selecting a default router is invoked during next-hop determination when
       no Destination Cache entry exists for an off-link destination or when
       communication through an existing router appears to be failing.  Under normal
       conditions, a router would be selected the first time traffic is sent to a
       destination, with subsequent traffic for that destination using the same router
       as indicated in the Destination Cache modulo any changes to the Destination
       Cache caused by Redirect messages.

       The policy for selecting routers from the Default Router List is as
       follows:*/

    /*1) Routers that are reachable or probably reachable (i.e., in any state
       other than INCOMPLETE) SHOULD be preferred over routers whose reachability
       is unknown or suspect (i.e., in the INCOMPLETE state, or for which no Neighbor
       Cache entry exists). An implementation may choose to always return the same
       router or cycle through the router list in a round-robin fashion as long as
       it always returns a reachable or a probably reachable router when one is
       available.*/
    DefaultRouterList& defaultRouters = neighbourCache.getDefaultRouterList();
    for (auto it = defaultRouters.begin(); it != defaultRouters.end(); ) {
        Neighbour& nce = *it;
        if (simTime() > nce.routerExpiryTime) {
            EV_INFO << "Found an expired default router. Deleting entry...\n";
            ++it;
            neighbourCache.remove(nce.nceKey->address, nce.nceKey->interfaceID);
            continue;
        }
        if (nce.reachabilityState != IPv6NeighbourCache::INCOMPLETE) {
            EV_INFO << "Found a probably reachable router in the default router list.\n";
            defaultRouters.setHead(*nce.nextDefaultRouter);
            outIfID = nce.nceKey->interfaceID;
            return nce.nceKey->address;
        }
        ++it;
    }

    /*2) When no routers on the list are known to be reachable or probably
       reachable, routers SHOULD be selected in a round-robin fashion, so that
       subsequent requests for a default router do not return the same router until
       all other routers have been selected.

       Cycling through the router list in this case ensures that all available
       routers are actively probed by the Neighbor Unreachability Detection algorithm.
       A request for a default router is made in conjunction with the sending of a
       packet to a router, and the selected router will be probed for reachability
       as a side effect.*/
    Neighbour *defaultRouter = defaultRouters.getHead();
    if (defaultRouter != nullptr) {
        EV_INFO << "Found a router in the neighbour cache (default router list).\n";
        defaultRouters.setHead(*defaultRouter->nextDefaultRouter);
        outIfID = defaultRouter->nceKey->interfaceID;
        return defaultRouter->nceKey->address;
    }

    EV_INFO << "No suitable routers found.\n";
    outIfID = -1;
    return IPv6Address::UNSPECIFIED_ADDRESS;
}

void IPv6NeighbourDiscovery::timeoutPrefixEntry(const IPv6Address& destPrefix,
        int prefixLength)    //REDUNDANT
{
    //RFC 2461: Section 6.3.5
    /*Whenever the invalidation timer expires for a Prefix List entry, that
       entry is discarded.*/
    rt6->deleteOnLinkPrefix(destPrefix, prefixLength);
    //hmmm... should the unicast address associated with this prefix be deleted
    //as well?-TODO: The address should be timeout/deleted as well!!

    /*No existing Destination Cache entries need be updated, however. Should a
       reachability problem arise with an existing Neighbor Cache entry, Neighbor
       Unreachability Detection will perform any needed recovery.*/
}

void IPv6NeighbourDiscovery::timeoutDefaultRouter(const IPv6Address& addr,
        int interfaceID)
{
    //RFC 2461: Section 6.3.5
    /*Whenever the Lifetime of an entry in the Default Router List expires,
       that entry is discarded.*/
    neighbourCache.remove(addr, interfaceID);

    /*When removing a router from the Default Router list, the node MUST update
       the Destination Cache in such a way that all entries using the router perform
       next-hop determination again rather than continue sending traffic to the
       (deleted) router.*/
    rt6->purgeDestCacheEntriesToNeighbour(addr, interfaceID);
}

void IPv6NeighbourDiscovery::initiateAddressResolution(const IPv6Address& dgSrcAddr,
        Neighbour *nce)
{
    const Key *nceKey = nce->nceKey;
    InterfaceEntry *ie = ift->getInterfaceById(nceKey->interfaceID);
    IPv6Address neighbourAddr = nceKey->address;
    int ifID = nceKey->interfaceID;

    //RFC2461: Section 7.2.2
    //When a node has a unicast packet to send to a neighbor, but does not
    //know the neighbor's link-layer address, it performs address
    //resolution.  For multicast-capable interfaces this entails creating a
    //Neighbor Cache entry in the INCOMPLETE state(already created if not done yet)
    //WEI-If entry already exists, we still have to ensure that its state is INCOMPLETE.
    nce->reachabilityState = IPv6NeighbourCache::INCOMPLETE;

    //and transmitting a Neighbor Solicitation message targeted at the
    //neighbor.  The solicitation is sent to the solicited-node multicast
    //address "corresponding to"(or "derived from") the target address.
    //(in this case, the target address is the address we are trying to resolve)
    EV_INFO << "Preparing to send NS to solicited-node multicast group\n";
    EV_INFO << "on the next hop interface\n";
    IPv6Address nsDestAddr = neighbourAddr.formSolicitedNodeMulticastAddress();    //for NS datagram
    IPv6Address nsTargetAddr = neighbourAddr;    //for the field within the NS
    IPv6Address nsSrcAddr;

    /*If the source address of the packet prompting the solicitation is the
       same as one of the addresses assigned to the outgoing interface,*/
    if (ie->ipv6Data()->hasAddress(dgSrcAddr))
        /*that address SHOULD be placed in the IP Source Address of the outgoing
           solicitation.*/
        nsSrcAddr = dgSrcAddr;
    else
        /*Otherwise, any one of the addresses assigned to the interface
           should be used.*/
        nsSrcAddr = ie->ipv6Data()->getPreferredAddress();
    ASSERT(ifID != -1);
    //Sending NS on specified interface.
    createAndSendNSPacket(nsTargetAddr, nsDestAddr, nsSrcAddr, ie);
    nce->numOfARNSSent = 1;
    nce->nsSrcAddr = nsSrcAddr;

    /*While awaiting a response, the sender SHOULD retransmit Neighbor Solicitation
       messages approximately every RetransTimer milliseconds, even in the absence
       of additional traffic to the neighbor. Retransmissions MUST be rate-limited
       to at most one solicitation per neighbor every RetransTimer milliseconds.*/
    cMessage *msg = new cMessage("arTimeout", MK_AR_TIMEOUT);    //AR msg timer
    nce->arTimer = msg;
    msg->setContextPointer(nce);
    scheduleAt(simTime() + ie->ipv6Data()->_getRetransTimer(), msg);
}

void IPv6NeighbourDiscovery::processARTimeout(cMessage *arTimeoutMsg)
{
    //AR timeouts are cancelled when a valid solicited NA is received.
    Neighbour *nce = (Neighbour *)arTimeoutMsg->getContextPointer();
    const Key *nceKey = nce->nceKey;
    IPv6Address nsTargetAddr = nceKey->address;
    InterfaceEntry *ie = ift->getInterfaceById(nceKey->interfaceID);
    EV_INFO << "Num Of NS Sent:" << nce->numOfARNSSent << endl;
    EV_INFO << "Max Multicast Solicitation:" << ie->ipv6Data()->_getMaxMulticastSolicit() << endl;

    if (nce->numOfARNSSent < ie->ipv6Data()->_getMaxMulticastSolicit()) {
        EV_INFO << "Sending another Address Resolution NS message" << endl;
        IPv6Address nsDestAddr = nsTargetAddr.formSolicitedNodeMulticastAddress();
        createAndSendNSPacket(nsTargetAddr, nsDestAddr, nce->nsSrcAddr, ie);
        nce->numOfARNSSent++;
        scheduleAt(simTime() + ie->ipv6Data()->_getRetransTimer(), arTimeoutMsg);
        return;
    }

    EV_WARN << "Address Resolution has failed." << endl;
    dropQueuedPacketsAwaitingAR(nce);
    EV_INFO << "Deleting AR timeout msg\n";
    delete arTimeoutMsg;
}

void IPv6NeighbourDiscovery::dropQueuedPacketsAwaitingAR(Neighbour *nce)
{
    const Key *nceKey = nce->nceKey;
    //RFC 2461: Section 7.2.2
    /*If no Neighbor Advertisement is received after MAX_MULTICAST_SOLICIT
       solicitations, address resolution has failed. The sender MUST return ICMP
       destination unreachable indications with code 3 (Address Unreachable) for
       each packet queued awaiting address resolution.*/
    MsgPtrVector& pendingPackets = nce->pendingPackets;
    EV_INFO << "Pending Packets empty:" << pendingPackets.empty() << endl;

    while (!pendingPackets.empty()) {
        auto i = pendingPackets.begin();
        cMessage *msg = (*i);
        IPv6Datagram *ipv6Msg = check_and_cast<IPv6Datagram *>(msg);
        //Assume msg is the packet itself. I need the datagram's source addr.
        //The datagram's src addr will be the destination of the unreachable msg.
        EV_INFO << "Sending ICMP unreachable destination." << endl;
        pendingPackets.erase(i);
        pendingQueue.remove(msg);
        icmpv6->sendErrorMessage(ipv6Msg, ICMPv6_DESTINATION_UNREACHABLE, ADDRESS_UNREACHABLE);
    }

    //RFC 2461: Section 7.3.3
    /*If address resolution fails, the entry SHOULD be deleted, so that subsequent
       traffic to that neighbor invokes the next-hop determination procedure again.*/
    EV_INFO << "Removing neighbour cache entry" << endl;
    neighbourCache.remove(nceKey->address, nceKey->interfaceID);
}

void IPv6NeighbourDiscovery::sendPacketToIPv6Module(cMessage *msg, const IPv6Address& destAddr,
        const IPv6Address& srcAddr, int interfaceId)
{
    IPv6ControlInfo *controlInfo = new IPv6ControlInfo();
    controlInfo->setProtocol(IP_PROT_IPv6_ICMP);
    controlInfo->setDestAddr(destAddr);
    controlInfo->setSrcAddr(srcAddr);
    controlInfo->setHopLimit(255);
    controlInfo->setInterfaceId(interfaceId);
    msg->setControlInfo(controlInfo);

    send(msg, "ipv6Out");
}

/**Not used yet-unsure if we really need it. --DELETED, Andras*/

void IPv6NeighbourDiscovery::sendQueuedPacketsToIPv6Module(Neighbour *nce)
{
    MsgPtrVector& pendingPackets = nce->pendingPackets;

    while (!pendingPackets.empty()) {
        auto i = pendingPackets.begin();
        cMessage *msg = (*i);
        pendingPackets.erase(i);
        pendingQueue.remove(msg);
        EV_INFO << "Sending queued packet " << msg << endl;
        send(msg, "ipv6Out");
    }
}

void IPv6NeighbourDiscovery::assignLinkLocalAddress(cMessage *timerMsg)
{
    //Node has booted up. Start assigning a link-local address for each
    //interface in this node.
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        InterfaceEntry *ie = ift->getInterface(i);

        //Skip the loopback interface.
        if (ie->isLoopback())
            continue;

        IPv6Address linkLocalAddr = ie->ipv6Data()->getLinkLocalAddress();
        if (linkLocalAddr.isUnspecified()) {
            //if no link local address exists for this interface, we assign one to it.
            EV_INFO << "No link local address exists. Forming one" << endl;
            linkLocalAddr = IPv6Address().formLinkLocalAddress(ie->getInterfaceToken());
            ie->ipv6Data()->assignAddress(linkLocalAddr, true, SIMTIME_ZERO, SIMTIME_ZERO);
        }

        //Before we can use this address, we MUST initiate DAD first.
        if (ie->ipv6Data()->isTentativeAddress(linkLocalAddr)) {
            if (ie->ipv6Data()->getDupAddrDetectTransmits() > 0)
                initiateDAD(linkLocalAddr, ie);
            else
                makeTentativeAddressPermanent(linkLocalAddr, ie);
        }
    }
    delete timerMsg;
}

void IPv6NeighbourDiscovery::initiateDAD(const IPv6Address& tentativeAddr, InterfaceEntry *ie)
{
#ifdef WITH_xMIPv6
    Enter_Method_Silent();
    EV_INFO << "----------INITIATING DUPLICATE ADDRESS DISCOVERY----------" << endl;
    ie->ipv6Data()->setDADInProgress(true);
#endif /* WITH_xMIPv6 */

    DADEntry *dadEntry = new DADEntry();
    dadEntry->interfaceId = ie->getInterfaceId();
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
    createAndSendNSPacket(tentativeAddr, destAddr,
            IPv6Address::UNSPECIFIED_ADDRESS, ie);
    dadEntry->numNSSent++;

    cMessage *msg = new cMessage("dadTimeout", MK_DAD_TIMEOUT);
    msg->setContextPointer(dadEntry);

#ifndef WITH_xMIPv6
    scheduleAt(simTime() + ie->ipv6Data()->getRetransTimer(), msg);
#else /* WITH_xMIPv6 */
    // update: added uniform(0, IPv6_MAX_RTR_SOLICITATION_DELAY) to account for joining the solicited-node multicast
    // group which is delay up to one 1 second (RFC 4862, 5.4.2) - 16.01.08, CB
    scheduleAt(simTime() + ie->ipv6Data()->getRetransTimer() + uniform(0, IPv6_MAX_RTR_SOLICITATION_DELAY), msg);
#endif /* WITH_xMIPv6 */

    emit(startDADSignal, 1);
}

void IPv6NeighbourDiscovery::processDADTimeout(cMessage *msg)
{
    DADEntry *dadEntry = (DADEntry *)msg->getContextPointer();
    InterfaceEntry *ie = (InterfaceEntry *)ift->getInterfaceById(dadEntry->interfaceId);
    IPv6Address tentativeAddr = dadEntry->address;
    //Here, we need to check how many DAD messages for the interface entry were
    //sent vs. DupAddrDetectTransmits
    EV_DETAIL << "numOfDADMessagesSent is: " << dadEntry->numNSSent << endl;
    EV_DETAIL << "dupAddrDetectTrans is: " << ie->ipv6Data()->getDupAddrDetectTransmits() << endl;

    if (dadEntry->numNSSent < ie->ipv6Data()->getDupAddrDetectTransmits()) {
        bubble("Sending another DAD NS message.");
        IPv6Address destAddr = tentativeAddr.formSolicitedNodeMulticastAddress();
        createAndSendNSPacket(dadEntry->address, destAddr, IPv6Address::UNSPECIFIED_ADDRESS, ie);
        dadEntry->numNSSent++;
        //Reuse the received msg
        scheduleAt(simTime() + ie->ipv6Data()->getRetransTimer(), msg);
    }
    else {
        bubble("Max number of DAD messages for interface sent. Address is unique.");
        dadList.erase(dadEntry);
        EV_DETAIL << "delete dadEntry and msg\n";
        delete dadEntry;
        delete msg;

        makeTentativeAddressPermanent(tentativeAddr, ie);
    }
}

void IPv6NeighbourDiscovery::makeTentativeAddressPermanent(const IPv6Address& tentativeAddr, InterfaceEntry *ie)
{
    ie->ipv6Data()->permanentlyAssign(tentativeAddr);

#ifdef WITH_xMIPv6
    ie->ipv6Data()->setDADInProgress(false);

    // update 28.09.07 - CB
    // after the link-local address was verified to be unique
    // we can assign the address and initiate the MIPv6 protocol
    // in case there are any pending entries in the list
    auto it = dadGlobalList.find(ie);
    if (it != dadGlobalList.end()) {
        DADGlobalEntry& entry = it->second;

        ie->ipv6Data()->assignAddress(entry.addr, false, simTime() + entry.validLifetime,
                simTime() + entry.preferredLifetime, entry.hFlag);

        // moved from processRAPrefixInfoForAddrAutoConf()
        // we can remove the old CoA now
        if (!entry.CoA.isUnspecified())
            ie->ipv6Data()->removeAddress(entry.CoA);

        // set addresses on this interface to tentative=false
        for (int i = 0; i < ie->ipv6Data()->getNumAddresses(); i++) {
            // TODO improve this code so that only addresses are permanently assigned
            // which are formed based on the new prefix from the RA
            IPv6Address addr = ie->ipv6Data()->getAddress(i);
            ie->ipv6Data()->permanentlyAssign(addr);
        }

        // if we have MIPv6 protocols on this node we will eventually have to
        // call some appropriate methods
        if (rt6->isMobileNode()) {
            if (entry.hFlag == false) // if we are not in the home network, send BUs
                mipv6->initiateMIPv6Protocol(ie, tentativeAddr);
            /*
               else if ( entry.returnedHome ) // if we are again in the home network
               {
                ASSERT(entry.CoA.isUnspecified() == false);
                mipv6->returningHome(entry.CoA, ie); // initiate the returning home procedure
               }*/
        }

        dadGlobalList.erase(it->first);
    }

    // an optimization to make sure that the access router on the link gets our L2 address
    //sendUnsolicitedNA(ie);

    // =================================Start: Zarrar Yousaf 08.07.07 ===============================================
    /* == Calling the routine to assign global scope adddresses to the the routers only. At present during the simulation initialization, the FlatNetworkConfigurator6 assigns a 64 bit prefix to the routers but for xMIPv6 operation, we need full 128bit global scope address, only for routers. The call to  autoConfRouterGlobalScopeAddress() will autoconfigure the full 128 bit global scope address, which will be used by the MN in its BU message destination address, especially for home registeration.
     */
    if (rt6->isRouter() && !(ie->isLoopback())) {
        for (int i = 0; i < ie->ipv6Data()->getNumAdvPrefixes(); i++) {
            IPv6Address globalAddress = ie->ipv6Data()->autoConfRouterGlobalScopeAddress(i);
            ie->ipv6Data()->assignAddress(globalAddress, false, 0, 0);
            // ie->ipv6Data()->deduceAdvPrefix(); //commented out but the above two statements can be replaced with this single statement. But i am using the above two statements for clarity reasons.
        }
    }
    // ==================================End: Zarrar Yousaf 08.07.07===========================================
#endif /* WITH_xMIPv6 */

    /*RFC 2461: Section 6.3.7 2nd Paragraph
       Before a host sends an initial solicitation, it SHOULD delay the
       transmission for a random amount of time between 0 and
       MAX_RTR_SOLICITATION_DELAY.  This serves to alleviate congestion when
       many hosts start up on a link at the same time, such as might happen
       after recovery from a power failure.*/
    //TODO: Placing these operations here means fast router solicitation is
    //not adopted. Will relocate.
    if (ie->ipv6Data()->getAdvSendAdvertisements() == false) {
        EV_INFO << "creating router discovery message timer\n";
        cMessage *rtrDisMsg = new cMessage("initiateRTRDIS", MK_INITIATE_RTRDIS);
        rtrDisMsg->setContextPointer(ie);
        simtime_t interval = uniform(0, ie->ipv6Data()->_getMaxRtrSolicitationDelay());    // random delay
        scheduleAt(simTime() + interval, rtrDisMsg);
    }
}

IPv6RouterSolicitation *IPv6NeighbourDiscovery::createAndSendRSPacket(InterfaceEntry *ie)
{
    ASSERT(ie->ipv6Data()->getAdvSendAdvertisements() == false);
    //RFC 2461: Section 6.3.7 Sending Router Solicitations
    //A host sends Router Solicitations to the All-Routers multicast address. The
    //IP source address is set to either one of the interface's unicast addresses
    //or the unspecified address.
    IPv6Address myIPv6Address = ie->ipv6Data()->getPreferredAddress();

    if (myIPv6Address.isUnspecified())
        myIPv6Address = ie->ipv6Data()->getLinkLocalAddress(); //so we use the link local address instead

    if (ie->ipv6Data()->isTentativeAddress(myIPv6Address))
        myIPv6Address = IPv6Address::UNSPECIFIED_ADDRESS; //set my IPv6 address to unspecified.

    IPv6Address destAddr = IPv6Address::ALL_ROUTERS_2;    //all_routers multicast
    IPv6RouterSolicitation *rs = new IPv6RouterSolicitation("RSpacket");
    rs->setType(ICMPv6_ROUTER_SOL);
    rs->setByteLength(ICMPv6_HEADER_BYTES);

    //The Source Link-Layer Address option SHOULD be set to the host's link-layer
    //address, if the IP source address is not the unspecified address.
    if (!myIPv6Address.isUnspecified()) {
        rs->setSourceLinkLayerAddress(ie->getMacAddress());
        rs->setByteLength(ICMPv6_HEADER_BYTES + IPv6ND_LINK_LAYER_ADDRESS_OPTION_LENGTH);
    }

    //Construct a Router Solicitation message
    sendPacketToIPv6Module(rs, destAddr, myIPv6Address, ie->getInterfaceId());
    return rs;
}

void IPv6NeighbourDiscovery::initiateRouterDiscovery(cMessage *msg)
{
    EV_INFO << "Initiating Router Discovery" << endl;
    InterfaceEntry *ie = (InterfaceEntry *)msg->getContextPointer();
    delete msg;
    //RFC2461: Section 6.3.7
    /*When an interface becomes enabled, a host may be unwilling to wait for the
       next unsolicited Router Advertisement to locate default routers or learn
       prefixes.  To obtain Router Advertisements quickly, a host SHOULD transmit up
       to MAX_RTR_SOLICITATIONS Router Solicitation messages each separated by at
       least RTR_SOLICITATION_INTERVAL seconds.(FIXME:Therefore this should be invoked
       at the beginning of the simulation-WEI)*/
    RDEntry *rdEntry = new RDEntry();
    rdEntry->interfaceId = ie->getInterfaceId();
    rdEntry->numRSSent = 0;
    createAndSendRSPacket(ie);
    rdEntry->numRSSent++;

    //Create and schedule a message for retransmission to this module
    cMessage *rdTimeoutMsg = new cMessage("processRDTimeout", MK_RD_TIMEOUT);
    rdTimeoutMsg->setContextPointer(ie);
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
    //simtime_t rndInterval = uniform(0, ie->ipv6Data()->_getMaxRtrSolicitationDelay());
    scheduleAt(simTime() + ie->ipv6Data()->_getRtrSolicitationInterval(), rdTimeoutMsg);
}

void IPv6NeighbourDiscovery::cancelRouterDiscovery(InterfaceEntry *ie)
{
    //Next we retrieve the rdEntry with the Interface Entry.
    RDEntry *rdEntry = fetchRDEntry(ie);
    if (rdEntry != nullptr) {
        EV_DETAIL << "rdEntry is not nullptr, RD cancelled!" << endl;
        cancelAndDelete(rdEntry->timeoutMsg);
        rdList.erase(rdEntry);
        delete rdEntry;
    }
    else
        EV_DETAIL << "rdEntry is nullptr, not cancelling RD!" << endl;
}

void IPv6NeighbourDiscovery::processRDTimeout(cMessage *msg)
{
    InterfaceEntry *ie = (InterfaceEntry *)msg->getContextPointer();
    RDEntry *rdEntry = fetchRDEntry(ie);

    if (rdEntry->numRSSent < ie->ipv6Data()->_getMaxRtrSolicitations()) {
        bubble("Sending another RS message.");
        createAndSendRSPacket(ie);
        rdEntry->numRSSent++;

        //Need to find out if this is the last RS we are sending out.
        if (rdEntry->numRSSent == ie->ipv6Data()->_getMaxRtrSolicitations())
            scheduleAt(simTime() + ie->ipv6Data()->_getMaxRtrSolicitationDelay(), msg);
        else
            scheduleAt(simTime() + ie->ipv6Data()->_getRtrSolicitationInterval(), msg);
    }
    else {
        //RFC 2461, Section 6.3.7
        /*If a host sends MAX_RTR_SOLICITATIONS solicitations, and receives no Router
           Advertisements after having waited MAX_RTR_SOLICITATION_DELAY seconds after
           sending the last solicitation, the host concludes that there are no routers
           on the link for the purpose of [ADDRCONF]. However, the host continues to
           receive and process Router Advertisements messages in the event that routers
           appear on the link.*/
        bubble("Max number of RS messages sent");
        EV_INFO << "No RA messages were received. Assume no routers are on-link";
        delete rdEntry;
        rdList.erase(rdEntry);
        delete msg;
    }
}

void IPv6NeighbourDiscovery::processRSPacket(IPv6RouterSolicitation *rs,
        IPv6ControlInfo *rsCtrlInfo)
{
    if (validateRSPacket(rs, rsCtrlInfo) == false) {
        delete rsCtrlInfo;
        delete rs;
        return;
    }

    //Find out which interface the RS message arrived on.
    InterfaceEntry *ie = ift->getInterfaceById(rsCtrlInfo->getInterfaceId());
    AdvIfEntry *advIfEntry = fetchAdvIfEntry(ie);    //fetch advertising interface entry.

    //RFC 2461: Section 6.2.6
    //A host MUST silently discard any received Router Solicitation messages.
    if (ie->ipv6Data()->getAdvSendAdvertisements()) {
        EV_INFO << "This is an advertising interface, processing RS\n";

        if (validateRSPacket(rs, rsCtrlInfo) == false) {
            delete rsCtrlInfo;
            delete rs;
            return;
        }

        EV_INFO << "RS message validated\n";

        //First we extract RS specific information from the received message
        MACAddress macAddr = rs->getSourceLinkLayerAddress();
        EV_INFO << "MAC Address '" << macAddr << "' extracted\n";
        delete rsCtrlInfo;
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
        simtime_t interval = uniform(0, ie->ipv6Data()->_getMaxRADelayTime());

        if (interval < advIfEntry->nextScheduledRATime) {
            simtime_t nextScheduledTime;
            nextScheduledTime = simTime() + interval;
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
    else {
        EV_INFO << "This interface is a host, discarding RA message\n";
        delete rsCtrlInfo;
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
    if (rsCtrlInfo->getHopLimit() != 255) {
        EV_WARN << "Hop limit is not 255! RS validation failed!\n";
        result = false;
    }

    //- ICMP Code is 0.
    if (rsCtrlInfo->getProtocol() != IP_PROT_IPv6_ICMP) {
        EV_WARN << "ICMP Code is not 0! RS validation failed!\n";
        result = false;
    }

    //- If the IP source address is the unspecified address, there is no
    //source link-layer address option in the message.
    if (rsCtrlInfo->getSrcAddr().isUnspecified()) {
        EV_WARN << "IP source address is unspecified\n";

        if (rs->getSourceLinkLayerAddress().isUnspecified() == false) {
            EV_WARN << " but source link layer address is provided. RS validation failed!\n";
        }
    }

    return result;
}

IPv6RouterAdvertisement *IPv6NeighbourDiscovery::createAndSendRAPacket(const IPv6Address& destAddr, InterfaceEntry *ie)
{
    EV_INFO << "Create and send RA invoked!\n";
    //Must use link-local addr. See: RFC2461 Section 6.1.2
    IPv6Address sourceAddr = ie->ipv6Data()->getLinkLocalAddress();

    //This operation includes all options, regardless whether it is solicited or unsolicited.
    if (ie->ipv6Data()->getAdvSendAdvertisements()) {    //if this is an advertising interface
        //Construct a Router Advertisment message
        IPv6RouterAdvertisement *ra = new IPv6RouterAdvertisement("RApacket");
        ra->setType(ICMPv6_ROUTER_AD);

        //RFC 2461: Section 6.2.3 Router Advertisment Message Content
        /*A router sends periodic as well as solicited Router Advertisements out
           its advertising interfaces.  Outgoing Router Advertisements are filled
           with the following values consistent with the message format given in
           Section 4.2:*/

        //- In the Router Lifetime field: the interface's configured AdvDefaultLifetime.
        ra->setRouterLifetime(SIMTIME_DBL(ie->ipv6Data()->getAdvDefaultLifetime()));

        //- In the M and O flags: the interface's configured AdvManagedFlag and
        //AdvOtherConfigFlag, respectively.  See [ADDRCONF].
        ra->setManagedAddrConfFlag(ie->ipv6Data()->getAdvManagedFlag());
        ra->setOtherStatefulConfFlag(ie->ipv6Data()->getAdvOtherConfigFlag());

#ifdef WITH_xMIPv6
        // Configuring the HomeAgentFlag (H-bit) (RFC 3775): Zarrar 25.02.07
        if (rt6->isHomeAgent())
            ra->setHomeAgentFlag(true); //Set H-bit if the router is a HA
        else
            ra->setHomeAgentFlag(ie->ipv6Data()->getAdvHomeAgentFlag()); //else unset it, which is default
#endif /* WITH_xMIPv6 */

        //- In the Cur Hop Limit field: the interface's configured CurHopLimit.
        ra->setCurHopLimit(ie->ipv6Data()->getAdvCurHopLimit());

        //- In the Reachable Time field: the interface's configured AdvReachableTime.
        ra->setReachableTime(ie->ipv6Data()->getAdvReachableTime());

        //- In the Retrans Timer field: the interface's configured AdvRetransTimer.
        ra->setRetransTimer(ie->ipv6Data()->getAdvRetransTimer());

        //- In the options:
        /*o Source Link-Layer Address option: link-layer address of the sending
            interface.  (Assumption: We always send this)*/
        ra->setSourceLinkLayerAddress(ie->getMacAddress());
        ra->setMTU(ie->ipv6Data()->getAdvLinkMTU());

        //Add all Advertising Prefixes to the RA
        int numAdvPrefixes = ie->ipv6Data()->getNumAdvPrefixes();
        EV_DETAIL << "Number of Adv Prefixes: " << numAdvPrefixes << endl;
        ra->setPrefixInformationArraySize(numAdvPrefixes);
        for (int i = 0; i < numAdvPrefixes; i++) {
            IPv6InterfaceData::AdvPrefix advPrefix = ie->ipv6Data()->getAdvPrefix(i);
            IPv6NDPrefixInformation prefixInfo;

#ifndef WITH_xMIPv6
            prefixInfo.setPrefix(advPrefix.prefix);
#else /* WITH_xMIPv6 */
            EV_DETAIL << "\n+=+=+=+= Appendign Prefix Info Option to RA +=+=+=+=\n";
            EV_DETAIL << "Prefix Value: " << advPrefix.prefix << endl;
            EV_DETAIL << "Prefix Length: " << advPrefix.prefixLength << endl;
            EV_DETAIL << "L-Flag: " << advPrefix.advOnLinkFlag << endl;
            EV_DETAIL << "A-Flag: " << advPrefix.advAutonomousFlag << endl;
            EV_DETAIL << "R-Flag: " << advPrefix.advRtrAddr << endl;
            EV_DETAIL << "Global Address from Prefix: " << advPrefix.rtrAddress << endl;

            if (rt6->isHomeAgent() && advPrefix.advRtrAddr == true)
                prefixInfo.setPrefix(advPrefix.rtrAddress); //add the global-scope address of the HA's interface in the prefix option list of the RA message.
            else
                prefixInfo.setPrefix(advPrefix.prefix); //adds the prefix only of the router's interface in the prefix option list of the RA message.
#endif /* WITH_xMIPv6 */

            prefixInfo.setPrefixLength(advPrefix.prefixLength);

            //- In the "on-link" flag: the entry's AdvOnLinkFlag.
            prefixInfo.setOnlinkFlag(advPrefix.advOnLinkFlag);
            //- In the Valid Lifetime field: the entry's AdvValidLifetime.
            prefixInfo.setValidLifetime(SIMTIME_DBL(advPrefix.advValidLifetime));
            //- In the "Autonomous address configuration" flag: the entry's
            //AdvAutonomousFlag.
            prefixInfo.setAutoAddressConfFlag(advPrefix.advAutonomousFlag);

#ifdef WITH_xMIPv6
            if (rt6->isHomeAgent())
                prefixInfo.setRouterAddressFlag(true); // set the R-bit if the node is a HA

            //- In the Valid Lifetime field: the entry's AdvValidLifetime.
            prefixInfo.setValidLifetime(SIMTIME_DBL(advPrefix.advValidLifetime));
#endif /* WITH_xMIPv6 */

            //- In the Preferred Lifetime field: the entry's AdvPreferredLifetime.
            prefixInfo.setPreferredLifetime(SIMTIME_DBL(advPrefix.advPreferredLifetime));
            //Now we pop the prefix info into the RA.
            ra->setPrefixInformation(i, prefixInfo);
        }

        ra->setByteLength(ICMPv6_HEADER_BYTES + numAdvPrefixes * IPv6ND_PREFIX_INFORMATION_OPTION_LENGTH);
        sendPacketToIPv6Module(ra, destAddr, sourceAddr, ie->getInterfaceId());
        return ra;
    }

    return nullptr;    //XXX is this OK?
}

void IPv6NeighbourDiscovery::processRAPacket(IPv6RouterAdvertisement *ra,
        IPv6ControlInfo *raCtrlInfo)
{
    InterfaceEntry *ie = ift->getInterfaceById(raCtrlInfo->getInterfaceId());

    if (ie->ipv6Data()->getAdvSendAdvertisements()) {
        EV_INFO << "Interface is an advertising interface, dropping RA message.\n";
        delete raCtrlInfo;
        delete ra;
        return;
    }
    else {
        if (validateRAPacket(ra, raCtrlInfo) == false) {
            delete raCtrlInfo;
            delete ra;
            return;
        }

#ifdef WITH_xMIPv6
        if (ie->ipv6Data()->isDADInProgress()) {
            // in case we are currently performing DAD we ignore this RA
            // TODO improve this procedure in order to allow reinitiating DAD
            // (which means cancel current DAD, start new DAD)
            delete raCtrlInfo;
            delete ra;
            return;
        }
#endif /* WITH_xMIPv6 */

        cancelRouterDiscovery(ie);    //Cancel router discovery if it is in progress.
        EV_INFO << "Interface is a host, processing RA.\n";

        processRAForRouterUpdates(ra, raCtrlInfo);    //See RFC2461: Section 6.3.4

        //Possible options
        //MACAddress macAddress = ra->getSourceLinkLayerAddress();
        //uint mtu = ra->getMTU();
        for (int i = 0; i < (int)ra->getPrefixInformationArraySize(); i++) {
            IPv6NDPrefixInformation& prefixInfo = ra->getPrefixInformation(i);
            if (prefixInfo.getAutoAddressConfFlag() == true) {    //If auto addr conf is set
#ifndef WITH_xMIPv6
                processRAPrefixInfoForAddrAutoConf(prefixInfo, ie);    //We process prefix Info and form an addr
#else /* WITH_xMIPv6 */
                processRAPrefixInfoForAddrAutoConf(prefixInfo, ie, ra->getHomeAgentFlag());    // then calling the overloaded function for address configuration. The address conf for MN is different from other nodes as it needs to classify the newly formed address as HoA or CoA, depending on the status of the H-Flag. (Zarrar Yousaf 20.07.07)
#endif /* WITH_xMIPv6 */
            }

#ifdef WITH_xMIPv6
            // When in foreign network(s), the MN needs info about its HA address and its own Home Address (HoA), when sending BU to HA and CN(s). Therefore while in the home network I intialise struct HomeNetworkInfo{} with HoA and HA address, which will eventually be used by the MN while sending BUs from within visit networks. (Zarrar Yousaf 12.07.07)
            if (ra->getHomeAgentFlag() && (prefixInfo.getRouterAddressFlag() == true)) {    //If R-Flag is set and RA is from HA
                // homeNetworkInfo now carries HoA, global unicast HA address and the home network prefix
                // update 4.9.07 - CB
                IPv6Address HoA = ie->ipv6Data()->getGlobalAddress();    //MN's home address
                IPv6Address HA = raCtrlInfo->getSrcAddr().setPrefix(prefixInfo.getPrefix(), prefixInfo.getPrefixLength());
                EV_DETAIL << "The HoA of MN is: " << HoA << ", MN's HA Address is: " << HA
                          << " and the home prefix is " << prefixInfo.getPrefix() << endl;
                ie->ipv6Data()->updateHomeNetworkInfo(HoA, HA, prefixInfo.getPrefix(), prefixInfo.getPrefixLength());    //populate the HoA of MN, the HA global scope address and the home network prefix
            }
#endif /* WITH_xMIPv6 */
        }
    }
    delete raCtrlInfo;
    delete ra;
}

void IPv6NeighbourDiscovery::processRAForRouterUpdates(IPv6RouterAdvertisement *ra,
        IPv6ControlInfo *raCtrlInfo)
{
    EV_INFO << "Processing RA for Router Updates\n";
    //RFC2461: Section 6.3.4
    //Paragraphs 1 and 2 omitted.

    //On receipt of a valid Router Advertisement, a host extracts the source
    //address of the packet and does the following:
    IPv6Address raSrcAddr = raCtrlInfo->getSrcAddr();
    InterfaceEntry *ie = ift->getInterfaceById(raCtrlInfo->getInterfaceId());
    int ifID = ie->getInterfaceId();

    /*- If the address is not already present in the host's Default Router List,
       and the advertisement's Router Lifetime is non-zero, create a new entry in
       the list, and initialize its invalidation timer value from the advertisement's
       Router Lifetime field.*/
    Neighbour *neighbour = neighbourCache.lookup(raSrcAddr, ifID);

#ifdef WITH_xMIPv6
    // update 3.9.07 - CB // if (neighbour == nullptr && (ra->homeAgentFlag() == true)) //the RA is from a Router acting as a Home Agent as well
#endif /* WITH_xMIPv6 */

    if (neighbour == nullptr) {
        EV_INFO << "Neighbour Cache Entry does not contain RA's source address\n";
        if (ra->getRouterLifetime() != 0) {
            EV_INFO << "RA's router lifetime is non-zero, creating an entry in the "
                    << "Host's default router list with lifetime=" << ra->getRouterLifetime() << "\n";

#ifdef WITH_xMIPv6
            // initiate neighbour unreachability detection for existing routers and remove default route(r), 3.9.07 - CB
            // TODO improve this code
            routersUnreachabilityDetection(ie);
#endif /* WITH_xMIPv6 */

            //If a Neighbor Cache entry is created for the router its reachability
            //state MUST be set to STALE as specified in Section 7.3.3.
            neighbour = neighbourCache.addRouter(raSrcAddr, ifID,
#ifndef WITH_xMIPv6
                        ra->getSourceLinkLayerAddress(), simTime() + ra->getRouterLifetime());
#else /* WITH_xMIPv6 */
                        ra->getSourceLinkLayerAddress(), simTime() + ra->getRouterLifetime(), ra->getHomeAgentFlag());
#endif /* WITH_xMIPv6 */
            //According to Greg, we should add a default route for hosts as well!
            rt6->addDefaultRoute(raSrcAddr, ifID, simTime() + ra->getRouterLifetime());
        }
        else {
            EV_INFO << "Router Lifetime is 0, adding NON-default router.\n";
            //WEI-The router is advertising itself, BUT not as a default router.
            if (ra->getSourceLinkLayerAddress().isUnspecified())
                neighbour = neighbourCache.addNeighbour(raSrcAddr, ifID);
            else
                neighbour = neighbourCache.addNeighbour(raSrcAddr, ifID,
                            ra->getSourceLinkLayerAddress());
            neighbour->isRouter = true;
        }
    }
    else {
        //If no Source Link-Layer Address is included, but a corresponding Neighbor
        //Cache entry exists, its IsRouter flag MUST be set to TRUE.
        neighbour->isRouter = true;

        //If a cache entry already exists and is updated with a different link-
        //layer address the reachability state MUST also be set to STALE.
        if (ra->getSourceLinkLayerAddress().isUnspecified() == false &&
            neighbour->macAddress.equals(ra->getSourceLinkLayerAddress()) == false)
            neighbour->macAddress = ra->getSourceLinkLayerAddress();

        /*- If the address is already present in the host's Default Router List
           as a result of a previously-received advertisement, reset its invalidation
           timer to the Router Lifetime value in the newly-received advertisement.*/
        neighbour->routerExpiryTime = simTime() + ra->getRouterLifetime();

        /*- If the address is already present in the host's Default Router List
           and the received Router Lifetime value is zero, immediately time-out the
           entry as specified in Section 6.3.5.*/
        if (ra->getRouterLifetime() == 0) {
            EV_INFO << "RA's router lifetime is ZERO. Timing-out entry.\n";
            timeoutDefaultRouter(raSrcAddr, ifID);
        }
    }

    //Paragraph Omitted.

    //If the received Cur Hop Limit value is non-zero the host SHOULD set
    //its CurHopLimit variable to the received value.
    if (ra->getCurHopLimit() != 0) {
        EV_INFO << "RA's Cur Hop Limit is non-zero. Setting host's Cur Hop Limit to "
                << "received value.\n";
        ie->ipv6Data()->setCurHopLimit(ra->getCurHopLimit());
    }

    //If the received Reachable Time value is non-zero the host SHOULD set its
    //BaseReachableTime variable to the received value.
    if (ra->getReachableTime() != 0) {
        EV_INFO << "RA's reachable time is non-zero ";

        if (ra->getReachableTime() != SIMTIME_DBL(ie->ipv6Data()->getReachableTime())) {
            EV_INFO << " and RA's and Host's reachable time differ, \nsetting host's base"
                    << " reachable time to received value.\n";
            ie->ipv6Data()->setBaseReachableTime(ra->getReachableTime());
            //If the new value differs from the previous value, the host SHOULD
            //recompute a new random ReachableTime value.
            ie->ipv6Data()->setReachableTime(ie->ipv6Data()->generateReachableTime());
        }

        EV_INFO << endl;
    }

    //The RetransTimer variable SHOULD be copied from the Retrans Timer field,
    //if the received value is non-zero.
    if (ra->getRetransTimer() != 0) {
        EV_INFO << "RA's retrans timer is non-zero, copying retrans timer variable.\n";
        ie->ipv6Data()->setRetransTimer(ra->getRetransTimer());
    }

    /*If the MTU option is present, hosts SHOULD copy the option's value into
       LinkMTU so long as the value is greater than or equal to the minimum link MTU
       [IPv6] and does not exceed the default LinkMTU value specified in the link
       type specific document (e.g., [IPv6-ETHER]).*/
    //TODO: not done yet

    processRAPrefixInfo(ra, ie);
}

void IPv6NeighbourDiscovery::processRAPrefixInfo(IPv6RouterAdvertisement *ra,
        InterfaceEntry *ie)
{
    //Continued from section 6.3.4
    /*Prefix Information options that have the "on-link" (L) flag set indicate a
       prefix identifying a range of addresses that should be considered on-link.
       Note, however, that a Prefix Information option with the on-link flag set to
       zero conveys no information concerning on-link determination and MUST NOT be
       interpreted to mean that addresses covered by the prefix are off-link. The
       only way to cancel a previous on-link indication is to advertise that prefix
       with the L-bit set and the Lifetime set to zero. The default behavior (see
       Section 5.2) when sending a packet to an address for which no information is
       known about the on-link status of the address is to forward the packet to a
       default router; the reception of a Prefix Information option with the "on-link "
       (L) flag set to zero does not change this behavior. The reasons for an address
       being treated as on-link is specified in the definition of "on-link" in
       Section 2.1. Prefixes with the on-link flag set to zero would normally have
       the autonomous flag set and be used by [ADDRCONF].*/
    IPv6NDPrefixInformation prefixInfo;
    //For each Prefix Information option
    for (int i = 0; i < (int)ra->getPrefixInformationArraySize(); i++) {
        prefixInfo = ra->getPrefixInformation(i);
        if (!prefixInfo.getOnlinkFlag())
            break; //skip to next prefix option

        //with the on-link flag set, a host does the following:
        EV_INFO << "Fetching Prefix Information:" << i + 1 << " of "
                << ra->getPrefixInformationArraySize() << endl;
        uint prefixLength = prefixInfo.getPrefixLength();
        simtime_t validLifetime = prefixInfo.getValidLifetime();
        //uint preferredLifetime = prefixInfo.getPreferredLifetime();
        IPv6Address prefix = prefixInfo.getPrefix();

        //- If the prefix is the link-local prefix, silently ignore the Prefix
        //Information option.
        if (prefix.isLinkLocal()) {
            EV_INFO << "Prefix is link-local, ignoring prefix.\n";
            return;
        }

        //- If the prefix is not already present in the Prefix List,
        if (!rt6->isPrefixPresent(prefix)) {
            //and the Prefix Information option's Valid Lifetime field is non-zero,
            if (validLifetime != 0) {
                /*create a new entry for the prefix and initialize its invalidation
                   timer to the Valid Lifetime value in the Prefix Information option.*/
                rt6->addOrUpdateOnLinkPrefix(prefix, prefixLength, ie->getInterfaceId(),
                        simTime() + validLifetime);
            }
            /*- If the Prefix Information option's Valid Lifetime field is zero,
               and the prefix is not present in the host's Prefix List,
               silently ignore the option.*/
        }
        else {
            /* If the new Lifetime value is zero, time-out the prefix immediately
               (see Section 6.3.5).*/
            if (validLifetime == 0) {
                EV_INFO << "Prefix Info's valid lifetime is 0, time-out prefix\n";
                rt6->deleteOnLinkPrefix(prefix, prefixLength);
                return;
            }

            /*- If the prefix is already present in the host's Prefix List as
               the result of a previously-received advertisement, reset its
               invalidation timer to the Valid Lifetime value in the Prefix
               Information option.*/
            rt6->addOrUpdateOnLinkPrefix(prefix, prefixLength, ie->getInterfaceId(),
                    simTime() + validLifetime);
        }

        /*Stateless address autoconfiguration [ADDRCONF] may in some
           circumstances increase the Valid Lifetime of a prefix or ignore it
           completely in order to prevent a particular denial of service attack.
           However, since the effect of the same denial of service targeted at
           the on-link prefix list is not catastrophic (hosts would send packets
           to a default router and receive a redirect rather than sending
           packets directly to a neighbor) the Neighbor Discovery protocol does
           not impose such a check on the prefix lifetime values.*/
    }
}

#ifndef WITH_xMIPv6
void IPv6NeighbourDiscovery::processRAPrefixInfoForAddrAutoConf(IPv6NDPrefixInformation& prefixInfo, InterfaceEntry *ie)
{
    EV_INFO << "Processing Prefix Info for address auto-configuration.\n";
    IPv6Address prefix = prefixInfo.getPrefix();
    uint prefixLength = prefixInfo.getPrefixLength();
    simtime_t preferredLifetime = prefixInfo.getPreferredLifetime();
    simtime_t validLifetime = prefixInfo.getValidLifetime();

    //RFC 2461: Section 5.5.3
    //First condition tested, the autonomous flag is already set

    //b) If the prefix is the link-local prefix, silently ignore the Prefix
    //Information option.
    if (prefixInfo.getPrefix().isLinkLocal() == true) {
        EV_INFO << "Prefix is link-local, ignore Prefix Information Option\n";
        return;
    }

    //c) If the preferred lifetime is greater than the valid lifetime, silently
    //ignore the Prefix Information option. A node MAY wish to log a system
    //management error in this case.
    if (preferredLifetime > validLifetime) {
        EV_INFO << "Preferred lifetime is greater than valid lifetime, ignore Prefix Information\n";
        return;
    }

    bool isPrefixAssignedToInterface = false;
    for (int i = 0; i < ie->ipv6Data()->getNumAddresses(); i++) {
        if (ie->ipv6Data()->getAddress(i).matches(prefix, prefixLength) == true)
            isPrefixAssignedToInterface = true;
    }

    /*d) If the prefix advertised does not match the prefix of an address already
         in the list, and the Valid Lifetime is not 0, form an address (and add
         it to the list) by combining the advertised prefix with the link�s
         interface identifier as follows:*/
    if (isPrefixAssignedToInterface == false && validLifetime != 0) {
        IPv6Address linkLocalAddress = ie->ipv6Data()->getLinkLocalAddress();
        ASSERT(linkLocalAddress.isUnspecified() == false);
        IPv6Address newAddr = linkLocalAddress.setPrefix(prefix, prefixLength);
        //TODO: for now we leave the newly formed address as not tentative,
        //according to Greg, we have to always perform DAD for a newly formed address.
        EV_INFO << "Assigning new address to: " << ie->getName() << endl;
        ie->ipv6Data()->assignAddress(newAddr, false, simTime() + validLifetime,
                simTime() + preferredLifetime);
    }

    //TODO: this is the simplified version.
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

#endif /* WITH_xMIPv6 */

void IPv6NeighbourDiscovery::createRATimer(InterfaceEntry *ie)
{
    cMessage *msg = new cMessage("sendPeriodicRA", MK_SEND_PERIODIC_RTRADV);
    msg->setContextPointer(ie);
    AdvIfEntry *advIfEntry = new AdvIfEntry();
    advIfEntry->interfaceId = ie->getInterfaceId();
    advIfEntry->numRASent = 0;

#ifdef WITH_xMIPv6
    // 20.9.07 - CB
    /*if ( rt6->isRouter() )
       {
        ie->ipv6()->setMinRtrAdvInterval(IPv6NeighbourDiscovery::getMinRAInterval()); //should be 0.07 for MIPv6 Support
        ie->ipv6()->setMaxRtrAdvInterval(IPv6NeighbourDiscovery::getMaxRAInterval()); //should be 0.03 for MIPv6 Support
       }*/
    // update 23.10.07 - CB

    if (canServeWirelessNodes(ie)) {
        EV_INFO << "This Interface is connected to a WLAN AP, hence using MIPv6 Default Values" << endl;
        simtime_t minRAInterval = par("minIntervalBetweenRAs");    //reading from the omnetpp.ini (ZY 23.07.09)
        simtime_t maxRAInterval = par("maxIntervalBetweenRAs");    //reading from the omnetpp.ini (ZY 23.07.09
        ie->ipv6Data()->setMinRtrAdvInterval(minRAInterval);
        ie->ipv6Data()->setMaxRtrAdvInterval(maxRAInterval);
    }
    else {
        EV_INFO << "This Interface is not connected to a WLAN AP, hence using default values" << endl;
        //interval = uniform( ie->ipv6()->minRtrAdvInterval(), ie->ipv6()->maxRtrAdvInterval() );
        //EV<<"\nThe random calculated RA_ND interval is: "<< interval<<" seconds\n";
    }
    // end CB
#endif /* WITH_xMIPv6 */

    simtime_t interval = uniform(ie->ipv6Data()->getMinRtrAdvInterval(), ie->ipv6Data()->getMaxRtrAdvInterval());
    advIfEntry->raTimeoutMsg = msg;

    simtime_t nextScheduledTime = simTime() + interval;
    advIfEntry->nextScheduledRATime = nextScheduledTime;
    advIfList.insert(advIfEntry);
    EV_DETAIL << "Interval: " << interval << endl;
    EV_DETAIL << "Next scheduled time: " << nextScheduledTime << endl;
    //now we schedule the msg for whatever time that was derived
    scheduleAt(nextScheduledTime, msg);
}

void IPv6NeighbourDiscovery::resetRATimer(InterfaceEntry *ie)
{    //Not used yet but could be useful later on.-WEI
     //Iterate through all RA timers within the Neighbour Discovery module.
/*
    for (auto it =raTimerList.begin(); it != raTimerList.end(); it++)
    {
        cMessage *msg = (*it);
        InterfaceEntry *msgIE = (InterfaceEntry *)msg->getContextPointer();
        //Find the timer that matches the given Interface Entry.
        if (msgIE->outputPort() == ie->outputPort())
        {
            EV << "Resetting RA timer for port: " << ie->outputPort();
            cancelEvent(msg);//Cancel the next scheduled msg.
            simtime_t interval
                = uniform(ie->ipv6Data()->getMinRtrAdvInterval(),ie->ipv6Data()->getMaxRtrAdvInterval());
            scheduleAt(simTime()+interval, msg);
        }
    }
 */
}

void IPv6NeighbourDiscovery::sendPeriodicRA(cMessage *msg)
{
    InterfaceEntry *ie = (InterfaceEntry *)msg->getContextPointer();
    AdvIfEntry *advIfEntry = fetchAdvIfEntry(ie);
    IPv6Address destAddr = IPv6Address("FF02::1");
    createAndSendRAPacket(destAddr, ie);
    advIfEntry->numRASent++;
    simtime_t nextScheduledTime;

    //RFC 2461, Section 6.2.4
    /*Whenever a multicast advertisement is sent from an interface, the timer is
       reset to a uniformly-distributed random value between the interface's
       configured MinRtrAdvInterval and MaxRtrAdvInterval; expiration of the timer
       causes the next advertisement to be sent and a new random value to be chosen.*/

    simtime_t interval;

#ifdef WITH_xMIPv6
    EV_DEBUG << "\n+=+=+= MIPv6 Feature: " << rt6->hasMIPv6Support() << " +=+=+=\n";
#endif /* WITH_xMIPv6 */

    interval = uniform(ie->ipv6Data()->getMinRtrAdvInterval(), ie->ipv6Data()->getMaxRtrAdvInterval());

#ifdef WITH_xMIPv6
    EV_DETAIL << "\n +=+=+= The random calculated interval is: " << interval << " +=+=+=\n";
#endif /* WITH_xMIPv6 */

    nextScheduledTime = simTime() + interval;

    /*For the first few advertisements (up to MAX_INITIAL_RTR_ADVERTISEMENTS)
       sent from an interface when it becomes an advertising interface,*/
    EV_DETAIL << "Num RA sent is: " << advIfEntry->numRASent << endl;
    EV_DETAIL << "maxInitialRtrAdvertisements is: " << ie->ipv6Data()->_getMaxInitialRtrAdvertisements() << endl;

    if (advIfEntry->numRASent <= ie->ipv6Data()->_getMaxInitialRtrAdvertisements()) {
        if (interval > ie->ipv6Data()->_getMaxInitialRtrAdvertInterval()) {
            //if the randomly chosen interval is greater than MAX_INITIAL_RTR_ADVERT_INTERVAL,
            //the timer SHOULD be set to MAX_INITIAL_RTR_ADVERT_INTERVAL instead.
            nextScheduledTime = simTime() + ie->ipv6Data()->_getMaxInitialRtrAdvertInterval();
            EV_INFO << "Sending initial RA but interval is too long. Using default value." << endl;
        }
        else
            EV_INFO << "Sending initial RA. Using randomly generated interval." << endl;
    }

    EV_DETAIL << "Next scheduled time: " << nextScheduledTime << endl;
    advIfEntry->nextScheduledRATime = nextScheduledTime;
    ASSERT(nextScheduledTime > simTime());
    scheduleAt(nextScheduledTime, msg);
}

void IPv6NeighbourDiscovery::sendSolicitedRA(cMessage *msg)
{
    EV_INFO << "Send Solicited RA invoked!\n";
    InterfaceEntry *ie = (InterfaceEntry *)msg->getContextPointer();
    IPv6Address destAddr = IPv6Address("FF02::1");
    EV_DETAIL << "Testing condition!\n";
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
    raCtrlInfo->getSrcAddr();

    //- IP Source Address is a link-local address.  Routers must use
    //  their link-local address as the source for Router Advertisement
    //  and Redirect messages so that hosts can uniquely identify
    //  routers.
    if (raCtrlInfo->getSrcAddr().isLinkLocal() == false) {
        EV_WARN << "RA source address is not link-local. RA validation failed!\n";
        result = false;
    }

    //- The IP Hop Limit field has a value of 255, i.e., the packet
    //  could not possibly have been forwarded by a router.
    if (raCtrlInfo->getHopLimit() != 255) {
        EV_WARN << "Hop limit is not 255! RA validation failed!\n";
        result = false;
    }

    //- ICMP Code is 0.
    if (raCtrlInfo->getProtocol() != IP_PROT_IPv6_ICMP) {
        EV_WARN << "ICMP Code is not 0! RA validation failed!\n";
        result = false;
    }

#ifdef WITH_xMIPv6
    // - All included options have a length that is greater than zero.
    // CB
    if (ra->getPrefixInformationArraySize() == 0) {
        EV_WARN << "No prefix information available! RA validation failed\n";
        result = false;
    }
#endif /* WITH_xMIPv6 */

    return result;
}

IPv6NeighbourSolicitation *IPv6NeighbourDiscovery::createAndSendNSPacket(const IPv6Address& nsTargetAddr, const IPv6Address& dgDestAddr,
        const IPv6Address& dgSrcAddr, InterfaceEntry *ie)
{
#ifdef WITH_xMIPv6
    Enter_Method_Silent();
#endif /* WITH_xMIPv6 */

    MACAddress myMacAddr = ie->getMacAddress();

    //Construct a Neighbour Solicitation message
    IPv6NeighbourSolicitation *ns = new IPv6NeighbourSolicitation("NSpacket");
    ns->setType(ICMPv6_NEIGHBOUR_SOL);

    //Neighbour Solicitation Specific Information
    ns->setTargetAddress(nsTargetAddr);
    ns->setByteLength(ICMPv6_HEADER_BYTES + IPv6_ADDRESS_SIZE);      // RFC 2461, Section 4.3.

    /*If the solicitation is being sent to a solicited-node multicast
       address, the sender MUST include its link-layer address (if it has
       one) as a Source Link-Layer Address option.*/
    if (dgDestAddr.matches(IPv6Address("FF02::1:FF00:0"), 104) &&    // FIXME what's this? make constant...
            !dgSrcAddr.isUnspecified()) {
        ns->setSourceLinkLayerAddress(myMacAddr);
        ns->addByteLength(IPv6ND_LINK_LAYER_ADDRESS_OPTION_LENGTH);
    }

    sendPacketToIPv6Module(ns, dgDestAddr, dgSrcAddr, ie->getInterfaceId());

    return ns;
}

void IPv6NeighbourDiscovery::processNSPacket(IPv6NeighbourSolicitation *ns,
        IPv6ControlInfo *nsCtrlInfo)
{
    //Control Information
    InterfaceEntry *ie = ift->getInterfaceById(nsCtrlInfo->getInterfaceId());

    IPv6Address nsTargetAddr = ns->getTargetAddress();

    //RFC 2461:Section 7.2.3
    //If target address is not a valid "unicast" or anycast address assigned to the
    //receiving interface, we should silently discard the packet.
    if (validateNSPacket(ns, nsCtrlInfo) == false
        || ie->ipv6Data()->hasAddress(nsTargetAddr) == false)
    {
        bubble("NS validation failed\n");
        delete nsCtrlInfo;
        delete ns;
        return;
    }

    bubble("NS validation passed.\n");

    if (ie->ipv6Data()->isTentativeAddress(nsTargetAddr)) {
        //If the Target Address is tentative, the Neighbor Solicitation should
        //be processed as described in [ADDRCONF].
        EV_INFO << "Process NS for Tentative target address.\n";
        processNSForTentativeAddress(ns, nsCtrlInfo);
    }
    else {
        //Otherwise, the following description applies.
        EV_INFO << "Process NS for Non-Tentative target address.\n";
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
    if (nsCtrlInfo->getHopLimit() != 255) {
        EV_WARN << "Hop limit is not 255! NS validation failed!\n";
        result = false;
    }

    //- Target Address is not a multicast address.
    if (ns->getTargetAddress().isMulticast() == true) {
        EV_WARN << "Target address is a multicast address! NS validation failed!\n";
        result = false;
    }

    //- If the IP source address is the unspecified address,
    if (nsCtrlInfo->getSrcAddr().isUnspecified()) {
        EV_WARN << "Source Address is unspecified\n";

        //the IP destination address is a solicited-node multicast address.
        if (nsCtrlInfo->getDestAddr().matches(IPv6Address::SOLICITED_NODE_PREFIX, 104) == false) {
            EV_WARN << " but IP dest address is not a solicited-node multicast address! NS validation failed!\n";
            result = false;
        }
        //there is no source link-layer address option in the message.
        else if (ns->getSourceLinkLayerAddress().isUnspecified() == false) {
            EV_WARN << " but Source link-layer address is not empty! NS validation failed!\n";
            result = false;
        }
        else
            EV_WARN << "NS Validation Passed\n";
    }

    return result;
}

void IPv6NeighbourDiscovery::processNSForTentativeAddress(IPv6NeighbourSolicitation *ns,
        IPv6ControlInfo *nsCtrlInfo)
{
    //Control Information
    IPv6Address nsSrcAddr = nsCtrlInfo->getSrcAddr();
    //IPv6Address nsDestAddr = nsCtrlInfo->getDestAddr();

    ASSERT(nsSrcAddr.isUnicast() || nsSrcAddr.isUnspecified());
    //solicitation is processed as described in RFC2462:section 5.4.3

    if (nsSrcAddr.isUnspecified()) {
        EV_INFO << "Source Address is UNSPECIFIED. Sender is performing DAD\n";

        //Sender performing Duplicate Address Detection
        if (rt6->isLocalAddress(nsSrcAddr)) // FIXME: isLocalAddress(UNSPECIFIED) is always false!!! Must write another check for detecting source is myself/foreign node!!!
            EV_INFO << "NS comes from myself. Ignoring NS\n";
        else {
            EV_INFO << "NS comes from another node. Address is duplicate!\n";
            throw cRuntimeError("Duplicate Address Detected! Manual Attention Required!");
        }
    }
    else if (nsSrcAddr.isUnicast()) {
        //Sender performing address resolution
        EV_INFO << "Sender is performing Address Resolution\n";
        EV_INFO << "Target Address is tentative. Ignoring NS.\n";
    }
}

void IPv6NeighbourDiscovery::processNSForNonTentativeAddress(IPv6NeighbourSolicitation *ns,
        IPv6ControlInfo *nsCtrlInfo, InterfaceEntry *ie)
{
    //Neighbour Solicitation Information
    //MACAddress nsMacAddr = ns->getSourceLinkLayerAddress();

    //target addr is not tentative addr
    //solicitation processed as described in RFC2461:section 7.2.3
    if (nsCtrlInfo->getSrcAddr().isUnspecified()) {
        EV_INFO << "Address is duplicate! Inform Sender of duplicate address!\n";
        sendSolicitedNA(ns, nsCtrlInfo, ie);
    }
    else {
        processNSWithSpecifiedSrcAddr(ns, nsCtrlInfo, ie);
    }
}

void IPv6NeighbourDiscovery::processNSWithSpecifiedSrcAddr(IPv6NeighbourSolicitation *ns,
        IPv6ControlInfo *nsCtrlInfo, InterfaceEntry *ie)
{
    //RFC 2461, Section 7.2.3
    /*If the Source Address is not the unspecified address and, on link layers
       that have addresses, the solicitation includes a Source Link-Layer Address
       option, then the recipient SHOULD create or update the Neighbor Cache entry
       for the IP Source Address of the solicitation.*/

    //Neighbour Solicitation Information
    MACAddress nsMacAddr = ns->getSourceLinkLayerAddress();

    int ifID = ie->getInterfaceId();

    //Look for the Neighbour Cache Entry
    Neighbour *entry = neighbourCache.lookup(nsCtrlInfo->getSrcAddr(), ifID);

    if (entry == nullptr) {
        /*If an entry does not already exist, the node SHOULD create a new one
           and set its reachability state to STALE as specified in Section 7.3.3.*/
        EV_INFO << "Neighbour Entry not found. Create a Neighbour Cache Entry.\n";
        neighbourCache.addNeighbour(nsCtrlInfo->getSrcAddr(), ifID, nsMacAddr);
    }
    else {
        /*If an entry already exists, and the cached link-layer address differs from
           the one in the received Source Link-Layer option,*/
        if (!(entry->macAddress.equals(nsMacAddr)) && !nsMacAddr.isUnspecified()) {
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

void IPv6NeighbourDiscovery::sendSolicitedNA(IPv6NeighbourSolicitation *ns,
        IPv6ControlInfo *nsCtrlInfo, InterfaceEntry *ie)
{
    IPv6NeighbourAdvertisement *na = new IPv6NeighbourAdvertisement("NApacket");
    na->setByteLength(ICMPv6_HEADER_BYTES + IPv6_ADDRESS_SIZE);      // FIXME set correct length

    //RFC 2461: Section 7.2.4
    /*A node sends a Neighbor Advertisement in response to a valid Neighbor
       Solicitation targeting one of the node's assigned addresses.  The
       Target Address of the advertisement is copied from the Target Address
       of the solicitation.*/
    na->setTargetAddress(ns->getTargetAddress());

    /*If the solicitation's IP Destination Address is not a multicast address,
       the Target Link-Layer Address option MAY be omitted; the neighboring node's
       cached value must already be current in order for the solicitation to have
       been received. If the solicitation's IP Destination Address is a multicast
       address, the Target Link-Layer option MUST be included in the advertisement.*/
    na->setTargetLinkLayerAddress(ie->getMacAddress());    //here, we always include the MAC addr.
    na->addByteLength(IPv6ND_LINK_LAYER_ADDRESS_OPTION_LENGTH);

    /*Furthermore, if the node is a router, it MUST set the Router flag to one;
       otherwise it MUST set the flag to zero.*/
    na->setRouterFlag(rt6->isRouter());

    /*If the (NS)Target Address is either an anycast address or a unicast
       address for which the node is providing proxy service, or the Target
       Link-Layer Address option is not included,*/
    //TODO:ANYCAST will not be implemented here!

    if (ns->getSourceLinkLayerAddress().isUnspecified())
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
    if (nsCtrlInfo->getSrcAddr().isUnspecified()) {
        /*the node MUST set the Solicited flag to zero and multicast the advertisement
           to the all-nodes address.*/
        na->setSolicitedFlag(false);
        naDestAddr = IPv6Address::ALL_NODES_2;
    }
    else {
        /*Otherwise, the node MUST set the Solicited flag to one and unicast
           the advertisement to the Source Address of the solicitation.*/
        na->setSolicitedFlag(true);
        naDestAddr = nsCtrlInfo->getSrcAddr();
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
    //TODO: if above mentioned happens, can addr resolution be performed for ND messages?
    //if no link-layer addr exists for unicast addr when sending solicited NA, we should
    //add the NA to the list of queued packets. What if we have a list of queued
    //packets for different unicast solicitations? each time addr resolution is
    //done we should check the destinations of the list of queued packets and send
    //off the respective ones.
    IPv6Address myIPv6Addr = ie->ipv6Data()->getPreferredAddress();
    sendPacketToIPv6Module(na, naDestAddr, myIPv6Addr, ie->getInterfaceId());
}

void IPv6NeighbourDiscovery::sendUnsolicitedNA(InterfaceEntry *ie)
{
    //RFC 2461
    //Section 7.2.6: Sending Unsolicited Neighbor Advertisements
#ifdef WITH_xMIPv6
    Enter_Method_Silent();
#endif /* WITH_xMIPv6 */

#ifndef WITH_xMIPv6
    // In some cases a node may be able to determine that its link-layer
    // address has changed (e.g., hot-swap of an interface card) and may
    // wish to inform its neighbors of the new link-layer address quickly.
    // In such cases a node MAY send up to MAX_NEIGHBOR_ADVERTISEMENT
    // unsolicited Neighbor Advertisement messages to the all-nodes
    // multicast address.  These advertisements MUST be separated by at
    // least RetransTimer seconds.
#else /* WITH_xMIPv6 */
    IPv6NeighbourAdvertisement *na = new IPv6NeighbourAdvertisement("NApacket");
    IPv6Address myIPv6Addr = ie->ipv6Data()->getPreferredAddress();
    na->setByteLength(ICMPv6_HEADER_BYTES + IPv6_ADDRESS_SIZE);
#endif /* WITH_xMIPv6 */

    // The Target Address field in the unsolicited advertisement is set to
    // an IP address of the interface, and the Target Link-Layer Address
    // option is filled with the new link-layer address.
#ifdef WITH_xMIPv6
    na->setTargetAddress(myIPv6Addr);
    na->setTargetLinkLayerAddress(ie->getMacAddress());
    na->addByteLength(IPv6ND_LINK_LAYER_ADDRESS_OPTION_LENGTH);
#endif /* WITH_xMIPv6 */

    // The Solicited flag MUST be set to zero, in order to avoid confusing
    // the Neighbor Unreachability Detection algorithm.
#ifdef WITH_xMIPv6
    na->setSolicitedFlag(false);
#endif /* WITH_xMIPv6 */

    // If the node is a router, it MUST set the Router flag to one;
    // otherwise it MUST set it to zero.
#ifdef WITH_xMIPv6
    na->setRouterFlag(rt6->isRouter());
#endif /* WITH_xMIPv6 */

    // The Override flag MAY be set to either zero or one.  In either case,
    // neighboring nodes will immediately change the state of their Neighbor
    // Cache entries for the Target Address to STALE, prompting them to
    // verify the path for reachability.  If the Override flag is set to
    // one, neighboring nodes will install the new link-layer address in
    // their caches.  Otherwise, they will ignore the new link-layer
    // address, choosing instead to probe the cached address.
#ifdef WITH_xMIPv6
    na->setOverrideFlag(true);
#endif /* WITH_xMIPv6 */

    // A node that has multiple IP addresses assigned to an interface MAY
    // multicast a separate Neighbor Advertisement for each address.  In
    // such a case the node SHOULD introduce a small delay between the
    // sending of each advertisement to reduce the probability of the
    // advertisements being lost due to congestion.

    // A proxy MAY multicast Neighbor Advertisements when its link-layer
    // address changes or when it is configured (by system management or
    // other mechanisms) to proxy for an address.  If there are multiple
    // nodes that are providing proxy services for the same set of addresses
    // the proxies SHOULD provide a mechanism that prevents multiple proxies
    // from multicasting advertisements for any one address, in order to
    // reduce the risk of excessive multicast traffic.

    // Also, a node belonging to an anycast address MAY multicast
    // unsolicited Neighbor Advertisements for the anycast address when the
    // node's link-layer address changes.

    // Note that because unsolicited Neighbor Advertisements do not reliably
    // update caches in all nodes (the advertisements might not be received
    // by all nodes), they should only be viewed as a performance
    // optimization to quickly update the caches in most neighbors.  The
    // Neighbor Unreachability Detection algorithm ensures that all nodes
    // obtain a reachable link-layer address, though the delay may be
    // slightly longer.
#ifdef WITH_xMIPv6
    sendPacketToIPv6Module(na, IPv6Address::ALL_NODES_2, myIPv6Addr, ie->getInterfaceId());
#endif /* WITH_xMIPv6 */
}

void IPv6NeighbourDiscovery::processNAPacket(IPv6NeighbourAdvertisement *na,
        IPv6ControlInfo *naCtrlInfo)
{
    if (validateNAPacket(na, naCtrlInfo) == false) {
        delete naCtrlInfo;
        delete na;
        return;
    }

    //Neighbour Advertisement Information
    IPv6Address naTargetAddr = na->getTargetAddress();

    //First, we check if the target address in NA is found in the interface it
    //was received on is tentative.
    InterfaceEntry *ie = ift->getInterfaceById(naCtrlInfo->getInterfaceId());
    if (ie->ipv6Data()->isTentativeAddress(naTargetAddr)) {
        throw cRuntimeError("Duplicate Address Detected! Manual attention needed!");
    }
    //Logic as defined in Section 7.2.5
    Neighbour *neighbourEntry = neighbourCache.lookup(naTargetAddr, ie->getInterfaceId());

    if (neighbourEntry == nullptr) {
        EV_INFO << "NA received. Target Address not found in Neighbour Cache\n";
        EV_INFO << "Dropping NA packet.\n";
        delete naCtrlInfo;
        delete na;
        return;
    }

    //Target Address has entry in Neighbour Cache
    EV_INFO << "NA received. Target Address found in Neighbour Cache\n";

    if (neighbourEntry->reachabilityState == IPv6NeighbourCache::INCOMPLETE)
        processNAForIncompleteNCEState(na, neighbourEntry);
    else
        processNAForOtherNCEStates(na, neighbourEntry);

    delete naCtrlInfo;
    delete na;
}

bool IPv6NeighbourDiscovery::validateNAPacket(IPv6NeighbourAdvertisement *na,
        IPv6ControlInfo *naCtrlInfo)
{
    bool result = true;    //adopt optimistic approach

    //RFC 2461:7.1.2 Validation of Neighbor Advertisments(some checks are omitted)
    //A node MUST silently discard any received Neighbor Advertisment messages
    //that do not satisfy all of the following validity checks:

    //- The IP Hop Limit field has a value of 255, i.e., the packet
    //  could not possibly have been forwarded by a router.
    if (naCtrlInfo->getHopLimit() != 255) {
        EV_WARN << "Hop Limit is not 255! NA validation failed!\n";
        result = false;
    }

    //- Target Address is not a multicast address.
    if (na->getTargetAddress().isMulticast() == true) {
        EV_WARN << "Target Address is a multicast address! NA validation failed!\n";
        result = false;
    }

    //- If the IP Destination Address is a multicast address the Solicited flag
    //  is zero.
    if (naCtrlInfo->getDestAddr().isMulticast()) {
        if (na->getSolicitedFlag() == true) {
            EV_WARN << "Dest Address is multicast address but solicted flag is 0!\n";
            result = false;
        }
    }

    if (result == true)
        bubble("NA validation passed.");
    else
        bubble("NA validation failed.");

    return result;
}

void IPv6NeighbourDiscovery::processNAForIncompleteNCEState(IPv6NeighbourAdvertisement *na, Neighbour *nce)
{
    MACAddress naMacAddr = na->getTargetLinkLayerAddress();
    bool naRouterFlag = na->getRouterFlag();
    bool naSolicitedFlag = na->getSolicitedFlag();
    const Key *nceKey = nce->nceKey;
    InterfaceEntry *ie = ift->getInterfaceById(nceKey->interfaceID);

    /*If the target's neighbour Cache entry is in the INCOMPLETE state when the
       advertisement is received, one of two things happens.*/
    if (naMacAddr.isUnspecified()) {
        /*If the link layer has addresses and no Target Link-Layer address option
           is included, the receiving node SHOULD silently discard the received
           advertisement.*/
        EV_INFO << "No MAC Address specified in NA. Ignoring NA\n";
        return;
    }
    else {
        //Otherwise, the receiving node performs the following steps:
        //- It records the link-layer address in the neighbour Cache entry.
        EV_INFO << "ND is updating Neighbour Cache Entry.\n";
        nce->macAddress = naMacAddr;

        //- If the advertisement's Solicited flag is set, the state of the
        //  entry is set to REACHABLE, otherwise it is set to STALE.
        if (naSolicitedFlag == true) {
            nce->reachabilityState = IPv6NeighbourCache::REACHABLE;
            EV_INFO << "Reachability confirmed through successful Addr Resolution.\n";
            nce->reachabilityExpires = simTime() + ie->ipv6Data()->_getReachableTime();
        }
        else
            nce->reachabilityState = IPv6NeighbourCache::STALE;

        //- It sets the IsRouter flag in the cache entry based on the Router
        //  flag in the received advertisement.
        nce->isRouter = naRouterFlag;
        if (nce->isDefaultRouter() && !nce->isRouter)
            neighbourCache.getDefaultRouterList().remove(*nce);

        //- It sends any packets queued for the neighbour awaiting address
        //  resolution.
        sendQueuedPacketsToIPv6Module(nce);
        cancelAndDelete(nce->arTimer);
        nce->arTimer = nullptr;
    }
}

void IPv6NeighbourDiscovery::processNAForOtherNCEStates(IPv6NeighbourAdvertisement *na, Neighbour *nce)
{
    bool naRouterFlag = na->getRouterFlag();
    bool naSolicitedFlag = na->getSolicitedFlag();
    bool naOverrideFlag = na->getOverrideFlag();
    MACAddress naMacAddr = na->getTargetLinkLayerAddress();
    const Key *nceKey = nce->nceKey;
    InterfaceEntry *ie = ift->getInterfaceById(nceKey->interfaceID);

    /*draft-ietf-ipv6-2461bis-04
       Section 7.2.5: Receipt of Neighbour Advertisements
       If the target's Neighbor Cache entry is in any state other than INCOMPLETE
       when the advertisement is received, the following actions take place:*/

    if (naOverrideFlag == false && !(naMacAddr.equals(nce->macAddress))
        && !(naMacAddr.isUnspecified()))
    {
        EV_INFO << "NA override is FALSE and NA MAC addr is different.\n";

        //I. If the Override flag is clear and the supplied link-layer address
        //   differs from that in the cache, then one of two actions takes place:
        //(Note: An unspecified MAC should not be compared with the NCE's mac!)
        //a. If the state of the entry is REACHABLE,
        if (nce->reachabilityState == IPv6NeighbourCache::REACHABLE) {
            EV_INFO << "NA mac is different. Change NCE state from REACHABLE to STALE\n";
            //set it to STALE, but do not update the entry in any other way.
            nce->reachabilityState = IPv6NeighbourCache::STALE;
        }
        else
            //b. Otherwise, the received advertisement should be ignored and
            //MUST NOT update the cache.
            EV_INFO << "NCE is not in REACHABLE state. Ignore NA.\n";
    }
    else if (naOverrideFlag == true || naMacAddr.equals(nce->macAddress)
             || naMacAddr.isUnspecified())
    {
        EV_INFO << "NA override flag is TRUE. or Advertised MAC is same as NCE's. or"
                << " NA MAC is not specified.\n";
        /*II. If the Override flag is set, or the supplied link-layer address
           is the same as that in the cache, or no Target Link-layer address
           option was supplied, the received advertisement MUST update the
           Neighbor Cache entry as follows:*/

        /*- The link-layer address in the Target Link-Layer Address option
            MUST be inserted in the cache (if one is supplied and is
            Different than the already recorded address).*/
        if (!(naMacAddr.isUnspecified()) && !(naMacAddr.equals(nce->macAddress))) {
            EV_INFO << "Updating NCE's MAC addr with NA's.\n";
            nce->macAddress = naMacAddr;
        }

        //- If the Solicited flag is set,
        if (naSolicitedFlag == true) {
            EV_INFO << "Solicited Flag is TRUE. Set NCE state to REACHABLE.\n";
            //the state of the entry MUST be set to REACHABLE.
            nce->reachabilityState = IPv6NeighbourCache::REACHABLE;
            //We have to cancel the NUD self timer message if there is one.

            cMessage *msg = nce->nudTimeoutEvent;
            if (msg != nullptr) {
                EV_INFO << "NUD in progress. Cancelling NUD Timer\n";
                bubble("Reachability Confirmed via NUD.");
                nce->reachabilityExpires = simTime() + ie->ipv6Data()->_getReachableTime();
                cancelAndDelete(msg);
                nce->nudTimeoutEvent = nullptr;
            }
        }
        else {
            //If the Solicited flag is zero
            EV_INFO << "Solicited Flag is FALSE.\n";
            //and the link layer address was updated with a different address

            if (!(naMacAddr.equals(nce->macAddress))) {
                EV_INFO << "NA's MAC is different from NCE's.Set NCE state to STALE\n";
                //the state MUST be set to STALE.
                nce->reachabilityState = IPv6NeighbourCache::STALE;
            }
            else
                //Otherwise, the entry's state remains unchanged.
                EV_INFO << "NA's MAC is the same as NCE's. State remains unchanged.\n";
        }
        //(Next paragraph with explanation is omitted.-WEI)

        /*- The IsRouter flag in the cache entry MUST be set based on the
           Router flag in the received advertisement.*/
        EV_INFO << "Updating NCE's router flag to " << naRouterFlag << endl;
        nce->isRouter = naRouterFlag;

        /*In those cases where the IsRouter flag changes from TRUE to FALSE as a
           result of this update, the node MUST remove that router from the Default
           Router List and update the Destination Cache entries for all destinations
           using that neighbor as a router as specified in Section 7.3.3. This is
           needed to detect when a node that is used as a router stops forwarding
           packets due to being configured as a host.*/
        if (nce->isDefaultRouter() && !nce->isRouter)
            neighbourCache.getDefaultRouterList().remove(*nce);

        //TODO: remove destination cache entries
    }
}

IPv6Redirect *IPv6NeighbourDiscovery::createAndSendRedirectPacket(InterfaceEntry *ie)
{
    //Construct a Redirect message
    IPv6Redirect *redirect = new IPv6Redirect("redirectMsg");
    redirect->setType(ICMPv6_REDIRECT);
    redirect->setByteLength(ICMPv6_HEADER_BYTES + 2 * IPv6_ADDRESS_SIZE);   // RFC 2461, Section 4.5

//FIXME incomplete code
#if 0
    //Redirect Message Specific Information
    redirect->setTargetAddress();
    redirect->setDestinationAddress();

    //Possible Option
    redirect->setTargetLinkLayerAddress();
    redirect->addByteLength(IPv6ND_LINK_LAYER_ADDRESS_OPTION_LENGTH);
#endif

    return redirect;
}

void IPv6NeighbourDiscovery::processRedirectPacket(IPv6Redirect *redirect,
        IPv6ControlInfo *ctrlInfo)
{
//FIXME incomplete code
#if 0
    //First we need to extract information from the redirect message
    IPv6Address targetAddr = redirect->getTargetAddress();    //Addressed to me
    IPv6Address destAddr = redirect->getDestinationAddress();    //new dest addr

    //Optional
    MACAddress macAddr = redirect->getTargetLinkLayerAddress();
#endif
}

#ifdef WITH_xMIPv6
//The overlaoded function has been added by zarrar yousaf on 20.07.07
void IPv6NeighbourDiscovery::processRAPrefixInfoForAddrAutoConf(IPv6NDPrefixInformation& prefixInfo, InterfaceEntry *ie, bool hFlag)
{
    EV_INFO << "Processing Prefix Info for address auto-configuration.\n";
    IPv6Address prefix = prefixInfo.getPrefix();
    uint prefixLength = prefixInfo.getPrefixLength();
    simtime_t preferredLifetime = prefixInfo.getPreferredLifetime();
    simtime_t validLifetime = prefixInfo.getValidLifetime();

    //EV << "/// prefix: " << prefix << std::endl; // CB

    //RFC 2461: Section 5.5.3
    //First condition tested, the autonomous flag is already set

    //b) If the prefix is the link-local prefix, silently ignore the Prefix
    //Information option.
    if (prefixInfo.getPrefix().isLinkLocal() == true) {
        EV_INFO << "Prefix is link-local, ignore Prefix Information Option\n";
        return;
    }

    //c) If the preferred lifetime is greater than the valid lifetime, silently
    //ignore the Prefix Information option. A node MAY wish to log a system
    //management error in this case.
    if (preferredLifetime > validLifetime) {
        EV_INFO << "Preferred lifetime is greater than valid lifetime, ignore Prefix Information\n";
        return;
    }

    // changed structure of code below, 12.9.07 - CB
    bool isPrefixAssignedToInterface = false;
    bool returnedHome = false;    // 4.9.07 - CB

    for (int i = 0; i < ie->ipv6Data()->getNumAddresses(); i++) {
        if (ie->ipv6Data()->getAddress(i).getScope() == IPv6Address::LINK)
            // skip the link local address - it's not relevant for movement detection
            continue;

        /*RFC 3775, 11.5.4
           A mobile node detects that it has returned to its home link through
           the movement detection algorithm in use (Section 11.5.1), when the
           mobile node detects that its home subnet prefix is again on-link.
         */
        if (ie->ipv6Data()->getAddress(i).matches(prefix, prefixLength) == true) {
            // A MN can have the following address combinations:
            // * only a link-local address -> at home
            // * link-local plus a HoA -> at home
            // * link-local, HoA plus CoA -> at foreign network
            // The prefix of the home network can only match the HoA
            // address, and if it does (=we received a RA from the HA),
            // and we have a CoA as well (three addresses) then we have
            // returned home
            // TODO the MN can have several global scope addresses configured from
            // different prefixes advertised via a RA -> not supported with this code
            if (rt6->isMobileNode() && ie->ipv6Data()->getAddressType(i) == IPv6InterfaceData::HoA && ie->ipv6Data()->getNumAddresses() > 2)
                returnedHome = true;
            else {
                isPrefixAssignedToInterface = true;
                EV_INFO << "The received Prefix is already assigned to the interface" << endl;    //Zarrar Yousaf 19.07.07
                break;
            }
        }
    }
    /*d) If the prefix advertised does not match the prefix of an address already
         in the list, and the Valid Lifetime is not 0, form an address (and add
         it to the list) by combining the advertised prefix with the link's
         interface identifier as follows:
     */
    if ((isPrefixAssignedToInterface == false) && (validLifetime != 0)) {
        EV_INFO << "Prefix not assigned to interface. Possible new router detected. Auto-configuring new address.\n";
        IPv6Address linkLocalAddress = ie->ipv6Data()->getLinkLocalAddress();
        ASSERT(linkLocalAddress.isUnspecified() == false);
        IPv6Address newAddr = linkLocalAddress.setPrefix(prefix, prefixLength);
        IPv6Address CoA;
        //TODO: for now we leave the newly formed address as not tentative,
        //according to Greg, we have to always perform DAD for a newly formed address.
        EV_INFO << "Assigning new address to: " << ie->getName() << endl;

        // we are for sure either in the home network or in a new foreign network
        // -> remove CoA
        //CoA = ie->ipv6()->removeCoAAddr();
        // moved code from above to processDADTimeout()

        // 27.9.07 - CB
        if (returnedHome) {
            // we have to remove the CoA before we create a new one
            EV_INFO << "Node returning home - removing CoA...\n";
            CoA = ie->ipv6Data()->removeAddress(IPv6InterfaceData::CoA);

            // nothing to do more wrt managing addresses, as we are at home and a HoA is
            // already existing at the interface

            // initiate the returning home procedure
            ASSERT(!CoA.isUnspecified());
            mipv6->returningHome(CoA, ie);
        }
        else {    // non-mobile nodes will never have returnedHome == true, so they will always assign a new address
            CoA = ie->ipv6Data()->getGlobalAddress(IPv6InterfaceData::CoA);

            // form new address and initiate DAD, as we are in a foreign network

            if (ie->ipv6Data()->getNumAddresses() == 1) {
                // we only have a link-layer and no unicast address of scope > link-local
                // this means DAD is already running or has already been completed
                // create a unicast address with scope > link-local
                bool isLinkLocalTentative = ie->ipv6Data()->isTentativeAddress(linkLocalAddress);
                // if the link local address is tentative, then we make the global unicast address tentative as well
                ie->ipv6Data()->assignAddress(newAddr, isLinkLocalTentative,
                        simTime() + validLifetime, simTime() + preferredLifetime, hFlag);
            }
            else {
                // set tentative flag for all addresses on this interface
                for (int j = 0; j < ie->ipv6Data()->getNumAddresses(); j++) {
                    // TODO improve this code so that only addresses are set to tentative which are
                    // formed based on the link-local address from above
                    ie->ipv6Data()->tentativelyAssign(j);
                    EV_INFO << "Setting address " << ie->ipv6Data()->getAddress(j) << " to tentative.\n";
                }

                initiateDAD(ie->ipv6Data()->getLinkLocalAddress(), ie);

                // set MIPv6Init structure that will later on be used for initiating MIPv6 protocol after DAD was performed
                dadGlobalList[ie].hFlag = hFlag;
                dadGlobalList[ie].validLifetime = validLifetime;
                dadGlobalList[ie].preferredLifetime = preferredLifetime;
                dadGlobalList[ie].addr = newAddr;
                //dadGlobalList[ie].returnedHome = returnedHome;
                dadGlobalList[ie].CoA = CoA;
            }
        }
    }
}

void IPv6NeighbourDiscovery::routersUnreachabilityDetection(const InterfaceEntry *ie)
{
    // remove all entries from the destination cache for this interface
    //rt6->purgeDestCacheForInterfaceID( ie->interfaceId() );
    // invalidate entries in the destination cache for this interface
    //neighbourCache.invalidateEntriesForInterfaceID( ie->interfaceId() );
    // remove default routes on this interface
    rt6->deleteDefaultRoutes(ie->getInterfaceId());
    rt6->deletePrefixes(ie->getInterfaceId());

    for (auto it = neighbourCache.begin(); it != neighbourCache.end(); ) {
        if ((*it).first.interfaceID == ie->getInterfaceId() && it->second.isDefaultRouter()) {
            // update 14.9.07 - CB
            IPv6Address rtrLnkAddress = (*it).first.address;
            EV_INFO << "Setting router (address=" << rtrLnkAddress << ", ifID="
                    << (*it).first.interfaceID << ") to unreachable" << endl;
            ++it;

            //if ( rtrLnkAddress.isLinkLocal() )
            timeoutDefaultRouter(rtrLnkAddress, ie->getInterfaceId());

            // reset reachability state of this router
            //Neighbour* nbor = neighbourCache.lookup( (*it).first.address, (*it).first.interfaceID );
            //nbor->reachabilityState = IPv6NeighbourCache::STALE;
            //initiateNeighbourUnreachabilityDetection(nbor);
        }
        else
            ++it;
    }
}

void IPv6NeighbourDiscovery::invalidateNeigbourCache()
{
    //Enter_Method("Invalidating Neigbour Cache Entries");
    neighbourCache.invalidateAllEntries();
}

bool IPv6NeighbourDiscovery::canServeWirelessNodes(InterfaceEntry *ie)
{
    if (isWirelessInterface(ie))
        return true;

    // check if this interface is directly connected to an AccessPoint.
    cModule *node = getContainingNode(this);
    cGate *gate = node->gate(ie->getNodeOutputGateId());
    ASSERT(gate != nullptr);
    cGate *connectedGate = gate->getPathEndGate();
    if (connectedGate != gate) {
        cModule *connectedNode = getContainingNode(connectedGate->getOwnerModule());
        if(!connectedNode)
            throw cRuntimeError("The connected module %s is not in a network node.", connectedGate->getOwnerModule()->getFullPath().c_str());
        if (isWirelessAccessPoint(connectedNode))
            return true;
    }

    // FIXME The AccessPoint can be connected to this router via Ethernet switches and/or hubs.

    return false;
}

bool IPv6NeighbourDiscovery::isWirelessInterface(const InterfaceEntry *ie)
{
    // TODO should be a flag in the InterfaceEntry
    return strncmp("wlan", ie->getName(), 4) == 0;
}

bool IPv6NeighbourDiscovery::isWirelessAccessPoint(cModule *module)
{
    // AccessPoint is defined as a node containing "relayUnit" and
    // "wlan" submodules
    return isNetworkNode(module) && module->getSubmodule("relayUnit") &&
           (module->getSubmodule("wlan", 0) || module->getSubmodule("wlan"));
}

#endif /* WITH_xMIPv6 */

} // namespace inet

