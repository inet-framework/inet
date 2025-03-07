//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.source;

import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPacketServer;
import inet.queueing.contract.IPassivePacketSource;
import inet.queueing.contract.ITokenGenerator;


//
// This module implements an adaptive packet source that monitors a queue's
// occupancy and generates packets as needed to prevent the queue from becoming
// empty. It helps maintain a minimum level of traffic and ensures continuous
// packet availability for downstream consumers.
//
// Key features:
// - Monitors queue occupancy through a token generator
// - Produces packets only when the queue needs replenishment
// - Adapts packet production rate to queue consumption rate
// - Prevents queue starvation while minimizing unnecessary packet generation
// - Configurable packet provider for different packet types and characteristics
//
// Operation mechanism:
// 1. The tokenGenerator monitors the target queue's occupancy
// 2. When the queue level drops below a threshold, tokens are generated
// 3. These tokens enable the server to request packets from the provider
// 4. The provider creates packets which are then sent to the queue
// 5. This feedback loop maintains the queue at the desired occupancy level
//
// @see ~QueueBasedTokenGenerator, ~TokenBasedServer, ~PassivePacketSource
//
module QueueFiller like IActivePacketSource
{
    parameters:
        @display("i=block/source");
    gates:
        output out @labels(pull);
    submodules:
        provider: <default("PassivePacketSource")> like IPassivePacketSource {
            parameters:
                providingInterval = default(0s);
                @display("p=250,100");
        }
        server: <default("TokenBasedServer")> like IPacketServer {
            parameters:
                @display("p=250,200");
        }
        tokenGenerator: <default("QueueBasedTokenGenerator")> like ITokenGenerator {
            parameters:
                storageModule = default("^.server");
                @display("p=100,100");
        }
    connections:
        provider.out --> server.in;
        server.out --> { @display("m=s"); } --> out;
}
