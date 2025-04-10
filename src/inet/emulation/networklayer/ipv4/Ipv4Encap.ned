//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.emulation.networklayer.ipv4;

import inet.common.SimpleModule;

simple Ipv4Encap extends SimpleModule
{
    parameters:
        @class(Ipv4Encap);
        @display("i=block/square");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}
