//
// Copyright (C) 2014 OpenSim Ltd.
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

#ifndef __INET_IEEE80211OFDMDEFS_H
#define __INET_IEEE80211OFDMDEFS_H

namespace inet {

namespace physicallayer {

#define NUMBER_OF_OFDM_DATA_SUBCARRIERS    48
#define DECODED_SIGNAL_FIELD_LENGTH        24
#define ENCODED_SIGNAL_FIELD_LENGTH        48
#define SIGNAL_RATE_FIELD_START            0
#define SIGNAL_RATE_FIELD_END              3
#define SIGNAL_LENGTH_FIELD_START          5
#define SIGNAL_LENGTH_FIELD_END            16
#define SIGNAL_PARITY_FIELD                17
#define PPDU_SERVICE_FIELD_BITS_LENGTH     16
#define PPDU_TAIL_BITS_LENGTH              6
} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211OFDMDEFS_H

