//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.linklayer.ieee802154;

import inet.linklayer.csma.CSMA;

simple Ieee802154NarrowbandMac extends CSMA
{
    parameters:
        useMACAcks = true;

        // Time needed to switch from sleep to rx for ATmega256RFR2
        // SLEEP -> TRX_OFF 240 us
        // TRX_OFF -> RX_ON 110 us
        rxSetupTime = 240us + 110us;

        // aMaxPHYPacketSize = 127 Octets (802.15.4-2006, page 45)
        // aMinMPDUOverhead = 9 Octets (802.15.4-2006, page 159)
        // aMaxMACPayloadSize = aMaxPHYPacketSize - aMinMPDUOverhead (802.15.4-2006, page 159)
        mtu = 127 Byte - 9 Byte;

        // length of MAC header
        // aMinMPDUOverhead = 9 Octets (802.15.4-2006, page 159)
        headerLength = default(72 bit);

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
