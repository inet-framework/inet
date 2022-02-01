//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STATIONRETRYCOUNTERS_H
#define __INET_STATIONRETRYCOUNTERS_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

class INET_API StationRetryCounters
{
  protected:
    int stationShortRetryCount = 0;
    int stationLongRetryCount = 0;

  public:
    int getStationLongRetryCount() const { return stationLongRetryCount; }
    int getStationShortRetryCount() const { return stationShortRetryCount; }

    void resetStationShortRetryCount() { stationShortRetryCount = 0; }
    void resetStationLongRetryCount() { stationLongRetryCount = 0; }

    void incrementStationShortRetryCount() { stationShortRetryCount++; }
    void incrementStationLongRetryCount() { stationLongRetryCount++; }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

