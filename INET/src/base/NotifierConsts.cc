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

#include <stdio.h>
#include "NotifierConsts.h"


const char *notificationCategoryName(int category)
{
    const char *s;
    static char buf[8];
    switch (category)
    {
        case NF_SUBSCRIBERLIST_CHANGED: return "SUBSCRIBERS";

        case NF_HOSTPOSITION_UPDATED: return "POS";
        case NF_NODE_FAILURE: return "FAILURE";
        case NF_NODE_RECOVERY: return "RECOVERY";

        case NF_RADIOSTATE_CHANGED: return "RADIO-STATE";
        case NF_RADIO_CHANNEL_CHANGED: return "RADIO-CHANNEL";
        case NF_PP_TX_BEGIN: return "TX-BEG";
        case NF_PP_TX_END: return "TX-END";
        case NF_PP_RX_END: return "RX-END";
        case NF_L2_Q_DROP: return "DROP";
        case NF_MAC_BECAME_IDLE: return "MAC-IDLE";
        case NF_L2_BEACON_LOST: return "BEACON-LOST";
        case NF_L2_ASSOCIATED: return "ASSOCIATED";

        case NF_INTERFACE_CREATED: return "IF-CREATED";
        case NF_INTERFACE_DELETED: return "IF-DELETED";
        case NF_INTERFACE_STATE_CHANGED: return "IF-STATE";
        case NF_INTERFACE_CONFIG_CHANGED: return "IF-CFG";
        case NF_INTERFACE_IPv4CONFIG_CHANGED: return "IPv4-CFG";
        case NF_INTERFACE_IPv6CONFIG_CHANGED: return "IPv6-CFG";

        case NF_IPv4_ROUTE_ADDED: return "IPv4-ROUTE-ADD";
        case NF_IPv4_ROUTE_DELETED: return "IPv4-ROUTE-DEL";
        case NF_IPv6_ROUTE_ADDED: return "IPv6-ROUTE-ADD";
        case NF_IPv6_ROUTE_DELETED: return "IPv6-ROUTE-DEL";

        case NF_IPv6_HANDOVER_OCCURRED: return "IPv6-HANDOVER";

        case NF_OVERLAY_TRANSPORTADDRESS_CHANGED: return "OVERLAY-TRANSPORTADDESS";
        case NF_OVERLAY_NODE_LEAVE: return "OVERLAY-NODE-LEAVE";
        case NF_OVERLAY_NODE_GRACEFUL_LEAVE: return "NODE-GRACEFUL-LEAVE";

        default: sprintf(buf, "%d", category); s = buf;
    }
    return s;
}

void printNotificationBanner(int category, const cPolymorphic *details)
{
    EV << "** Notification at T=" << simTime()
       << " to " << simulation.getContextModule()->getFullPath() << ": "
       << notificationCategoryName(category) << " "
       << (details ? details->info() : "") << "\n";
}





