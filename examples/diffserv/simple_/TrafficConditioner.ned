//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.diffserv.simple_;

import inet.queueing.contract.ITrafficConditioner;
import inet.networklayer.diffserv.BehaviorAggregateClassifier;
import inet.networklayer.diffserv.DscpMarker;
import inet.networklayer.diffserv.MultiFieldClassifier;
import inet.networklayer.diffserv.SingleRateThreeColorMeter;
import inet.networklayer.diffserv.TokenBucketMeter;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.sink.PassivePacketSink;

//
// TODO documentation
//
module TrafficConditioner like ITrafficConditioner
{
    parameters:
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        @display("i=block/classifier");
    gates:
        input in;
        output out;
    submodules:
        mfClassifier: MultiFieldClassifier {
            filters = xmldoc("filters.xml");
            @display("p=100,250");
        }
        efMeter: TokenBucketMeter {
            @display("p=400,100");
        }
        mux: PacketMultiplexer {
            @display("p=350,300");
        }
        defaultMeter: SingleRateThreeColorMeter {
            @display("p=450,300");
        }
        efMarker: DscpMarker {
            dscps = "EF";
            @display("p=250,100");
        }
        af11Marker: DscpMarker {
            dscps = "AF11";
            @display("p=250,200");
        }
        af21Marker: DscpMarker {
            dscps = "AF21";
            @display("p=250,300");
        }
        af13Marker: DscpMarker {
            dscps = "AF13";
            @display("p=700,350");
        }
        af23Marker: DscpMarker {
            dscps = "AF23";
            @display("p=700,450");
        }
        join: PacketMultiplexer {
            @display("p=850,250");
        }
        beMarker: DscpMarker {
            dscps = "BE";
            @display("p=250,400");
        }
        baClassifier: BehaviorAggregateClassifier {
            dscps = "AF11 AF21";
            @display("p=550,350");
        }
        af11Marker2: DscpMarker {
            dscps = "AF11";
            @display("p=550,100");
        }
        dropper: PassivePacketSink {
            @display("p=550,450");
        }
    connections:
        in --> mfClassifier.in;
        mfClassifier.out++ --> efMarker.in++;
        mfClassifier.out++ --> af11Marker.in++;
        mfClassifier.out++ --> af21Marker.in++;
        mfClassifier.defaultOut --> beMarker.in++;
        efMarker.out --> efMeter.in;
        efMeter.greenOut --> { @display("ls=green"); } --> join.in++;
        efMeter.redOut --> { @display("ls=red"); } --> af11Marker2.in++;
        af11Marker2.out --> join.in++;
        af11Marker.out --> mux.in++;
        af21Marker.out --> mux.in++;
        beMarker.out --> mux.in++;
        mux.out --> defaultMeter.in;
        defaultMeter.greenOut --> { @display("ls=green"); } --> join.in++;
        defaultMeter.yellowOut --> { @display("ls=yellow"); } --> baClassifier.in;
        baClassifier.out++ --> af13Marker.in++;
        baClassifier.out++ --> af23Marker.in++;
        baClassifier.defaultOut --> join.in++;
        af13Marker.out --> join.in++;
        af23Marker.out --> join.in++;
        join.out --> out;
        defaultMeter.redOut --> { @display("ls=red"); } --> dropper.in;
}

