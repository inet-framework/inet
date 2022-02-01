//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// This module interface is implemented by packet policing modules. A packet
// policing connects one input to one output. It operates in passive mode on
// its input and in active mode on its output. These modules usually meter
// various traffic properties and drop packets, but they never modify, delay
// or reorder packets.
//
moduleinterface IPacketPolicing extends IPassivePacketSink, IActivePacketSource
{
    parameters:
        @display("i=block/downarrow");
}
