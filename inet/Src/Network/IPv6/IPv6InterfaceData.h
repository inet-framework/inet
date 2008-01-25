//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2005 Wei Yang, Ng
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#ifndef __IPv6INTERFACEDATA_H
#define __IPv6INTERFACEDATA_H


#include <vector>
#include <omnetpp.h>
#include "IPv6Address.h"
#include "INETDefs.h"

#define IPv6_DEFAULT_DUPADDRDETECTTRANSMITS 1   // send NS once (RFC2462:Section 5.1)

#define IPv6_MIN_MTU                        1280// octets
#define IPv6_DEFAULT_ADVCURHOPLIMIT         30

#define IPv6_DEFAULT_MAX_RTR_ADV_INT        600 //seconds-decrease to enable more periodic RAs
#define IPv6_DEFAULT_ADV_REACHABLE_TIME     3600// seconds
#define IPv6_DEFAULT_ADV_RETRANS_TIMER      1   // seconds
#define IPv6_DEFAULT_ROUTER_HOPLIMIT        64

/**************RFC 2461: Section 10 Protocol Constants*************************/
//Router Constants
#define IPv6_MAX_INITIAL_RTR_ADVERT_INTERVAL 16  // seconds
#define IPv6_MAX_INITIAL_RTR_ADVERTISEMENTS   3  // transmissions
#define IPv6_MAX_FINAL_RTR_ADVERTISEMENTS     3  // transmissions
#define IPv6_MIN_DELAY_BETWEEN_RAS            3  // seconds
#define IPv6_MAX_RA_DELAY_TIME               .5  // seconds

//Host Constants
#define IPv6_MAX_RTR_SOLICITATION_DELAY       1  // seconds
#define IPv6_RTR_SOLICITATION_INTERVAL        4  // seconds
#define IPv6_MAX_RTR_SOLICITATIONS            3  // transmissions

//Node Constants
#define IPv6_MAX_MULTICAST_SOLICIT            3  // transmissions
#define IPv6_MAX_UNICAST_SOLICIT              3  // transmissions
#define IPv6_MAX_ANYCAST_DELAY_TIME           1  // seconds
#define IPv6_MAX_NEIGHBOUR_ADVERTISEMENT      3  // transmissions
#define IPv6_REACHABLE_TIME                  30  // seconds
#define IPv6_RETRANS_TIMER                    1  // seconds
#define IPv6_DELAY_FIRST_PROBE_TIME           5  // seconds
#define IPv6_MIN_RANDOM_FACTOR              0.5
#define IPv6_MAX_RANDOM_FACTOR              1.5
/***************END of RFC 2461 Protocol Constants*****************************/

/**
 * IPv6-specific data for InterfaceEntry. Most of this comes from
 * section 6.2.1 of RFC 2461 (IPv6 Neighbor Discovery, Router Configuration
 * Variables).
 */
class INET_API IPv6InterfaceData : public cPolymorphic
{
  public:
    /**
     * For routers: advertised prefix configuration.
     *
     * advValidLifetime, advPreferredLifetime:
     *     - >0 value means absolute expiry time
     *     - <0 means lifetime (delta)
     *     - 0 means infinite lifetime
     */
    struct AdvPrefix
    {
        IPv6Address prefix;
        short prefixLength;
        simtime_t advValidLifetime; // see comment above
        bool advOnLinkFlag;
        simtime_t advPreferredLifetime; // see comment above
        bool advAutonomousFlag;
        // USE_MOBILITY: bool advRtrAddr;
    };

    /*************RFC 2461: Section 10 Protocol Constants**********************/
    struct RouterConstants
    {
        simtime_t maxInitialRtrAdvertInterval;
        uint maxInitialRtrAdvertisements;
        uint maxFinalRtrAdvertisements;
        simtime_t minDelayBetweenRAs;
        simtime_t maxRADelayTime;
    };
    RouterConstants routerConstants;

    struct HostConstants
    {
        simtime_t maxRtrSolicitationDelay;
        simtime_t rtrSolicitationInterval;
        int maxRtrSolicitations;
    };
    HostConstants hostConstants;

    struct NodeConstants
    {
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

  private:
    // addresses
    struct AddressData
    {
        IPv6Address address;  // address itself
        bool tentative;       // true if currently undergoing Duplicate Address Detection
        simtime_t expiryTime; // end of valid lifetime; 0 means infinity
        simtime_t prefExpiryTime; // end of preferred lifetime; 0 means infinity
    };
    typedef std::vector<AddressData> AddressDataVector;
    // TBD should be std::map, so that localDeliver() is faster?

