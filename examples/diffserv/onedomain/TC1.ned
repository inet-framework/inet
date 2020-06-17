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


//
// Traffic conditioner used in Experiments 1.1-1.6 and 5.1.
//
module TC1 like ITrafficConditioner
{
    parameters:
        int numClasses;
        @display("i=block/classifier");

    gates:
        input in;
        output out;
    submodules:
        classifier: MultiFieldClassifier {
            @display("p=100,100");
        }
        marker: DscpMarker {
            @display("p=250,100");
        }
    connections:
        in --> classifier.in;
        for i=1..numClasses {
            classifier.out++ --> marker.in++;
        }
        classifier.defaultOut --> marker.in++;
        marker.out --> out;
}

