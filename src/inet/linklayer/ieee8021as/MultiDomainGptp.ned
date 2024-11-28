//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021as;

import inet.linklayer.contract.IGptp;
import inet.queueing.classifier.PacketClassifier;
import inet.queueing.common.PacketMultiplexer;
import inet.linklayer.ieee8021as.HotStandby;

//
// This module combines multiple ~Gptp modules, one per time domain into a multi
// time domain time synchronization module. Each gPTP time domain is automatically
// configured to use the corresponding subclock of the clock passed into this
// module.
//
// @see ~MultiClock
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
        hotstandby: HotStandby {
            @display("p=300,150");
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