    AddressDataVector addresses;   // interface addresses
    IPv6Address preferredAddr; // cached result of preferred address selection
    simtime_t preferredAddrExpiryTime;

    /***************RFC 2462: Section 5.1 Node Configuration Variables*********/
    struct NodeVariables
    {
        /**
         *  The number of consecutive Neighbor Solicitation messages sent while
         *  performing Duplicate Address Detection on a tentative address. A
         *  value of zero indicates that Duplicate Address Detection is not performed
         *  on tentative addresses. A value of one indicates a single transmission
         *  with no follow up retransmissions.
         *
         *  Default: 1, but may be overridden by a link-type specific value in
         *  the document that covers issues related to the transmission of IP
         *  over a particular link type (e.g., [IPv6-ETHER]).
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
    struct HostVariables
    {
        /**
         *  The MTU of the link.
         *  Default: The valued defined in the specific document that describes
         *           how IPv6 operates over the particular link layer (e.g., [IPv6-ETHER]).
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
    struct RouterVariables
    {
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
        bool advOtherConfigFlag;// also false as not disseminating other config info from routers
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
        //bool fastRA;
        //uint maxFastRAS;
        //mutable uint fastRACounter;

        // These are either learned from routers or configured manually(router)
        AdvPrefixList advPrefixList;
    };
    RouterVariables rtrVars;
    /***************END of RFC 2461 Host Variables*****************************/

  private:
    int findAddress(const IPv6Address& addr) const;
    void choosePreferredAddress();
    static bool addrLess(const AddressData& a, const AddressData& b);

  public:
    IPv6InterfaceData();
    virtual ~IPv6InterfaceData() {}

    std::string info() const;  //displayed in Tkenv
    std::string detailedInfo() const;  //displayed in Tkenv

    /** @name Addresses */
    //@{
    /**
     * Assigns the given address to the interface.
     */
    void assignAddress(const IPv6Address& addr, bool tentative,
                       simtime_t expiryTime, simtime_t prefExpiryTime);

    /**
     * Update expiry times of addresses. Expiry times possibly come from
     * prefixes (with on-link flag set to either zero or one)
     * in Router Advertisements. Zero expiry time means infinity.
     */
    void updateMatchingAddressExpiryTimes(const IPv6Address& prefix, int length,
                                          simtime_t expiryTime=0, simtime_t prefExpiryTime=0);

    /**
     * Returns the number of addresses the interface has.
     */
    int numAddresses() const {return addresses.size();}

    /**
     * Returns ith address of the interface.
     */
    const IPv6Address& address(int i) const;

    /**
     * Returns true if the ith address of the interface is tentative.
     */
    bool isTentativeAddress(int i) const;

// FIXME address validity check missing. introduce hasValidAddress(addr, now) which would compare lifetimes too?

    /**
     * Returns true if the given address is one of the addresses assigned,
     * regardless whether it is tentative or not.
     */
    bool hasAddress(const IPv6Address& addr) const;

    /**
     * Returns true if the interface has an address matching the given
     * solicited-node multicast addresses.
     */
    bool matchesSolicitedNodeMulticastAddress(const IPv6Address& solNodeAddr) const;

    /**
     * Returns true if the interface has the given address and it is tentative.
     */
    bool isTentativeAddress(const IPv6Address& addr) const;

    /**
     * Clears the "tentative" flag of an existing interface address.
     */
    void permanentlyAssign(const IPv6Address& addr);

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
    const IPv6Address& preferredAddress() const {return preferredAddr;}  // FIXME TBD check expiry time!

    /**
     * Returns the first valid link-local address of the interface,
     * or UNSPECIFIED_ADDRESS if there's none.
     */
    const IPv6Address& linkLocalAddress() const;

    /**
     * Removes the address. Called when the valid lifetime expires.
     */
    void removeAddress(const IPv6Address& address);

