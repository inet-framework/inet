//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021ae;

import inet.queueing.base.PacketFlowBase;

//
// TODO this module is incomplete
//
simple Ieee8021aeTagEpdHeaderInserter extends PacketFlowBase like IIeee8021aeTagEpdHeaderInserter
{
    parameters:
        @class(Ieee8021aeTagEpdHeaderInserter);
        @display("i=block/cogwheel");
}

