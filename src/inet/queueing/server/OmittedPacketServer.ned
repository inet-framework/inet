//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.server;

import inet.queueing.contract.IPacketServer;

//
// This module implements the given interface and can be used as an optional
// module that removes itself from the module hierarchy during initialization.
//
module OmittedPacketServer like IPacketServer
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

