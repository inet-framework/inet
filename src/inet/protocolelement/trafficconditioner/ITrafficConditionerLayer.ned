//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.trafficconditioner;

//
// Defines a module interface for traffic conditioner layer implementations.
// Traffic conditioner layers provide mechanisms for controlling packet flow
// through the network stack. They can shape, police, and condition traffic
// by applying various policies for dropping, delaying, or reordering packets
// in both ingress and egress directions.
//
moduleinterface ITrafficConditionerLayer
{
    parameters:
        @omittedTypename(OmittedTrafficConditionerLayer);
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

