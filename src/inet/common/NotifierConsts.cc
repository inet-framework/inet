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
#include "inet/common/NotifierConsts.h"

namespace inet {

simsignal_t NF_SUBSCRIBERLIST_CHANGED = cComponent::registerSignal("NF_SUBSCRIBERLIST_CHANGED");

simsignal_t NF_NODE_FAILURE = cComponent::registerSignal("NF_NODE_FAILURE");
simsignal_t NF_NODE_RECOVERY = cComponent::registerSignal("NF_NODE_RECOVERY");

simsignal_t NF_PP_TX_BEGIN = cComponent::registerSignal("NF_PP_TX_BEGIN");
simsignal_t NF_PP_TX_END = cComponent::registerSignal("NF_PP_TX_END");
simsignal_t NF_PP_RX_END = cComponent::registerSignal("NF_PP_RX_END");
simsignal_t NF_TX_ACKED = cComponent::registerSignal("NF_TX_ACKED");
simsignal_t NF_L2_Q_DROP = cComponent::registerSignal("NF_L2_Q_DROP");
simsignal_t NF_MAC_BECAME_IDLE = cComponent::registerSignal("NF_MAC_BECAME_IDLE");
simsignal_t NF_L2_BEACON_LOST = cComponent::registerSignal("NF_L2_BEACON_LOST");
simsignal_t NF_L2_ASSOCIATED = cComponent::registerSignal("NF_L2_ASSOCIATED");
simsignal_t NF_L2_ASSOCIATED_NEWAP = cComponent::registerSignal("NF_L2_ASSOCIATED_NEWAP");
simsignal_t NF_L2_ASSOCIATED_OLDAP = cComponent::registerSignal("NF_L2_ASSOCIATED_OLDAP");
simsignal_t NF_L2_DISASSOCIATED = cComponent::registerSignal("NF_L2_DISASSOCIATED");
simsignal_t NF_L2_AP_ASSOCIATED = cComponent::registerSignal("NF_L2_AP_ASSOCIATED");
simsignal_t NF_L2_AP_DISASSOCIATED = cComponent::registerSignal("NF_L2_AP_DISASSOCIATED");

simsignal_t NF_LINK_BREAK = cComponent::registerSignal("NF_LINK_BREAK");
simsignal_t NF_LINK_PROMISCUOUS = cComponent::registerSignal("NF_LINK_PROMISCUOUS");
simsignal_t NF_LINK_FULL_PROMISCUOUS = cComponent::registerSignal("NF_LINK_FULL_PROMISCUOUS");

simsignal_t NF_INTERFACE_CREATED = cComponent::registerSignal("NF_INTERFACE_CREATED");
simsignal_t NF_INTERFACE_DELETED = cComponent::registerSignal("NF_INTERFACE_DELETED");
simsignal_t NF_INTERFACE_STATE_CHANGED = cComponent::registerSignal("NF_INTERFACE_STATE_CHANGED");
simsignal_t NF_INTERFACE_CONFIG_CHANGED = cComponent::registerSignal("NF_INTERFACE_CONFIG_CHANGED");
simsignal_t NF_INTERFACE_GENERICNETWORKPROTOCOLCONFIG_CHANGED = cComponent::registerSignal("NF_INTERFACE_GENERICNETWORKPROTOCOLCONFIG_CHANGED");
simsignal_t NF_INTERFACE_IPv4CONFIG_CHANGED = cComponent::registerSignal("NF_INTERFACE_IPv4CONFIG_CHANGED");
simsignal_t NF_INTERFACE_IPv6CONFIG_CHANGED = cComponent::registerSignal("NF_INTERFACE_IPv6CONFIG_CHANGED");
simsignal_t NF_TED_CHANGED = cComponent::registerSignal("NF_TED_CHANGED");

simsignal_t NF_ROUTE_ADDED = cComponent::registerSignal("NF_ROUTE_ADDED");
simsignal_t NF_ROUTE_DELETED = cComponent::registerSignal("NF_ROUTE_DELETED");
simsignal_t NF_ROUTE_CHANGED = cComponent::registerSignal("NF_ROUTE_CHANGED");
simsignal_t NF_MROUTE_ADDED = cComponent::registerSignal("NF_MROUTE_ADDED");
simsignal_t NF_MROUTE_DELETED = cComponent::registerSignal("NF_MROUTE_DELETED");
simsignal_t NF_MROUTE_CHANGED = cComponent::registerSignal("NF_MROUTE_CHANGED");

simsignal_t NF_IPv4_MCAST_JOIN = cComponent::registerSignal("NF_IPv4_MCAST_JOIN");
simsignal_t NF_IPv4_MCAST_LEAVE = cComponent::registerSignal("NF_IPv4_MCAST_LEAVE");
simsignal_t NF_IPv4_MCAST_CHANGE = cComponent::registerSignal("NF_IPv4_MCAST_CHANGE");
simsignal_t NF_IPv4_MCAST_REGISTERED = cComponent::registerSignal("NF_IPv4_MCAST_REGISTERED");
simsignal_t NF_IPv4_MCAST_UNREGISTERED = cComponent::registerSignal("NF_IPv4_MCAST_UNREGISTERED");

simsignal_t NF_IPv4_NEW_MULTICAST = cComponent::registerSignal("NF_IPv4_NEW_MULTICAST");
simsignal_t NF_IPv4_NEW_IGMP_ADDED = cComponent::registerSignal("NF_IPv4_NEW_IGMP_ADDED");
simsignal_t NF_IPv4_NEW_IGMP_REMOVED = cComponent::registerSignal("NF_IPv4_NEW_IGMP_REMOVED");
simsignal_t NF_IPv4_DATA_ON_NONRPF = cComponent::registerSignal("NF_IPv4_DATA_ON_NONRPF");
simsignal_t NF_IPv4_DATA_ON_RPF = cComponent::registerSignal("NF_IPv4_DATA_ON_RPF");
simsignal_t NF_IPv4_RPF_CHANGE = cComponent::registerSignal("NF_IPv4_RPF_CHANGE");
simsignal_t NF_IPv4_DATA_ON_RPF_PIMSM = cComponent::registerSignal("NF_IPv4_DATA_ON_RPF_PIMSM");
simsignal_t NF_IPv4_MDATA_REGISTER = cComponent::registerSignal("NF_IPv4_MDATA_REGISTER");
simsignal_t NF_IPv4_NEW_IGMP_ADDED_PISM = cComponent::registerSignal("NF_IPv4_NEW_IGMP_ADDED_PISM");
simsignal_t NF_IPv4_NEW_IGMP_REMOVED_PIMSM = cComponent::registerSignal("NF_IPv4_NEW_IGMP_REMOVED_PIMSM");

simsignal_t NF_PIM_NEIGHBOR_ADDED = cComponent::registerSignal("NF_PIM_NEIGHBOR_ADDED");
simsignal_t NF_PIM_NEIGHBOR_DELETED = cComponent::registerSignal("NF_PIM_NEIGHBOR_DELETED");
simsignal_t NF_PIM_NEIGHBOR_CHANGED = cComponent::registerSignal("NF_PIM_NEIGHBOR_CHANGED");

simsignal_t NF_IPv6_HANDOVER_OCCURRED = cComponent::registerSignal("NF_IPv6_HANDOVER_OCCURRED");
simsignal_t NF_MIPv6_RO_COMPLETED = cComponent::registerSignal("NF_MIPv6_RO_COMPLETED");
simsignal_t NF_IPv6_MCAST_JOIN = cComponent::registerSignal("NF_IPv6_MCAST_JOIN");
simsignal_t NF_IPv6_MCAST_LEAVE = cComponent::registerSignal("NF_IPv6_MCAST_LEAVE");
simsignal_t NF_IPv6_MCAST_REGISTERED = cComponent::registerSignal("NF_IPv6_MCAST_REGISTERED");
simsignal_t NF_IPv6_MCAST_UNREGISTERED = cComponent::registerSignal("NF_IPv6_MCAST_UNREGISTERED");

simsignal_t NF_CLNS_ROUTE_ADDED = cComponent::registerSignal("NF_CLNS_ROUTE_ADDED");
simsignal_t NF_CLNS_ROUTE_DELETED = cComponent::registerSignal("NF_CLNS_ROUTE_DELETED");
simsignal_t NF_CLNS_ROUTE_CHANGED = cComponent::registerSignal("NF_CLNS_ROUTE_CHANGED");

simsignal_t NF_ISIS_ADJ_CHANGED = cComponent::registerSignal("NF_ISIS_ADJ_CHANGED");

simsignal_t NF_OVERLAY_TRANSPORTADDRESS_CHANGED = cComponent::registerSignal("NF_OVERLAY_TRANSPORTADDRESS_CHANGED");
simsignal_t NF_OVERLAY_NODE_GRACEFUL_LEAVE = cComponent::registerSignal("NF_OVERLAY_NODE_GRACEFUL_LEAVE");
simsignal_t NF_OVERLAY_NODE_LEAVE = cComponent::registerSignal("NF_OVERLAY_NODE_LEAVE");
simsignal_t NF_BATTERY_CHANGED = cComponent::registerSignal("NF_BATTERY_CHANGED");
simsignal_t NF_BATTERY_CPUTIME_CONSUMED = cComponent::registerSignal("NF_BATTERY_CPUTIME_CONSUMED");

const char *notificationCategoryName(simsignal_t signalID)
{
    return cComponent::getSignalName(signalID);
}

void printNotificationBanner(simsignal_t signalID, const cObject *obj)
{
    EV << "** Notification at T=" << simTime()
       << " to " << getSimulation()->getContextModule()->getFullPath() << ": "
       << notificationCategoryName(signalID) << " "
       << (obj ? obj->info() : "") << "\n";
}

} // namespace inet

