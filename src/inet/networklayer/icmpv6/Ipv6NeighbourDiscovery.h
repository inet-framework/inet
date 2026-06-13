//
// Copyright (C) 2005 OpenSim Ltd.
// Copyright (C) 2005 Wei Yang, Ng
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV6NEIGHBOURDISCOVERY_H
#define __INET_IPV6NEIGHBOURDISCOVERY_H

#include <map>
#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"
#include "inet/networklayer/icmpv6/Ipv6NdMessage_m.h"
#include "inet/networklayer/icmpv6/Ipv6NeighbourCache.h"
#include "inet/common/checksum/ChecksumMode_m.h"

namespace inet {

// Forward declarations:
class Icmpv6;
class IInterfaceTable;
class NetworkInterface;
class Ipv6Header;
class Ipv6RoutingTable;

class Mipv6;

/**
 * Implements RFC 2461 Neighbor Discovery for Ipv6.
 */
class INET_API Ipv6NeighbourDiscovery : public OperationalBase, protected cListener
{
  public:
    typedef std::vector<Packet *> MsgPtrVector;
    typedef Ipv6NeighbourCache::Key Key; // for convenience
    typedef Ipv6NeighbourCache::Neighbour Neighbour; // for convenience
    typedef Ipv6NeighbourCache::DefaultRouterList DefaultRouterList; // for convenience

  public:
    Ipv6NeighbourDiscovery();
    virtual ~Ipv6NeighbourDiscovery();

  public:
    static simsignal_t startDadSignal;
    static simsignal_t dadCompletedSignal;
    static simsignal_t dadFailedSignal;

  public:
    /**
     * Public method, to be invoked from the Ipv6 module to determine
     * link-layer address and the output interface of the next hop.
     *
     * If the neighbor cache does not contain this address or it's in the
     * state INCOMPLETE, this method will return the nullptr address, and the
     * Ipv6 module should then send the datagram here to Ipv6NeighbourDiscovery
     * where it will be stored until neighbour resolution completes.
     *
     * If the neighbour cache entry is STALE (or REACHABLE but more than
     * reachableTime elapsed since reachability was last confirmed),
     * the link-layer address is still returned and Ipv6 can send the
     * datagram, but simultaneously, this call should trigger the Neighbour
     * Unreachability Detection procedure to start in the
     * Ipv6NeighbourDiscovery module.
     */
    const MacAddress& resolveNeighbour(const Ipv6Address& nextHop, int interfaceId);

    /**
     * Public method, it can be invoked from the Ipv6 module or any other
     * module to let Neighbour Discovery know that the reachability
     * of the given neighbor has just been confirmed (e.g. TCP received
     * ACK of new data from it). Neighbour Discovery can then update
     * the neighbour cache with this information, and cancel the
     * Neighbour Unreachability Detection procedure if it is currently
     * running.
     */
    virtual void reachabilityConfirmed(const Ipv6Address& neighbour, int interfaceId);

  protected:

    // Packets awaiting Address Resolution or Next-Hop Determination.
    cQueue pendingQueue;
    long numSent = 0;
    long numReceived = 0;

    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<Ipv6RoutingTable> rt6;
    ModuleRefByPar<Icmpv6> icmpv6;
    ChecksumMode checksumMode = CHECKSUM_MODE_UNDEFINED;

    ModuleRefByPar<Mipv6> mipv6; // in case the node has MIP support

    Ipv6NeighbourCache neighbourCache;
    typedef std::vector<cMessage *> RaTimerList;

    // stores information about a pending Duplicate Address Detection for
    // an interface
    struct DadEntry {
        int interfaceId; // interface on which DAD is performed
        Ipv6Address address; // link-local address subject to DAD
        int numNSSent; // number of DAD solicitations sent since start of sim
        cMessage *timeoutMsg; // the message to cancel when NA is received
    };
    typedef std::vector<DadEntry *> DadList;

