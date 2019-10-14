//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package inet.examples.diffserv.simple_;

import inet.linklayer.contract.ITrafficConditioner;
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
        *.interfaceTableModule = default(absPath(interfaceTableModule));
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

