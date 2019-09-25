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

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * This enum provides constants for initialization stages for modules overriding
 * cComponent::initialize(int stage). The stage numbering is not necessarily
 * sequential, because several initialization stages don't depend on each other.
 */
enum InitStages {
    /**
     * Initialization of local state that don't use or affect other modules includes:
     *  - initializing member variables
     *  - initializing statistic collection
     *  - reading module parameters
     *  - reading configuration files
     *  - adding watches
     *  - looking up other modules without actually using them
     *  - subscribing to module signals
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
     * Initialization of group mobility modules: calculating the initial position and orientation.
     */
    INITSTAGE_GROUP_MOBILITY = 1,

    /**
     * Initialization of single mobility modules: calculating the initial position and orientation.
     */
    INITSTAGE_SINGLE_MOBILITY = 2,

    /**
     * Initialization of the power model: energy storage, energy consumer, energy generator, and energy management modules.
     */
    INITSTAGE_POWER = 1,

    /**
     * Initialization of queueing modules.
     */
    INITSTAGE_QUEUEING = 2,

    /**
     * Initialization of physical layer protocols includes:
     *  - registering radios in the RadioMedium
     *  - initializing radio mode, transmission and reception states
     */
    INITSTAGE_PHYSICAL_LAYER = 2,

    /**
     * Initialization of physical layer neighbor cache.
     */
    INITSTAGE_PHYSICAL_LAYER_NEIGHBOR_CACHE = 3,

    /**
     * Initialization of network interfaces includes:
     *  - assigning MAC addresses
     *  - registering network interfaces in the InterfaceTable
     */
    INITSTAGE_NETWORK_INTERFACE_CONFIGURATION = 2,

    /**
     * Initialization of link-layer protocols.
     */
    INITSTAGE_LINK_LAYER = 3,

    /**
     * Initialization of network configuration (e.g. Ipv4NetworkConfigurator) includes:
     *  - determining IP addresses and static routes
     *  - adding protocol-specific data (e.g. Ipv4InterfaceData) to InterfaceEntry
     */
    INITSTAGE_NETWORK_CONFIGURATION = 4,

    /**
     * Initialization of network addresses.
     */
    INITSTAGE_NETWORK_ADDRESS_ASSIGNMENT = 5,

    /**
     * Initialization of network addresses.
     */
    INITSTAGE_ROUTER_ID_ASSIGNMENT = 6,

    /**
     * Initialization of static routing.
     */
    INITSTAGE_STATIC_ROUTING = 7,

    /**
     * Initialization of network layer protocols.
     */
    INITSTAGE_NETWORK_LAYER = 8,

    /**
     * Initialization of transport-layer protocols.
     */
    INITSTAGE_TRANSPORT_LAYER = 9,

    /**
     * Initialization of routing protocols.
     */
    INITSTAGE_ROUTING_PROTOCOLS = 10,

    /**
     * Initialization of applications.
     */
    INITSTAGE_APPLICATION_LAYER = 11,

    /**
     * Operations that no other initializations can depend on, e.g. display string updates.
     */
    INITSTAGE_LAST = 12,

    /**
     * The number of initialization stages.
     */
    NUM_INIT_STAGES = 13,
};

} // namespace inet

#endif    // __INET_INITSTAGES

