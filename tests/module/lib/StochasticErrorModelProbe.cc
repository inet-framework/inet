//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "StochasticErrorModelProbe.h"

namespace inet {

using namespace physicallayer;

Define_Module(StochasticErrorModelProbe);

void StochasticErrorModelProbe::Range::add(double value)
{
    count++;
    if (std::isnan(value))
        nanCount++;
    else {
        min = std::min(min, value);
        max = std::max(max, value);
    }
}

void StochasticErrorModelProbe::record(const char *rate, IRadioSignal::SignalPart part, double value) const
{
    ranges[std::string(rate) + "." + IRadioSignal::getSignalPartName(part)].add(value);
}

double StochasticErrorModelProbe::computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    double result = StochasticErrorModel::computePacketErrorRate(snir, part);
    record("packetErrorRate", part, result);
    if (part == IRadioSignal::SIGNAL_PART_WHOLE) {
        for (auto otherPart : { IRadioSignal::SIGNAL_PART_HEADER, IRadioSignal::SIGNAL_PART_DATA }) {
            record("packetErrorRate", otherPart, StochasticErrorModel::computePacketErrorRate(snir, otherPart));
            record("bitErrorRate", otherPart, StochasticErrorModel::computeBitErrorRate(snir, otherPart));
            record("symbolErrorRate", otherPart, StochasticErrorModel::computeSymbolErrorRate(snir, otherPart));
        }
    }
    return result;
}

double StochasticErrorModelProbe::computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    double result = StochasticErrorModel::computeBitErrorRate(snir, part);
    record("bitErrorRate", part, result);
    return result;
}

double StochasticErrorModelProbe::computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    double result = StochasticErrorModel::computeSymbolErrorRate(snir, part);
    record("symbolErrorRate", part, result);
    return result;
}

void StochasticErrorModelProbe::finish()
{
    for (auto& entry : ranges) {
        recordScalar((entry.first + ".count").c_str(), entry.second.count);
        recordScalar((entry.first + ".nanCount").c_str(), entry.second.nanCount);
        if (entry.second.count > entry.second.nanCount) {
            recordScalar((entry.first + ".min").c_str(), entry.second.min);
            recordScalar((entry.first + ".max").c_str(), entry.second.max);
        }
    }
}

} // namespace inet
