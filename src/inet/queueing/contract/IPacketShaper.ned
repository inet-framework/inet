//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// This module interface is implemented by packet shaper modules. A packet
// shaper connects one input to one output. It operates in passive mode both
// on its input and its output. These modules may delay and reorder packets
// but they never modify or drop packets.
//
moduleinterface IPacketShaper extends IPassivePacketSink, IActivePacketSource
{
    parameters:
        @display("i=block/bucket");
}
