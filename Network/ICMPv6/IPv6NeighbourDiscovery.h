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

#ifndef IPV6NEIGHBOURDISCOVERY_H
#define IPV6NEIGHBOURDISCOVERY_H

#include <stdio.h>
#include <string.h>
#include <vector>
#include <set>
#include <omnetpp.h>
#include "IPv6Address.h"
#include "IPv6Datagram.h"
#include "IPv6NDMessage_m.h"
#include "IPv6ControlInfo_m.h"
#include "IPv6InterfaceData.h"
#include "InterfaceEntry.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "RoutingTable6.h"
#include "RoutingTable6Access.h"
#include "IPv6NeighbourCache.h"

/**
 * Constants
 */
#define MAX_MULTICAST_SOLICIT//TODO: Add values
#define MAX_RTR_SOLICITATIONS

/**
 * Implements RFC 2461 Neighbor Discovery for IPv6.
 */
class INET_API IPv6NeighbourDiscovery : public cSimpleModule
{
    public:
        typedef IPv6NeighbourCache::Neighbour Neighbour;  // for convenience

        Module_Class_Members(IPv6NeighbourDiscovery, cSimpleModule, 0);

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
        const MACAddress& resolveNeighbour(const IPv6Address& nextHop, int interfaceId);

        /**
         * Public method, it can be invoked from the IPv6 module or any other
         * module to let Neighbour Discovery know that the reachability
         * of the given neighbor has just been confirmed (e.g. TCP received
         * ACK of new data from it). Neighbour Discovery can then update
         * the neighbour cache with this information, and cancel the
         * Neighbour Unreachability Detection procedure if it is currently
         * running.
         */
        void reachabilityConfirmed(const IPv6Address& neighbour, int interfaceId);
        
    protected:

        int NUM_OF_SOLICIT_SENT;
        simtime_t MAX_ANYCAST_DELAY_TIME;

        //Packets awaiting Address Resolution or Next-Hop Determination.
        cQueue pendingQueue;

        InterfaceTable *ift;
        RoutingTable6 *rt6;
        IPv6NeighbourCache neighbourCache;
        typedef std::set<cMessage*> RATimerList;

        // stores information about a pending Duplicate Address Detection for
        // an interface
        struct DADEntry {
            int interfaceId;// interface on which DAD is performed
            IPv6Address address;// link-local address subject to DAD
            int numNSSent;// number of DAD solicitations sent since start of sim
            cMessage *timeoutMsg;// the message to cancel when NA is received
        };
        typedef std::set<DADEntry*> DADList;

        //stores information about Router Discovery for an interface
        struct RDEntry {
            int interfaceId; //interface on which Router Discovery is performed
            int numRSSent; //number of Router Solicitations sent since start of sim
            cMessage *timeoutMsg; //the message to cancel when RA is received
        };
        typedef std::set<RDEntry*> RDList;

        //An entry that stores information for an Advertising Interface
        struct AdvIfEntry {
            int interfaceId;
            int numRASent;//number of Router Advertisements sent since start of sim
            simtime_t nextScheduledRATime;//stores time when next RA will be sent.
            cMessage *raTimeoutMsg;//the message to cancel when resetting RA timer
        };
        typedef std::set<AdvIfEntry*> AdvIfList;

        //List of periodic RA msgs(used only for router interfaces)
        RATimerList raTimerList;
        //List of pending Duplicate Address Detections
        DADList dadList;
        //List of pending Router & Prefix Discoveries
        RDList rdList;
        //List of Advertising Interfaces
        AdvIfList advIfList;

    protected:
        /************************Miscellaneous Stuff***************************/
        virtual int numInitStages() const {return 4;}
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
        void processNDMessage(ICMPv6Message *msg, IPv6ControlInfo *ctrlInfo);
        virtual void finish();

        void processIPv6Datagram(IPv6Datagram *datagram);
        IPv6NeighbourDiscovery::AdvIfEntry *fetchAdvIfEntry(InterfaceEntry *ie);
        IPv6NeighbourDiscovery::RDEntry *fetchRDEntry(InterfaceEntry *ie);
        /************************End of Miscellaneous Stuff********************/

        /**
         *  This function accepts the datagram's destination address and attempts
         *  to determine the destination's next hop address and interface ID by:
         *  (1) looking up the destination cache, (2)looking up the routing table,
         *  or (3) selecting a default router. It then updates the destination
         *  cache. If no default router can be selected than we assume the
         *  destination address to be onlink and simply return any available
         *  interface.
         */
        const IPv6Address& determineNextHop(const IPv6Address& destAddr, int& outIfID);
        void initiateNeighbourUnreachabilityDetection(Neighbour *neighbour);
        const IPv6Address& selectDefaultRouter(int& outIfID);
        void initiateAddressResolution(const IPv6Address& dgSrcAddr,
            const IPv6Address& neighbourAddr, int ifID);
        void sendPacketToIPv6Module(cMessage *msg, const IPv6Address& destAddr,
            const IPv6Address& srcAddr, int inputGateIndex);
        /**
         *  Convenience class for sending Neighbour Discovery messages that
         *  requires some form of delay. This could potentially save the usage
         *  of too many self messages and its associated complexity.
         */
        void sendDelayedPacketToIPv6Module(cMessage *msg, const IPv6Address& destAddr,
            const IPv6Address& srcAddr, int inputGateIndex, simtime_t delay);
        /**
         *  Send off any queued packets within the Neighbour Discovery module
         *  awaiting address resolution.
         *  TODO: Assuming queue has packets awaiting for address resolution on
         *  different IP addresses, shouldn't this method have a smart mechanism
         *  to send off the correct packet?
         */
        void sendQueuedPacketToIPv6Module();

