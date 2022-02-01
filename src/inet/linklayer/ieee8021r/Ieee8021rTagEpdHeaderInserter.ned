//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021r;

import inet.queueing.base.PacketFlowBase;

simple Ieee8021rTagEpdHeaderInserter extends PacketFlowBase like IIeee8021rTagEpdHeaderInserter
{
    parameters:
        string nextProtocol = default("ieee8021qctag");
        @class(Ieee8021rTagEpdHeaderInserter);
        @display("i=block/inserter");
}

