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

#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ScalarTransmission.h"

namespace inet {

namespace physicallayer {

Ieee80211ScalarTransmission::Ieee80211ScalarTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation, const IModulation *modulation, int headerBitLength, int payloadBitLength, Hz carrierFrequency, Hz bandwidth, bps bitrate, W power, const IIeee80211Mode *mode, const Ieee80211Channel *channel) :
    ScalarTransmission(transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, modulation, headerBitLength, payloadBitLength, carrierFrequency, bandwidth, bitrate, power),
    Ieee80211TransmissionBase(mode, channel)
{
}

std::ostream& Ieee80211ScalarTransmission::printToStream(std::ostream& stream, int level) const
{
    stream << "Ieee80211ScalarTransmission";
    Ieee80211TransmissionBase::printToStream(stream, level);
    return ScalarTransmission::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

