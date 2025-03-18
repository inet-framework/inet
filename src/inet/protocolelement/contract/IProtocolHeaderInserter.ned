//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.contract;

import inet.queueing.contract.IPacketFlow;

//
// TODO
//
moduleinterface IProtocolHeaderInserter extends IPacketFlow
{
    parameters:
        @display("i=block/inserter");
}