    /**
     *  Getters/Setters for all variables and constants defined in RFC 2461/2462
     *  can be found here. Operations responsible for protocol constants are marked
     *  with a "_" prefix. Constants in this class are stored as instance variables.
     *  Default values for certain variables are defined at the top of this file,
     *  while certain variables have to be generated. Protocol constants are subject
     *  to change as specified in RFC2461:section 10 depending on different link
     *  layer operation. Getters and setters have been implemented for protocol
     *  constants so that a wireless interface may be set to a different set of
     *  constant values. (ie. changed by the FlatNetworkConfigurator) Such a design
     *  allows both wired and wireless networks to co-exist within a simulation run.
     */
    /************Getters for Router Protocol Constants*************************/
    simtime_t _maxInitialRtrAdvertInterval() {return routerConstants.maxInitialRtrAdvertInterval;}
    uint _maxInitialRtrAdvertisements() {return routerConstants.maxInitialRtrAdvertisements;}
    uint _maxFinalRtrAdvertisements() {return routerConstants.maxFinalRtrAdvertisements;}
    simtime_t _minDelayBetweenRAs() {return routerConstants.minDelayBetweenRAs;}
    simtime_t _maxRADelayTime() {return routerConstants.maxRADelayTime;}
    /************Setters for Router Protocol Constants*************************/
    void _setMaxInitialRtrAdvertInterval(simtime_t d) {routerConstants.maxInitialRtrAdvertInterval = d;}
    void _setMaxInitialRtrAdvertisements(uint d) {routerConstants.maxInitialRtrAdvertisements = d;}
    void _setMaxFinalRtrAdvertisements(uint d) {routerConstants.maxFinalRtrAdvertisements = d;}
    void _setMinDelayBetweenRAs(simtime_t d) {routerConstants.minDelayBetweenRAs = d;}
    void _setMaxRADelayTime(simtime_t d) {routerConstants.maxRADelayTime = d;}
    /************End of Router Protocol Constant getters and setters***********/

    /************Getters for Host Protocol Constants***************************/
    simtime_t _maxRtrSolicitationDelay() {return hostConstants.maxRtrSolicitationDelay;}
    simtime_t _rtrSolicitationInterval() {return hostConstants.rtrSolicitationInterval;}
    uint _maxRtrSolicitations() {return hostConstants.maxRtrSolicitations;}
    /************Setters for Host Protocol Constants***************************/
    void _setMaxRtrSolicitationDelay(simtime_t d) {hostConstants.maxRtrSolicitationDelay = d;}
    void _setRtrSolicitationInterval(simtime_t d) {hostConstants.rtrSolicitationInterval = d;}
    void _setMaxRtrSolicitations(uint d) {hostConstants.maxRtrSolicitations = d;}
    /************End of Host Protocol Constant getters and setters*************/

    /************Getters for Node Protocol Constants***************************/
    uint _maxMulticastSolicit() {return nodeConstants.maxMulticastSolicit;}
    uint _maxUnicastSolicit() {return nodeConstants.maxUnicastSolicit;}
    simtime_t _maxAnycastDelayTime() {return nodeConstants.maxAnycastDelayTime;}
    uint _maxNeighbourAdvertisement() {return nodeConstants.maxNeighbourAdvertisement;}
    simtime_t _reachableTime() {return nodeConstants.reachableTime;}
    simtime_t _retransTimer() {return nodeConstants.retransTimer;}
    simtime_t _delayFirstProbeTime() {return nodeConstants.delayFirstProbeTime;}
    double _minRandomFactor() {return nodeConstants.minRandomFactor;}
    double _maxRandomFactor() {return nodeConstants.maxRandomFactor;}
    /************Setters for Node Protocol Constants***************************/
    void _setMaxMulticastSolicit(uint d) {nodeConstants.maxMulticastSolicit = d;}
    void _setMaxUnicastSolicit(uint d) {nodeConstants.maxUnicastSolicit = d;}
    void _setMaxAnycastDelayTime(simtime_t d) {nodeConstants.maxAnycastDelayTime = d;}
    void _setMaxNeighbourAdvertisement(uint d) {nodeConstants.maxNeighbourAdvertisement = d;}
    void _setReachableTime(simtime_t d) {nodeConstants.reachableTime = d;}
    void _setRetransTimer(simtime_t d) {nodeConstants.retransTimer = d;}
    void _setDelayFirstProbeTime(simtime_t d) {nodeConstants.delayFirstProbeTime = d;}
    void _setMinRandomFactor(double d) {nodeConstants.minRandomFactor = d;}
    void _setMaxRandomFactor(double d) {nodeConstants.maxRandomFactor = d;}
    /************End of Node Protocol Constant getters and setters*************/

