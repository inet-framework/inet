//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// This is a base module for various packet puller modules.
//
simple PacketPullerBase extends PacketProcessorBase
{
    parameters:
        @class(PacketPullerBase);
    gates:
        input in;
        output out;
}
