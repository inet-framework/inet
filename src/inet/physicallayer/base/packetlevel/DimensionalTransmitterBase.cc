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

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/base/packetlevel/DimensionalTransmitterBase.h"
#include "inet/physicallayer/analogmodel/packetlevel/DimensionalTransmission.h"

namespace inet {

namespace physicallayer {

DimensionalTransmitterBase::DimensionalTransmitterBase() :
    interpolationMode((Mapping::InterpolationMethod)-1)
{
}

void DimensionalTransmitterBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        cModule *module = dynamic_cast<cModule *>(this);
        // TODO: factor parsing?
        // dimensions
        const char *dimensionsString = module->par("dimensions");
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
        // interpolation mode
        const char *interpolationModeString = module->par("interpolationMode");
        if (!strcmp("linear", interpolationModeString))
            interpolationMode = Mapping::LINEAR;
        else if (!strcmp("sample-hold", interpolationModeString))
            interpolationMode = Mapping::STEPS;
        else
            throw cRuntimeError("Unknown interpolation mode: '%s'", interpolationModeString);
        // time gains
        cStringTokenizer timeGainsTokenizer(module->par("timeGains"));
        while (timeGainsTokenizer.hasMoreTokens()) {
            char *end;
            const char *timeString = timeGainsTokenizer.nextToken();
            double time = strtod(timeString, &end);
            char timeUnit;
            if (!end)
                timeUnit = 's';
            else if (*end == '%') {
                timeUnit = '%';
                time /= 100;
                if (time < 0 || time > 1)
                    throw cRuntimeError("Invalid percentage in timeGains parameter");
            }
            else {
                timeUnit = 's';
                time = cNEDValue::parseQuantity(timeString, "s");
            }
            const char *gainString = timeGainsTokenizer.nextToken();
            double gain = strtod(gainString, &end);
            if (end && !strcmp(end, "dB"))
                gain = math::dB2fraction(gain);
            if (gain < 0 || gain > 1)
                throw cRuntimeError("Invalid gain in timeGains parameter");
            timeGains.push_back(TimeGainEntry(timeUnit, time, gain));
        }
        // frequency gains
        cStringTokenizer frequencyGainsTokenizer(module->par("frequencyGains"));
        while (frequencyGainsTokenizer.hasMoreTokens()) {
            char *end;
            const char *frequencyString = frequencyGainsTokenizer.nextToken();
            double frequency = strtod(frequencyString, &end);
            char frequencyUnit;
            if (!end)
                frequencyUnit = 'H';
            else if (*end == '%') {
                frequencyUnit = '%';
                frequency /= 100;
                if (frequency < 0 || frequency > 1)
                    throw cRuntimeError("Invalid percentage in frequencyGains parameter");
            }
            else {
                frequencyUnit = 'H';
                frequency = cNEDValue::parseQuantity(frequencyString, "Hz");
            }
            const char *gainString = frequencyGainsTokenizer.nextToken();
            double gain = strtod(gainString, &end);
            if (end && !strcmp(end, "dB"))
                gain = math::dB2fraction(gain);
            if (gain < 0 || gain > 1)
                throw cRuntimeError("Invalid gain in frequencyGains parameter");
            frequencyGains.push_back(FrequencyGainEntry(frequencyUnit, frequency, gain));
        }
    }
}

std::ostream& DimensionalTransmitterBase::printToStream(std::ostream& stream, int level) const
{
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", interpolationMode = " << interpolationMode
               << ", dimensions = " << dimensions ;
               // TODO: << "timeGains = " << timeGains
               // TODO: << "frequencyGains = " << frequencyGains ;
    return stream;
}

ConstMapping *DimensionalTransmitterBase::createPowerMapping(const simtime_t startTime, const simtime_t endTime, Hz carrierFrequency, Hz bandwidth, W power) const
{
    Mapping *powerMapping = MappingUtils::createMapping(Argument::MappedZero, dimensions, interpolationMode);
    Argument position(dimensions);
    bool hasTimeDimension = dimensions.hasDimension(Dimension::time);
    bool hasFrequencyDimension = dimensions.hasDimension(Dimension::frequency);
    if (hasTimeDimension && hasFrequencyDimension) {
        double startFrequency = (carrierFrequency - bandwidth / 2).get();
        double endFrequency = (carrierFrequency + bandwidth / 2).get();
        // must be 0 before the start
        position.setTime(0);
        position.setArgValue(Dimension::frequency, 0);
        powerMapping->setValue(position, 0);
        position.setTime(startTime);
        powerMapping->setValue(position, 0);
        position.setTime(endTime);
        powerMapping->setValue(position, 0);
        position.setTime(0);
        position.setArgValue(Dimension::frequency, startFrequency);
        powerMapping->setValue(position, 0);
        position.setArgValue(Dimension::frequency, endFrequency);
        powerMapping->setValue(position, 0);
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
    else if (hasTimeDimension) {
        // must be 0 before the start
        position.setTime(0);
        powerMapping->setValue(position, 0);
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
        position.setTime(endTime);
        powerMapping->setValue(position, 0);
    }
    else if (hasFrequencyDimension) {
        double startFrequency = (carrierFrequency - bandwidth / 2).get();
        double endFrequency = (carrierFrequency + bandwidth / 2).get();
        // must be 0 before the start
        position.setArgValue(Dimension::frequency, 0);
        powerMapping->setValue(position, 0);
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
        position.setArgValue(Dimension::frequency, endFrequency);
        powerMapping->setValue(position, 0);
    }
    else
        throw cRuntimeError("Unknown dimensions");
    return powerMapping;
}

} // namespace physicallayer

} // namespace inet

