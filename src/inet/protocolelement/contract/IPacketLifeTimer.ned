//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.contract;

//
// Module interface for packet lifetime management components. Modules that
// implement this interface monitor a packet collection and determine when
// packets should be removed from the collection based on specific conditions.
// These modules can be used to implement various packet lifetime policies such
// as dropping packets when a network interface loses carrier, when a timeout
// occurs, or when other network conditions change.
//
moduleinterface IPacketLifeTimer
{
    parameters:
        @display("i=block/timer");
}
