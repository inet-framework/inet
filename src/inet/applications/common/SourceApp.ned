//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.common;

import inet.applications.contract.IApp;
import inet.queueing.contract.IActivePacketSource;

//
// Provides a minimal, protocol-independent packet source application that generates
// and sends packets through any transport protocol. Serves as a flexible traffic
// generator for network testing scenarios where only the traffic generation
// behavior is relevant, without implementing any specific application-layer protocol.
//
// The module uses a configurable packet source component that can be customized to
// implement different packet generation behaviors. By default, it uses an active
// packet source that produces packets according to its configuration, but it can be
// replaced with other source implementations that provide different traffic patterns,
// packet contents, or timing characteristics. Designed to be used in conjunction
// with various traffic sinks for network performance testing and evaluation.
//
// @see ~SinkApp, ~IActivePacketSource, ~ActivePacketSource
//
module SourceApp like IApp
{
    parameters:
        @application;
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
    submodules:
        source: <default("ActivePacketSource")> like IActivePacketSource {
            @display("p=150,150");
        }
    connections allowunconnected:
        source.out --> socketOut;
}