    // stores information about Router Discovery for an interface
    struct RdEntry {
        int interfaceId; // interface on which Router Discovery is performed
        unsigned int numRSSent; // number of Router Solicitations sent since start of sim
        cMessage *timeoutMsg; // the message to cancel when RA is received
    };
    typedef std::vector<RdEntry *> RdList;

    // An entry that stores information for an Advertising Interface
    struct AdvIfEntry {
        int interfaceId;
        unsigned int numRASent; // number of Router Advertisements sent since start of sim
        simtime_t nextScheduledRATime; // stores time when next RA will be sent.
        cMessage *raTimeoutMsg; // the message to cancel when resetting RA timer
    };
    typedef std::vector<AdvIfEntry *> AdvIfList;

    // Timer for link-local address assignment at boot
    cMessage *assignLinkLocalAddrTimer = nullptr;

    // List of periodic RA msgs(used only for router interfaces)
    RaTimerList raTimerList;

    // List of pending Duplicate Address Detections
    DadList dadList;

    // List of pending Router & Prefix Discoveries
    RdList rdList;

    // List of Advertising Interfaces
    AdvIfList advIfList;

    // An entry that stores information for configuring the global unicast
    // address, after DAD was succesfully performed
    struct DadGlobalEntry {
        bool hFlag; // home network flag from RA
        simtime_t validLifetime; // valid lifetime of the received prefix
        simtime_t preferredLifetime; // preferred lifetime of the received prefix
        Ipv6Address addr; // the address with scope > link local that the interface will get

        // bool returnedHome; // MIPv6-related: whether we returned home after a visit in a foreign network
        Ipv6Address CoA; // MIPv6-related: the old CoA, in case we returned home
    };
    typedef std::map<int, DadGlobalEntry> DadGlobalList; // keyed by interfaceId
    DadGlobalList dadGlobalList;

    // If true, (re)start Router Discovery whenever an interface associates at the
    // link layer (l2Associated signal, e.g. a wireless handover to a new AP), so a
    // mobile node solicits a Router Advertisement and detects movement immediately
    // instead of waiting for the next unsolicited RA. Set from the NED parameter.
    bool detectL2Movement = false;

  protected:
    /************************Miscellaneous Stuff***************************/
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    // Reacts to the l2Associated signal when detectL2Movement is set.
    using cListener::receiveSignal;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    // lifecycle:
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_NETWORK_LAYER_PROTOCOLS; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void start();
    virtual void stop();
    virtual void processNDMessage(Packet *packet, const Icmpv6Header *msg);
    virtual void finish() override;

    virtual void processIpv6Datagram(Packet *packet);
    virtual Ipv6NeighbourDiscovery::AdvIfEntry *fetchAdvIfEntry(NetworkInterface *ie);
    virtual Ipv6NeighbourDiscovery::RdEntry *fetchRdEntry(NetworkInterface *ie);
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
    virtual Ipv6Address determineNextHop(const Ipv6Address& destAddr, int& outIfID);
    virtual void initiateNeighbourUnreachabilityDetection(Neighbour *neighbour);
    virtual void processNudTimeout(cMessage *timeoutMsg);
    virtual Ipv6Address selectDefaultRouter(int& outIfID);
    /**
     *  RFC 2461: Section 6.3.5
     *  Whenever the Lifetime of an entry in the Default Router List expires,
     *  that entry is discarded. When removing a router from the Default
     *  Router list, the node MUST update the Destination Cache in such a way
     *  that all entries using the router perform next-hop determination
     *  again rather than continue sending traffic to the (deleted) router.
     */
    virtual void timeoutDefaultRouter(const Ipv6Address& addr, int interfaceID);
    /**
     *  This method attempts to resolve the given neighbour's link-layer address.
     *  The source address of the packet prompting address resolution is also
     *  given in order to decide the source address of the NS to be sent.
     *  nceKey stores 2 pieces of information (Neighbour address and Interface ID)
     *  which is needed for addr resolution and access to the corresponding
     *  nce.
     */
    virtual void initiateAddressResolution(const Ipv6Address& dgSrcAddr,
            Neighbour *nce);
    /**
     *  Resends a NS packet to the address intended for address resolution.
     *  TODO Not implemented yet!
     */
    virtual void processArTimeout(cMessage *arTimeoutMsg);
    /**
     *  Drops specific queued packets for a specific NCE AR-timeout.
     *  TODO Not implemented yet!
     */
    virtual void dropQueuedPacketsAwaitingAr(Neighbour *nce);
    /**
     *  Create control info and assigns it to a msg. Returns a copy of the
     *  msg with the control info.
     */
    virtual void sendPacketToIpv6Module(Packet *msg, const Ipv6Address& destAddr,
            const Ipv6Address& srcAddr, int interfaceId);

