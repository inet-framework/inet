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

#include "inet/physicallayer/dimensional/DimensionalTransmitter.h"
#include "inet/physicallayer/dimensional/DimensionalTransmission.h"
#include "inet/physicallayer/contract/IRadio.h"
#include "inet/mobility/contract/IMobility.h"

namespace inet {

namespace physicallayer {

Define_Module(DimensionalTransmitter);

DimensionalTransmitter::DimensionalTransmitter() :
    FlatTransmitterBase(),
    interpolationMode((Mapping::InterpolationMethod)-1)
{
}

void DimensionalTransmitter::initialize(int stage)
{
    FlatTransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        // TODO: factor parsing?
        const char *dimensionsString = par("dimensions");
        cStringTokenizer tokenizer(dimensionsString);
        while (tokenizer.hasMoreTokens()) {
            const char *dimensionString = tokenizer.nextToken();
            if (!strcmp("time", dimensionString))
                dimensions.addDimension(Dimension::time);
            else if (!strcmp("frequency", dimensionString))
                dimensions.addDimension(Dimension::frequency);
            else
                throw cRuntimeError("Unknown dimension");
        }
        const char *interpolationModeString = par("interpolationMode");
        if (!strcmp("linear", interpolationModeString))
            interpolationMode = Mapping::LINEAR;
        else if (!strcmp("sample-hold", interpolationModeString))
            interpolationMode = Mapping::STEPS;
        else
            throw cRuntimeError("Unknown interpolation mode: '%s'", interpolationModeString);
        const char *timeGains = par("timeGains");
        const char *freqencyGains = par("frequencyGains");
    }
}

void DimensionalTransmitter::printToStream(std::ostream& stream) const
{
    stream << "dimensional transmitter, "
           << "bitrate = " << bitrate << ", "
           << "power = " << power << ", "
           << "carrierFrequency = " << carrierFrequency << ", "
           << "bandwidth = " << bandwidth;
}

ConstMapping *DimensionalTransmitter::createPowerMapping(const simtime_t startTime, const simtime_t endTime, Hz carrierFrequency, Hz bandwidth, W power) const
{
    if (interpolationMode == Mapping::STEPS) {
        Mapping *powerMapping = MappingUtils::createMapping(Argument::MappedZero, dimensions, Mapping::STEPS);
        Argument position(dimensions);
        bool hasTimeDimension = dimensions.hasDimension(Dimension::time);
        bool hasFrequencyDimension = dimensions.hasDimension(Dimension::frequency);
        if (hasTimeDimension && hasFrequencyDimension) {
            // 1.
            position.setTime(0);
            position.setArgValue(Dimension::frequency, 0);
            powerMapping->setValue(position, 0);
            position.setTime(startTime);
            powerMapping->setValue(position, 0);
            position.setTime(endTime);
            powerMapping->setValue(position, 0);
            // 2.
            position.setTime(0);
            position.setArgValue(Dimension::frequency, (carrierFrequency - bandwidth / 2).get());
            powerMapping->setValue(position, 0);
            position.setTime(startTime);
            powerMapping->setValue(position, power.get());
            position.setTime(endTime);
            powerMapping->setValue(position, 0);
            // 3.
            position.setTime(0);
            position.setArgValue(Dimension::frequency, (carrierFrequency + bandwidth / 2).get());
            powerMapping->setValue(position, 0);
            position.setTime(startTime);
            powerMapping->setValue(position, power.get());
            position.setTime(endTime);
            powerMapping->setValue(position, 0);
        }
        else if (hasTimeDimension) {
            position.setTime(0);
            powerMapping->setValue(position, 0);
            position.setTime(startTime);
            powerMapping->setValue(position, power.get());
            position.setTime(endTime);
            powerMapping->setValue(position, 0);
        }
        else if (hasFrequencyDimension) {
            position.setArgValue(Dimension::frequency, 0);
            powerMapping->setValue(position, 0);
            position.setArgValue(Dimension::frequency, (carrierFrequency - bandwidth / 2).get());
            powerMapping->setValue(position, power.get());
            position.setArgValue(Dimension::frequency, (carrierFrequency + bandwidth / 2).get());
            powerMapping->setValue(position, 0);
        }
        else
            throw cRuntimeError("Unknown dimensions");
        return powerMapping;
    }
    else if (interpolationMode == Mapping::LINEAR) {
        const simtime_t leadInTime = 1E-10;
        const simtime_t leadOutTime = 1E-10;
        Mapping *powerMapping = MappingUtils::createMapping(Argument::MappedZero, dimensions, Mapping::LINEAR);
        Argument position(dimensions);
        if (dimensions.hasDimension(Dimension::frequency))
            position.setArgValue(Dimension::frequency, carrierFrequency.get() - bandwidth.get() / 2);
        if (startTime > leadInTime) {
            position.setTime(startTime - leadInTime);
            powerMapping->setValue(position, 0);
        }
        position.setTime(startTime);
        powerMapping->setValue(position, power.get());
        position.setTime(endTime);
        powerMapping->setValue(position, power.get());
        position.setTime(endTime + leadOutTime);
        powerMapping->setValue(position, 0);
        if (dimensions.hasDimension(Dimension::frequency))
        {
            position.setArgValue(Dimension::frequency, carrierFrequency.get() + bandwidth.get() / 2);
            if (startTime > leadInTime) {
                position.setTime(startTime - leadInTime);
                powerMapping->setValue(position, 0);
            }
            position.setTime(startTime);
            powerMapping->setValue(position, power.get());
            position.setTime(endTime);
            powerMapping->setValue(position, power.get());
            position.setTime(endTime + leadOutTime);
            powerMapping->setValue(position, 0);
        }
        return powerMapping;
    }
    else
        throw cRuntimeError("Unknown interpolation mode");
}

const ITransmission *DimensionalTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime) const
{
    const simtime_t duration = macFrame->getBitLength() / bitrate.get();
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    const ConstMapping *powerMapping = createPowerMapping(startTime, endTime, carrierFrequency, bandwidth, power);
    return new DimensionalTransmission(transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, modulation, headerBitLength, macFrame->getBitLength(), carrierFrequency, bandwidth, bitrate, powerMapping);
}

} // namespace physicallayer

} // namespace inet

