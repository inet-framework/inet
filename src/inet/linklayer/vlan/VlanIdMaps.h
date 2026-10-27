//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_VLANIDMAPS_H
#define __INET_VLANIDMAPS_H

#include <map>
#include <set>
#include <string>

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Parses the `acceptedVlanIds` parameter of a VLAN filter: a map from
 * interface name to a list of VLAN ids, each an integer from -1 to 4094.
 * Throws a cRuntimeError on behalf of the component otherwise.
 */
INET_API std::map<std::string, std::set<int>> parseAcceptedVlanIds(const cComponent *component, const cValueMap *acceptedVlanIds);

/**
 * Parses the `mappedVlanIds` parameter of a VLAN mapper: a map from interface
 * name to a map from VLAN id to VLAN id. A key is the decimal form of a VLAN id
 * from -1 to 4094; a mapped VLAN id is an integer, -1, or from minMappedVlanId
 * to 4094. Throws a cRuntimeError on behalf of the component otherwise.
 */
INET_API std::map<std::string, std::map<int, int>> parseMappedVlanIds(const cComponent *component, const cValueMap *mappedVlanIds, int minMappedVlanId);

/**
 * Returns the entry of the named interface, or the entry of the "*" interface
 * name if the interface has none, or nullptr if neither exists.
 */
template<typename T>
const T *findInterfaceEntry(const std::map<std::string, T>& entries, const std::string& interfaceName)
{
    auto it = entries.find(interfaceName);
    if (it == entries.end())
        it = entries.find("*");
    return it != entries.end() ? &it->second : nullptr;
}

} // namespace inet

#endif
