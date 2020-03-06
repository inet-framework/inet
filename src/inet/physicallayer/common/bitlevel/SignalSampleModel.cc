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

std::ostream& SignalSampleModel::printToStream(std::ostream& stream, int level) const
{
    stream << "SignalSampleModel";
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", headerSampleLength = " << headerSampleLength
               << ", headerSampleRate = " << headerSampleRate
               << ", dataSampleLength = " << dataSampleLength
               << ", dataSampleRate = " << dataSampleRate;
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

