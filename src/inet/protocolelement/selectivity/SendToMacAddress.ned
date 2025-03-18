//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

//
// Adds a destination MAC address header to packets. The header contains either
// the configured destination address or the address from the packet's
// MacAddressReq tag. Used by the corresponding ReceiveAtMacAddress module to
// filter packets.
//
simple SendToMacAddress
{
    parameters:
        string address = default("");
        @display("i=block/tx");
    gates:
        input in;
        output out;
}
