//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Interface for active packet source modules in the queueing framework.
// Active packet sources are starting points for packets in push-based
// packet processing, where packet flow is driven by producers sending
// packets to consumers.
//
// Key features:
// - Operates in push mode (active producer)
// - Autonomously generates and pushes packets without external requests
// - Controls the timing and rate of packet generation
// - Initiates packet flow in the network
// - Serves as a starting point for packet flows
// - Determines when packets enter the system
//
// @see ~ActivePacketSource, ~BurstyPacketProducer, ~PcapFilePacketProducer, ~IPassivePacketSink
//
moduleinterface IActivePacketSource
{
    parameters:
        @display("i=block/source");
    gates:
        output out @labels(push);
}
