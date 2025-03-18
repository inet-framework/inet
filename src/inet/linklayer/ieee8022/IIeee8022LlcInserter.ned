//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8022;

//
// Module interface for IEEE 802.2 LLC header inserters. Defines the interface for
// modules that create and insert LLC headers into packets at the link layer.
// Implementations determine the DSAP and SSAP fields based on the encapsulated
// protocol, and set the control field. Provides a standard interface for different
// LLC header inserter implementations.
//
moduleinterface IIeee8022LlcInserter
{
    parameters:
        @display("i=block/inserter");
    gates:
        input in;
        output out;
}

