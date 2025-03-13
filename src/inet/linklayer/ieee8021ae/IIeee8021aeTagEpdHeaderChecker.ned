//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021ae;

//
// Defines the interface for modules that process IEEE 802.1AE MAC Security (MACsec)
// tag headers in Ethernet frames. MACsec provides connectionless data confidentiality,
// frame integrity, and data origin authenticity in Layer 2 networks.
//
// The IEEE 802.1AE standard, also known as MACsec, enables secure communication
// between devices on Ethernet LANs by encrypting and authenticating frames at
// the MAC layer. This protects against various attacks including eavesdropping,
// man-in-the-middle attacks, and replay attacks.
//
// Modules implementing this interface are responsible for:
// - Extracting and processing MACsec tag headers from incoming frames
// - Validating security parameters like Packet Numbers (PN)
// - Processing Secure Channel Identifier (SCI) information
// - Verifying Tag Control Information/Association Number (TCI/AN) values
// - Checking for replay attacks using packet numbering
// - Removing the MACsec tag header before forwarding the frame
//
// The MACsec tag header in EPD (EtherType Protocol Discrimination) format includes
// fields for TCI/AN, Secure Length (SL), Packet Number (PN), and optionally the
// Secure Channel Identifier (SCI).
//
// @see ~Ieee8021aeTagEpdHeaderChecker, ~IIeee8021aeTagEpdHeaderInserter
//
moduleinterface IIeee8021aeTagEpdHeaderChecker
{
    parameters:
        @display("i=block/cogwheel");
    gates:
        input in;
        output out;
}
