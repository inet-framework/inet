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

#include "inet/physicallayer/analogmodel/packetlevel/DimensionalTransmission.h"

namespace inet {

namespace physicallayer {

DimensionalTransmission::DimensionalTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation, int headerBitLength, int payloadBitLength, bps bitrate, const IModulation *modulation, Hz carrierFrequency, Hz bandwidth, const ConstMapping *power) :
    FlatTransmissionBase(transmitter, macFrame, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, headerBitLength, payloadBitLength, bitrate, modulation, carrierFrequency, bandwidth),
    power(power)
{
}

std::ostream& DimensionalTransmission::printToStream(std::ostream& stream, int level) const
{
    stream << "DimensionalTransmission";
    if (level >= PRINT_LEVEL_DETAIL)
        stream << ", powerDimensionSet = " << power->getDimensionSet();
    if (level >= PRINT_LEVEL_DEBUG)
        stream << ", powerMax = " << MappingUtils::findMax(*power)
               << ", powerMin = " << MappingUtils::findMin(*power);
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", power = " << power;
    return FlatTransmissionBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

