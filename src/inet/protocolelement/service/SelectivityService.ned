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

//
// Provides comprehensive packet addressing and filtering services at both
// transport and network layers. Combines protocol elements to handle port
// and address-based selectivity in a layered architecture.
//
// Operation:
// - Transmission path (upper to lower):
//   * Adds port headers to outgoing packets
//   * Adds network layer address headers
//   * Enables addressing at multiple protocol layers
// - Reception path (lower to upper):
//   * Filters packets based on network layer address
//   * Filters packets based on port number
//   * Removes addressing headers from matching packets
//
// @see ~DataService, ~ForwardingService, ~InterfaceService, ~MacService
//
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
