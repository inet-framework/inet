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

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/analogmodel/packetlevel/DimensionalTransmission.h"
#include "inet/physicallayer/common/packetlevel/NoiseDimensionalTransmitter.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"

namespace inet {

namespace physicallayer {

Define_Module(NoiseDimensionalTransmitter);

void NoiseDimensionalTransmitter::initialize(int stage)
{
    TransmitterBase::initialize(stage);
    DimensionalTransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        durationParameter = &par("duration");
        centerFrequencyParameter = &par("centerFrequency");
        bandwidthParameter = &par("bandwidth");
        powerParameter = &par("power");
    }
}

std::ostream& NoiseDimensionalTransmitter::printToStream(std::ostream& stream, int level) const
{
    stream << "NoiseDimensionalTransmitter";
    TransmitterBase::printToStream(stream, level);
    return DimensionalTransmitterBase::printToStream(stream, level);
}

const ITransmission *NoiseDimensionalTransmitter::createTransmission(const IRadio *transmitter, const Packet *packet, simtime_t startTime) const
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
    const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& powerFunction = createPowerFunction(startTime, endTime, centerFrequency, bandwidth, power);
    return new DimensionalTransmission(transmitter, nullptr, startTime, endTime, 0, 0, duration, startPosition, endPosition, startOrientation, endOrientation, nullptr, b(-1), b(-1), centerFrequency, bandwidth, bps(NaN), powerFunction);
}

} // namespace physicallayer

} // namespace inet

