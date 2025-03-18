//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.trafficconditioner;

import inet.queueing.contract.ITrafficConditioner;

//
// Implements a bidirectional traffic conditioner layer with separate conditioners
// for ingress and egress traffic flows. The ingress conditioner processes packets
// coming from the upper layer, while the egress conditioner processes packets
// coming from the lower layer. Each conditioner can independently shape, police,
// and condition traffic according to configured policies.
//
module TrafficConditionerLayer like ITrafficConditionerLayer
{
    parameters:
        @display("i=block/layer");
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        egressConditioner: <default("")> like ITrafficConditioner {
            @display("p=100,100");
        }
        ingressConditioner: <default("")> like ITrafficConditioner {
            @display("p=300,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> ingressConditioner.in;
        ingressConditioner.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> egressConditioner.in;
        egressConditioner.out --> { @display("m=n"); } --> upperLayerOut;
}
