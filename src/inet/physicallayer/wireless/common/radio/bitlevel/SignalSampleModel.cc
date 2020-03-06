//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSampleModel.h"

namespace inet {
namespace physicallayer {

SignalSampleModel::SignalSampleModel(int headerSampleLength, double headerSampleRate, int dataSampleLength, double dataSampleRate, const std::vector<W> *samples) :
    headerSampleLength(headerSampleLength),
    headerSampleRate(headerSampleRate),
    dataSampleLength(dataSampleLength),
    dataSampleRate(dataSampleRate),
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
        stream << EV_FIELD(headerSampleLength)
               << EV_FIELD(headerSampleRate)
               << EV_FIELD(dataSampleLength)
               << EV_FIELD(dataSampleRate);
    return stream;
}

TransmissionSampleModel::TransmissionSampleModel(int headerSampleLength, double headerSampleRate, int dataSampleLength, double dataSampleRate, const std::vector<W> *samples) :
    SignalSampleModel(headerSampleLength, headerSampleRate, dataSampleLength, dataSampleRate, samples)
{
}

ReceptionSampleModel::ReceptionSampleModel(int headerSampleLength, double headerSampleRate, int dataSampleLength, double dataSampleRate, const std::vector<W> *samples) :
    SignalSampleModel(headerSampleLength, headerSampleRate, dataSampleLength, dataSampleRate, samples)
{
}

} // namespace physicallayer
} // namespace inet

