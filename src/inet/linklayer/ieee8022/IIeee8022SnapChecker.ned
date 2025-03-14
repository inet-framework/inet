//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

//
// Interface for modules that validate and process IEEE 802.2 SNAP (SubNetwork Access Protocol) headers 
// in incoming packets. Defines the contract for components that extract protocol information from SNAP 
// headers, which extend LLC headers with a 5-byte protocol identifier (3-byte OUI and 2-byte protocol type). 
// Implementations typically validate header structure, remove SNAP headers, and attach protocol tags.
//
// @see ~Ieee8022SnapChecker, ~IIeee8022SnapInserter, ~IIeee8022LlcChecker
//
moduleinterface IIeee8022SnapChecker
{
    parameters:
        @display("i=block/checker");
    gates:
        input in;
        output out;
}
