//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IActivePacketSource;

//
// Protocol adapter that converts between send-based and push-based packet handling.
// Enables integration between traditional OMNeT++ modules and queueing framework modules.
//
// Operation mechanism:
// - Receives packets via send operations from traditional OMNeT++ modules
// - Forwards packets using push operations to queueing framework modules
// - Acts as an active packet source in the queueing framework
// - Serves as a bridge between different module communication paradigms
//
// @see ~PacketPushToSend
//
simple PacketSendToPush extends PacketProcessorBase like IActivePacketSource
{
    parameters:
        @class(PacketSendToPush);
        @display("i=block/cogwheel");
    gates:
        input in @labels(push);
        output out @labels(push);
}
