//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.trafficconditioner;

import inet.queueing.contract.ITrafficConditioner;

//
// Provides bidirectional traffic conditioning functionality as a protocol layer.
// Contains separate traffic conditioners for ingress (incoming) and egress (outgoing)
// packet flows, enabling comprehensive traffic management.
//
// Operation:
// - Processes incoming packets through the ingress conditioner
// - Processes outgoing packets through the egress conditioner
// - Supports configurable traffic conditioner implementations
// - Enables different conditioning policies for each direction
// - Integrates with the interface table for network interface information
//
// @see ~ITrafficConditionerLayer, ~OmittedTrafficConditionerLayer
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
