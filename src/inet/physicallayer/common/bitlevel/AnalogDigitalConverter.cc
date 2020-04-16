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

#include "inet/physicallayer/common/bitlevel/AnalogDigitalConverter.h"

namespace inet {
namespace physicallayer {

ScalarAnalogDigitalConverter::ScalarAnalogDigitalConverter() :
    power(W(NaN)),
    centerFrequency(Hz(NaN)),
    bandwidth(Hz(NaN)),
    sampleRate(NaN)
{}

const IReceptionSampleModel *ScalarAnalogDigitalConverter::convertAnalogToDigital(const IReceptionAnalogModel *analogModel) const
{
    const simtime_t duration = analogModel->getDuration();
    const int sampleLength = std::ceil(duration.dbl() / sampleRate);
    return new ReceptionSampleModel(sampleLength, sampleRate, nullptr, W(0));
}

} // namespace physicallayer
} // namespace inet

