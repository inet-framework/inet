//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_NOTIFIERCONSTS_H
#define __INET_NOTIFIERCONSTS_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * signals for NotificationBoard
 */
// TODO document associated notification detail structs
extern simsignal_t    // admin
    NF_SUBSCRIBERLIST_CHANGED,

// - host
    NF_NODE_FAILURE,
    NF_NODE_RECOVERY,

// - layer 2 (data-link)
//XXX generalize constants (remove "PP"?) - could be used by 80211 and ethernet as well
// they generally carry TxNotifDetails as "details" to identify the interface and the frame
    NF_PP_TX_BEGIN,    // point-to-point transmission begins (currently PPP)
    NF_PP_TX_END,    // point-to-point transmission ends (currently PPP)
    NF_PP_RX_END,    // point-to-point reception ends (currently PPP)
    NF_TX_ACKED,    // transmitted frame got acked (currently Ieee80211)
    NF_L2_Q_DROP,
    NF_MAC_BECAME_IDLE,
    NF_L2_BEACON_LOST,    // missed several consecutive beacons (currently Ieee80211)
    NF_L2_ASSOCIATED,    // successfully associated with an AP (currently Ieee80211)
    NF_L2_ASSOCIATED_NEWAP,    // successfully associated with an AP (currently Ieee80211)
    NF_L2_ASSOCIATED_OLDAP,
    NF_L2_DISASSOCIATED,    // same as BEACON_LOST but used in higher layers
    NF_L2_AP_ASSOCIATED,    // emitted by the AP, successfully associated with this AP (currently Ieee80211)
    NF_L2_AP_DISASSOCIATED,    // emitted by the AP, successfully disassociated from this AP (currently Ieee80211)

    NF_LINK_BREAK,    // used for manet link layer feedback
    NF_LINK_PROMISCUOUS,    // used for manet promiscuous mode, the packets that have this node how destination are no promiscuous send
    NF_LINK_FULL_PROMISCUOUS,    // Used for manet promiscuous mode, all packets are promiscuous

// - layer 3 (network)
    NF_INTERFACE_CREATED,
    NF_INTERFACE_DELETED,
    NF_INTERFACE_STATE_CHANGED,
    NF_INTERFACE_CONFIG_CHANGED,
    NF_INTERFACE_GENERICNETWORKPROTOCOLCONFIG_CHANGED,
    NF_INTERFACE_IPv4CONFIG_CHANGED,
    NF_INTERFACE_IPv6CONFIG_CHANGED,
    NF_TED_CHANGED,

// layer 3 - Routing Table
    NF_ROUTE_ADDED,
    NF_ROUTE_DELETED,
    NF_ROUTE_CHANGED,
    NF_MROUTE_ADDED,
    NF_MROUTE_DELETED,
    NF_MROUTE_CHANGED,

// layer 3 - IPv4
    NF_IPv4_MCAST_JOIN,
    NF_IPv4_MCAST_LEAVE,
    NF_IPv4_MCAST_CHANGE,
    NF_IPv4_MCAST_REGISTERED,
    NF_IPv4_MCAST_UNREGISTERED,

// for PIM
    NF_IPv4_NEW_MULTICAST,
    NF_IPv4_NEW_MULTICAST_DENSE,
    NF_IPv4_NEW_MULTICAST_SPARSE,
    NF_IPv4_NEW_IGMP_ADDED,
    NF_IPv4_NEW_IGMP_REMOVED,
    NF_IPv4_DATA_ON_NONRPF,
    NF_IPv4_DATA_ON_RPF,
    NF_IPv4_RPF_CHANGE,
    NF_IPv4_DATA_ON_RPF_PIMSM,
    NF_IPv4_MDATA_REGISTER,
    NF_IPv4_NEW_IGMP_ADDED_PISM,
    NF_IPv4_NEW_IGMP_REMOVED_PIMSM,

// layer 3 - IPv6
    NF_IPv6_HANDOVER_OCCURRED,
    NF_MIPv6_RO_COMPLETED,
    NF_IPv6_MCAST_JOIN,
    NF_IPv6_MCAST_LEAVE,
    NF_IPv6_MCAST_REGISTERED,
    NF_IPv6_MCAST_UNREGISTERED,

// layer 3 - CLNS
    NF_CLNS_ROUTE_ADDED,
    NF_CLNS_ROUTE_DELETED,
    NF_CLNS_ROUTE_CHANGED,

    NF_ISIS_ADJ_CHANGED,

// - layer 4 (transport)
//...

// - layer 7 - OverSim
    NF_OVERLAY_TRANSPORTADDRESS_CHANGED,    // OverSim
    NF_OVERLAY_NODE_GRACEFUL_LEAVE,    // OverSim
    NF_OVERLAY_NODE_LEAVE;    // OverSim

// - layer 7 (application)
//...

/**
 * Utility function
 */
const char *notificationCategoryName(simsignal_t signalID);

/**
 * Utility function
 */
void printNotificationBanner(simsignal_t signalID, const cObject *obj);

} // namespace inet

#endif // ifndef __INET_NOTIFIERCONSTS_H

