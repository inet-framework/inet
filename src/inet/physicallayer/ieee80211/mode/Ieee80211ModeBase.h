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

#ifndef __INET_IEEE80211MODEBASE_H
#define __INET_IEEE80211MODEBASE_H

#include "IIeee80211Mode.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211ModeBase : public IIeee80211Mode
{
    public:
        virtual int getAifsNumber(AccessCategory ac) const override;
        virtual const simtime_t getAifsTime(AccessCategory ac) const override;
        virtual const simtime_t getEifsTime(const IIeee80211Mode *slowestMandatoryMode, AccessCategory ac, int ackLength) const override;
        virtual const simtime_t getDifsTime() const override;
        virtual const simtime_t getPifsTime() const override;
        // TODO: These are the default parameters when dot11OCBActivated is false.
        virtual int getCwMin(AccessCategory ac) const override;
        virtual int getCwMax(AccessCategory ac) const override;
};

} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211MODEBASE_H
