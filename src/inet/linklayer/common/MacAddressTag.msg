//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.linklayer.common.MacAddress;

namespace inet;

class MacAddressTagBase extends TagBase
{
    MacAddress srcAddress;      // may be unspecified
    MacAddress destAddress; // always specified
}

//
// Determines the source and destination MAC addresses for an outgoing packet.
//
class MacAddressReq extends MacAddressTagBase
{
}

//
// Specifies the source and destination MAC addresses for an incoming packet.
//
class MacAddressInd extends MacAddressTagBase
{
}
