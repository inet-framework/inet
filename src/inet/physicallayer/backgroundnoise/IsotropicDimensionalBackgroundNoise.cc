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

#include "inet/physicallayer/analogmodel/packetlevel/DimensionalNoise.h"
#include "inet/physicallayer/backgroundnoise/IsotropicDimensionalBackgroundNoise.h"
#include "inet/physicallayer/common/packetlevel/BandListening.h"

namespace inet {

namespace physicallayer {

Define_Module(IsotropicDimensionalBackgroundNoise);

IsotropicDimensionalBackgroundNoise::IsotropicDimensionalBackgroundNoise() :
    interpolationMode(static_cast<Mapping::InterpolationMethod>(-1)),
    power(W(NaN))
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
        timeGains.push_back(TimeGainEntry('%', 0, 1));
        timeGains.push_back(TimeGainEntry('%', 1, 1));
        frequencyGains.push_back(FrequencyGainEntry('%', 0, 1));
        frequencyGains.push_back(FrequencyGainEntry('%', 1, 1));
    }
}

std::ostream& IsotropicDimensionalBackgroundNoise::printToStream(std::ostream& stream, int level) const
{
    stream << "IsotropicDimensionalBackgroundNoise";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", power = " << power;
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", interpolationMode = " << interpolationMode
               << ", dimensions = " << dimensions ;
    return stream;
}

