//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.source;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPacketSource;
import inet.queueing.contract.IPassivePacketSource;

//
// This module is an active/passive empty packet source. It doesn't push packets
// into the connected module, and it doesn't allow packets to be pulled from it
// by the connected module. The output gate generates backpressure for pull packet
// operations.
//
simple EmptyPacketSource extends PacketProcessorBase like IPacketSource, IActivePacketSource, IPassivePacketSource
{
    parameters:
        displayStringTextFormat = default("created 0 pk (0 B)");
        @class(EmptyPacketSource);
        @display("i=block/source");
    gates:
        output out @labels(push,pull);
}
