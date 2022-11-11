//
// Copyright (C) 2005 OpenSim Ltd.
// Copyright (C) 2005 Wei Yang, Ng
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV6INTERFACEDATA_H
#define __INET_IPV6INTERFACEDATA_H

#include <vector>

#include "inet/common/INETDefs.h"

#ifndef INET_WITH_IPv6
#error "IPv6 feature disabled"
#endif

#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"

namespace inet {

// Forward declarations:
#ifdef INET_WITH_xMIPv6
class Ipv6RoutingTable;
#endif /* INET_WITH_xMIPv6 */

#define IPv6_DEFAULT_DUPADDRDETECTTRANSMITS     1   // send NS once (RFC2462:Section 5.1)

#define IPv6_MIN_MTU                            1280 // octets
#define IPv6_DEFAULT_ADVCURHOPLIMIT             30

#define IPv6_DEFAULT_MAX_RTR_ADV_INT            600 // seconds-decrease to enable more periodic RAs
#define IPv6_DEFAULT_ADV_REACHABLE_TIME         3600 // seconds
#define IPv6_DEFAULT_ADV_RETRANS_TIMER          1   // seconds
#define IPv6__INET_DEFAULT_ROUTER_HOPLIMIT      64

/**************RFC 2461: Section 10 Protocol Constants*************************/
// Router Constants
#define IPv6_MAX_INITIAL_RTR_ADVERT_INTERVAL    16  // seconds
#define IPv6_MAX_INITIAL_RTR_ADVERTISEMENTS     3  // transmissions
#define IPv6_MAX_FINAL_RTR_ADVERTISEMENTS       3  // transmissions
#define IPv6_MIN_DELAY_BETWEEN_RAS              3  // seconds
#define IPv6_MAX_RA_DELAY_TIME                  .5  // seconds

// Host Constants
#define IPv6_MAX_RTR_SOLICITATION_DELAY         1  // seconds
#define IPv6_RTR_SOLICITATION_INTERVAL          4  // seconds
#define IPv6_MAX_RTR_SOLICITATIONS              3  // transmissions

// Node Constants
#define IPv6_MAX_MULTICAST_SOLICIT              3  // transmissions
#define IPv6_MAX_UNICAST_SOLICIT                3  // transmissions
#define IPv6_MAX_ANYCAST_DELAY_TIME             1  // seconds
#define IPv6_MAX_NEIGHBOUR_ADVERTISEMENT        3  // transmissions
#define IPv6_REACHABLE_TIME                     30  // seconds
#define IPv6_RETRANS_TIMER                      1  // seconds
#define IPv6_DELAY_FIRST_PROBE_TIME             5  // seconds
#define IPv6_MIN_RANDOM_FACTOR                  0.5
#define IPv6_MAX_RANDOM_FACTOR                  1.5
/***************END of RFC 2461 Protocol Constants*****************************/

#ifdef INET_WITH_xMIPv6
/***************RFC 3775: Section 12 Protocol Constants************************/
#define MIPv6_DHAAD_RETRIES                    4 // retransmissions
#define MIPv6_INITIAL_BINDACK_TIMEOUT          1 // second
#define MIPv6_INITIAL_DHAAD_TIMEOUT            3 // seconds
#define MIPv6_INITIAL_SOLICIT_TIMER            3 // seconds
#define MIPv6_MAX_BINDACK_TIMEOUT              32 // seconds
#define MIPv6_MAX_NONCE_LIFETIME               240 // seconds
#define MIPv6_MAX_TOKEN_LIFETIME               210 // seconds
#define MIPv6_MAX_UPDATE_RATE                  3 // times
#define MIPv6_PREFIX_ADV_RETRIES               3 // retransmissions
#define MIPv6_PREFIX_ADV_TIMEOUT               3 // seconds
// update 12.9.07 - CB
#define MIPv6_INITIAL_BINDACK_TIMEOUT_FIRST    1 // seconds
#define MIPv6_MAX_RR_BINDING_LIFETIME          420 // seconds
#define MIPv6_MAX_HA_BINDING_LIFETIME          3600 // seconds (1 hour)
/***************END of RFC 3775 Protocol Constants*****************************/
#endif /* INET_WITH_xMIPv6 */

/*
 * Info for ipv6MulticastGroupJoinedSignal and ipv6MulticastGroupLeftSignal notifications
 */
struct INET_API Ipv6MulticastGroupInfo : public cObject
{
    Ipv6MulticastGroupInfo(NetworkInterface *const ie, const Ipv6Address& groupAddress)
        : ie(ie), groupAddress(groupAddress) {}
    NetworkInterface *ie;
    Ipv6Address groupAddress;
};

/**
 * Ipv6-specific data for NetworkInterface. Most of this comes from
 * section 6.2.1 of RFC 2461 (Ipv6 Neighbor Discovery, Router Configuration
 * Variables).
 */
class INET_API Ipv6InterfaceData : public InterfaceProtocolData
{
  public:
    typedef std::vector<Ipv6Address> Ipv6AddressVector;

