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
        *.interfaceTableModule = default(absPath(interfaceTableModule));
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

