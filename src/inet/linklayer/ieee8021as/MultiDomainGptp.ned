//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021as;

import inet.linklayer.contract.IGptp;
import inet.queueing.classifier.PacketClassifier;
import inet.queueing.common.PacketMultiplexer;

//
// Implements support for multiple IEEE 802.1AS time synchronization domains within
// a single network device. This module enables a device to participate in several
// independent time domains simultaneously, each with its own time reference.
//
// Multiple time domains are useful in scenarios where:
// - Different applications require different time references
// - Network segments need to be isolated for redundancy or security
// - Hierarchical time distribution is required
// - Different timing accuracy requirements exist within the same network
//
// The module operates by:
// - Creating a separate gPTP instance for each time domain
// - Associating each gPTP instance with a corresponding subclock
// - Routing incoming gPTP messages to the appropriate domain based on the domain number
// - Multiplexing outgoing gPTP messages from all domains
//
// Each time domain operates independently with its own:
// - Grandmaster clock
// - Synchronization hierarchy
// - Timing parameters
// - Domain number identifier
//
// This implementation requires a ~MultiClock module that provides separate subclock
// instances for each time domain. The module automatically configures each gPTP
// instance to use the corresponding subclock.
//
// @see ~MultiClock, ~Gptp, ~GptpMaster, ~GptpSlave, ~GptpBridge
//
module MultiDomainGptp like IGptp
{
    parameters:
        string clockModule; // Relative module path of the multi clock
        string interfaceTableModule; // Relative module path of the interface table
        int numDomains; // Number of time synchronization domains
        string gptpNodeType; // @enum("GptpNodeType"): MASTER_NODE, BRIDGE_NODE, SLAVE_NODE
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
    submodules:
        domain[numDomains]: <default("Gptp")> like IGptp {
            gptpNodeType = default(parent.gptpNodeType);
            clockModule = default(absPath(parent.clockModule) + ".clock[" + string(this.index) + "]");
            interfaceTableModule = default(absPath(parent.interfaceTableModule));
            domainNumber = default(this.index);
            @display("p=150,150,row,150");
        }
        multiplexer: PacketMultiplexer {
            @display("p=150,350");
        }
        classifier: PacketClassifier {
            classifierClass = default("inet::GptpDomainNumberClassifier");
            @display("p=300,350");
        }
    connections:
        for i=0..numDomains-1
        {
            domain[i].socketOut --> multiplexer.in++;
            classifier.out++ --> domain[i].socketIn;
        }

        socketIn --> { @display("m=s"); } --> classifier.in;
        multiplexer.out --> { @display("m=s"); } --> socketOut;
}
