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

#include "inet/physicallayer/layered/DigitalAnalogConverter.h"

namespace inet {

namespace physicallayer {

ScalarDigitalAnalogConverter::ScalarDigitalAnalogConverter() :
    power(W(sNaN)),
    carrierFrequency(Hz(sNaN)),
    bandwidth(Hz(sNaN)),
    sampleRate(sNaN)
{}

const ITransmissionAnalogModel *ScalarDigitalAnalogConverter::convertDigitalToAnalog(const ITransmissionSampleModel *sampleModel) const
{
    const simtime_t duration = sampleModel->getSampleLength() / sampleModel->getSampleRate();
    return new ScalarTransmissionAnalogModel(duration, power, carrierFrequency, bandwidth);
}

} // namespace physicallayer

} // namespace inet
