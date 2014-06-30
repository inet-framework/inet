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

#ifndef __INET_SCALARNOISE_H_
#define __INET_SCALARNOISE_H_

#include "FlatNoiseBase.h"

namespace inet {

namespace physicallayer
{

class INET_API ScalarNoise : public FlatNoiseBase
{
    protected:
        const std::map<simtime_t, W> *powerChanges;

    public:
        ScalarNoise(simtime_t startTime, simtime_t endTime, Hz carrierFrequency, Hz bandwidth, const std::map<simtime_t, W> *powerChanges) :
            FlatNoiseBase(startTime, endTime, carrierFrequency, bandwidth),
            powerChanges(powerChanges)
        {}

        virtual ~ScalarNoise() { delete powerChanges; }
        virtual void printToStream(std::ostream &stream) const { stream << "scalar noise"; }
        virtual const std::map<simtime_t, W> *getPowerChanges() const { return powerChanges; }
        virtual W computeMaxPower(simtime_t startTime, simtime_t endTime) const;
};

}

} //namespace


#endif
