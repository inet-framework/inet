//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.base;

import inet.queueing.base.PacketFlowBase;

//
// This is a base module for various protocol header inserter modules.
//
simple ProtocolHeaderInserterBase extends PacketFlowBase
{
    parameters:
        @class(ProtocolHeaderInserterBase);
}