    /**
     *  Send off any queued packets within the Neighbour Discovery module
     *  awaiting address resolution.
     */
    virtual void sendQueuedPacketsToIpv6Module(Neighbour *nce);

    /**
     *  Initiating DAD means to send off a Neighbour Solicitation with its
     *  target address set as this node's tentative link-local address.
     */
    virtual void initiateDad(const Ipv6Address& tentativeAddr, NetworkInterface *ie);

    /**
     *  Sends a scheduled DAD NS packet. If number of sends is equals or more
     *  than dupAddrDetectTransmits, then permantly assign target link local
     *  address as permanent address for given interface entry.
     */
    virtual void processDadTimeout(cMessage *msg);

    /**
     * Permanently assign the given address for the given interface entry.
     * To be called after successful DAD.
     */
    virtual void makeTentativeAddressPermanent(const Ipv6Address& tentativeAddr, NetworkInterface *ie);

    /**
     * Called when DAD detects a duplicate address (RFC 4862, Section 5.4.5).
     * Cancels the DAD timer, removes the tentative address from the interface,
     * and emits a dadFailed signal.
     */
    virtual void dadHasFailed(const Ipv6Address& duplicateAddr, NetworkInterface *ie);

    /************Address Autoconfiguration Stuff***************************/
    /**
     *  as it is not possbile to explicitly define RFC 2462. ND is the next
     *  best place to do this.
     *
     *  RFC 2462-Ipv6 Stateless Address Autoconfiguration: Section 1
     *  The autoconfiguration process specified in this document applies only
     *  to hosts and not routers. Since host autoconfiguration uses
     *  information advertised by routers, routers will need to be configured
     *  by some other means. However, it is expected that routers will
     *  generate link-local addresses using the mechanism described in this
     *  document. In addition, routers are expected to successfully pass the
     *  Duplicate Address Detection procedure described in this document on
     *  all addresses prior to assigning them to an interface.
     */
    virtual void assignLinkLocalAddress(cMessage *timerMsg);

    /************End Of Address Autoconfiguration Stuff********************/

    /************Router Solicitation Stuff*********************************/
    virtual void createAndSendRsPacket(NetworkInterface *ie);
    virtual void initiateRouterDiscovery(cMessage *msg);
    // Start the Router Discovery procedure on the given interface (send the first
    // Router Solicitation and schedule retransmissions). Shared by the post-DAD
    // startup path and the l2Associated handler.
    virtual void startRouterDiscovery(NetworkInterface *ie);
    /**
     *  RFC 2461: Section 6.3.7 4th paragraph
     *  Once the host sends a Router Solicitation, and receives a valid
     *  Router Advertisement with a non-zero Router Lifetime, the host MUST
     *  desist from sending additional solicitations on that interface,
     *
     *  Cancel Router Discovery on the Interface where a RA was received with
     *  the given Interface Entry.
     */
    virtual void cancelRouterDiscovery(NetworkInterface *ie);
    virtual void processRdTimeout(cMessage *msg);
    virtual void processRsPacket(Packet *packet, const Ipv6RouterSolicitation *rs);
    virtual bool validateRsPacket(Packet *packet, const Ipv6RouterSolicitation *rs);
    /************End of Router Solicitation Stuff**************************/

