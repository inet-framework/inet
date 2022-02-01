//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSampleModel.h"

namespace inet {
namespace physicallayer {

SignalSampleModel::SignalSampleModel(int sampleLength, double sampleRate, const std::vector<W> *samples) :
    sampleLength(sampleLength),
    sampleRate(sampleRate),
    samples(samples)
{
}

SignalSampleModel::~SignalSampleModel()
{
    delete samples;
}

std::ostream& SignalSampleModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "SignalSampleModel";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(sampleLength)
               << EV_FIELD(sampleRate);
    return stream;
}

TransmissionSampleModel::TransmissionSampleModel(int sampleLength, double sampleRate, const std::vector<W> *samples) :
    SignalSampleModel(sampleLength, sampleRate, samples)
{
}

ReceptionSampleModel::ReceptionSampleModel(int sampleLength, double sampleRate, const std::vector<W> *samples, W rssi) :
    SignalSampleModel(sampleLength, sampleRate, samples),
    rssi(rssi)
{
}

} // namespace physicallayer
} // namespace inet

