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

import inet.common.queue.Join;
import inet.common.queue.Sink;
import inet.linklayer.contract.ITrafficConditioner;
import inet.networklayer.diffserv.BehaviorAggregateClassifier;
import inet.networklayer.diffserv.DscpMarker;
import inet.networklayer.diffserv.MultiFieldClassifier;
import inet.networklayer.diffserv.SingleRateThreeColorMeter;
import inet.networklayer.diffserv.TokenBucketMeter;


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
        defaultMeter: SingleRateThreeColorMeter {
            @display("p=400,300");
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
        join: Join {
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
        dropper: Sink {
            @display("p=550,450");
        }
    connections:
        in --> mfClassifier.in;
        mfClassifier.outs++ --> efMarker.in++;
        mfClassifier.outs++ --> af11Marker.in++;
        mfClassifier.outs++ --> af21Marker.in++;
        mfClassifier.defaultOut --> beMarker.in++;
        efMarker.out --> efMeter.in++;
        efMeter.greenOut --> { @display("ls=green"); } --> join.in++;
        efMeter.redOut --> { @display("ls=red"); } --> af11Marker2.in++;
        af11Marker2.out --> join.in++;
        af11Marker.out --> defaultMeter.in++;
        af21Marker.out --> defaultMeter.in++;
        beMarker.out --> defaultMeter.in++;
        defaultMeter.greenOut --> { @display("ls=green"); } --> join.in++;
        defaultMeter.yellowOut --> { @display("ls=yellow"); } --> baClassifier.in;
        baClassifier.outs++ --> af13Marker.in++;
        baClassifier.outs++ --> af23Marker.in++;
        baClassifier.defaultOut --> join.in++;
        af13Marker.out --> join.in++;
        af23Marker.out --> join.in++;
        join.out --> out;
        defaultMeter.redOut --> { @display("ls=red"); } --> dropper.in++;
}

