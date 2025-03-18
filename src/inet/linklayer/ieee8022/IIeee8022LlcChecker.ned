//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8022;

//
// Module interface for IEEE 802.2 LLC header checkers. Defines the interface for
// modules that process LLC headers in packets at the link layer. Implementations
// validate and remove the LLC header, extract the DSAP and SSAP fields, and
// determine the encapsulated protocol based on these values. Provides a standard
// interface for different LLC header checker implementations.
//
moduleinterface IIeee8022LlcChecker
{
    parameters:
        @display("i=block/checker");
    gates:
        input in;
        output out;
}

