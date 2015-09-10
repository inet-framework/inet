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
import inet.networklayer.diffserv.DSCPMarker;
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
            @display("p=52,296");
        }
        efMeter: TokenBucketMeter {
            @display("p=339,113");
        }
        defaultMeter: SingleRateThreeColorMeter {
            @display("p=339,356");
        }
        efMarker: DSCPMarker {
            dscps = "EF";
            @display("p=204,113");
        }
        af11Marker: DSCPMarker {
            dscps = "AF11";
            @display("p=204,236");
        }
        af21Marker: DSCPMarker {
            dscps = "AF21";
            @display("p=204,356");
        }
        af13Marker: DSCPMarker {
            dscps = "AF13";
            @display("p=556,405");
        }
        af23Marker: DSCPMarker {
            dscps = "AF23";
            @display("p=556,512");
        }
        join: Join {
            @display("p=665,296");
        }
        beMarker: DSCPMarker {
            dscps = "BE";
            @display("p=204,479");
        }
        baClassifier: BehaviorAggregateClassifier {
            dscps = "AF11 AF21";
            @display("p=450,405");
        }
        af11Marker2: DSCPMarker {
            dscps = "AF11";
            @display("p=517,114");
        }
        dropper: Sink {
            @display("p=450,512");
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

