//
// Copyright (C) 2007
// Faqir Zarrar Yousaf, Communication Networks Institute, University of Dortmund, Germany.
// Christian Bauer, Institute of Communications and Navigation, German Aerospace Center (DLR)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_MIPV6INTERFACEDATA_H
#define __INET_MIPV6INTERFACEDATA_H

#include "inet/common/INETDefs.h"

#ifndef INET_WITH_IPv6
#error "IPv6 feature disabled"
#endif

#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"

namespace inet {

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
#define MIPv6_INITIAL_BINDACK_TIMEOUT_FIRST    1 // seconds
#define MIPv6_MAX_RR_BINDING_LIFETIME          420 // seconds
#define MIPv6_MAX_HA_BINDING_LIFETIME          3600 // seconds (1 hour)
/***************END of RFC 3775 Protocol Constants*****************************/

/**
 * Mobile IPv6 (RFC 3775) per-interface data: the Mobile Node's home address and
 * home agent, plus the MIPv6 host protocol constants. Attached to a
 * NetworkInterface alongside ~Ipv6InterfaceData.
 */
class INET_API Mipv6InterfaceData : public InterfaceProtocolData
{
  public:
    struct HomeNetworkInfo {
        Ipv6Address HoA; // Home Address of the MN
        Ipv6Address homeAgentAddr;
        Ipv6Address prefix;
    };
    friend std::ostream& operator<<(std::ostream& os, const HomeNetworkInfo& homeNetInfo);

  protected:
    HomeNetworkInfo homeInfo;

    struct HostConstants {
        // MIPv6 host constants (RFC 3775 Section 12)
        uint initialBindAckTimeout;
        uint maxBindAckTimeout;
        simtime_t initialBindAckTimeoutFirst;
        uint maxRRBindingLifeTime;
        uint maxHABindingLifeTime;
        uint maxTokenLifeTime;
    };
    HostConstants hostConstants;

  public:
    Mipv6InterfaceData();

    /** @name Home network information */
    //@{
    const Ipv6Address& getHomeAgentAddress() const { return homeInfo.homeAgentAddr; }
    const Ipv6Address& getMNHomeAddress() const { return homeInfo.HoA; }
    const Ipv6Address& getMnPrefix() const { return homeInfo.prefix; }

    /**
     * Updates the Home Network Information with the MN's HoA, the HA's
     * address, and the home network prefix. Assigns the HoA to the sibling
     * ~Ipv6InterfaceData if it does not yet have one.
     */
    void updateHomeNetworkInfo(const Ipv6Address& hoa, const Ipv6Address& ha, const Ipv6Address& prefix, const int prefixLength);
    //@}

    /** @name MIPv6 host constant getters (RFC 3775) */
    //@{
    simtime_t _getInitialBindAckTimeout() const { return hostConstants.initialBindAckTimeout; }
    simtime_t _getMaxBindAckTimeout() const { return hostConstants.maxBindAckTimeout; }
    simtime_t _getInitialBindAckTimeoutFirst() const { return hostConstants.initialBindAckTimeoutFirst; }
    uint _getMaxRrBindingLifeTime() const { return hostConstants.maxRRBindingLifeTime; }
    uint _getMaxHaBindingLifeTime() const { return hostConstants.maxHABindingLifeTime; }
    uint _getMaxTokenLifeTime() const { return hostConstants.maxTokenLifeTime; }
    //@}

    /** @name MIPv6 host constant setters (RFC 3775) */
    //@{
    virtual void _setInitialBindAckTimeout(simtime_t d) { hostConstants.initialBindAckTimeout = SIMTIME_DBL(d); }
    virtual void _setMaxBindAckTimeout(simtime_t d) { hostConstants.maxBindAckTimeout = SIMTIME_DBL(d); }
    virtual void _setInitialBindAckTimeoutFirst(simtime_t d) { hostConstants.initialBindAckTimeoutFirst = d; }
    virtual void _setMaxRrBindingLifeTime(uint d) { hostConstants.maxRRBindingLifeTime = d; }
    virtual void _setMaxHaBindingLifeTime(uint d) { hostConstants.maxHABindingLifeTime = d; }
    virtual void _setMaxTokenLifeTime(uint d) { hostConstants.maxTokenLifeTime = d; }
    //@}
};

} // namespace inet

#endif
