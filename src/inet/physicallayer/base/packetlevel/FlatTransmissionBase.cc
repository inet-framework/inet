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

#include "inet/physicallayer/base/packetlevel/FlatTransmissionBase.h"

namespace inet {

namespace physicallayer {

FlatTransmissionBase::FlatTransmissionBase(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation, int headerBitLength, int payloadBitLength, bps bitrate, const IModulation *modulation, Hz carrierFrequency, Hz bandwidth) :
    NarrowbandTransmissionBase(transmitter, macFrame, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, modulation, carrierFrequency, bandwidth),
    headerBitLength(headerBitLength),
    dataBitLength(payloadBitLength),
    bitrate(bitrate)
{
}

std::ostream& FlatTransmissionBase::printToStream(std::ostream& stream, int level) const
{
    if (level >= PRINT_LEVEL_DETAIL)
        stream << ", bitrate = " << bitrate;
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", headerBitLength = " << headerBitLength
               << ", payloadBitLength = " << dataBitLength;
    return NarrowbandTransmissionBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

