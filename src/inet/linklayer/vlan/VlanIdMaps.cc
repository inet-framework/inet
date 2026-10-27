//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/vlan/VlanIdMaps.h"

namespace inet {

static const cValueArray *getVlanIdList(const cComponent *component, const std::string& interfaceName, const cValue& value)
{
    auto array = value.getType() == cValue::POINTER ? dynamic_cast<const cValueArray *>(value.objectValue()) : nullptr;
    if (array == nullptr)
        throw cRuntimeError(component, "Invalid acceptedVlanIds entry for interface '%s': expected a list of VLAN ids, got %s", interfaceName.c_str(), value.str().c_str());
    return array;
}

static const cValueMap *getVlanIdMap(const cComponent *component, const std::string& interfaceName, const cValue& value)
{
    auto map = value.getType() == cValue::POINTER ? dynamic_cast<const cValueMap *>(value.objectValue()) : nullptr;
    if (map == nullptr)
        throw cRuntimeError(component, "Invalid mappedVlanIds entry for interface '%s': expected a map of VLAN ids, got %s", interfaceName.c_str(), value.str().c_str());
    return map;
}

static int parseVlanId(const cComponent *component, const char *parameterName, const std::string& interfaceName, const cValue& value, int minVlanId)
{
    if (value.getType() != cValue::INT || (value.intValue() != -1 && (value.intValue() < minVlanId || value.intValue() > 4094)))
        throw cRuntimeError(component, "Invalid VLAN id %s in the %s entry for interface '%s': expected an integer, -1 or %d to 4094", value.str().c_str(), parameterName, interfaceName.c_str(), minVlanId);
    return value.intValue();
}

static int parseVlanIdKey(const cComponent *component, const std::string& interfaceName, const std::string& key)
{
    char *end = nullptr;
    long vlanId = strtol(key.c_str(), &end, 10);
    if (key.empty() || *end != '\0' || vlanId < -1 || vlanId > 4094 || std::to_string(vlanId) != key)
        throw cRuntimeError(component, "Invalid VLAN id key '%s' in the mappedVlanIds entry for interface '%s': expected the decimal form of -1 or 0 to 4094", key.c_str(), interfaceName.c_str());
    return vlanId;
}

std::map<std::string, std::set<int>> parseAcceptedVlanIds(const cComponent *component, const cValueMap *acceptedVlanIds)
{
    std::map<std::string, std::set<int>> result;
    for (const auto& [interfaceName, value] : acceptedVlanIds->getFields()) {
        auto vlanIds = getVlanIdList(component, interfaceName, value);
        auto& interfaceVlanIds = result[interfaceName];
        for (int i = 0; i < vlanIds->size(); i++)
            interfaceVlanIds.insert(parseVlanId(component, "acceptedVlanIds", interfaceName, vlanIds->get(i), 0));
    }
    return result;
}

std::map<std::string, std::map<int, int>> parseMappedVlanIds(const cComponent *component, const cValueMap *mappedVlanIds, int minMappedVlanId)
{
    std::map<std::string, std::map<int, int>> result;
    for (const auto& [interfaceName, value] : mappedVlanIds->getFields()) {
        auto& interfaceMappedVlanIds = result[interfaceName];
        for (const auto& [key, mappedVlanId] : getVlanIdMap(component, interfaceName, value)->getFields()) {
            int vlanId = parseVlanIdKey(component, interfaceName, key);
            interfaceMappedVlanIds[vlanId] = parseVlanId(component, "mappedVlanIds", interfaceName, mappedVlanId, minMappedVlanId);
        }
    }
    return result;
}

} // namespace inet
