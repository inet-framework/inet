//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_FLATNOISEBASE_H_
#define __INET_FLATNOISEBASE_H_

#include "NoiseBase.h"

namespace radio
{

class INET_API FlatNoiseBase : public NoiseBase
{
    protected:
        const Hz carrierFrequency;
        const Hz bandwidth;

    public:
        FlatNoiseBase(simtime_t startTime, simtime_t endTime, Hz carrierFrequency, Hz bandwidth) :
            NoiseBase(startTime, endTime),
            carrierFrequency(carrierFrequency),
            bandwidth(bandwidth)
        {}

        virtual Hz getCarrierFrequency() const { return carrierFrequency; }
        virtual Hz getBandwidth() const { return bandwidth; }

        virtual W computeMaxPower(simtime_t startTime, simtime_t endTime) const = 0;
};

}

#endif