    // field ids for change notifications
    enum { F_IP_ADDRESS, F_MULTICAST_ADDRESSES, F_MULTICAST_LISTENERS }; // FIXME missed field IDs and missed notifications in setter functions

  protected:
    struct HostMulticastData {
        Ipv6AddressVector joinedMulticastGroups;
        std::vector<int> refCounts;

        std::string str();
        std::string detailedInfo();
    };

    struct RouterMulticastData {
        Ipv6AddressVector reportedMulticastGroups; ///< multicast groups that have listeners on the link connected to this interface

        std::string str();
        std::string detailedInfo();
    };

    HostMulticastData *hostMcastData = nullptr;
    RouterMulticastData *routerMcastData = nullptr;

  public:
    /**
     * For routers: advertised prefix configuration.
     *
     * advValidLifetime, advPreferredLifetime:
     *     - >0 value means absolute expiry time
     *     - <0 means lifetime (delta)
     *     - 0 means infinite lifetime
     */
    struct AdvPrefix {
        short prefixLength;
        bool advOnLinkFlag; // L-flag
        bool advAutonomousFlag; // A-flag

#ifdef INET_WITH_xMIPv6
        bool advRtrAddr; // R-flag (Zarrar Yousaf 09.07.07)
#endif /* INET_WITH_xMIPv6 */

        simtime_t advValidLifetime; // see comment above
        simtime_t advPreferredLifetime; // see comment above
        Ipv6Address prefix;
#ifdef INET_WITH_xMIPv6
        Ipv6Address rtrAddress; // global scope, present when advRtrAddr is true (Zarrar Yousaf 09.07.07)
#endif /* INET_WITH_xMIPv6 */

        AdvPrefix() {}
        AdvPrefix(const Ipv6Address& addr, short preflength)
            : prefixLength(preflength),
              advOnLinkFlag(false), advAutonomousFlag(false),
#ifdef INET_WITH_xMIPv6
              advRtrAddr(false),
#endif /* INET_WITH_xMIPv6 */
              prefix(addr)
        {
        }
    };

    /*************RFC 2461: Section 10 Protocol Constants**********************/
    struct RouterConstants {
        simtime_t maxInitialRtrAdvertInterval;
        uint maxInitialRtrAdvertisements;
        uint maxFinalRtrAdvertisements;
        simtime_t minDelayBetweenRAs;
        simtime_t maxRADelayTime;
    };
    RouterConstants routerConstants;

    struct HostConstants {
        simtime_t maxRtrSolicitationDelay;
        simtime_t rtrSolicitationInterval;
        int maxRtrSolicitations;

#ifdef INET_WITH_xMIPv6
        uint initialBindAckTimeout; // MIPv6: added by Zarrar Yousaf @ CNI UniDo 17.06.07
        uint maxBindAckTimeout; // MIPv6: added by Zarrar Yousaf @ CNI UniDo 17.06.07
        simtime_t initialBindAckTimeoutFirst; // MIPv6: 12.9.07 - CB
        uint maxRRBindingLifeTime; // 14.9.07 - CB
        uint maxHABindingLifeTime; // 14.9.07 - CB
        uint maxTokenLifeTime; // 10.07.08 - CB
#endif /* INET_WITH_xMIPv6 */
    };
    HostConstants hostConstants;

    struct NodeConstants {
        uint maxMulticastSolicit;
        uint maxUnicastSolicit;
        simtime_t maxAnycastDelayTime;
        uint maxNeighbourAdvertisement;
        simtime_t reachableTime;
        simtime_t retransTimer;
        simtime_t delayFirstProbeTime;
        double minRandomFactor;
        double maxRandomFactor;
    };
    NodeConstants nodeConstants;
    /***************END of RFC 2461 Protocol Constants*************************/

#ifdef INET_WITH_xMIPv6
    /**
     * The enum AddressType defined below and the new member of the struct AddressData {AddressType addrType} is
     * relevant for MN(s) only as the address(es) configured on the MN interface needs to be tagged as either HoA
     * or CoA, based upon the status of the H-Flag recieved in the RA. This is
     */
    enum AddressType { HoA, CoA }; // to tag a MN's address as Home-Address or Care-of-Address. Zarrar Yousaf 20.07.07

