//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.authentication;

import inet.protocolelement.contract.IProtocolHeaderInserter;
import inet.queueing.base.PacketFlowBase;

simple MessageAuthenticationCodeInserter extends PacketFlowBase like IProtocolHeaderInserter
{
    parameters:
        int headerLength @unit(b) = default(4B);
        @class(MessageAuthenticationCodeInserter);
        @display("i=block/inserter");
}
