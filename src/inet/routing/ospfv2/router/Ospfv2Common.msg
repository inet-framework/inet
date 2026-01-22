//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.networklayer.contract.ipv4.Ipv4Address;

cplusplus{{
#include "inet/routing/ospfv2/router/Ospfv2Common.h"
}}

namespace inet::ospfv2;

struct DesignatedRouterId
{
    @existingClass;
    @descriptor(readonly);

    Ipv4Address routerID;
    Ipv4Address ipInterfaceAddress;
};

struct Ipv4AddressRange
{
    @existingClass;
    @descriptor(readonly);
    @opaque;
    @toString(.str());
};
