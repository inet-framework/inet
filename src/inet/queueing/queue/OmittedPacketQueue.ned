//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.queue;

import inet.common.SimpleModule;
import inet.queueing.contract.IPacketQueue;

//
// Implements the given module interface and can be used as an optional
// module that removes itself from the module hierarchy during initialization.
//
module OmittedPacketQueue extends SimpleModule like IPacketQueue
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
