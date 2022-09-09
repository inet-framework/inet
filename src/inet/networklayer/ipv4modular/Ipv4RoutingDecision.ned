//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4modular;

module Ipv4RoutingDecision
{
    parameters:
        @class(Ipv4RoutingDecision);
        string interfaceTableModule;
        string routingTableModule;
        string directBroadcastInterfaces;
        string displayStringTextFormat = default("");   // TODO required for PacketProcessorBase
        @display("i=block/classifier");
    gates:
        input in;
        output localOut;
        output forwardOut;
}
