//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.queueing.base;

//
// Base module for packet duplicator implementations. Provides common functionality
// for creating and forwarding multiple copies of incoming packets. Derived
// modules can implement specific duplication strategies by overriding the
// getNumPacketDuplicates method.
//
simple PacketDuplicatorBase extends PacketProcessorBase
{
    parameters:
        @class(PacketDuplicatorBase);
        @display("i=block/star");
}
