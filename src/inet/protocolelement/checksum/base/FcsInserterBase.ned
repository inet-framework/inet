//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum.base;

import inet.queueing.base.PacketFlowBase;

simple FcsInserterBase extends PacketFlowBase
{
    parameters:
        string fcsMode @enum("disabled","declared","computed") = default("declared");
        @class(FcsInserterBase);
        @display("i=block/inserter");
}
