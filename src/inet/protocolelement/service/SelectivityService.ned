//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.service;

import inet.protocolelement.selectivity.ReceiveAtL3Address;
import inet.protocolelement.selectivity.ReceiveAtPort;
import inet.protocolelement.selectivity.SendToL3Address;
import inet.protocolelement.selectivity.SendToPort;

module SelectivityService
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        sendToPort: SendToPort {
            @display("p=150,100");
        }
        sendToL3Address: SendToL3Address {
            @display("p=150,200");
        }
        receiveAtPort: ReceiveAtPort {
            @display("p=350,100");
        }
        receiveAtL3Address: ReceiveAtL3Address {
            @display("p=350,200");
        }
    connections allowunconnected:
        upperLayerIn --> sendToPort.in;
        sendToPort.out --> sendToL3Address.in;
        sendToL3Address.out --> { @display("m=s"); } --> lowerLayerOut;
        lowerLayerIn --> { @display("m=s"); } --> receiveAtL3Address.in;
        receiveAtL3Address.out --> receiveAtPort.in;
        receiveAtPort.out --> upperLayerOut;
}
