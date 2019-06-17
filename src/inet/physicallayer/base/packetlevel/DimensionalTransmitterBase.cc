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

void DimensionalTransmitterBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        cModule *module = check_and_cast<cModule *>(this);
        parseTimeGains(module->par("timeGains"));
        parseFrequencyGains(module->par("frequencyGains"));
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
            gain = math::dB2fraction(gain);
        if (gain < 0 || gain > 1)
            throw cRuntimeError("Gain must be in the range [0, 1]");
        auto interpolator = math::createInterpolator<T, double>(tokenizer.nextToken());
        gains.push_back(GainEntry<T>(interpolator, *where, lengthMultiplier, offset, gain));
    }
    return gains;
}

void DimensionalTransmitterBase::parseTimeGains(const char *text)
{
    if (strcmp(text, "smaller s 0dB either e 0dB greater")) {
        firstTimeInterpolator = math::createInterpolator<simtime_t, double>(cStringTokenizer(text).nextToken());
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
        firstFrequencyInterpolator = math::createInterpolator<Hz, double>(cStringTokenizer(text).nextToken());
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

Ptr<const math::IFunction<W, simtime_t, Hz>> DimensionalTransmitterBase::createPowerFunction(const simtime_t startTime, const simtime_t endTime, Hz carrierFrequency, Hz bandwidth, W power) const
{
    if (timeGains.size() == 0 && frequencyGains.size() == 0)
        return makeShared<math::TwoDimensionalBoxcarFunction<W, simtime_t, Hz>>(startTime, endTime, carrierFrequency - bandwidth / 2, carrierFrequency + bandwidth / 2, power);
    else {
        Ptr<const math::IFunction<double, simtime_t>> timeGainFunction;
        if (timeGains.size() != 0) {
            auto centerTime = (startTime + endTime) / 2;
            auto duration = endTime - startTime;
            std::map<simtime_t, std::pair<double, const math::IInterpolator<simtime_t, double> *>> ts;
            ts[math::getLowerBoundary<simtime_t>()] = {0, firstTimeInterpolator};
            ts[math::getUpperBoundary<simtime_t>()] = {0, nullptr};
            for (const auto & entry : timeGains) {
                simtime_t time = entry.where == 's' ? startTime : (entry.where == 'e' ? endTime : centerTime) + duration * entry.length + entry.offset;
                ts[time] = {entry.gain, entry.interpolator};
            }
            timeGainFunction = makeShared<math::OneDimensionalInterpolatedFunction<double, simtime_t>>(ts);
        }
        else
            timeGainFunction = makeShared<math::OneDimensionalBoxcarFunction<double, simtime_t>>(startTime, endTime, 1);
        Ptr<const math::IFunction<double, Hz>> frequencyGainFunction;
        if (frequencyGains.size() != 0) {
            auto startFrequency = carrierFrequency - bandwidth / 2;
            auto endFrequency = carrierFrequency + bandwidth / 2;
            std::map<Hz, std::pair<double, const math::IInterpolator<Hz, double>*>> fs;
            fs[math::getLowerBoundary<Hz>()] = {0, firstFrequencyInterpolator};
            fs[math::getUpperBoundary<Hz>()] = {0, nullptr};
            for (const auto & entry : frequencyGains) {
                Hz frequency = entry.where == 's' ? startFrequency : (entry.where == 'e' ? endFrequency : carrierFrequency) + bandwidth * entry.length + Hz(entry.offset);
                fs[frequency] = {entry.gain, entry.interpolator};
            }
            frequencyGainFunction = makeShared<math::OneDimensionalInterpolatedFunction<double, Hz>>(fs);
        }
        else
            frequencyGainFunction = makeShared<math::OneDimensionalBoxcarFunction<double, Hz>>(carrierFrequency - bandwidth / 2, carrierFrequency + bandwidth / 2, 1);
        auto gainFunction = makeShared<math::OrthogonalCombinatorFunction<double, simtime_t, Hz>>(timeGainFunction, frequencyGainFunction);
        auto powerFunction = makeShared<math::ConstantFunction<W, simtime_t, Hz>>(power);
        return powerFunction->multiply(gainFunction);
    }
}

} // namespace physicallayer

} // namespace inet

