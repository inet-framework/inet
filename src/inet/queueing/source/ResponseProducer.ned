//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.source;

import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.server.TokenBasedServer;
import inet.queueing.tokengenerator.SignalBasedTokenGenerator;

//
// This module is capable of generating several different streams of packets,
// one after the other. For each traffic category, the packets are produced by
// a separate token based server. The traffic stream can be started by adding
// tokens the corresponding server module. The packet data, packet length, and
// timing distributions can be configured in the corresponding packet provider.
//
// @see ~RequestConsumer
//
module ResponseProducer like IActivePacketSource
{
    parameters:
        int numKind;
        string requestConsumerModule = default("^.sink");
        @display("i=block/source");
    gates:
        output out;
    submodules:
        provider[numKind]: PassivePacketSource {
            @display("p=300,100,row,200");
        }
        server[numKind]: TokenBasedServer {
            @display("p=300,225,row,200");
        }
        multiplexer: PacketMultiplexer {
            @display("p=300,350");
        }
        tokenGenerator: SignalBasedTokenGenerator {
            storageModule = default("^." + parent.requestConsumerModule + ".server");
            signals = default("tokensDepleted");
            @display("p=100,100");
        }
    connections:
        for i=0..sizeof(provider) - 1 {
            provider[i].out --> server[i].in;
            server[i].out --> multiplexer.in++;
        }
        multiplexer.out --> { @display("m=s"); } --> out;
}
