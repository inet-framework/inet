//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8022;

//
// Module interface for IEEE 802.2 SNAP (SubNetwork Access Protocol) header
// inserter implementations. Defines a component that adds SNAP headers to
// packets in LLC frames. Implementations examine protocol tags in incoming
// packets to determine appropriate OUI and protocol ID values for the SNAP
// header, then create and insert headers at the front of packets.
//
moduleinterface IIeee8022SnapInserter
{
    parameters:
        @display("i=block/inserter");
    gates:
        input in;
        output out;
}

