//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.networklayer.common.L3Address;

namespace inet;

class L3AddressTagBase extends TagBase
{
    L3Address srcAddress;      // may be unspecified
    L3Address destAddress; // always specified
}

//
// Determines the source and destination network addresses for an outgoing packet.
//
class L3AddressReq extends L3AddressTagBase
{
    bool nonLocalSrcAddress = false;
}

//
// Specifies the source and destination network addresses for an incoming packet.
//
class L3AddressInd extends L3AddressTagBase
{
}
