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

#include "inet/physicallayer/backgroundnoise/IsotropicDimensionalBackgroundNoise.h"
#include "inet/physicallayer/analogmodel/DimensionalNoise.h"
#include "inet/physicallayer/common/BandListening.h"

namespace inet {

namespace physicallayer {

Define_Module(IsotropicDimensionalBackgroundNoise);

IsotropicDimensionalBackgroundNoise::IsotropicDimensionalBackgroundNoise() :
    interpolationMode((Mapping::InterpolationMethod)-1),
    power(W(sNaN))
{
}

void IsotropicDimensionalBackgroundNoise::initialize(int stage)
{
    cModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
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
        power = mW(math::dBm2mW(par("power")));
    }
}

void IsotropicDimensionalBackgroundNoise::printToStream(std::ostream& stream) const
{
    stream << "IsotropicDimensionalBackgroundNoise, "
           // TODO: << "dimensions = { " << dimensions << " }, "
           << "interpolationMode = " << interpolationMode << ", "
           << "power = " << power;
}

const INoise *IsotropicDimensionalBackgroundNoise::computeNoise(const IListening *listening) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const simtime_t startTime = listening->getStartTime();
    const simtime_t endTime = listening->getEndTime();
    Hz carrierFrequency = bandListening->getCarrierFrequency();
    Hz bandwidth = bandListening->getBandwidth();
    Mapping *powerMapping = MappingUtils::createMapping(Argument::MappedZero, dimensions, interpolationMode);
    Argument position(dimensions);
    bool hasTimeDimension = dimensions.hasDimension(Dimension::time);
    bool hasFrequencyDimension = dimensions.hasDimension(Dimension::frequency);
    if (hasTimeDimension && hasFrequencyDimension) {
        // before the start
        position.setTime(0);
        position.setArgValue(Dimension::frequency, 0);
        powerMapping->setValue(position, 0);
        position.setTime(startTime);
        powerMapping->setValue(position, 0);
        position.setTime(endTime);
        powerMapping->setValue(position, 0);
        // start frequency
        position.setTime(0);
        position.setArgValue(Dimension::frequency, (carrierFrequency - bandwidth / 2).get());
        powerMapping->setValue(position, 0);
        position.setTime(startTime);
        powerMapping->setValue(position, power.get());
        position.setTime(endTime);
        powerMapping->setValue(position, 0);
        // end frequency
        position.setTime(0);
        position.setArgValue(Dimension::frequency, (carrierFrequency + bandwidth / 2).get());
        powerMapping->setValue(position, 0);
        position.setTime(startTime);
        powerMapping->setValue(position, 0);
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
    return new DimensionalNoise(startTime, endTime, carrierFrequency, bandwidth, powerMapping);
}

} // namespace physicallayer

} // namespace inet

