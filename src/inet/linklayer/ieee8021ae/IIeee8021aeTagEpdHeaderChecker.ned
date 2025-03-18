//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8021ae;

//
// Module interface for IEEE 802.1AE (MACsec) tag header checkers. Processes
// incoming frames to verify and extract IEEE 802.1AE security tags with EtherType
// Protocol Discrimination (EPD). Implements packet filtering based on MACsec
// header fields including Secure Channel Identifier (SCI), packet number (PN),
// and other security parameters defined in the IEEE 802.1AE standard.
//
moduleinterface IIeee8021aeTagEpdHeaderChecker
{
    parameters:
        @display("i=block/cogwheel");
    gates:
        input in;
        output out;
}

