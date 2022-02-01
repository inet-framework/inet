//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// This module interface is implemnted by network interfaces.
//
moduleinterface INetworkInterface
{
    parameters:
        @display("i=block/ifcard");
    gates:
        input upperLayerIn;
        output upperLayerOut;
}