  protected:
    /**
     * Zarrar 03.09.07: Home Network Information maintains home network information like the MN's home address
     * (HoA) and the HA's address and its prefix. The information from this list will be used by the MN in
     * sending BU towards HA for home registration while in visit network. This data will be updated as soon as
     * the MN processes the prefix information recieved in the RA from the HA and when it auto-configures its
     * global scope address. This home network info is defined in the Ipv6InterfaceData in order to support MN(s)
     * with multiple interfaces, where there could be a possibility that multiple interfaces in a single MN may
     * belong to different home networks. Therefore it is necessary to maintain home network info on a per
     * interface basis
     */
    struct HomeNetworkInfo {
        Ipv6Address HoA; // Home Address of the MN, configured while in the home network
        Ipv6Address homeAgentAddr;
//        Ipv6NdPrefixInformation prefix;
        Ipv6Address prefix;
    };
    friend std::ostream& operator<<(std::ostream& os, const HomeNetworkInfo& homeNetInfo);
    HomeNetworkInfo homeInfo;
    bool dadInProgress = false;

  public:
    bool isDadInProgress() { return dadInProgress; };
    void setDadInProgress(bool val) { dadInProgress = val; }
#endif /* INET_WITH_xMIPv6 */

  protected:
    // addresses
    struct AddressData {
        Ipv6Address address; // address itself
        bool tentative; // true if currently undergoing Duplicate Address Detection
        simtime_t expiryTime; // end of valid lifetime; 0 means infinity
        simtime_t prefExpiryTime; // end of preferred lifetime; 0 means infinity

#ifdef INET_WITH_xMIPv6
        AddressType addrType; // HoA or CoA: Zarrar 20.07.07
#endif /* INET_WITH_xMIPv6 */
    };
    typedef std::vector<AddressData> AddressDataVector;
    // TODO should be std::map, so that isLocalAddress() is faster?

    AddressDataVector addresses; // interface addresses
    Ipv6Address preferredAddr; // cached result of preferred address selection
    simtime_t preferredAddrExpiryTime;

    /***************RFC 2462: Section 5.1 Node Configuration Variables*********/
    struct NodeVariables {
        /**
         *  The number of consecutive Neighbor Solicitation messages sent while
         *  performing Duplicate Address Detection on a tentative address. A
         *  value of zero indicates that Duplicate Address Detection is not performed
         *  on tentative addresses. A value of one indicates a single transmission
         *  with no follow up retransmissions.
         *
         *  Default: 1, but may be overridden by a link-type specific value in
         *  the document that covers issues related to the transmission of IP
         *  over a particular link type (e.g., [Ipv6-ETHER]).
         *
         *  Autoconfiguration also assumes the presence of the variable RetransTimer
         *  as defined in [DISCOVERY]. For autoconfiguration purposes, RetransTimer
         *  specifies the delay between consecutive Neighbor Solicitation transmissions
         *  performed during Duplicate Address Detection (if DupAddrDetectTransmits
         *  is greater than 1), as well as the time a node waits after sending
         *  the last Neighbor Solicitation before ending the Duplicate Address
         *  Detection process.
         */
        int dupAddrDetectTransmits;
    };
    NodeVariables nodeVars;
    /***************END of RFC 2461 Node Configuration Variables***************/

    /***************RFC 2461: Section 6.3.2 Host Variables*********************/
    struct HostVariables {
        /**
         *  The MTU of the link.
         *  Default: The valued defined in the specific document that describes
         *           how Ipv6 operates over the particular link layer (e.g., [Ipv6-ETHER]).
         */
        uint linkMTU;
        /**
         *  The default hop limit to be used when sending (unicast) IP packets.
         *  Default: The value specified in the "Assigned Numbers" RFC [ASSIGNED]
         *           that was in effect at the time of implementation.
         */
        short curHopLimit;
        /**
         *  A base value used for computing the random ReachableTime value.
         *  Default: REACHABLE_TIME milliseconds. > protocol constants
         */
        uint baseReachableTime;
        /**
         *  The time a neighbor is considered reachable after receiving a
         *  reachability confirmation.
         *  reachableTime =
         *      uniform(MIN_RANDOM_FACTOR, MAX_RANDOM_FACTOR) * baseReachableTime
         *  New value should be calculated when baseReachableTime changes or at
         *  least every few hours even if no Router Advertisements are received.
         */
        simtime_t reachableTime;
        /**
         *  The time between retransmissions of Neighbor Solicitation messages
         *  to a neighbor when resolving the address or when probing the
         *  reachability of a neighbor.
         *  Default: RETRANS_TIMER milliseconds
         */
        uint retransTimer;
    };
    HostVariables hostVars;
    /***************END of RFC 2461 Host Variables*****************************/

