//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021r;

import inet.queueing.base.PacketFilterBase;

simple Ieee8021rTagEpdHeaderChecker extends PacketFilterBase like IIeee8021rTagEpdHeaderChecker
{
    parameters:
        @class(Ieee8021rTagEpdHeaderChecker);
        @display("i=block/checker");
}

