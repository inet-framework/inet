//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.service;

import inet.common.MessageDispatcher;
import inet.protocolelement.forwarding.Forwarding;
import inet.protocolelement.forwarding.ReceiveWithHopLimit;
import inet.protocolelement.forwarding.SendWithHopLimit;

module ForwardingService
{
    parameters:
        @display("i=block/routing");
    gates:
        input upperLayerIn @loose;
        output upperLayerOut @loose;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        d1: MessageDispatcher {
            @display("p=300,100");
        }
        forwarding: Forwarding {
            @display("p=300,200");
        }
        sendWithHopLimit: SendWithHopLimit {
            @display("p=150,300");
        }
        receiveWithHopLimit: ReceiveWithHopLimit {
            @display("p=450,300");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> d1.in++;
        d1.out++ --> forwarding.in;
        forwarding.out --> d1.in++;
        d1.out++ --> sendWithHopLimit.in;
        sendWithHopLimit.out --> { @display("m=s"); } --> lowerLayerOut;
        lowerLayerIn --> { @display("m=s"); } --> receiveWithHopLimit.in;
        receiveWithHopLimit.out --> d1.in++;
        d1.out++ --> { @display("m=n"); } --> upperLayerOut;
}
