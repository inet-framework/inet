//
// Copyright (C) 2006 Levente Meszaros
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

#ifndef IEEE80211_CONSTS_H
#define IEEE80211_CONSTS_H

// frame lengths in bits
const unsigned int LENGTH_RTS = 160; //bits
const unsigned int LENGTH_CTS = 112; //bits
const unsigned int LENGTH_ACK = 112; //bits
const unsigned int LENGTH_MGMT = 28 * 8; //bits
const unsigned int LENGTH_DATAHDR = 34 * 8; //bits

const unsigned int SNAP_HEADER_BYTES = 8;

// time slot ST, short interframe space SIFS, distributed interframe
// space DIFS, and extended interframe space EIFS

const_simtime_t ST = 20E-6;
const_simtime_t SIFS = 10E-6;
const_simtime_t DIFS = 2 * ST + SIFS;
const_simtime_t MAX_PROPAGATION_DELAY = 2E-6;  // 300 meters at the speed of light

const int RETRY_LIMIT = 7;

/** Minimum size (initial size) of contention window */
const int CW_MIN = 31;

/** Maximum size of contention window */
const int CW_MAX = 1023;

const int HEADER_WITHOUT_PREAMBLE = 48;
const double BITRATE_HEADER = 1E+6;
const double BANDWIDTH = 2E+6;

/** @brief Center frequencies for 802.11b */
const double CENTER_FREQUENCIES[] = {
-1, //channel 0 does not exist
        2.412e9, // 1
        2.417e9, // 2
        2.422e9, // 3
        2.427e9, // 4
        2.432e9, // 5
        2.437e9, // 6
        2.442e9, // 7
        2.447e9, // 8
        2.452e9, // 9
        2.457e9, // 10
        2.462e9, // 11
        2.467e9, // 12
        2.472e9, // 13
        2.484e9, // 14
        };

/** @brief duration of the PHY header
 *
 * If the radio was switched to
 * late, a partially received header is still ok, the total received
 * packet duration can be shorter by this amount compared to the send
 * packet.
 */
const double RED_PHY_HEADER_DURATION = 0.000020;

/* @brief Length of PLCP header and preamble */
const double PHY_HEADER_LENGTH = 192;

#endif