    /***************RFC 2461: Section 6.2.1 Router Configuration Variables*****/
    typedef std::vector<AdvPrefix> AdvPrefixList;
    struct RouterVariables {
        /**
         *  A flag indicating whether or not the router sends periodic Router
         *  Advertisements and responds to Router Solicitations.
         *
         *  Default: FALSE
         */
        bool advSendAdvertisements;
        /**
         *  The maximum time allowed between sending unsolicited multicast
         *  Router Advertisements from the interface, in seconds.  MUST be no
         *  less than 4 seconds and no greater than 1800 seconds.
         *
         *  Default: 600 seconds
         */
        simtime_t maxRtrAdvInterval;
        /**
         *  The minimum time allowed between sending unsolicited multicast Router
         *  Advertisements from the interface, in seconds.  MUST be no less than
         *  3 seconds and no greater than .75 * maxRtrAdvInterval
         *
         *  Default: 0.33 * MaxRtrAdvInterval
         */
        simtime_t minRtrAdvInterval;
        /**
         *  The TRUE/FALSE value to be placed in the "Managed address configuration"
         *  flag field in the Router Advertisement.  See [ADDRCONF].
         *
         *  Default: FALSE
         */
        bool advManagedFlag;
        /**
         *  The TRUE/FALSE value to be placed in the "Other stateful configuration"
         *  flag field in the Router Advertisement.  See [ADDRCONF].
         *
         *  Default: FALSE
         */
        bool advOtherConfigFlag; // also false as not disseminating other config info from routers

#ifdef INET_WITH_xMIPv6
        /**
         *  The TRUE/FALSE value to be placed in the "Home Agent"
         *  flag field in the Router Advertisement. See [ADDRCONF].
         *
         *  Default: FALSE
         */
        bool advHomeAgentFlag; // also false as not disseminating other config info from routers
#endif /* INET_WITH_xMIPv6 */

        /**
         *  The value to be placed in MTU options sent by the router. A value of
         *  zero indicates that no MTU options are sent.
         *
         *  Default: 0
         */
        int advLinkMTU;
        /**
         *  The value to be placed in the Reachable Time field in the Router
         *  Advertisement messages sent by the router.  The value zero means
         *  unspecified (by this router).  MUST be no greater than 3,600,000
         *  milliseconds (1 hour).
         *
         *  Default: 0
         */
        int advReachableTime; // host BaseReachableTime?
        /**
         *  The value to be placed in the Retrans Timer field in the Router
         *  Advertisement messages sent by the router.  The value zero means
         *  unspecified (by this router).
         *
         *  Default: 0
         */
        int advRetransTimer; // host RetransTimer?
        /**
         *  The default value to be placed in the Cur Hop Limit field in the
         *  Router Advertisement messages sent by the router. The value should be
         *  set to that current diameter of the Internet.  The value zero means
         *  unspecified (by this router).
         *
         *  Default:  The value specified in the "Assigned Numbers" RFC [ASSIGNED]
         *  that was in effect at the time of implementation.
         */
        short advCurHopLimit; // host CurHopLimit
        /**
         *  The value to be placed in the Router Lifetime field of Router
         *  Advertisements sent from the interface, in seconds.  MUST be either
         *  zero or between MaxRtrAdvInterval and 9000 seconds.  A value of zero
         *  indicates that the router is not to be used as a default router.
         *
         *  Default: 3 * MaxRtrAdvInterval
         */
        simtime_t advDefaultLifetime; // integer seconds in ND rfc but for MIPv6 really need better granularity
//        bool fastRA;
//        uint maxFastRAS;
//        mutable uint fastRACounter;

        // These are either learned from routers or configured manually(router)
        AdvPrefixList advPrefixList;
    };
    RouterVariables rtrVars;
    /***************END of RFC 2461 Host Variables*****************************/

  protected:
    int findAddress(const Ipv6Address& addr) const;
    void choosePreferredAddress();
    void changed1(int fieldId) { changed(interfaceIpv6ConfigChangedSignal, fieldId); }
    HostMulticastData *getHostData() { if (!hostMcastData) hostMcastData = new HostMulticastData(); return hostMcastData; }
    const HostMulticastData *getHostData() const { return const_cast<Ipv6InterfaceData *>(this)->getHostData(); }
    RouterMulticastData *getRouterData() { if (!routerMcastData) routerMcastData = new RouterMulticastData(); return routerMcastData; }
    const RouterMulticastData *getRouterData() const { return const_cast<Ipv6InterfaceData *>(this)->getRouterData(); }

    static bool addrLess(const AddressData& a, const AddressData& b);

  public:
    Ipv6InterfaceData();
    virtual ~Ipv6InterfaceData();
    std::string str() const override;
    std::string detailedInfo() const;

