//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.source;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IActivePacketSource;

//
// This module is an empty packet source which never pushes packets into the
// connected packet consumer.
//
simple EmptyPacketSource extends PacketProcessorBase like IActivePacketSource
{
    parameters:
        displayStringTextFormat = default("created 0 pk (0 B)");
        @class(EmptyPacketSource);
        @display("i=block/source");
    gates:
        output out @labels(push);
}
