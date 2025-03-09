//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.trafficconditioner;

//
// Defines the interface for traffic conditioner protocol layers. Traffic conditioners
// shape, police, mark, or otherwise control network traffic to ensure it conforms
// to specific requirements or policies.
//
// Operation:
// - Processes incoming and outgoing packets
// - May shape traffic by delaying packets
// - May police traffic by dropping non-conforming packets
// - May mark packets for downstream processing
// - Implements Quality of Service (QoS) mechanisms
//
// @see ~OmittedTrafficConditionerLayer, ~TrafficConditionerLayer
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
