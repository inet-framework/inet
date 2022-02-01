//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

import inet.queueing.base.PacketFlowBase;

simple Ieee8022SnapInserter extends PacketFlowBase like IIeee8022SnapInserter
{
    parameters:
        @class(Ieee8022SnapInserter);
        @display("i=block/inserter");
}

