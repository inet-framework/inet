//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.sink;

import inet.queueing.base.PassivePacketSinkBase;
import inet.queueing.contract.IPassivePacketSink;

//
// This module is a full packet sink which cannot be pushed with packets from
// the connected packet source.
//
simple FullPacketSink extends PassivePacketSinkBase like IPassivePacketSink
{
    parameters:
        @class(FullPacketSink);
        @display("i=block/sink");
    gates:
        input in @labels(push);
}
