//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// This is a base module for various packet meter modules. Derived modules must
// implement a single meterPacket() function which meters the flow of and attaches
// the required tags.
//
// @see ~IPacketMeter
//
simple PacketMeterBase extends PacketFlowBase
{
    parameters:
        displayStringTextFormat = default("metered %p pk (%l)");
        @display("i=block/star");
        @class(PacketMeterBase);
}
