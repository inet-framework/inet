//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// This module interface is implemented by traffic conditioner modules. A traffic
// conditioner connects one input to one output. It operates in passive mode on
// its input and in active mode on its output. Packets can be pushed into its
// input and packets can be pulled from its output. A traffic conditioner can
// drop, delay and reorder packets, but it doesn't modify packets in any way.
//
moduleinterface ITrafficConditioner extends IPassivePacketSink, IActivePacketSource
{
    parameters:
        @omittedTypename(OmittedTrafficConditioner);
        @display("i=block/downarrow");
}