    /** @name Addresses */
    //@{
#ifndef INET_WITH_xMIPv6
    /**
     * Assigns the given address to the interface.
     */
    virtual void assignAddress(const Ipv6Address& addr, bool tentative,
            simtime_t expiryTime, simtime_t prefExpiryTime);
#else /* INET_WITH_xMIPv6 */
    /**
     * Assigns the given address to the interface.
     *
     * INET_WITH_xMIPv6:
     * Also takes into account the status of the H-Flag in the recieved RA.
     * Called from  Ipv6NeighbourDiscovery::processRAPrefixInfoForAddrAutoConf(
     *                      Ipv6NdPrefixInformation& prefixInfo, NetworkInterface* ie, bool hFlag).
     * Relevant only when MIPv6 is supported. (Zarrar Yousaf 20.07.07)
     */
    virtual void assignAddress(const Ipv6Address& addr, bool tentative,
            simtime_t expiryTime, simtime_t prefExpiryTime, bool hFlag = false);
#endif /* INET_WITH_xMIPv6 */

    /**
     * Update expiry times of addresses. Expiry times possibly come from
     * prefixes (with on-link flag set to either zero or one)
     * in Router Advertisements. Zero expiry time means infinity.
     */
    virtual void updateMatchingAddressExpiryTimes(const Ipv6Address& prefix, int length,
            simtime_t expiryTime = SIMTIME_ZERO, simtime_t prefExpiryTime = SIMTIME_ZERO);

    /**
     * Returns the number of addresses the interface has.
     */
    int getNumAddresses() const { return addresses.size(); }

    /**
     * Returns ith address of the interface.
     */
    const Ipv6Address& getAddress(int i) const;
    /*
     * Returns Global address of the interface.
     * */
    const Ipv6Address& getGlblAddress() const;

    /**
     * Returns true if the ith address of the interface is tentative.
     */
    bool isTentativeAddress(int i) const;

#ifdef INET_WITH_xMIPv6
    /**
     * Returns the address type (HoA, CoA) of the ith address of the interface.
     * 4.9.07 - CB
     */
    AddressType getAddressType(int i) const;

    /**
     * Returns the address type (HoA, CoA) of the provided address of the interface.
     * 27.9.07 - CB
     */
    AddressType getAddressType(const Ipv6Address& addr) const;
#endif /* INET_WITH_xMIPv6 */

// FIXME address validity check missing. introduce hasValidAddress(addr, now) which would compare lifetimes too?

    /**
     * Returns true if the given address is one of the addresses assigned,
     * regardless whether it is tentative or not.
     */
    bool hasAddress(const Ipv6Address& addr) const;

    /**
     * Returns true if the interface has an address matching the given
     * solicited-node multicast addresses.
     */
    bool matchesSolicitedNodeMulticastAddress(const Ipv6Address& solNodeAddr) const;

    /**
     * Returns true if the interface has the given address and it is tentative.
     */
    bool isTentativeAddress(const Ipv6Address& addr) const;

    /**
     * Clears the "tentative" flag of an existing interface address.
     */
    virtual void permanentlyAssign(const Ipv6Address& addr);

    const Ipv6AddressVector& getJoinedMulticastGroups() const { return getHostData()->joinedMulticastGroups; }
    const Ipv6AddressVector& getReportedMulticastGroups() const { return getRouterData()->reportedMulticastGroups; }

    bool isMemberOfMulticastGroup(const Ipv6Address& multicastAddress) const;
    virtual void joinMulticastGroup(const Ipv6Address& multicastAddress);
    virtual void leaveMulticastGroup(const Ipv6Address& multicastAddress);

    bool hasMulticastListener(const Ipv6Address& multicastAddress) const;
    virtual void addMulticastListener(const Ipv6Address& multicastAddress);
    virtual void removeMulticastListener(const Ipv6Address& multicastAddress);

#ifdef INET_WITH_xMIPv6
    /**
     * Sets the "tentative" flag of of the ith address of the interface.
     * 28.09.07 - CB
     */
    void tentativelyAssign(int i);
#endif /* INET_WITH_xMIPv6 */

    /**
     * Chooses a preferred address for the interface and returns it.
     * This is the address that should be used as source address for outgoing
     * datagrams, if one is not explicitly specified.
     *
     * Selection is based on scope (globally routable addresses are preferred),
     * then on lifetime (the one that expires last is chosen). See private
     * choosePreferredAddress() function.
     *
     * FIXME turn into preferredGLOBALAddress()!
     */
    const Ipv6Address& getPreferredAddress() const { return preferredAddr; } // FIXME TODO check expiry time!

    /**
     * Returns the first valid link-local address of the interface,
     * or UNSPECIFIED_ADDRESS if there's none.
     */
    const Ipv6Address& getLinkLocalAddress() const;

    /**
     * Removes the address. Called when the valid lifetime expires.
     */
    virtual void removeAddress(const Ipv6Address& address);

