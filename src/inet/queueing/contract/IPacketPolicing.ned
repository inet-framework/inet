//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Interface for packet policing modules in the queueing framework.
// Packet policing modules enforce traffic policies by metering traffic
// and selectively dropping packets that exceed defined constraints,
// without modifying packet contents or ordering.
//
// Key features:
// - Acts as both a passive packet sink and an active packet source
// - Enforces traffic policies through selective packet dropping
// - Meters traffic characteristics (rate, burst size, etc.)
// - Preserves packet order for non-dropped packets
// - Never modifies packet contents or metadata
// - Operates in push mode (passive sink, active source)
//
// @see ~PacketPolicing, ~MultiPacketPolicing, ~TokenBucket, ~LeakyBucket
//
moduleinterface IPacketPolicing extends IPassivePacketSink, IActivePacketSource
{
    parameters:
        @display("i=block/downarrow");
}
