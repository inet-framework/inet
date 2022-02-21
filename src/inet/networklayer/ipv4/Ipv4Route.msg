//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.networklayer.contract.ipv4.Ipv4Address;

cplusplus {{
#include "inet/networklayer/ipv4/Ipv4Route.h"
}}

namespace inet;

class Ipv4Route extends cNamedObject
{
    @existingClass;
    @descriptor(readonly);
    Ipv4Address destination;
    Ipv4Address netmask;
    Ipv4Address gateway;
    int metric;
    int prefixLength;
    string interfaceName;
}
