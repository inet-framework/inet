//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/DimensionalTransmitterBase.h"

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalTransmission.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {

namespace physicallayer {

using namespace inet::math;

void DimensionalTransmitterBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        cModule *module = check_and_cast<cModule *>(this);
        gainFunctionCacheLimit = module->par("gainFunctionCacheLimit");
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
        char *token = const_cast<char *>(tokenizer.nextToken());
        char where;
        char *end;
        if (*token == 's' || *token == 'c' || *token == 'e') {
            where = *token;
            end = token + 1;
        }
        else {
            where = ' ';
            end = token;
        }
        // TODO replace this BS with the expression evaluator when it supports simtime_t and bindings
        // Allowed syntax:
        // +-quantity
        // s|c|e
        // s|c|e+-quantity
        // s|c|e+-b|d
        // s|c|e+-b|d+-quantity
        // s|c|e+-b|d*number
        // s|c|e+-b|d*number+-quantity
        double lengthMultiplier = 0;
        if ((*(token + 1) == '+' || *(token + 1) == '-') &&
            (*(token + 2) == 'b' || *(token + 2) == 'd'))
        {
            if (*(token + 3) == '*')
                lengthMultiplier = strtod(token + 4, &end);
            else {
                lengthMultiplier = 1;
                end += 2;
            }
            if (*(token + 1) == '-')
                lengthMultiplier *= -1;
        }
        T offset = T(0);
        if (end && strlen(end) != 0)
            offset = T(cNEDValue::parseQuantity(end, (std::is_same<T, simsec>::value == true ? "s" : (std::is_same<T, Hz>::value == true ? "Hz" : ""))));
        double gain = strtod(tokenizer.nextToken(), &end);
        if (end && !strcmp(end, "dB"))
            gain = dB2fraction(gain);
        if (gain < 0)
            throw cRuntimeError("Gain must be in the range [0, inf)");
        auto interpolator = createInterpolator<T, double>(tokenizer.nextToken());
        gains.push_back(GainEntry<T>(interpolator, where, lengthMultiplier, offset, gain));
    }
    return gains;
}

void DimensionalTransmitterBase::parseTimeGains(const char *text)
{
    if (strcmp(text, "left s 0dB either e 0dB right")) {
        firstTimeInterpolator = createInterpolator<simsec, double>(cStringTokenizer(text).nextToken());
        timeGains = parseGains<simsec>(text);
    }
    else {
        firstTimeInterpolator = nullptr;
        timeGains.clear();
    }
}

void DimensionalTransmitterBase::parseFrequencyGains(const char *text)
{
    if (strcmp(text, "left s 0dB either e 0dB right")) {
        firstFrequencyInterpolator = createInterpolator<Hz, double>(cStringTokenizer(text).nextToken());
        frequencyGains = parseGains<Hz>(text);
    }
    else {
        firstFrequencyInterpolator = nullptr;
        frequencyGains.clear();
    }
}

std::ostream& DimensionalTransmitterBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
//  stream << EV_FIELD(timeGains);
//  stream << EV_FIELD(frequencyGains);
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

