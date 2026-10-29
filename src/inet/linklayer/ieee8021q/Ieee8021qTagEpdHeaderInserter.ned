//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketFlowBase;

simple Ieee8021qTagEpdHeaderInserter extends PacketFlowBase like IIeee8021qTagEpdHeaderInserter
{
    parameters:
        string vlanTagType @enum("c","s") = default("c");
        string nextProtocol = default("ethernetmac");
        int defaultVlanId = default(-1);
        int defaultPcp = default(-1); // PCP if no PCP request sets it; -1: unspecified, the tag gets PCP 0 and no PCP indication
        int defaultDropEligible = default(-1); // <0: unspecified, 0: false, >0: true
        bool mapUserPriorityReqToPcpReq = default(true); // Whether a user priority request sets the PCP if no PCP request of this tag type does
        bool removeUserPriorityReq = default(false); // Whether the user priority request is removed, so the layers below do not get it; requires mapUserPriorityReqToPcpReq
        bool mapPcpIndToUserPriorityInd = default(true); // Whether the PCP of the tag is indicated as the user priority of the frame; in a stack of tags, the last inserter that does it wins
        @class(Ieee8021qTagEpdHeaderInserter);
        @display("i=block/inserter");
}

