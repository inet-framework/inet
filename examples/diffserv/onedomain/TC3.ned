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

import inet.base.Join;
import inet.base.Sink;
import inet.linklayer.ITrafficConditioner;
import inet.networklayer.diffserv.MultiFieldClassifier;
import inet.networklayer.diffserv.TokenBucketMeter;


//
// Traffic conditioner used in Experiment 3.2.
//
module TC3 like ITrafficConditioner
{
    @display("i=block/classifier");

    gates:
        input in;
        output out;
    submodules:
        classifier: MultiFieldClassifier {
            filters = xml("<filters><filter gate=\"0\"/></filters>");
            @display("p=40,186");
        }
        meter: TokenBucketMeter {
            @display("p=162,111");
        }
        sink: Sink {
            @display("p=271,60");
        }
        join: Join {
            @display("p=284,186");
        }
    connections:
        in --> classifier.in;
        classifier.outs++ --> meter.in++;
        classifier.defaultOut --> join.in++;
        meter.redOut --> sink.in++;
        meter.greenOut --> join.in++;
        join.out --> out;
}

