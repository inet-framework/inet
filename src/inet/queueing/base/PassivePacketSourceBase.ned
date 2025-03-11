//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for passive packet source components in the queueing framework.
// Passive packet sources wait for packets to be pulled from them by active
// packet sinks, as opposed to actively pushing packets to sinks.
//
// Passive sources operate in pull mode, where they generate packets on demand
// when active sinks request them. This is in contrast to active sources
// that operate in push mode, where they initiate packet transfers.
//
// Key features:
// - Generates packets when requested by active sinks
// - Provides packets with configurable content and attributes
// - Collects statistics on generated packets
// - Supports on-demand packet creation for efficient resource usage
//
// This module is particularly useful for:
// - Modeling network endpoints that respond to requests
// - Implementing packet generators that produce traffic on demand
// - Creating data sources for pull-based communication patterns
// - Simulating server responses in client-server architectures
//
// @see ~PacketSourceBase, ~PassivePacketSource, ~IPassivePacketSource
//
simple PassivePacketSourceBase extends PacketSourceBase
{
    parameters:
        @class(PassivePacketSourceBase);
        @signal[packetPulled](type=inet::Packet);
        // the statistical value is the packet
        @statistic[packets](title="packets"; source=packetPulled; record=count; unit=pk);
        // the statistical value is the length of the packet
        @statistic[packetLengths](title="packet lengths"; source=packetLength(packetPulled); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the packets
        @statistic[dataRate](title="data rate"; source=throughput(packetPulled); record=vector; unit=bps; interpolationmode=linear);
}
