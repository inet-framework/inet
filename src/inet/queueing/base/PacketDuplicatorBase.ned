//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base type for packet duplicator implementations. Provides common functionality
// for creating and forwarding multiple copies of incoming packets.
//
simple PacketDuplicatorBase extends PacketProcessorBase
{
    parameters:
        @class(PacketDuplicatorBase);
        @display("i=block/star");
}
