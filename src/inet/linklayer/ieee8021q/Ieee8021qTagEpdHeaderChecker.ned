//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketFilterBase;

simple Ieee8021qTagEpdHeaderChecker extends PacketFilterBase like IIeee8021qTagEpdHeaderChecker
{
    parameters:
        string vlanTagType @enum("c","s") = default("c");
        object vlanIdFilter @mutable = default([]);
        @class(Ieee8021qTagEpdHeaderChecker);
        @display("i=block/checker");
}

