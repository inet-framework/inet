//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PassivePacketSinkBase;
import inet.queueing.contract.IPassivePacketSink;

simple PacketPushToSend extends PassivePacketSinkBase like IPassivePacketSink
{
    parameters:
        @class(PacketPushToSend);
        @display("i=block/cogwheel");
    gates:
        input in @labels(push);
        output out @labels(push);
}
