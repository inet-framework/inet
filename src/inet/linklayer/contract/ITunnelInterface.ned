//
// Copyright (C) 2015 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// This module interface is implemnted by tunnel network interfaces.
//
moduleinterface ITunnelInterface extends INetworkInterface
{
    parameters:
        @display("i=block/ifcard");
}

