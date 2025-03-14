//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

//
// Interface for modules that create and insert IEEE 802.2 SNAP (SubNetwork Access Protocol) headers 
// into outgoing packets. Defines the contract for components that prepare packets from upper layers 
// for transmission using SNAP headers, which extend LLC headers with a 5-byte protocol identifier 
// (3-byte OUI and 2-byte protocol type) to support a wider range of protocol identification.
//
// @see ~Ieee8022SnapInserter, ~IIeee8022SnapChecker, ~IIeee8022LlcInserter
//
moduleinterface IIeee8022SnapInserter
{
    parameters:
        @display("i=block/inserter");
    gates:
        input in;
        output out;
}
