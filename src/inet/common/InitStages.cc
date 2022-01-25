//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/common/InitStages.h"

#include <algorithm>

namespace inet {

Define_InitStage(LOCAL);

Define_InitStage(CLOCK);
Define_InitStage_Dependency(CLOCK, LOCAL);

Define_InitStage(PHYSICAL_ENVIRONMENT);
Define_InitStage_Dependency(PHYSICAL_ENVIRONMENT, LOCAL);

Define_InitStage(PHYSICAL_OBJECT_CACHE);
Define_InitStage_Dependency(PHYSICAL_OBJECT_CACHE, PHYSICAL_ENVIRONMENT);

Define_InitStage(GROUP_MOBILITY);
Define_InitStage_Dependency(GROUP_MOBILITY, LOCAL);

Define_InitStage(SINGLE_MOBILITY);
Define_InitStage_Dependency(SINGLE_MOBILITY, GROUP_MOBILITY);

Define_InitStage(POWER);
Define_InitStage_Dependency(POWER, LOCAL);

Define_InitStage(PHYSICAL_LAYER);
Define_InitStage_Dependency(PHYSICAL_LAYER, POWER);
Define_InitStage_Dependency(PHYSICAL_LAYER, GROUP_MOBILITY);
Define_InitStage_Dependency(PHYSICAL_LAYER, PHYSICAL_ENVIRONMENT);

Define_InitStage(PHYSICAL_LAYER_NEIGHBOR_CACHE);
Define_InitStage_Dependency(PHYSICAL_LAYER_NEIGHBOR_CACHE, PHYSICAL_LAYER);

Define_InitStage(NETWORK_INTERFACE_CONFIGURATION);
Define_InitStage_Dependency(NETWORK_INTERFACE_CONFIGURATION, LOCAL);

Define_InitStage(QUEUEING);
Define_InitStage_Dependency(QUEUEING, PHYSICAL_LAYER);

Define_InitStage(LINK_LAYER);
Define_InitStage_Dependency(LINK_LAYER, PHYSICAL_LAYER);

Define_InitStage(NETWORK_CONFIGURATION);
Define_InitStage_Dependency(NETWORK_CONFIGURATION, LINK_LAYER);

Define_InitStage(NETWORK_ADDRESS_ASSIGNMENT);
Define_InitStage_Dependency(NETWORK_ADDRESS_ASSIGNMENT, NETWORK_CONFIGURATION);

Define_InitStage(ROUTER_ID_ASSIGNMENT);
Define_InitStage_Dependency(ROUTER_ID_ASSIGNMENT, NETWORK_ADDRESS_ASSIGNMENT);

Define_InitStage(STATIC_ROUTING);
Define_InitStage_Dependency(STATIC_ROUTING, ROUTER_ID_ASSIGNMENT);

Define_InitStage(NETWORK_LAYER);
Define_InitStage_Dependency(NETWORK_LAYER, STATIC_ROUTING);

Define_InitStage(NETWORK_LAYER_PROTOCOLS);
Define_InitStage_Dependency(NETWORK_LAYER_PROTOCOLS, NETWORK_LAYER);

Define_InitStage(TRANSPORT_LAYER);
Define_InitStage_Dependency(TRANSPORT_LAYER, NETWORK_LAYER_PROTOCOLS);

Define_InitStage(ROUTING_PROTOCOLS);
Define_InitStage_Dependency(ROUTING_PROTOCOLS, TRANSPORT_LAYER);

Define_InitStage(APPLICATION_LAYER);
Define_InitStage_Dependency(APPLICATION_LAYER, ROUTING_PROTOCOLS);

Define_InitStage(LAST);
Define_InitStage_Dependency(LAST, APPLICATION_LAYER);

} // namespace inet

