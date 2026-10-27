//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_VLANIDMAPS_H
#define __INET_VLANIDMAPS_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Checks the `acceptedVlanIds` parameter of a VLAN filter: a map from
 * interface name to a list of VLAN ids, each from -1 to 4094. Throws a
 * cRuntimeError on behalf of the component otherwise.
 */
INET_API void checkAcceptedVlanIds(const cComponent *component, const cValueMap *acceptedVlanIds);

/**
 * Checks the `mappedVlanIds` parameter of a VLAN mapper: a map from interface
 * name to a map from VLAN id to VLAN id. A key is the decimal form of a VLAN id
 * from -1 to 4094; a mapped VLAN id is -1, or from minMappedVlanId to 4094.
 * Throws a cRuntimeError on behalf of the component otherwise.
 */
INET_API void checkMappedVlanIds(const cComponent *component, const cValueMap *mappedVlanIds, int minMappedVlanId);

} // namespace inet

#endif
