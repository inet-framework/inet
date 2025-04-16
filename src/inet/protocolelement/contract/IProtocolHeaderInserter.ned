//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.contract;

import inet.queueing.contract.IPacketFlow;

//
// Module interface for protocol header inserter components. Modules that
// implement this interface add protocol-specific headers to packets before
// they are transmitted. These components process packets by prepending or
// appending headers that contain protocol information such as addressing,
// control data, or authentication codes.
//
moduleinterface IProtocolHeaderInserter extends IPacketFlow
{
    parameters:
        @display("i=block/inserter");
}

