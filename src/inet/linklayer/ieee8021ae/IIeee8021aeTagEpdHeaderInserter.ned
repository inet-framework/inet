//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8021ae;

//
// Module interface for IEEE 802.1AE (MACsec) tag header inserters. Processes
// outgoing frames to add IEEE 802.1AE security tags with EtherType Protocol
// Discrimination (EPD). Encapsulates packet data in encrypted chunks and adds
// security parameters including Secure Channel Identifier (SCI) and packet
// number (PN) as defined in the IEEE 802.1AE standard for MAC security.
//
moduleinterface IIeee8021aeTagEpdHeaderInserter
{
    parameters:
        @display("i=block/cogwheel");
    gates:
        input in;
        output out;
}
