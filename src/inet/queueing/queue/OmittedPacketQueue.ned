//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.queue;

import inet.queueing.contract.IPacketQueue;

//
// This module implements the given module interface and can be used as an optional
// module that removes itself from the module hierarchy during initialization.
//
module OmittedPacketQueue like IPacketQueue
{
    parameters:
        @class(::inet::OmittedModule);
        @display("i=block/queue,black,50");
    gates:
        input in;
        output out;
    connections:
        in --> out;
}
