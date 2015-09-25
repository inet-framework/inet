//
// Copyright (C) 2015 Andras Varga
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// Author: Andras Varga
//

cplusplus {{
#include "UpperMacContext.h"
}}

namespace inet::ieee80211;

class MACAddress extends void;

class UpperMacContext
{
    @existingClass();
    @descriptor(readonly);
    MACAddress address;
    int numAccessCategories;
    simtime_t slotTime;
    simtime_t aifsTime[] @sizeGetter(getNumAccessCategories);
    simtime_t sifsTime;
    simtime_t difsTime;
    simtime_t eifsTime[] @sizeGetter(getNumAccessCategories);
    simtime_t pifsTime;
    simtime_t rifsTime;
    int cwMin[] @sizeGetter(getNumAccessCategories);
    int cwMax[] @sizeGetter(getNumAccessCategories);
    int shortRetryLimit;
    int rtsThreshold;
    simtime_t txopLimit[] @sizeGetter(getNumAccessCategories);
    simtime_t ackTimeout;
    simtime_t ackDuration;
    simtime_t ctsTimeout;
    simtime_t ctsDuration;
};
