//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PacketDuplicatorBase;


//
// This module duplicates packets based on the ordinal number of the packet in
// the received stream of packets.
//
simple OrdinalBasedDuplicator extends PacketDuplicatorBase
{
    parameters:
        string duplicatesVector = default(""); // the duplicatesVector format is "<packetNumber>;<packetNumber>;...;"
                                               // packet numbers in ascending order e.g. "5;6;12;13;"
        @display("i=block/control");
        @class(OrdinalBasedDuplicator);
    gates:
        input in;
        output out;
}

