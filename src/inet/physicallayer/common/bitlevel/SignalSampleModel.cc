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

#include "inet/physicallayer/common/bitlevel/SignalSampleModel.h"

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

std::ostream& SignalSampleModel::printToStream(std::ostream& stream, int level) const
{
    stream << "SignalSampleModel";
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", sampleLength = " << sampleLength
               << ", sampleRate = " << sampleRate;
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

