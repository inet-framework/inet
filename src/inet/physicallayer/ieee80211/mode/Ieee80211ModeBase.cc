//
// Copyright (C) 2015 OpenSim Ltd.
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

#include "Ieee80211ModeBase.h"

namespace inet {
namespace physicallayer {

int Ieee80211ModeBase::getAifsNumber(AccessCategory ac) const
{
    switch (ac)
    {
        case AC_BK: return 7;
        case AC_BE: return 3;
        case AC_VI: return 2;
        case AC_VO: return 2;
        case AC_LEGACY: return 2;
        case AC_NUMCATEGORIES: break;
    }
    throw cRuntimeError("Unknown access category = %d", ac);
    return -1;
}

int Ieee80211ModeBase::getCwMax(AccessCategory ac) const
{
    int legacyCwMax = getLegacyCwMax();
    int legacyCwMin = getLegacyCwMin();
    switch (ac)
    {
        case AC_BK: return legacyCwMax;
        case AC_BE: return legacyCwMax;
        case AC_VI: return legacyCwMin;
        case AC_VO: return (legacyCwMin + 1) / 2 - 1;
        case AC_LEGACY: return legacyCwMax;
        case AC_NUMCATEGORIES: break;
    }
    throw cRuntimeError("Unknown access category = %d", ac);
    return -1;
}

int Ieee80211ModeBase::getCwMin(AccessCategory ac) const
{
    int legacyCwMin = getLegacyCwMin();
    switch (ac)
    {
        case AC_BK: return legacyCwMin;
        case AC_BE: return legacyCwMin;
        case AC_VI: return (legacyCwMin + 1) / 2 - 1;
        case AC_VO: return (legacyCwMin + 1) / 4 - 1;
        case AC_LEGACY: return legacyCwMin;
        case AC_NUMCATEGORIES: break;
    }
    throw cRuntimeError("Unknown access category = %d", ac);
    return -1;
}

const simtime_t Ieee80211ModeBase::getAifsTime(AccessCategory ac) const
{
    return getSlotTime() * getAifsNumber(ac) + getSifsTime();
}

const simtime_t Ieee80211ModeBase::getEifsTime(const IIeee80211Mode* slowestMandatoryMode, AccessCategory ac, int ackLength) const
{
    return getSifsTime() + getAifsTime(ac) + slowestMandatoryMode->getDuration(ackLength);
}

const simtime_t Ieee80211ModeBase::getDifsTime() const
{
    return getSifsTime() + 2 * getSlotTime();
}

const simtime_t Ieee80211ModeBase::getPifsTime() const
{
    return getSifsTime() + getSlotTime();
}

} /* namespace physicallayer */
} /* namespace inet */