Ptr<const IFunction<double, Domain<simsec, Hz>>> DimensionalTransmitterBase::createGainFunction(const simtime_t startTime, const simtime_t endTime, Hz centerFrequency, Hz bandwidth) const
{
    if (timeGains.size() == 0 && frequencyGains.size() == 0) {
        double value = 1;
        if (!strcmp("integral", timeGainsNormalization))
            value /= (endTime - startTime).dbl();
        if (!strcmp("integral", frequencyGainsNormalization))
            value /= bandwidth.get();
        return makeShared<Boxcar2DFunction<double, simsec, Hz>>(simsec(startTime), simsec(endTime), centerFrequency - bandwidth / 2, centerFrequency + bandwidth / 2, value);
    }
    else {
        Ptr<const IFunction<double, Domain<simsec>>> timeGainFunction;
        if (timeGains.size() != 0) {
            auto centerTime = (startTime + endTime) / 2;
            auto duration = endTime - startTime;
            std::map<simsec, std::pair<double, const IInterpolator<simsec, double> *>> ts;
            ts[getLowerBound<simsec>()] = { 0, firstTimeInterpolator };
            ts[getUpperBound<simsec>()] = { 0, nullptr };
            for (const auto& entry : timeGains) {
                simsec time;
                switch (entry.where) {
                    case 's': time = simsec(startTime); break;
                    case 'e': time = simsec(endTime); break;
                    case 'c': time = simsec(centerTime); break;
                    case ' ': time = simsec(0); break;
                    default: throw cRuntimeError("Unknown qualifier");
                }
                time += simsec(duration) * entry.length + entry.offset;
                ts[time] = { entry.gain, entry.interpolator };
            }
            timeGainFunction = makeShared<Interpolated1DFunction<double, simsec>>(ts);
        }
        else
            timeGainFunction = makeShared<Boxcar1DFunction<double, simsec>>(simsec(startTime), simsec(endTime), 1);
        Ptr<const IFunction<double, Domain<Hz>>> frequencyGainFunction;
        if (frequencyGains.size() != 0) {
            auto startFrequency = centerFrequency - bandwidth / 2;
            auto endFrequency = centerFrequency + bandwidth / 2;
            std::map<Hz, std::pair<double, const IInterpolator<Hz, double> *>> fs;
            fs[getLowerBound<Hz>()] = { 0, firstFrequencyInterpolator };
            fs[getUpperBound<Hz>()] = { 0, nullptr };
            for (const auto& entry : frequencyGains) {
                Hz frequency;
                switch (entry.where) {
                    case 's': frequency = startFrequency; break;
                    case 'e': frequency = endFrequency; break;
                    case 'c': frequency = centerFrequency; break;
                    case ' ': frequency = Hz(0); break;
                    default: throw cRuntimeError("Unknown qualifier");
                }
                frequency += bandwidth * entry.length + entry.offset;
                ASSERT(!std::isnan(frequency.get()));
                fs[frequency] = { entry.gain, entry.interpolator };
            }
            frequencyGainFunction = makeShared<Interpolated1DFunction<double, Hz>>(fs);
        }
        else
            frequencyGainFunction = makeShared<Boxcar1DFunction<double, Hz>>(centerFrequency - bandwidth / 2, centerFrequency + bandwidth / 2, 1);
        return makeShared<Combined2DFunction<double, simsec, Hz>>(normalize<simsec>(timeGainFunction, timeGainsNormalization), normalize<Hz>(frequencyGainFunction, frequencyGainsNormalization));
    }
}

Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> DimensionalTransmitterBase::createPowerFunction(const simtime_t startTime, const simtime_t endTime, Hz centerFrequency, Hz bandwidth, W power) const
{
    Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> powerFunction;
    if (gainFunctionCacheLimit == 0) {
        if (timeGains.size() == 0 && frequencyGains.size() == 0)
            powerFunction = makeShared<Boxcar2DFunction<WpHz, simsec, Hz>>(simsec(startTime), simsec(endTime), centerFrequency - bandwidth / 2, centerFrequency + bandwidth / 2, power / bandwidth);
        else {
            auto gainFunction = createGainFunction(startTime, endTime, centerFrequency, bandwidth);
            powerFunction = makeShared<ConstantFunction<WpHz, Domain<simsec, Hz>>>(power / Hz(1))->multiply(gainFunction);
        }
    }
    else {
        Ptr<const IFunction<double, Domain<simsec, Hz>>> gainFunction;
        std::tuple<simtime_t, Hz, Hz> key(endTime - startTime, centerFrequency, bandwidth);
        auto it = gainFunctionCache.find(key);
        if (it != gainFunctionCache.end())
            gainFunction = it->second;
        else {
            gainFunction = createGainFunction(0, endTime - startTime, Hz(0), bandwidth);
            gainFunctionCache[key] = gainFunction;
            if ((int)gainFunctionCache.size() == gainFunctionCacheLimit)
                gainFunctionCache.clear();
        }
        Point<simsec, Hz> shift(simsec(startTime), centerFrequency);
        auto shiftedGainFunction = makeShared<DomainShiftedFunction<double, Domain<simsec, Hz>>>(gainFunction, shift);
        powerFunction = makeShared<ConstantFunction<WpHz, Domain<simsec, Hz>>>(power / Hz(1))->multiply(shiftedGainFunction);
    }
    return makeFirstQuadrantLimitedFunction(powerFunction);
}

} // namespace physicallayer

} // namespace inet

