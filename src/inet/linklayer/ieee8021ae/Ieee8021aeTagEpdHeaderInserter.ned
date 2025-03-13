//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021ae;

import inet.queueing.base.PacketFlowBase;

//
// Implements the IEEE 802.1AE MAC Security (MACsec) tag header insertion functionality.
// This module adds MACsec headers to outgoing Ethernet frames to provide data
// confidentiality, integrity, and authenticity at the MAC layer.
//
// Note: This implementation is currently incomplete and serves as a placeholder
// for future development of full MACsec functionality. The current version provides
// the basic structure for header insertion but does not implement all security
// features defined in the IEEE 802.1AE standard.
//
// When fully implemented, this module will:
// - Add MACsec tag headers with appropriate security parameters
// - Generate and manage packet numbers for replay protection
// - Handle secure channel identification
// - Support encryption and integrity protection
// - Manage security associations
//
// The MACsec tag header in EPD (EtherType Protocol Discrimination) format includes
// fields for TCI/AN, SL, PN, and optionally SCI, which are essential for the
// security services provided by IEEE 802.1AE.
//
// @see ~IIeee8021aeTagEpdHeaderInserter, ~Ieee8021aeTagEpdHeaderChecker
//
simple Ieee8021aeTagEpdHeaderInserter extends PacketFlowBase like IIeee8021aeTagEpdHeaderInserter
{
    parameters:
        @class(Ieee8021aeTagEpdHeaderInserter);
        @display("i=block/cogwheel");
}