    /**
     *  Getters/Setters for all variables and constants defined in RFC 2461/2462
     *  can be found here. Operations responsible for protocol constants are marked
     *  with a "_" prefix. Constants in this class are stored as instance variables.
     *  Default values for certain variables are defined at the top of this file,
     *  while certain variables have to be generated. Protocol constants are subject
     *  to change as specified in RFC2461:section 10 depending on different link
     *  layer operation. Getters and setters have been implemented for protocol
     *  constants so that a wireless interface may be set to a different set of
     *  constant values. (ie. changed by the Ipv4FlatNetworkConfigurator) Such a design
     *  allows both wired and wireless networks to co-exist within a simulation run.
     */
    /************Getters for Router Protocol Constants*************************/
    simtime_t _getMaxInitialRtrAdvertInterval() const { return routerConstants.maxInitialRtrAdvertInterval; }
    uint _getMaxInitialRtrAdvertisements() const { return routerConstants.maxInitialRtrAdvertisements; }
    uint _getMaxFinalRtrAdvertisements() const { return routerConstants.maxFinalRtrAdvertisements; }
    simtime_t _getMinDelayBetweenRAs() const { return routerConstants.minDelayBetweenRAs; }
    simtime_t _getMaxRaDelayTime() const { return routerConstants.maxRADelayTime; }
    /************Setters for Router Protocol Constants*************************/
    virtual void _setMaxInitialRtrAdvertInterval(simtime_t d) { routerConstants.maxInitialRtrAdvertInterval = d; }
    virtual void _setMaxInitialRtrAdvertisements(uint d) { routerConstants.maxInitialRtrAdvertisements = d; }
    virtual void _setMaxFinalRtrAdvertisements(uint d) { routerConstants.maxFinalRtrAdvertisements = d; }
    virtual void _setMinDelayBetweenRAs(simtime_t d) { routerConstants.minDelayBetweenRAs = d; }
    virtual void _setMaxRaDelayTime(simtime_t d) { routerConstants.maxRADelayTime = d; }
    /************End of Router Protocol Constant getters and setters***********/

    /************Getters for Host Protocol Constants***************************/
    simtime_t _getMaxRtrSolicitationDelay() const { return hostConstants.maxRtrSolicitationDelay; }
    simtime_t _getRtrSolicitationInterval() const { return hostConstants.rtrSolicitationInterval; }
    uint _getMaxRtrSolicitations() const { return hostConstants.maxRtrSolicitations; }

#ifdef INET_WITH_xMIPv6
    simtime_t _getInitialBindAckTimeout() const { return hostConstants.initialBindAckTimeout; } // MIPv6: added by Zarrar Yousaf @ CNI UniDo 17.06.07
    simtime_t _getMaxBindAckTimeout() const { return hostConstants.maxBindAckTimeout; } // MIPv6: added by Zarrar Yousaf @ CNI UniDo 17.06.07
    simtime_t _getInitialBindAckTimeoutFirst() const { return hostConstants.initialBindAckTimeoutFirst; } // MIPv6, 12.9.07 - CB
    uint _getMaxRrBindingLifeTime() const { return hostConstants.maxRRBindingLifeTime; } // MIPv6, 14.9.07 - CB
    uint _getMaxHaBindingLifeTime() const { return hostConstants.maxHABindingLifeTime; } // MIPv6, 14.9.07 - CB
    uint _getMaxTokenLifeTime() const { return hostConstants.maxTokenLifeTime; } // MIPv6, 10.07.08 - CB
#endif /* INET_WITH_xMIPv6 */

    /************Setters for Host Protocol Constants***************************/
    virtual void _setMaxRtrSolicitationDelay(simtime_t d) { hostConstants.maxRtrSolicitationDelay = d; }
    virtual void _setRtrSolicitationInterval(simtime_t d) { hostConstants.rtrSolicitationInterval = d; }
    virtual void _setMaxRtrSolicitations(uint d) { hostConstants.maxRtrSolicitations = d; }

#ifdef INET_WITH_xMIPv6
    virtual void _setInitialBindAckTimeout(simtime_t d) { hostConstants.initialBindAckTimeout = SIMTIME_DBL(d); }
    virtual void _setMaxBindAckTimeout(simtime_t d) { hostConstants.maxBindAckTimeout = SIMTIME_DBL(d); }
    virtual void _setInitialBindAckTimeoutFirst(simtime_t d) { hostConstants.initialBindAckTimeoutFirst = d; }
    virtual void _setMaxRrBindingLifeTime(uint d) { hostConstants.maxRRBindingLifeTime = d; }
    virtual void _setMaxHaBindingLifeTime(uint d) { hostConstants.maxHABindingLifeTime = d; }
    virtual void _setMaxTokenLifeTime(uint d) { hostConstants.maxTokenLifeTime = d; }
#endif /* INET_WITH_xMIPv6 */
    /************End of Host Protocol Constant getters and setters*************/

