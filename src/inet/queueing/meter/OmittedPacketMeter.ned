//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.meter;

import inet.queueing.contract.IPacketMeter;

//
// This module implements the module given interface and can be used as an omitted
// optional module that removes itself from the module hierarchy during initialize.
//
module OmittedPacketMeter like IPacketMeter
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

