//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8022;

//
// Module interface for IEEE 802.2 SNAP (SubNetwork Access Protocol) header
// checker implementations. Defines a component that validates and processes
// SNAP headers in LLC frames. Implementations check if incoming packets have
// valid SNAP headers with recognized protocol identifiers, extract protocol
// information, and forward valid packets while dropping invalid ones.
//
moduleinterface IIeee8022SnapChecker
{
    parameters:
        @display("i=block/checker");
    gates:
        input in;
        output out;
}