    /************Getters for Node Protocol Constants***************************/
    uint _getMaxMulticastSolicit() const { return nodeConstants.maxMulticastSolicit; }
    uint _getMaxUnicastSolicit() const { return nodeConstants.maxUnicastSolicit; }
    simtime_t _getMaxAnycastDelayTime() const { return nodeConstants.maxAnycastDelayTime; }
    uint _getMaxNeighbourAdvertisement() const { return nodeConstants.maxNeighbourAdvertisement; }
    simtime_t _getReachableTime() const { return nodeConstants.reachableTime; }
    simtime_t _getRetransTimer() const { return nodeConstants.retransTimer; }
    simtime_t _getDelayFirstProbeTime() const { return nodeConstants.delayFirstProbeTime; }
    double _getMinRandomFactor() const { return nodeConstants.minRandomFactor; }
    double _getMaxRandomFactor() const { return nodeConstants.maxRandomFactor; }
    /************Setters for Node Protocol Constants***************************/
    virtual void _setMaxMulticastSolicit(uint d) { nodeConstants.maxMulticastSolicit = d; }
    virtual void _setMaxUnicastSolicit(uint d) { nodeConstants.maxUnicastSolicit = d; }
    virtual void _setMaxAnycastDelayTime(simtime_t d) { nodeConstants.maxAnycastDelayTime = d; }
    virtual void _setMaxNeighbourAdvertisement(uint d) { nodeConstants.maxNeighbourAdvertisement = d; }
    virtual void _setReachableTime(simtime_t d) { nodeConstants.reachableTime = d; }
    virtual void _setRetransTimer(simtime_t d) { nodeConstants.retransTimer = d; }
    virtual void _setDelayFirstProbeTime(simtime_t d) { nodeConstants.delayFirstProbeTime = d; }
    virtual void _setMinRandomFactor(double d) { nodeConstants.minRandomFactor = d; }
    virtual void _setMaxRandomFactor(double d) { nodeConstants.maxRandomFactor = d; }
    /************End of Node Protocol Constant getters and setters*************/

    /************Getters for Node Variables************************************/
    int getDupAddrDetectTransmits() const { return nodeVars.dupAddrDetectTransmits; }
    /************Setters for Node Variables************************************/
    virtual void setDupAddrDetectTransmits(int d) { nodeVars.dupAddrDetectTransmits = d; }
    /************End of Node Variables getters and setters*********************/

    /************Getters for Host Variables************************************/
    uint getLinkMtu() const { return hostVars.linkMTU; }
    short getCurHopLimit() const { return hostVars.curHopLimit; }
    uint getBaseReachableTime() const { return hostVars.baseReachableTime; }
    simtime_t getReachableTime() const { return hostVars.reachableTime; }
    uint getRetransTimer() const { return hostVars.retransTimer; }
    /************Setters for Host Variables************************************/
    virtual void setLinkMtu(uint d) { hostVars.linkMTU = d; }
    virtual void setCurHopLimit(short d) { hostVars.curHopLimit = d; }
    virtual void setBaseReachableTime(uint d) { hostVars.baseReachableTime = d; }
    virtual void setReachableTime(simtime_t d) { hostVars.reachableTime = d; }
    virtual void setRetransTimer(uint d) { hostVars.retransTimer = d; }
    /************End of Host Variables getters and setters*********************/

    /************Getters for Router Configuration Variables********************/
    bool getAdvSendAdvertisements() const { return rtrVars.advSendAdvertisements; }
    simtime_t getMaxRtrAdvInterval() const { return rtrVars.maxRtrAdvInterval; }
    simtime_t getMinRtrAdvInterval() const { return rtrVars.minRtrAdvInterval; }
    bool getAdvManagedFlag() const { return rtrVars.advManagedFlag; }
    bool getAdvOtherConfigFlag() const { return rtrVars.advOtherConfigFlag; }

#ifdef INET_WITH_xMIPv6
    bool getAdvHomeAgentFlag() const { return rtrVars.advHomeAgentFlag; }
#endif /* INET_WITH_xMIPv6 */

