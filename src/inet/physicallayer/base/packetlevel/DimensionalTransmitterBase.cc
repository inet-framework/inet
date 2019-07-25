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
#include "inet/physicallayer/analogmodel/packetlevel/DimensionalTransmission.h"
#include "inet/physicallayer/base/packetlevel/DimensionalTransmitterBase.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"

namespace inet {

namespace physicallayer {

using namespace inet::math;

void DimensionalTransmitterBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        cModule *module = check_and_cast<cModule *>(this);
        parseTimeGains(module->par("timeGains"));
        parseFrequencyGains(module->par("frequencyGains"));
        timeGainsNormalization = module->par("timeGainsNormalization");
        frequencyGainsNormalization = module->par("frequencyGainsNormalization");
    }
}

template<typename T>
std::vector<DimensionalTransmitterBase::GainEntry<T>> DimensionalTransmitterBase::parseGains(const char *text) const
{
    std::vector<GainEntry<T>> gains;
    cStringTokenizer tokenizer(text);
    tokenizer.nextToken();
    while (tokenizer.hasMoreTokens()) {
        auto where = tokenizer.nextToken();
        char *end = (char *)where + 1;
// TODO: replace this BS with the expression evaluator when it supports simtime_t and bindings
//      Allowed syntax:
//        s|c|e
//        s|c|e+-quantity
//        s|c|e+-b|d
//        s|c|e+-b|d+-quantity
//        s|c|e+-b|d*number
//        s|c|e+-b|d*number+-quantity
        double lengthMultiplier = 0;
        if ((*(where + 1) == '+' || *(where + 1) == '-') &&
            (*(where + 2) == 'b' || *(where + 2) == 'd'))
        {
            if (*(where + 3) == '*')
                lengthMultiplier = strtod(where + 4, &end);
            else {
                lengthMultiplier = 1;
                end += 2;
            }
            if (*(where + 1) == '-')
                lengthMultiplier *= -1;
        }
        T offset = T(0);
        if (end && strlen(end) != 0)
            offset = T(cNEDValue::parseQuantity(end, (std::is_same<T, simtime_t>::value == true ? "s" : (std::is_same<T, Hz>::value == true ? "Hz" : ""))));
        double gain = strtod(tokenizer.nextToken(), &end);
        if (end && !strcmp(end, "dB"))
            gain = dB2fraction(gain);
        if (gain < 0)
            throw cRuntimeError("Gain must be in the range [0, inf)");
        auto interpolator = createInterpolator<T, double>(tokenizer.nextToken());
        gains.push_back(GainEntry<T>(interpolator, *where, lengthMultiplier, offset, gain));
    }
    return gains;
}

void DimensionalTransmitterBase::parseTimeGains(const char *text)
{
    if (strcmp(text, "smaller s 0dB either e 0dB greater")) {
        firstTimeInterpolator = createInterpolator<simtime_t, double>(cStringTokenizer(text).nextToken());
        timeGains = parseGains<simtime_t>(text);
    }
    else {
        firstTimeInterpolator = nullptr;
        timeGains.clear();
    }
}

void DimensionalTransmitterBase::parseFrequencyGains(const char *text)
{
    if (strcmp(text, "smaller s 0dB either e 0dB greater")) {
        firstFrequencyInterpolator = createInterpolator<Hz, double>(cStringTokenizer(text).nextToken());
        frequencyGains = parseGains<Hz>(text);
    }
    else {
        firstFrequencyInterpolator = nullptr;
        frequencyGains.clear();
    }
}

std::ostream& DimensionalTransmitterBase::printToStream(std::ostream& stream, int level) const
{
    // TODO: << ", timeGains = " << timeGains
    // TODO: << ", frequencyGains = " << frequencyGains;
    return stream;
}

