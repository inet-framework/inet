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

#ifndef __INET_FLATRECEPTIONBASE_H_
#define __INET_FLATRECEPTIONBASE_H_

#include "ReceptionBase.h"

namespace inet {

namespace physicallayer
{

class INET_API FlatReceptionBase : public ReceptionBase
{
    protected:
        const Hz carrierFrequency;
        const Hz bandwidth;

    public:
        FlatReceptionBase(const IRadio *receiver, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation, Hz carrierFrequency, Hz bandwidth) :
            ReceptionBase(receiver, transmission, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation),
            carrierFrequency(carrierFrequency),
            bandwidth(bandwidth)
        {}

        virtual Hz getCarrierFrequency() const { return carrierFrequency; }
        virtual Hz getBandwidth() const { return bandwidth; }
        virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const = 0;
};

}

}


#endif
