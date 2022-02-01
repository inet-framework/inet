//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// This is a base module for various packet marker modules. Derived modules must
// implement a single markPacket() function which marks the individual packets
// by attaching tags.
//
// @see ~IPacketMarker, ~PacketLabeler
//
simple PacketMarkerBase extends PacketFlowBase
{
    parameters:
        displayStringTextFormat = default("marked %p pk (%l)");
        @class(PacketMarkerBase);
}
