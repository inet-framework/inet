//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketFlowBase;

simple Ieee8021qTagTpidHeaderInserter extends PacketFlowBase like IIeee8021qTagTpidHeaderInserter
{
    parameters:
        string vlanTagType @enum("c","s") = default("c");
        string nextProtocol = default("ethernetmac");
        int defaultVlanId = default(-1);
        int defaultPcp = default(-1);
        int defaultUserPriority = default(-1);
        @class(Ieee8021qTagTpidHeaderInserter);
        @display("i=block/inserter");
}

