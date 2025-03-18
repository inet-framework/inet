//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.socket;

//
// Defines a module interface for socket layer implementations. Socket layers
// provide an abstraction for communication between applications and network
// protocols. They manage socket connections, handle socket commands, and route
// packets between upper and lower protocol layers based on socket bindings.
//
moduleinterface ISocketLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}
