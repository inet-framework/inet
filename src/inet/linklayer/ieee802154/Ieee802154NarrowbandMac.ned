//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.ieee802154;

module Ieee802154NarrowbandMac extends Ieee802154Mac
{
    parameters:
        useMACAcks = true;

        // Time needed to switch from sleep to rx.
        // TODO This is probabily wrong, since it is used to compute
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

