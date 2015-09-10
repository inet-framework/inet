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
     * Local initializations. Initializations that don't use or affect
     * other modules take place (e.g. reading of parameters); modules may
     * subscribe to notifications. NodeStatus, IPassiveQueue,
     * etc. are available for other modules after this stage.
     */
    INITSTAGE_LOCAL = 0,

    /**
     * Physical environment initializations (mobility, obstacles, battery, annotations, etc).
     */
    INITSTAGE_PHYSICAL_ENVIRONMENT = 1,

    /**
     * Additional physical environment initializations that depend on the previous stage.
     * Some mobility modules (namely group mobility) compute and publish locations in this stage,
     * because they learn their mobility coordinator in the previous stage.
     */
    INITSTAGE_PHYSICAL_ENVIRONMENT_2 = 2,

    /**
     * Initialization of the physical layer of protocol stacks. Radio publishes the initial RadioState;
     * radios are registered in RadioMedium.
     */
    INITSTAGE_PHYSICAL_LAYER = 3,

    /**
     * Initialization of link-layer protocols. Automatic MAC addresses are
     * assigned; interfaces are registered in InterfaceTable.
     */
    INITSTAGE_LINK_LAYER = 4,

    /**
     * Additional link-layer initializations that depend on the previous stage.
     */
    INITSTAGE_LINK_LAYER_2 = 5,

    /**
     * Initialization of network-layer protocols, stage 1. Network configurators
     * (e.g. IPv4NetworkConfigurator) run in this stage and compute IP addresses
     * and static routes; protocol-specific data (e.g. IPv4InterfaceData)
     * are added to InterfaceEntry; netf7ilter hooks are registered in IPv4; etc.
     */
    INITSTAGE_NETWORK_LAYER = 6,

    /**
     * Initialization of network-layer protocols, stage 2. IP addresses
     * are assigned in this stage.
     */
    INITSTAGE_NETWORK_LAYER_2 = 7,

    /**
     * Initialization of network-layer protocols, stage 3. Static routes
     * are added, routerIDs are computed, etc.
     */
    INITSTAGE_NETWORK_LAYER_3 = 8,

    /**
     * Initialization of transport-layer protocols. Transport protocols register
     * their protocol IDs in IP, etc.
     */
    INITSTAGE_TRANSPORT_LAYER = 9,

    /**
     * Initialization of transport-layer protocols, 2nd stage. Exists because SCTP
     * may be transported over UDP.
     */
    INITSTAGE_TRANSPORT_LAYER_2 = 10,

    /**
     * Initialization of routing protocols.
     */
    INITSTAGE_ROUTING_PROTOCOLS = 11,

    /**
     * Initialization of applications.
     */
    INITSTAGE_APPLICATION_LAYER = 12,

    /**
     * Operations that no other initializations can depend on, e.g. display string updates.
     */
    INITSTAGE_LAST = 13,

    /**
     * The number of initialization stages.
     */
    NUM_INIT_STAGES,
};

} // namespace inet

#endif    // __INET_INITSTAGES

