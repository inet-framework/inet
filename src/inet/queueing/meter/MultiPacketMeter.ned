//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.meter;

import inet.queueing.contract.IPacketMeter;

//
// This module combines multiple packet meters into one.
//
module MultiPacketMeter like IPacketMeter
{
    parameters:
        int numMeters;
        @display("i=block/star");
    gates:
        input in @labels(send,push,pull,pass,stream);
        output out @labels(send,push,pull,pass,stream);
    submodules:
        meter[numMeters]: <> like IPacketMeter {
            @display("p=150,150,row,150");
        }
    connections:
        in --> { @display("m=w"); } --> meter[0].in;
        for i=1..numMeters-1 {
            meter[i - 1].out --> meter[i].in;
        }
        meter[numMeters - 1].out --> { @display("m=e"); } --> out;
}
