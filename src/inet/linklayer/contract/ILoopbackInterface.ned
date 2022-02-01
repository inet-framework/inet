//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// This module interface is implemented by loopback network interfaces.
//
moduleinterface ILoopbackInterface extends INetworkInterface
{
    parameters:
        @display("i=block/ifcard");
}

