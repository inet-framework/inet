//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for packet duplicator components in the queueing framework.
// Packet duplicators create copies of incoming packets and forward both
// the original packet and its duplicates to the output.
//
// Key features:
// - Creates exact copies of incoming packets
// - Preserves packet data and metadata in duplicates
// - Supports various duplication strategies in derived modules
// - Can be used in both push and pull packet processing modes
//
// Derived modules can implement specific duplication strategies
// by defining how many duplicates to create and how to handle them.
//
// @see ~PacketDuplicator, ~ContentBasedDuplicator
//
simple PacketDuplicatorBase extends PacketProcessorBase
{
    parameters:
        @class(PacketDuplicatorBase);
        @display("i=block/star");
}
