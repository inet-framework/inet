//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.contract;

import inet.queueing.contract.IPacketFilter;

//
// TODO
//
moduleinterface IProtocolHeaderChecker extends IPacketFilter
{
    parameters:
        @display("i=block/checker");
}

