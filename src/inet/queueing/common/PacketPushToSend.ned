//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PassivePacketSinkBase;
import inet.queueing.contract.IPassivePacketSink;

//
// Protocol adapter that converts between push-based and send-based packet handling.
// Enables integration between queueing framework modules and traditional OMNeT++ modules.
//
// Operation mechanism:
// - Receives packets via push operations from queueing framework modules
// - Forwards packets using send operations to traditional OMNeT++ modules
// - Acts as a passive packet sink in the queueing framework
// - Serves as a bridge between different module communication paradigms
//
// @see ~PacketSendToPush
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
