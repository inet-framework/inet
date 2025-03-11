//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.socket;

//
// Defines the interface for socket layer modules that handle socket-based
// communication in protocol stacks. Socket layers typically manage connections,
// addressing, and data transfer between applications and network protocols.
//
// Operation:
// - Processes packets from upper layers (applications) and lower layers (protocols)
// - Manages socket connections and their states
// - Handles addressing and multiplexing between applications and protocols
// - Provides socket-specific services like connection establishment and termination
// - Implements flow control and buffering mechanisms
//
// Particularly useful for:
// - Building modular protocol stacks with socket-based communication
// - Implementing transport layer protocols like TCP and UDP
// - Creating custom socket implementations for specialized protocols
// - Supporting multiple application connections over shared network interfaces
// - Developing protocol gateways with socket-based interfaces
//
// @see ~OmittedSocketLayer
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
