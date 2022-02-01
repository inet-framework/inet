//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IActivePacketSource;

simple PacketSendToPush extends PacketProcessorBase like IActivePacketSource
{
    parameters:
        @class(PacketSendToPush);
        @display("i=block/cogwheel");
    gates:
        input in @labels(push);
        output out @labels(push);
}