template<typename T>
const Ptr<const IFunction<double, Domain<T>>> DimensionalTransmitterBase::normalize(const Ptr<const IFunction<double, Domain<T>>>& gainFunction, const char *normalization) const
{
    if (!strcmp("", normalization))
        return gainFunction;
    else if (!strcmp("maximum", normalization)) {
        auto max = gainFunction->getMax();
        ASSERT(max != 0);
        if (max == 1.0)
            return gainFunction;
        else
            return gainFunction->divide(makeShared<ConstantFunction<double, Domain<T>>>(max));
    }
    else if (!strcmp("integral", normalization)) {
        double integral = gainFunction->getIntegral();
        ASSERT(integral != 0);
        if (integral == 1.0)
            return gainFunction;
        else
            return gainFunction->divide(makeShared<ConstantFunction<double, Domain<T>>>(integral));
    }
    else
        throw cRuntimeError("Unknown normalization: '%s'", normalization);
}

Ptr<const IFunction<WpHz, Domain<simtime_t, Hz>>> DimensionalTransmitterBase::createPowerFunction(const simtime_t startTime, const simtime_t endTime, Hz carrierFrequency, Hz bandwidth, W power) const
{
    if (timeGains.size() == 0 && frequencyGains.size() == 0)
        return makeShared<TwoDimensionalBoxcarFunction<WpHz, simtime_t, Hz>>(startTime, endTime, carrierFrequency - bandwidth / 2, carrierFrequency + bandwidth / 2, power / bandwidth);
    else {
        Ptr<const IFunction<double, Domain<simtime_t>>> timeGainFunction;
        if (timeGains.size() != 0) {
            auto centerTime = (startTime + endTime) / 2;
            auto duration = endTime - startTime;
            std::map<simtime_t, std::pair<double, const IInterpolator<simtime_t, double> *>> ts;
            ts[getLowerBoundary<simtime_t>()] = {0, firstTimeInterpolator};
            ts[getUpperBoundary<simtime_t>()] = {0, nullptr};
            for (const auto & entry : timeGains) {
                simtime_t time = entry.where == 's' ? startTime : (entry.where == 'e' ? endTime : centerTime) + duration * entry.length + entry.offset;
                ts[time] = {entry.gain, entry.interpolator};
            }
            timeGainFunction = makeShared<OneDimensionalInterpolatedFunction<double, simtime_t>>(ts);
        }
        else
            timeGainFunction = makeShared<OneDimensionalBoxcarFunction<double, simtime_t>>(startTime, endTime, 1);
        Ptr<const IFunction<double, Domain<Hz>>> frequencyGainFunction;
        if (frequencyGains.size() != 0) {
            auto startFrequency = carrierFrequency - bandwidth / 2;
            auto endFrequency = carrierFrequency + bandwidth / 2;
            std::map<Hz, std::pair<double, const IInterpolator<Hz, double>*>> fs;
            fs[getLowerBoundary<Hz>()] = {0, firstFrequencyInterpolator};
            fs[getUpperBoundary<Hz>()] = {0, nullptr};
            for (const auto & entry : frequencyGains) {
                Hz frequency = entry.where == 's' ? startFrequency : (entry.where == 'e' ? endFrequency : carrierFrequency) + bandwidth * entry.length + Hz(entry.offset);
                fs[frequency] = {entry.gain, entry.interpolator};
            }
            frequencyGainFunction = makeShared<OneDimensionalInterpolatedFunction<double, Hz>>(fs);
        }
        else
            frequencyGainFunction = makeShared<OneDimensionalBoxcarFunction<double, Hz>>(carrierFrequency - bandwidth / 2, carrierFrequency + bandwidth / 2, 1);
        auto gainFunction = makeShared<OrthogonalCombinatorFunction<double, simtime_t, Hz>>(normalize<simtime_t>(timeGainFunction, timeGainsNormalization), normalize<Hz>(frequencyGainFunction, frequencyGainsNormalization));
        auto powerFunction = makeShared<ConstantFunction<WpHz, Domain<simtime_t, Hz>>>(power / Hz(1));
        return powerFunction->multiply(gainFunction);
    }
}

} // namespace physicallayer

} // namespace inet

