//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021ae;

//
// Defines the interface for modules that insert IEEE 802.1AE MAC Security (MACsec)
// tag headers into Ethernet frames. MACsec provides connectionless data confidentiality,
// frame integrity, and data origin authenticity in Layer 2 networks.
//
// The IEEE 802.1AE standard, also known as MACsec, enables secure communication
// between devices on Ethernet LANs by encrypting and authenticating frames at
// the MAC layer. This protects against various attacks including eavesdropping,
// man-in-the-middle attacks, and replay attacks.
//
// Modules implementing this interface are responsible for:
// - Adding MACsec tag headers to outgoing frames
// - Generating and managing security parameters like Packet Numbers (PN)
// - Handling Secure Channel Identifier (SCI) information
// - Setting appropriate Tag Control Information/Association Number (TCI/AN) values
// - Setting the correct EtherType for the next protocol in the stack
//
// The MACsec tag header in EPD (EtherType Protocol Discrimination) format includes
// fields for TCI/AN, Secure Length (SL), Packet Number (PN), and optionally the
// Secure Channel Identifier (SCI).
//
// @see ~Ieee8021aeTagEpdHeaderInserter, ~IIeee8021aeTagEpdHeaderChecker
//
moduleinterface IIeee8021aeTagEpdHeaderInserter
{
    parameters:
        @display("i=block/cogwheel");
    gates:
        input in;
        output out;
}
