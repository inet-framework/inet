//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

