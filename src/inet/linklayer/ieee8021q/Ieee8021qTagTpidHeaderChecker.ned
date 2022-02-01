//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketFilterBase;

simple Ieee8021qTagTpidHeaderChecker extends PacketFilterBase like IIeee8021qTagTpidHeaderChecker
{
    parameters:
        string vlanTagType @enum("c","s") = default("c");
        object vlanIdFilter = default([]);
        @class(Ieee8021qTagTpidHeaderChecker);
        @display("i=block/checker");
}