    /************Getters for Node Variables************************************/
    int dupAddrDetectTransmits() {return nodeVars.dupAddrDetectTransmits;}
    /************Setters for Node Variables************************************/
    void setDupAddrDetectTransmits(int d) {nodeVars.dupAddrDetectTransmits = d;}
    /************End of Node Variables getters and setters*********************/

    /************Getters for Host Variables************************************/
    uint linkMTU() {return hostVars.linkMTU;}
    short curHopLimit() {return hostVars.curHopLimit;}
    uint baseReachableTime() {return hostVars.baseReachableTime;}
    simtime_t reachableTime() {return hostVars.reachableTime;}
    uint retransTimer() {return hostVars.retransTimer;}
    /************Setters for Host Variables************************************/
    void setLinkMTU(uint d) {hostVars.linkMTU = d;}
    void setCurHopLimit(short d) {hostVars.curHopLimit = d;}
    void setBaseReachableTime(uint d) {hostVars.baseReachableTime = d;}
    void setReachableTime(simtime_t d) {hostVars.reachableTime = d;}
    void setRetransTimer(uint d) {hostVars.retransTimer = d;}
    /************End of Host Variables getters and setters*********************/

    /************Getters for Router Configuration Variables********************/
    bool advSendAdvertisements() {return rtrVars.advSendAdvertisements;}
    simtime_t maxRtrAdvInterval() {return rtrVars.maxRtrAdvInterval;}
    simtime_t minRtrAdvInterval() {return rtrVars.minRtrAdvInterval;}
    bool advManagedFlag() {return rtrVars.advManagedFlag;}
    bool advOtherConfigFlag() {return rtrVars.advOtherConfigFlag;}
    int advLinkMTU() {return rtrVars.advLinkMTU;}
    int advReachableTime() {return rtrVars.advReachableTime;}
    int advRetransTimer() {return rtrVars.advRetransTimer;}
    short advCurHopLimit() {return rtrVars.advCurHopLimit;}
    simtime_t advDefaultLifetime()  {return rtrVars.advDefaultLifetime;}
    /************Setters for Router Configuration Variables********************/
    void setAdvSendAdvertisements(bool d) {rtrVars.advSendAdvertisements = d;}
    void setMaxRtrAdvInterval(simtime_t d) {rtrVars.maxRtrAdvInterval = d;}
    void setMinRtrAdvInterval(simtime_t d) {rtrVars.minRtrAdvInterval = d;}
    void setAdvManagedFlag(bool d) {rtrVars.advManagedFlag = d;}
    void setAdvOtherConfigFlag(bool d) {rtrVars.advOtherConfigFlag = d;}
    void setAdvLinkMTU(int d) {rtrVars.advLinkMTU = d;}
    void setAdvReachableTime(int d) {rtrVars.advReachableTime = d;}
    void setAdvRetransTimer(int d) {rtrVars.advRetransTimer = d;}
    void setAdvCurHopLimit(short d) {rtrVars.advCurHopLimit = d;}
    void setAdvDefaultLifetime(simtime_t d) {rtrVars.advDefaultLifetime = d;}
    /************End of Router Configuration Variables getters and setters*****/

    /** @name Router advertised prefixes */
    //@{
    /**
     * Adds the given advertised prefix to the interface.
     */
    void addAdvPrefix(const AdvPrefix& advPrefix);

    /**
     * Returns the number of advertised prefixes on the interface.
     */
    int numAdvPrefixes() const {return rtrVars.advPrefixList.size();}

    /**
     * Returns the ith advertised prefix on the interface.
     */
    const AdvPrefix& advPrefix(int i) const;

    /**
     * Changes the configuration of the ith advertised prefix on the
     * interface. The prefix itself should stay the same.
     */
    void setAdvPrefix(int i, const AdvPrefix& advPrefix);

    /**
     * Remove the ith advertised prefix on the interface. Prefixes
     * at larger indices will shift down.
     */
    void removeAdvPrefix(int i);

    /**
     *  This method randomly generates a reachableTime given the MIN_RANDOM_FACTOR
     *  MAX_RANDOM_FACTOR and baseReachableTime. Refer to RFC 2461: Section 6.3.2
     */
    simtime_t generateReachableTime(double MIN_RANDOM_FACTOR,
        double MAX_RANDOM_FACTOR, uint baseReachableTime);

    /**
     * Arg-less version.
     */
    simtime_t generateReachableTime();
  };

#endif