    int getAdvLinkMtu() const { return rtrVars.advLinkMTU; }
    int getAdvReachableTime() const { return rtrVars.advReachableTime; }
    int getAdvRetransTimer() const { return rtrVars.advRetransTimer; }
    short getAdvCurHopLimit() const { return rtrVars.advCurHopLimit; }
    simtime_t getAdvDefaultLifetime() const { return rtrVars.advDefaultLifetime; }
    /************Setters for Router Configuration Variables********************/
    virtual void setAdvSendAdvertisements(bool d) { rtrVars.advSendAdvertisements = d; }
    virtual void setMaxRtrAdvInterval(simtime_t d) { rtrVars.maxRtrAdvInterval = d; }
    virtual void setMinRtrAdvInterval(simtime_t d) { rtrVars.minRtrAdvInterval = d; }
    virtual void setAdvManagedFlag(bool d) { rtrVars.advManagedFlag = d; }
    virtual void setAdvOtherConfigFlag(bool d) { rtrVars.advOtherConfigFlag = d; }

#ifdef INET_WITH_xMIPv6
    virtual void setAdvHomeAgentFlag(bool d) { rtrVars.advHomeAgentFlag = d; }
#endif /* INET_WITH_xMIPv6 */

    virtual void setAdvLinkMtu(int d) { rtrVars.advLinkMTU = d; }
    virtual void setAdvReachableTime(int d) { rtrVars.advReachableTime = d; }
    virtual void setAdvRetransTimer(int d) { rtrVars.advRetransTimer = d; }
    virtual void setAdvCurHopLimit(short d) { rtrVars.advCurHopLimit = d; }
    virtual void setAdvDefaultLifetime(simtime_t d) { rtrVars.advDefaultLifetime = d; }
    /************End of Router Configuration Variables getters and setters*****/

    /** @name Router advertised prefixes */
    //@{
    /**
     * Adds the given advertised prefix to the interface.
     */
    virtual void addAdvPrefix(const AdvPrefix& advPrefix);

    /**
     * Returns the number of advertised prefixes on the interface.
     */
    int getNumAdvPrefixes() const { return rtrVars.advPrefixList.size(); }

    /**
     * Returns the ith advertised prefix on the interface.
     */
    const AdvPrefix& getAdvPrefix(int i) const;

    /**
     * Changes the configuration of the ith advertised prefix on the
     * interface. The prefix itself should stay the same.
     */
    virtual void setAdvPrefix(int i, const AdvPrefix& advPrefix);

    /**
     * Remove the ith advertised prefix on the interface. Prefixes
     * at larger indices will shift down.
     */
    virtual void removeAdvPrefix(int i);

    /**
     *  This method randomly generates a reachableTime given the MIN_RANDOM_FACTOR
     *  MAX_RANDOM_FACTOR and baseReachableTime. Refer to RFC 2461: Section 6.3.2
     */
    virtual simtime_t generateReachableTime(double MIN_RANDOM_FACTOR, double MAX_RANDOM_FACTOR,
            uint baseReachableTime);

    /**
     * Arg-less version.
     */
    virtual simtime_t generateReachableTime();

#ifdef INET_WITH_xMIPv6
//############## Additional Function defined by Zarrar YOusaf @ CNI, Uni Dortmund  #########
    /**
     * Returns the first valid global address of the interface,
     * or UNSPECIFIED_ADDRESS if there's none.
     */
    const Ipv6Address& getGlobalAddress(AddressType type = HoA) const; // 24.9.07 - CB

    /**
     * This function autoconfigures a global scope address for the router only,
     * if and only the prefix is provided via some exernal method, For instance
     * Ipv6FlatNetworkConfigurator assigning prefixes to routers interfaces during
     * initialization.
     */
    const Ipv6Address autoConfRouterGlobalScopeAddress(int i); // removed return-by-reference - CB

    void autoConfRouterGlobalScopeAddress(AdvPrefix& p);

    void deduceAdvPrefix();

    /**
     * 03.09.07
     * This updates the struct HomeNetwork Info with the MN's Home Address(HoA), the global
     * scope address of the MNs Home Agent (HA) and the home network prefix.
     */
    void updateHomeNetworkInfo(const Ipv6Address& hoa, const Ipv6Address& ha, const Ipv6Address& prefix, const int prefixLength);

    const Ipv6Address& getHomeAgentAddress() const { return homeInfo.homeAgentAddr; } // Zarrar 03.09.07
    const Ipv6Address& getMNHomeAddress() const { return homeInfo.HoA; } // Zarrar 03.09.07
    const Ipv6Address& getMnPrefix() const { return homeInfo.prefix /*.prefix()*/; } // Zarrar 03.09.07

    /**
     * Removes a CoA address from the interface if one exists.
     */
    Ipv6Address removeAddress(Ipv6InterfaceData::AddressType type); // update 06.08.08 - CB

  protected:
    Ipv6RoutingTable *rt6 = nullptr; // A pointer variable, specifically used to access the type of node (MN, HA, Router, CN). Used in info(). (Zarrar Yousaf 20.07.07)
#endif /* INET_WITH_xMIPv6 */
};

} // namespace inet

#endif

