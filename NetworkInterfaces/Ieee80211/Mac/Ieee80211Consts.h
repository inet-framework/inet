//
// Copyright (C) 2006 Levente Mészáros
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef IEEE80211_CONSTS_H
#define IEEE80211_CONSTS_H

//frame lengths in bits
//XXX this is duplicate, it's already in Ieee80211Frame.msg
const unsigned int LENGTH_RTS = 160;
const unsigned int LENGTH_CTS = 112;
const unsigned int LENGTH_ACK = 112;

//time slot ST, short interframe space SIFS, distributed interframe
//space DIFS, and extended interframe space EIFS
const double ST = 20E-6;
const double SIFS = 10E-6;
const double DIFS = 2*ST + SIFS;

const int RETRY_LIMIT = 7;

/** Minimum size (initial size) of contention window */
const int CW_MIN = 7;

/** Maximum size of contention window */
const int CW_MAX = 255;

const int PHY_HEADER_LENGTH=192;
const int HEADER_WITHOUT_PREAMBLE=48;
const double BITRATE_HEADER=1E+6;
const double BANDWIDTH=2E+6;

/** FIXME comment: what's this? */
const double PROCESSING_TIMEOUT = 0.001;

#endif
