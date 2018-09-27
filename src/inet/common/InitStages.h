//
// Copyright (C) 2013 OpenSim Ltd.
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
// author: Zoltan Bojthe
//

#ifndef __INET_INITSTAGES
#define __INET_INITSTAGES

namespace inet {

/**
 * Initialization stages.
 */
enum InitStages {
    /**
     * Initialization of local state that don't use or affect other modules takes
     * place (e.g. reading of parameters); modules may subscribe to notifications.
     */
    INITSTAGE_LOCAL = 0,

    /**
     * Initialization of the physical environment.
     */
    INITSTAGE_PHYSICAL_ENVIRONMENT = 1,

    /**
     * Initialization of the cache of physical objects present in the physical environment.
     */
    INITSTAGE_PHYSICAL_OBJECT_CACHE = 2,

    /**
     * Initialization stage of power model: energy storage, energy consumer, energy generator, and energy management models.
     */
    INITSTAGE_POWER = 3,

    /**
     * Initialization of mobility modules regarding position and orientation.
     */
    INITSTAGE_MOBILITY = 4,

    /**
     * Initialization of the physical layer of protocol stacks.
     * Radio publishes the initial RadioState; radios are registered in RadioMedium.
     */
    INITSTAGE_PHYSICAL_LAYER = 5,

    /**
     * Initialization of network interfaces, automatic MAC address assignment and
     * network interfaces are registered in the InterfaceTable.
     */
    INITSTAGE_NETWORK_INTERFACE_CONFIGURATION = 6,

    /**
     * Initialization of link-layer protocols.
     */
    INITSTAGE_LINK_LAYER = 7,

    /**
     * Initialization of network configurators (e.g. Ipv4NetworkConfigurator).
     * Network configurators compute IP addresses and static routes;
     * protocol-specific data (e.g. Ipv4InterfaceData) are added to InterfaceEntry;
     * netfilter hooks are registered in Ipv4; etc.
     */
    INITSTAGE_NETWORK_CONFIGURATOR = 9,

    /**
     * Initialization of network addresses.
     */
    INITSTAGE_NETWORK_ADDRESS_ASSIGNMENT = 10,

    /**
     * Initialization of static routing.
     */
    INITSTAGE_STATIC_ROUTING = 11,

    /**
     * Initialization of network layer protocols.
     */
    INITSTAGE_NETWORK_LAYER = 12,

    /**
     * Initialization of transport-layer protocols.
     */
    INITSTAGE_TRANSPORT_LAYER = 13,

    /**
     * Initialization of routing protocols.
     */
    INITSTAGE_ROUTING_PROTOCOLS = 14,

    /**
     * Initialization of applications.
     */
    INITSTAGE_APPLICATION_LAYER = 15,

    /**
     * Operations that no other initializations can depend on, e.g. display string updates.
     */
    INITSTAGE_LAST = 16,

    /**
     * The number of initialization stages.
     */
    NUM_INIT_STAGES = 17,
};

} // namespace inet

#endif    // __INET_INITSTAGES