        /**
         *  Assign a tentative address to the given Interface entry. This is a
         *  pre-requisite step for performing Duplicate Address Detection.
         */
        void resolveTentativeAddress(cMessage *timerMsg);

        /**
         *  Initiating DAD means to send off a Neighbour Solicitation with its
         *  target address set as this node's tentative link-local address.
         */
        void initiateDAD(const IPv6Address& tentativeAddr, InterfaceEntry *ie);
        
        /**
         *  Sends a scheduled DAD NS packet. If number of sends is equals or more
         *  than dupAddrDetectTransmits, then permantly assign target link local
         *  address as permanent address for given interface entry.
         */
        void processDADTimeout(cMessage *msg);

        /************Router Solicitation Stuff*********************************/
        IPv6RouterSolicitation *createAndSendRSPacket(InterfaceEntry *ie);
        void initiateRouterDiscovery(cMessage *msg);
        /**
         *  Once the host sends a Router Solicitation, and receives a valid
         *  Router Advertisement with a non-zero Router Lifetime, the host MUST
         *  desist from sending additional solicitations on that interface,
         *
         *  Cancel Router Discovery on the Interface where a RA was received with
         *  the given Interface Entry.
         */
        void cancelRouterDiscovery(InterfaceEntry *ie);
        void processRDTimeout(cMessage *msg);
        void processRSPacket(IPv6RouterSolicitation *rs, IPv6ControlInfo *rsCtrlInfo);
        bool validateRSPacket(IPv6RouterSolicitation *rs, IPv6ControlInfo *rsCtrlInfo);
        /************End of Router Solicitation Stuff**************************/

        /************Router Advertisment Stuff*********************************/
        IPv6RouterAdvertisement *createAndSendRAPacket(const IPv6Address& destAddr,
            InterfaceEntry *ie);
        void processRAPacket(IPv6RouterAdvertisement *ra, IPv6ControlInfo *raCtrlInfo);
        void updateRouterList(IPv6RouterAdvertisement *ra, IPv6ControlInfo *raCtrlInfo);
        void processRAPrefixInfoForAutoConf(IPv6NDPrefixInformation& prefixInfo,
            InterfaceEntry *ie);
        /**
         *  Create a timer for the given interface entry that sends periodic
         *  Router Advertisements
         */
        void createRATimer(InterfaceEntry *ie);
        /**
         *  Reset the given interface entry's Router Advertisement timer. This is
         *  usually done when a router interface responds (by replying with a Router
         *  Advertisement sent to the All-Node multicast group)to a router solicitation
         *  Also see: RFC 2461, Section 6.2.6
         */
        void resetRATimer(InterfaceEntry *ie);
        void sendPeriodicRA(cMessage *msg);
        void sendSolicitedRA(cMessage *msg);
        bool validateRAPacket(IPv6RouterAdvertisement *ra, IPv6ControlInfo *raCtrlInfo);
        /************End of Router Advertisement Stuff*************************/

        /************Neighbour Solicitaton Stuff*******************************/
        IPv6NeighbourSolicitation *createAndSendNSPacket(
            const IPv6Address& nsTargetAddr, const IPv6Address& dgDestAddr,
            const IPv6Address& dgSrcAddr, InterfaceEntry *ie);
        void processNSPacket(IPv6NeighbourSolicitation *ns, IPv6ControlInfo *naCtrlInfo);
        bool validateNSPacket(IPv6NeighbourSolicitation *ns, IPv6ControlInfo *nsCtrlInfo);
        void processNSForTentativeAddress(IPv6NeighbourSolicitation *ns,
            IPv6ControlInfo *ctrlInfo);
        void processNSForNonTentativeAddress(IPv6NeighbourSolicitation *ns,
            IPv6ControlInfo *ctrlInfo, InterfaceEntry *ie);
        void processNSWithSpecifiedSrcAddr(IPv6NeighbourSolicitation *ns,
            IPv6ControlInfo *ctrlInfo, InterfaceEntry *ie);
        /************End Of Neighbour Solicitation Stuff***********************/

        /************Neighbour Advertisment Stuff)*****************************/
        IPv6NeighbourAdvertisement *createAndSendNAPacket(IPv6NeighbourSolicitation *ns,
            const IPv6Address& nsSrcAddr, const IPv6Address& nsDestAddr, InterfaceEntry *ie);
        void sendSolicitedNA(IPv6NeighbourSolicitation *ns,
            IPv6ControlInfo *nsCtrlInfo, InterfaceEntry *ie);
        void sendUnsolicitedNA(InterfaceEntry *ie);
        void processNAPacket(IPv6NeighbourAdvertisement *na, IPv6ControlInfo *naCtrlInfo);
        bool validateNAPacket(IPv6NeighbourAdvertisement *na, IPv6ControlInfo *naCtrlInfo);
        void processNAForIncompleteNeighbourEntry(IPv6NeighbourAdvertisement *na,
            IPv6NeighbourCache::Neighbour *neighbourEntry);
        void processNAForExistingNeighborEntry(IPv6NeighbourAdvertisement *na,
            IPv6NeighbourCache::Neighbour *neighbourEntry);
        /************End Of Neighbour Advertisement Stuff**********************/

        /************Redirect Message Stuff************************************/
        IPv6Redirect *createAndSendRedirectPacket(InterfaceEntry *ie);
        void processRedirectPacket(IPv6Redirect *redirect, IPv6ControlInfo *ctrlInfo);
        /************End Of Redirect Message Stuff*****************************/

        /**
         *  RFC2463 Section 3.1: Destination Unreachable Message
         *  Send an unreachable message to the IPv6 module.
         */
        ICMPv6DestUnreachableMsg *createUnreachableMessage(IPv6Address destAddress);
};
#endif //IPV6NEIGHBOURDISCOVERY_H
