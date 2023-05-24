//
// Copyright (C) OpenSim Ltd.
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

#include "inet/physicallayer/wireless/noise/NoiseTransmitter.h"

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/wireless/noise/NoiseTransmission.h"

namespace inet {

namespace physicallayer {

Define_Module(NoiseTransmitter);

void NoiseTransmitter::initialize(int stage)
{
    TransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        durationParameter = &par("duration");
        centerFrequencyParameter = &par("centerFrequency");
        bandwidthParameter = &par("bandwidth");
        powerParameter = &par("power");
    }
}

std::ostream& NoiseTransmitter::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "NoiseTransmitter";
    return TransmitterBase::printToStream(stream, level);
}

const ITransmission *NoiseTransmitter::createTransmission(const IRadio *transmitter, const Packet *packet, simtime_t startTime) const
{
    Hz centerFrequency = Hz(centerFrequencyParameter->doubleValue());
    Hz bandwidth = Hz(bandwidthParameter->doubleValue());
    W power = W(powerParameter->doubleValue());
    const simtime_t duration = durationParameter->doubleValue();
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const Quaternion startOrientation = mobility->getCurrentAngularPosition();
    const Quaternion endOrientation = mobility->getCurrentAngularPosition();
    auto analogModel = getAnalogModel()->createAnalogModel(0, 0, duration, centerFrequency, bandwidth, power);
    return new NoiseTransmission(transmitter, nullptr, startTime, endTime, 0, 0, duration, startPosition, endPosition, startOrientation, endOrientation, nullptr, nullptr, nullptr, nullptr, analogModel);
}

} // namespace physicallayer

} // namespace inet

