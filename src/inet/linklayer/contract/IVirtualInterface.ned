//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// This module interface is implemnted by virtual network interfaces.
//
moduleinterface IVirtualInterface extends INetworkInterface
{
    parameters:
        @display("i=block/tunnel");
}

