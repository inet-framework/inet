//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.source;

import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPacketServer;
import inet.queueing.contract.IPassivePacketSource;
import inet.queueing.tokengenerator.QueueBasedTokenGenerator;

//
// This module produces packets in order to prevent a queue from becoming empty.
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
        tokenGenerator: QueueBasedTokenGenerator {
            parameters:
                storageModule = default("^.server");
                @display("p=100,100");
        }
    connections:
        provider.out --> server.in;
        server.out --> { @display("m=s"); } --> out;
}
