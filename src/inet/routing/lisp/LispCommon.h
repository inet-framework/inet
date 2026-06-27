//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_LISPCOMMON_H
#define __INET_LISPCOMMON_H

#include <string>

#include "inet/networklayer/common/L3Address.h"

namespace inet {
namespace lisp {

// numeric defaults (RFC 6830)
inline constexpr unsigned char DEFAULT_EIDLENGTH_VAL = 0;
inline constexpr unsigned char DEFAULT_PRIORITY_VAL = 1;
inline constexpr unsigned char DEFAULT_WEIGHT_VAL = 100;
inline constexpr unsigned char DEFAULT_MPRIORITY_VAL = 255;
inline constexpr unsigned char DEFAULT_MWEIGHT_VAL = 0;
inline constexpr unsigned short DEFAULT_TTL_VAL = 1440; // minutes
inline constexpr unsigned short NOETR_TTL_VAL = 1;
inline constexpr unsigned short NOEID_TTL_VAL = 15;
inline constexpr unsigned short DEFAULT_MAPVER_VAL = 0;
inline constexpr short DATA_PORT_VAL = 4341;
inline constexpr short CONTROL_PORT_VAL = 4342;
inline constexpr unsigned short LISPHDR_SIZE = 8;

// XML configuration tags / attribute names
inline constexpr const char *ENABLED_VAL = "enabled";
inline constexpr const char *LISP_TAG = "LISP";
inline constexpr const char *ROUTING_TAG = "Routing";
inline constexpr const char *EID_TAG = "EID";
inline constexpr const char *RLOC_TAG = "RLOC";
inline constexpr const char *ETRMAPSERVER_TAG = "EtrMapServer";
inline constexpr const char *ITRMAPRESOLVER_TAG = "ItrMapResolver";
inline constexpr const char *MAPSERVER_TAG = "MapServer";
inline constexpr const char *MAPRESOLVER_TAG = "MapResolver";
inline constexpr const char *ETRMAP_TAG = "EtrMapping";
inline constexpr const char *MAPCACHE_TAG = "MapCache";
inline constexpr const char *ADDRESS_ATTR = "address";
inline constexpr const char *PRIORITY_ATTR = "priority";
inline constexpr const char *WEIGHT_ATTR = "weight";
inline constexpr const char *LOCAL_ATTR = "local";
inline constexpr const char *KEY_ATTR = "key";
inline constexpr const char *PROXY_ATTR = "proxy";
inline constexpr const char *NOTIFY_ATTR = "want-map-notify";
inline constexpr const char *QUICKREG_ATTR = "quick-registration";
inline constexpr const char *IPV4_ATTR = "ipv4";
inline constexpr const char *IPV6_ATTR = "ipv6";

/**
 * Constants, enumerations and address/prefix helpers shared across the LISP
 * implementation.
 */
class INET_API LispCommon
{
  public:
    enum EKeyIds {
        KID_NONE = 0,
        KID_HMAC_SHA_1_96 = 1,
        KID_HMAC_SHA_256_128 = 2,
    };

    enum EAct {
        NO_ACTION = 0,
        NATIVELY_FORWARD = 1,
        SEND_MAP_REQUEST = 2,
        DROP = 3,
    };

    enum Afi { AFI_UNKNOWN = 0, AFI_IPV4 = 1, AFI_IPV6 = 2 };

    /** Splits an "address/length" string into its address and prefix-length parts. */
    static void parseIpAddress(const char *str, std::string& address, std::string& length);

    /**
     * Returns the number of matching leftmost bits of the two addresses, or -1 for an
     * exact match, or -2 when the address families differ.
     */
    static int doPrefixMatch(L3Address addr1, L3Address addr2);
    static int getNumMatchingPrefixBits4(Ipv4Address addr1, Ipv4Address addr2);
    static int getNumMatchingPrefixBits6(Ipv6Address addr1, Ipv6Address addr2);

    /** Masks the address to the given prefix length. */
    static L3Address getNetworkAddress(L3Address address, int length);
};

} // namespace lisp
} // namespace inet

#endif
