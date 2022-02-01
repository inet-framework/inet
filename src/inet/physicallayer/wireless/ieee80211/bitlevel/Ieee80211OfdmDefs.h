//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#endif

