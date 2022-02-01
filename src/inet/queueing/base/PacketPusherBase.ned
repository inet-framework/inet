//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// This is a base module for various packet pusher modules.
//
simple PacketPusherBase extends PacketProcessorBase
{
    parameters:
        @class(PacketPusherBase);
    gates:
        input in;
        output out;
}
