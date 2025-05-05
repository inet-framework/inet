//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.server;

import inet.common.Module;
import inet.queueing.contract.IPacketServer;

//
// Implements the given interface and can be used as an optional
// module that removes itself from the module hierarchy during initialization.
//
module OmittedPacketServer extends Module like IPacketServer
{
    parameters:
        @class(::inet::OmittedModule);
        @display("i=block/star,black,50");
    gates:
        input in;
        output out;
    connections:
        in --> out;
}

