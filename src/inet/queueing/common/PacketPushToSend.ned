//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PassivePacketSinkBase;
import inet.queueing.contract.IPassivePacketSink;

//
// Acts as an adapter between the queueing framework and standard OMNeT++
// modules. It receives packets through the push mechanism (as a passive packet
// sink) and forwards them using the standard OMNeT++ send operation. This
// module is useful for connecting queueing components to non-queueing modules
// that expect to receive packets via message sending.
//
simple PacketPushToSend extends PassivePacketSinkBase like IPassivePacketSink
{
    parameters:
        @class(PacketPushToSend);
        @display("i=block/cogwheel");
    gates:
        input in @labels(push);
        output out @labels(push);
}
