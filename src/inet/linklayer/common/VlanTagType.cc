//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/common/VlanTagType.h"

namespace inet {

VlanTagType parseVlanTagType(const char *vlanTagType)
{
    if (!strcmp(vlanTagType, "c"))
        return VLAN_TAG_TYPE_C;
    else if (!strcmp(vlanTagType, "s"))
        return VLAN_TAG_TYPE_S;
    else
        throw cRuntimeError("Unknown VLAN tag type '%s', must be 'c' or 's'", vlanTagType);
}

const Protocol *getVlanTagProtocol(VlanTagType vlanTagType)
{
    switch (vlanTagType) {
        case VLAN_TAG_TYPE_C: return &Protocol::ieee8021qCTag;
        case VLAN_TAG_TYPE_S: return &Protocol::ieee8021qSTag;
        default: throw cRuntimeError("Unknown VLAN tag type %d", vlanTagType);
    }
}

} // namespace inet

