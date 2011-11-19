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

const int PHY_HEADER_LENGTH = 192;
const int HEADER_WITHOUT_PREAMBLE = 48;
const double BITRATE_HEADER = 1E+6;
const double BANDWIDTH = 2E+6;

#endif

