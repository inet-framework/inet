//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021ae;

import inet.queueing.base.PacketFilterBase;

//
// Implements the IEEE 802.1AE MAC Security (MACsec) tag header checking functionality.
// This module processes MACsec headers in incoming Ethernet frames to verify data
// confidentiality, integrity, and authenticity at the MAC layer.
//
// Note: This implementation is currently incomplete and serves as a placeholder
// for future development of full MACsec functionality. The current version provides
// the basic structure for header checking but does not implement all security
// features defined in the IEEE 802.1AE standard.
//
// When fully implemented, this module will:
// - Process MACsec tag headers and extract security parameters
// - Verify packet numbers for replay protection
// - Validate secure channel identification
// - Support decryption and integrity verification
// - Manage security associations
//
// The MACsec tag header in EPD (EtherType Protocol Discrimination) format includes
// fields for TCI/AN, SL, PN, and optionally SCI, which are essential for the
// security services provided by IEEE 802.1AE.
//
// @see ~IIeee8021aeTagEpdHeaderChecker, ~Ieee8021aeTagEpdHeaderInserter
//
simple Ieee8021aeTagEpdHeaderChecker extends PacketFilterBase like IIeee8021aeTagEpdHeaderChecker
{
    parameters:
        @class(Ieee8021aeTagEpdHeaderChecker);
        @display("i=block/cogwheel");
}
