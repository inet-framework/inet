//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum.base;

import inet.queueing.base.PacketFlowBase;

simple CrcInserterBase extends PacketFlowBase
{
    parameters:
        string crcMode @enum("disabled","declared","computed") = default("declared");
        @class(CrcInserterBase);
        @display("i=block/inserter");
}
