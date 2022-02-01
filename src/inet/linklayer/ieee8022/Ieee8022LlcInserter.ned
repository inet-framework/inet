//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

import inet.queueing.base.PacketFlowBase;

simple Ieee8022LlcInserter extends PacketFlowBase like IIeee8022LlcInserter
{
    parameters:
        @class(Ieee8022LlcInserter);
        @display("i=block/inserter");
}

