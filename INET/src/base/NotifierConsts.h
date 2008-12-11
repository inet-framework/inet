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

#include "INETDefs.h"


/**
 * Category constants for NotificationBoard
 */
// TODO consider: use allocated IDs, like: const int NF_FOO = registerCategory("FOO");
// or maybe use structs and dynamic_cast? so we can have a hierarchy of notifications
// TODO document associated notification detail structs
enum
{
    // admin
    NF_SUBSCRIBERLIST_CHANGED,

    // - host
    NF_HOSTPOSITION_UPDATED,
    NF_NODE_FAILURE,
    NF_NODE_RECOVERY,

    // - layer 1 (physical)
    NF_RADIOSTATE_CHANGED,
    NF_RADIO_CHANNEL_CHANGED,

    // - layer 2 (data-link)
    //XXX generalize constants (remove "PP"?) - could be used by 80211 and ethernet as well
    // they generally carry TxNotifDetails as "details" to identify the interface and the frame
    NF_PP_TX_BEGIN,   // point-to-point transmission begins (currently PPP)
    NF_PP_TX_END,     // point-to-point transmission ends (currently PPP)
    NF_PP_RX_END,     // point-to-point reception ends (currently PPP)
    NF_TX_ACKED,      // transmitted frame got acked (currently Ieee80211)
    NF_L2_Q_DROP,
    NF_MAC_BECAME_IDLE,
    NF_L2_BEACON_LOST,   // missed several consecutive beacons (currently Ieee80211)
    NF_L2_ASSOCIATED,    // successfully associated with an AP (currently Ieee80211)

    // - layer 3 (network)
    NF_INTERFACE_CREATED,
    NF_INTERFACE_DELETED,
    NF_INTERFACE_STATE_CHANGED,
    NF_INTERFACE_CONFIG_CHANGED,
    NF_INTERFACE_IPv4CONFIG_CHANGED,
    NF_INTERFACE_IPv6CONFIG_CHANGED,
    NF_TED_CHANGED,

    // layer 3 - IPv4
    NF_IPv4_ROUTE_ADDED,
    NF_IPv4_ROUTE_DELETED,
    NF_IPv6_ROUTE_ADDED,
    NF_IPv6_ROUTE_DELETED,

    // layer 3 - IPv6
    NF_IPv6_HANDOVER_OCCURRED,

    // - layer 4 (transport)
    //...

    // - layer 7 - OverSim
    NF_OVERLAY_TRANSPORTADDRESS_CHANGED,  // OverSim
    NF_OVERLAY_NODE_GRACEFUL_LEAVE,       // OverSim
    NF_OVERLAY_NODE_LEAVE,                // OverSim


    // - layer 7 (application)
    //...
};

/**
 * Utility function
 */
const char *notificationCategoryName(int category);

/**
 * Utility function
 */
void printNotificationBanner(int category, const cPolymorphic *details);

#endif




