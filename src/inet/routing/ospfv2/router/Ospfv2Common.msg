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

class Metric
{
    @existingClass;
    @opaque;
    @byValue;
    @fromString(string2ulong($));
    @toString(ulong2string($));
    @defaultValue(0);
    @toValue((intval_t)$);
    @fromValue($.intValue());
}

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

struct HostRouteParameters
{
    @existingClass;
    @descriptor(readonly);

    unsigned char ifIndex;
    Ipv4Address address;
    Metric linkCost;
};