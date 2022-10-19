//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4layer.ipv4_modular;

import inet.queueing.base.PacketProcessorBase;


module Ipv4DeliveryDecision extends PacketProcessorBase
{
    parameters:
        @class(Ipv4DeliveryDecision);
        string interfaceTableModule;
        string routingTableModule;
        string directBroadcastInterfaces;
        @display("i=block/classifier");
    gates:
        input in;
        output localOut;
        output forwardOut;
}
