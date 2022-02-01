//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211HrDsssMode.h"

namespace inet {

namespace physicallayer {

Ieee80211HrDsssPreambleMode::Ieee80211HrDsssPreambleMode(const Ieee80211HrDsssPreambleType preambleType) :
    preambleType(preambleType)
{
}

Ieee80211HrDsssHeaderMode::Ieee80211HrDsssHeaderMode(const Ieee80211HrDsssPreambleType preambleType) :
    preambleType(preambleType)
{
}

Ieee80211HrDsssDataMode::Ieee80211HrDsssDataMode(bps bitrate) :
    bitrate(bitrate)
{
}

const simtime_t Ieee80211HrDsssDataMode::getDuration(b length) const
{
    return (simtime_t)(lrint(ceil((double)length.get() / bitrate.get() * 1E+6))) / 1E+6;
}

Ieee80211HrDsssMode::Ieee80211HrDsssMode(const char *name, const Ieee80211HrDsssPreambleMode *preambleMode, const Ieee80211HrDsssHeaderMode *headerMode, const Ieee80211HrDsssDataMode *dataMode) :
    Ieee80211ModeBase(name),
    preambleMode(preambleMode),
    headerMode(headerMode),
    dataMode(dataMode)
{
}

// preamble modes
const Ieee80211HrDsssPreambleMode Ieee80211HrDsssCompliantModes::hrDsssPreambleMode1MbpsLongPreamble(IEEE80211_HRDSSS_PREAMBLE_TYPE_LONG);
const Ieee80211HrDsssPreambleMode Ieee80211HrDsssCompliantModes::hrDsssPreambleMode1MbpsShortPreamble(IEEE80211_HRDSSS_PREAMBLE_TYPE_SHORT);

// header modes
const Ieee80211HrDsssHeaderMode Ieee80211HrDsssCompliantModes::hrDsssHeaderMode1MbpsLongPreamble(IEEE80211_HRDSSS_PREAMBLE_TYPE_LONG);
const Ieee80211HrDsssHeaderMode Ieee80211HrDsssCompliantModes::hrDsssHeaderMode2MbpsShortPreamble(IEEE80211_HRDSSS_PREAMBLE_TYPE_SHORT);

// data modes
const Ieee80211HrDsssDataMode Ieee80211HrDsssCompliantModes::hrDsssDataMode1MbpsLongPreamble(Mbps(1));
const Ieee80211HrDsssDataMode Ieee80211HrDsssCompliantModes::hrDsssDataMode2MbpsLongPreamble(Mbps(2));
const Ieee80211HrDsssDataMode Ieee80211HrDsssCompliantModes::hrDsssDataMode2MbpsShortPreamble(Mbps(2));
const Ieee80211HrDsssDataMode Ieee80211HrDsssCompliantModes::hrDsssDataMode5_5MbpsCckLongPreamble(Mbps(5.5));
const Ieee80211HrDsssDataMode Ieee80211HrDsssCompliantModes::hrDsssDataMode5_5MbpsPbccLongPreamble(Mbps(5.5));
const Ieee80211HrDsssDataMode Ieee80211HrDsssCompliantModes::hrDsssDataMode5_5MbpsCckShortPreamble(Mbps(5.5));
const Ieee80211HrDsssDataMode Ieee80211HrDsssCompliantModes::hrDsssDataMode11MbpsCckLongPreamble(Mbps(11));
const Ieee80211HrDsssDataMode Ieee80211HrDsssCompliantModes::hrDsssDataMode11MbpsPbccLongPreamble(Mbps(11));
const Ieee80211HrDsssDataMode Ieee80211HrDsssCompliantModes::hrDsssDataMode11MbpsCckShortPreamble(Mbps(11));
const Ieee80211HrDsssDataMode Ieee80211HrDsssCompliantModes::hrDsssDataMode11MbpsPbccShortPreamble(Mbps(11));

// modes
const Ieee80211HrDsssMode Ieee80211HrDsssCompliantModes::hrDsssMode1MbpsLongPreamble("hrDsssMode1MbpsLongPreamble", &hrDsssPreambleMode1MbpsLongPreamble, &hrDsssHeaderMode1MbpsLongPreamble, &hrDsssDataMode1MbpsLongPreamble);

const Ieee80211HrDsssMode Ieee80211HrDsssCompliantModes::hrDsssMode2MbpsLongPreamble("hrDsssMode2MbpsLongPreamble", &hrDsssPreambleMode1MbpsLongPreamble, &hrDsssHeaderMode1MbpsLongPreamble, &hrDsssDataMode2MbpsLongPreamble);
const Ieee80211HrDsssMode Ieee80211HrDsssCompliantModes::hrDsssMode2MbpsShortPreamble("hrDsssMode2MbpsShortPreamble", &hrDsssPreambleMode1MbpsShortPreamble, &hrDsssHeaderMode2MbpsShortPreamble, &hrDsssDataMode2MbpsShortPreamble);

const Ieee80211HrDsssMode Ieee80211HrDsssCompliantModes::hrDsssMode5_5MbpsCckLongPreamble("hrDsssMode5_5MbpsCckLongPreamble", &hrDsssPreambleMode1MbpsLongPreamble, &hrDsssHeaderMode1MbpsLongPreamble, &hrDsssDataMode5_5MbpsCckLongPreamble);
const Ieee80211HrDsssMode Ieee80211HrDsssCompliantModes::hrDsssMode5_5MbpsPbccLongPreamble("hrDsssMode5_5MbpsPbccLongPreamble", &hrDsssPreambleMode1MbpsLongPreamble, &hrDsssHeaderMode1MbpsLongPreamble, &hrDsssDataMode5_5MbpsPbccLongPreamble);
const Ieee80211HrDsssMode Ieee80211HrDsssCompliantModes::hrDsssMode5_5MbpsCckShortPreamble("hrDsssMode5_5MbpsCckShortPreamble", &hrDsssPreambleMode1MbpsShortPreamble, &hrDsssHeaderMode2MbpsShortPreamble, &hrDsssDataMode5_5MbpsCckShortPreamble);

const Ieee80211HrDsssMode Ieee80211HrDsssCompliantModes::hrDsssMode11MbpsCckLongPreamble("hrDsssMode11MbpsCckLongPreamble", &hrDsssPreambleMode1MbpsLongPreamble, &hrDsssHeaderMode1MbpsLongPreamble, &hrDsssDataMode11MbpsCckLongPreamble);
const Ieee80211HrDsssMode Ieee80211HrDsssCompliantModes::hrDsssMode11MbpsPbccLongPreamble("hrDsssMode11MbpsPbccLongPreamble", &hrDsssPreambleMode1MbpsLongPreamble, &hrDsssHeaderMode1MbpsLongPreamble, &hrDsssDataMode11MbpsPbccLongPreamble);
const Ieee80211HrDsssMode Ieee80211HrDsssCompliantModes::hrDsssMode11MbpsCckShortPreamble("hrDsssMode11MbpsCckShortPreamble", &hrDsssPreambleMode1MbpsShortPreamble, &hrDsssHeaderMode2MbpsShortPreamble, &hrDsssDataMode11MbpsCckShortPreamble);
const Ieee80211HrDsssMode Ieee80211HrDsssCompliantModes::hrDsssMode11MbpsPbccShortPreamble("hrDsssMode11MbpsPbccShortPreamble", &hrDsssPreambleMode1MbpsShortPreamble, &hrDsssHeaderMode2MbpsShortPreamble, &hrDsssDataMode11MbpsPbccShortPreamble);

const simtime_t Ieee80211HrDsssMode::getRifsTime() const
{
    return -1;
}

} // namespace physicallayer

} // namespace inet