    /************Router Advertisment Stuff*********************************/
    virtual void createAndSendRaPacket(const Ipv6Address& destAddr,
            NetworkInterface *ie);
    virtual void processRaPacket(Packet *packet, const Ipv6RouterAdvertisement *ra);
    virtual void processRaForRouterUpdates(Packet *packet, const Ipv6RouterAdvertisement *ra);
    // RFC 2461: Section 6.3.4
    /*Note: Implementations can choose to process the on-link aspects of the
       prefixes separately from the address autoconfiguration aspects of the
       prefixes by, e.g., passing a copy of each valid Router Advertisement message
       to both an "on-link" and an "addrconf" function. Each function can then
       operate independently on the prefixes that have the appropriate flag set.*/
    virtual void processRaPrefixInfo(const Ipv6RouterAdvertisement *ra, NetworkInterface *ie);

    virtual void processRaPrefixInfoForAddrAutoConf(const Ipv6NdPrefixInformation& prefixInfo,
            NetworkInterface *ie, bool hFlag = false);

    /**
     *  Create a timer for the given interface entry that sends periodic
     *  Router Advertisements
     */
    virtual void createRaTimer(NetworkInterface *ie);

    /**
     *  Reset the given interface entry's Router Advertisement timer. This is
     *  usually done when a router interface responds (by replying with a Router
     *  Advertisement sent to the All-Node multicast group)to a router solicitation
     *  Also see: RFC 2461, Section 6.2.6
     */
    virtual void resetRaTimer(NetworkInterface *ie);
    virtual void sendPeriodicRa(cMessage *msg);
    virtual void sendSolicitedRa(cMessage *msg);
    virtual bool validateRaPacket(Packet *packet, const Ipv6RouterAdvertisement *ra);
    /************End of Router Advertisement Stuff*************************/

    /************Neighbour Solicitaton Stuff*******************************/
    virtual void createAndSendNsPacket(const Ipv6Address& nsTargetAddr, const Ipv6Address& dgDestAddr,
            const Ipv6Address& dgSrcAddr, NetworkInterface *ie);
    virtual void processNsPacket(Packet *packet, const Ipv6NeighbourSolicitation *ns);
    virtual bool validateNsPacket(Packet *packet, const Ipv6NeighbourSolicitation *ns);
    virtual void processNsForTentativeAddress(Packet *packet, const Ipv6NeighbourSolicitation *ns);
    virtual void processNsForNonTentativeAddress(Packet *packet, const Ipv6NeighbourSolicitation *ns, NetworkInterface *ie);
    virtual void processNsWithSpecifiedSrcAddr(Packet *packet, const Ipv6NeighbourSolicitation *ns, NetworkInterface *ie);
    /************End Of Neighbour Solicitation Stuff***********************/

    /************Neighbour Advertisment Stuff)*****************************/

    virtual void sendSolicitedNa(Packet *packet, const Ipv6NeighbourSolicitation *ns, NetworkInterface *ie);

  public:
    virtual void sendUnsolicitedNa(NetworkInterface *ie);

  protected:

    virtual void processNaPacket(Packet *packet, const Ipv6NeighbourAdvertisement *na);
    virtual bool validateNaPacket(Packet *packet, const Ipv6NeighbourAdvertisement *na);
    virtual void processNaForIncompleteNceState(const Ipv6NeighbourAdvertisement *na, Ipv6NeighbourCache::Neighbour *nce);
    virtual void processNaForOtherNceStates(const Ipv6NeighbourAdvertisement *na, Ipv6NeighbourCache::Neighbour *nce);
    /************End Of Neighbour Advertisement Stuff**********************/

    /************Redirect Message Stuff************************************/
    virtual void createAndSendRedirectPacket(NetworkInterface *ie);
    virtual void processRedirectPacket(Packet *packet, const Ipv6Redirect *redirect);
    /************End Of Redirect Message Stuff*****************************/

  public:
    void invalidateNeigbourCache();
    virtual void sendRedirect(Packet *redirectedPacket, const Ipv6Address& targetAddr,
            const Ipv6Address& destAddr, NetworkInterface *ie);

  protected:
    void routersUnreachabilityDetection(const NetworkInterface *ie); // 3.9.07 - CB
};

} // namespace inet

#endif

