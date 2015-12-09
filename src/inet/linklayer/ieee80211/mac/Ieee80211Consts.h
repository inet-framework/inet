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

#ifndef __INET_IEEE80211CONSTS_H
#define __INET_IEEE80211CONSTS_H

namespace inet {

namespace ieee80211 {

// time slot ST, short interframe space SIFS, distributed interframe
// space DIFS, and extended interframe space EIFS

const_simtime_t ST = 20E-6;
const_simtime_t SIFS = 10E-6;
const_simtime_t DIFS = 2 * ST + SIFS;
const_simtime_t MAX_PROPAGATION_DELAY = 2E-6;    // 300 meters at the speed of light

const int RETRY_LIMIT = 7;

/** Minimum size (initial size) of contention window */
const int CW_MIN = 31;

/** Maximum size of contention window */
const int CW_MAX = 1023;

const int HEADER_WITHOUT_PREAMBLE = 48;
const double BITRATE_HEADER = 1E+6;
const double BANDWIDTH = 2E+6;

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

} // namespace ieee80211

} // namespace inet

#endif // ifndef __INET_IEEE80211CONSTS_H

