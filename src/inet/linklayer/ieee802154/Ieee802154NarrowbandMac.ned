//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.ieee802154;

//
// Implements the IEEE 802.15.4 narrowband MAC (Medium Access Control) protocol
// with parameters specifically configured according to the standard specifications.
// This module extends the base ~Ieee802154Mac with appropriate default settings
// for narrowband operation.
//
// The IEEE 802.15.4 narrowband PHY operates in various frequency bands (868 MHz,
// 915 MHz, and 2.4 GHz) with data rates ranging from 20 kbps to 250 kbps. This
// MAC implementation is designed to work with these narrowband physical layers.
//
// Key features and configurations:
// - MAC acknowledgments enabled by default
// - Maximum frame size set to 118 bytes (127 - 9 bytes overhead) per standard
// - Exponential backoff for CSMA/CA with standard-compliant parameters
// - Retry and backoff limits configured according to the IEEE 802.15.4 specification
//
// All timing parameters and configuration values are derived directly from the
// IEEE 802.15.4-2006 standard, providing an accurate implementation for
// simulation of narrowband LR-WPAN networks.
//
// @see ~Ieee802154Mac, ~Ieee802154NarrowbandInterface
//
module Ieee802154NarrowbandMac extends Ieee802154Mac
{
    parameters:
        useMACAcks = true;

        // Time needed to switch from sleep to rx.
        // TODO This is probably wrong, since it is used to compute
        //      the time for the CCA (and that might follow an RX state)
        rxSetupTime = 0s;

        // aMaxPHYPacketSize = 127 Octets (802.15.4-2006, page 45)
        // aMinMPDUOverhead = 9 Octets (802.15.4-2006, page 159)
        // aMaxMACPayloadSize = aMaxPHYPacketSize - aMinMPDUOverhead (802.15.4-2006, page 159)
        mtu = 127 Byte - 9 Byte;

        // length of MAC header
        // aMinMPDUOverhead = 9 Octets (802.15.4-2006, page 159)
        headerLength = default(72 b);

        // Exponential Backoff
        backoffMethod = default("exponential");

        // Maximum number of frame retransmission
        // 802.15.4-2006, page 164
        macMaxFrameRetries = default(3);

        // Maximum number of extra backoffs (excluding the first unconditional one) before frame drop
        // 802.15.4-2006, page 163
        macMaxCSMABackoffs = default(4);

        // Minimum backoff exponent
        // 802.15.4-2006, page 164
        macMinBE = default(3);

        // Maximum backoff exponent
        // 802.15.4-2006, page 163
        macMaxBE = default(5);
}
