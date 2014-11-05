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

#ifndef __INET_DIMENSIONALTRANSMISSION_H
#define __INET_DIMENSIONALTRANSMISSION_H

#include "inet/physicallayer/base/NarrowbandTransmissionBase.h"
#include "inet/physicallayer/mapping/MappingBase.h"
#include "inet/physicallayer/mapping/MappingUtils.h"

namespace inet {

namespace physicallayer {

class INET_API DimensionalTransmission : public NarrowbandTransmissionBase
{
  protected:
    const ConstMapping *power;

  public:
    DimensionalTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation, const IModulation *modulation, int headerBitLength, int payloadBitLength, Hz carrierFrequency, Hz bandwidth, bps bitrate, const ConstMapping *power);
    virtual ~DimensionalTransmission() { delete power; }

    virtual const ConstMapping *getPower() const { return power; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_DIMENSIONALTRANSMISSION_H

