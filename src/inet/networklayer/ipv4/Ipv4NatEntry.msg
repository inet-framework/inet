//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.networklayer.contract.ipv4.Ipv4Address;

namespace inet;

class Ipv4NatEntry extends cObject
{
    Ipv4Address destAddress;
    int destPort = -1;
    Ipv4Address srcAddress;
    int srcPort = -1;
}
