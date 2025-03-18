//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8022;

import inet.queueing.base.PacketFilterBase;

//
// Validates and processes SNAP (SubNetwork Access Protocol) headers in IEEE 802.2
// LLC frames. Checks if incoming packets have valid SNAP headers with recognized
// protocol identifiers. Extracts protocol information from the headers, adds
// appropriate protocol tags to packets, and forwards them. Drops packets with
// unknown protocols. Maps between OUI (Organizationally Unique Identifier) and
// protocol ID values to actual protocol objects.
//
simple Ieee8022SnapChecker extends PacketFilterBase like IIeee8022SnapChecker
{
    parameters:
        @class(Ieee8022SnapChecker);
        @display("i=block/checker");
        @signal[packetDropped](type=inet::Packet);
}

