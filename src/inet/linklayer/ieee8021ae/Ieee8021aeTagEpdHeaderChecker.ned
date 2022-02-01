//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021ae;

import inet.queueing.base.PacketFilterBase;

//
// TODO this module is incomplete
//
simple Ieee8021aeTagEpdHeaderChecker extends PacketFilterBase like IIeee8021aeTagEpdHeaderChecker
{
    parameters:
        @class(Ieee8021aeTagEpdHeaderChecker);
        @display("i=block/cogwheel");
}

