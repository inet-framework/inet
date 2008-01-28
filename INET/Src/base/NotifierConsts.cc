//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include <stdio.h>
#include "NotifierConsts.h"


const char *notificationCategoryName(int category)
{
    const char *s;
    static char buf[8];
    switch (category)
    {
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

        case NF_INTERFACE_STATE_CHANGED: return "IFACE";
        case NF_INTERFACE_CONFIG_CHANGED: return "IFACE-CFG";

        case NF_IPv4_INTERFACECONFIG_CHANGED: return "IPv4-CFG";
        case NF_IPv4_ROUTINGTABLE_CHANGED: return "ROUTINGTABLE";

        case NF_IPv6_INTERFACECONFIG_CHANGED: return "IPv6-CFG";
        case NF_IPv6_ROUTINGTABLE_CHANGED: return "IPv6-ROUTINGTABLE";
        case NF_IPv6_HANDOVER_OCCURRED: return "IPv6-HANDOVER";
        default: sprintf(buf, "%d", category); s = buf;
    }
    return s;
}

void printNotificationBanner(int category, cPolymorphic *details)
{
    EV << "** Notification at T=" << simulation.simTime() 
       << " to " << simulation.contextModule()->fullPath() << ": " 
       << notificationCategoryName(category) << " " 
       << (details ? details->info() : "") << "\n";
}





