//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.processing;

import inet.queueing.contract.IPacketDelayer;

module DelayChannel
{
    parameters:
        @display("i=block/delay");
    gates:
        input leftIn;
        output leftOut;
        input rightIn;
        output rightOut;
    submodules:
        leftToRight: <default("PacketDelayer")> like IPacketDelayer {
            parameters:
                @display("p=150,150");
        }
        rightToLeft: <default("PacketDelayer")> like IPacketDelayer {
            parameters:
                @display("p=150,300");
        }
    connections:
        leftIn --> { @display("m=w"); } --> leftToRight.in;
        leftToRight.out --> { @display("m=e"); } --> rightOut;

        rightIn --> { @display("m=e"); } --> rightToLeft.in;
        rightToLeft.out --> { @display("m=w"); } --> leftOut;
}

