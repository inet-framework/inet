//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.diffserv.onedomain;

import inet.queueing.contract.ITrafficConditioner;
import inet.networklayer.diffserv.DscpMarker;
import inet.networklayer.diffserv.MultiFieldClassifier;
import inet.networklayer.diffserv.TokenBucketMeter;
import inet.queueing.common.PacketMultiplexer;

//
// Traffic conditioner used in Experiment 3.1.
//
module TC2 like ITrafficConditioner
{
    parameters:
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        @display("i=block/classifier");
    gates:
        input in;
        output out;
    submodules:
        classifier: MultiFieldClassifier {
            filters = xml("<filters><filter gate=\"0\"/></filters>");
            @display("p=100,200");
        }
        meter: TokenBucketMeter {
            @display("p=250,100");
        }
        marker: DscpMarker {
            @display("p=400,100");
        }
        join: PacketMultiplexer {
            @display("p=550,200");
        }
    connections:
        in --> classifier.in;
        classifier.out++ --> meter.in;
        classifier.defaultOut --> join.in++;
        meter.greenOut --> marker.in++;
        meter.redOut --> marker.in++;
        marker.out --> join.in++;
        join.out --> out;
}