ConstMapping *IsotropicDimensionalBackgroundNoise::createPowerMapping(const simtime_t startTime, const simtime_t endTime, Hz carrierFrequency, Hz bandwidth, W power) const
{
    Mapping *powerMapping = MappingUtils::createMapping(Argument::MappedZero, dimensions, interpolationMode);
    Argument position(dimensions);
    bool hasTimeDimension = dimensions.hasDimension(Dimension::time);
    bool hasFrequencyDimension = dimensions.hasDimension(Dimension::frequency);
    if (hasTimeDimension && hasFrequencyDimension) {
        double startFrequency = (carrierFrequency - bandwidth / 2).get();
        double endFrequency = (carrierFrequency + bandwidth / 2).get();
        // must be 0 before the start
        if (interpolationMode == Mapping::STEPS)
            ; // already 0 where unspecified
        else if (interpolationMode == Mapping::LINEAR) {
            auto previousTime = startTime;
            previousTime.setRaw(previousTime.raw() - 1);
            position.setTime(previousTime);
            position.setArgValue(Dimension::frequency, startFrequency);
            powerMapping->setValue(position, 0);
            position.setArgValue(Dimension::frequency, endFrequency);
            powerMapping->setValue(position, 0);
            position.setTime(startTime);
            position.setArgValue(Dimension::frequency, nexttoward(startFrequency, DBL_MIN));
            powerMapping->setValue(position, 0);
            position.setArgValue(Dimension::frequency, nexttoward(endFrequency, DBL_MAX));
            powerMapping->setValue(position, 0);
        }
        else
            throw cRuntimeError("Unknown interpolation mode");
        // iterate over timeGains and frequencyGains
        for (const auto & timeGainEntry : timeGains) {
            switch (timeGainEntry.timeUnit) {
                case 's':
                    position.setTime(timeGainEntry.time >= 0 ? startTime + timeGainEntry.time : endTime - timeGainEntry.time);
                    break;
                case '%':
                    position.setTime((1 - timeGainEntry.time) * startTime + timeGainEntry.time * endTime);
                    break;
                default:
                    throw cRuntimeError("Unknown time unit");
            }
            for (const auto & frequencyGainEntry : frequencyGains) {
                switch (frequencyGainEntry.frequencyUnit) {
                    case 's':
                        position.setArgValue(Dimension::frequency, frequencyGainEntry.frequency >= 0 ? startFrequency + frequencyGainEntry.frequency : endFrequency - frequencyGainEntry.frequency);
                        break;
                    case '%':
                        position.setArgValue(Dimension::frequency, (1 - frequencyGainEntry.frequency) * startFrequency + frequencyGainEntry.frequency * endFrequency);
                        break;
                    default:
                        throw cRuntimeError("Unknown frequency unit");
                }
                powerMapping->setValue(position, timeGainEntry.gain * frequencyGainEntry.gain * power.get());
            }
        }
        // must be 0 after the end
        if (interpolationMode == Mapping::STEPS) {
            position.setTime(startTime);
            position.setArgValue(Dimension::frequency, endFrequency);
            powerMapping->setValue(position, 0);
            position.setTime(endTime);
            position.setArgValue(Dimension::frequency, startFrequency);
            powerMapping->setValue(position, 0);
            position.setTime(endTime);
            position.setArgValue(Dimension::frequency, endFrequency);
            powerMapping->setValue(position, 0);
        }
        else if (interpolationMode == Mapping::LINEAR) {
            auto nextTime = endTime;
            nextTime.setRaw(nextTime.raw() + 1);
            position.setTime(nextTime);
            position.setArgValue(Dimension::frequency, startFrequency);
            powerMapping->setValue(position, 0);
            position.setArgValue(Dimension::frequency, endFrequency);
            powerMapping->setValue(position, 0);
            position.setTime(endTime);
            position.setArgValue(Dimension::frequency, nexttoward(startFrequency, DBL_MIN));
            powerMapping->setValue(position, 0);
            position.setArgValue(Dimension::frequency, nexttoward(endFrequency, DBL_MAX));
            powerMapping->setValue(position, 0);
        }
        else
            throw cRuntimeError("Unknown interpolation mode");
    }
    else if (hasTimeDimension) {
        // must be 0 before the start
        if (interpolationMode == Mapping::STEPS)
            ; // already 0 where unspecified
        else if (interpolationMode == Mapping::LINEAR) {
            auto previousTime = startTime;
            previousTime.setRaw(previousTime.raw() - 1);
            position.setTime(previousTime);
            powerMapping->setValue(position, 0);
        }
        else
            throw cRuntimeError("Unknown interpolation mode");
        // iterate over timeGains
        for (const auto & timeGainEntry : timeGains) {
            switch (timeGainEntry.timeUnit) {
                case 's':
                    position.setTime(timeGainEntry.time >= 0 ? startTime + timeGainEntry.time : endTime - timeGainEntry.time);
                    break;
                case '%':
                    position.setTime((1 - timeGainEntry.time) * startTime + timeGainEntry.time * endTime);
                    break;
                default:
                    throw cRuntimeError("Unknown time unit");
            }
            powerMapping->setValue(position, timeGainEntry.gain * power.get());
        }
        // must be 0 after the end
        if (interpolationMode == Mapping::STEPS)
            position.setTime(endTime);
        else if (interpolationMode == Mapping::LINEAR) {
            auto nextTime = endTime;
            nextTime.setRaw(nextTime.raw() + 1);
            position.setTime(nextTime);
        }
        else
            throw cRuntimeError("Unknown interpolation mode");
        powerMapping->setValue(position, 0);
    }
    else if (hasFrequencyDimension) {
        double startFrequency = (carrierFrequency - bandwidth / 2).get();
        double endFrequency = (carrierFrequency + bandwidth / 2).get();
        // must be 0 before the start
        if (interpolationMode == Mapping::STEPS)
            ; // already 0 where unspecified
        else if (interpolationMode == Mapping::LINEAR) {
            position.setArgValue(Dimension::frequency, nexttoward(startFrequency, DBL_MIN));
            powerMapping->setValue(position, 0);
        }
        else
            throw cRuntimeError("Unknown interpolation mode");
        // iterate over frequencyGains
        for (const auto & frequencyGainEntry : frequencyGains) {
            switch (frequencyGainEntry.frequencyUnit) {
                case 's':
                    position.setArgValue(Dimension::frequency, frequencyGainEntry.frequency >= 0 ? startFrequency + frequencyGainEntry.frequency : endFrequency - frequencyGainEntry.frequency);
                    break;
                case '%':
                    position.setArgValue(Dimension::frequency, (1 - frequencyGainEntry.frequency) * startFrequency + frequencyGainEntry.frequency * endFrequency);
                    break;
                default:
                    throw cRuntimeError("Unknown frequency unit");
            }
            powerMapping->setValue(position, frequencyGainEntry.gain * power.get());
        }
        // must be 0 after the end
        if (interpolationMode == Mapping::STEPS)
            position.setArgValue(Dimension::frequency, endFrequency);
        else if (interpolationMode == Mapping::LINEAR)
            position.setArgValue(Dimension::frequency, nexttoward(endFrequency, DBL_MAX));
        else
            throw cRuntimeError("Unknown interpolation mode");
        powerMapping->setValue(position, 0);
    }
    else
        throw cRuntimeError("Unknown dimensions");
    return powerMapping;
}

const INoise *IsotropicDimensionalBackgroundNoise::computeNoise(const IListening *listening) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const simtime_t startTime = listening->getStartTime();
    const simtime_t endTime = listening->getEndTime();
    Hz carrierFrequency = bandListening->getCarrierFrequency();
    Hz bandwidth = bandListening->getBandwidth();
    auto powerMapping = createPowerMapping(startTime, endTime, carrierFrequency, bandwidth, power);
    return new DimensionalNoise(startTime, endTime, carrierFrequency, bandwidth, powerMapping);
}

} // namespace physicallayer

} // namespace inet

