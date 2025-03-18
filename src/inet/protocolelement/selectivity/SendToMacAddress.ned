//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.selectivity;

import inet.common.SimpleModule;

//
// Adds a destination MAC address header to packets. The header contains either
// the configured destination address or the address from the packet's
// MacAddressReq tag. Used by the corresponding ReceiveAtMacAddress module to
// filter packets.
//
simple SendToMacAddress extends SimpleModule
{
    parameters:
        @class(SendToMacAddress);
        string address = default("");
        @display("i=block/tx");
    gates:
        input in;
        output out;
}
