//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_DIGITALANALOGCONVERTER_H
#define __INET_DIGITALANALOGCONVERTER_H

#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/ScalarSignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/PhysicalLayerDefs.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IDigitalAnalogConverter.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSampleModel.h"

namespace inet {
namespace physicallayer {

class INET_API ScalarDigitalAnalogConverter : public IDigitalAnalogConverter
{
  protected:
    W power;
    // TODO: why centerFrequency and bandwidth here? why not in the shaper
    Hz centerFrequency;
    Hz bandwidth;
    double sampleRate;

  public:
    ScalarDigitalAnalogConverter();

    virtual const ITransmissionAnalogModel *convertDigitalToAnalog(const ITransmissionSampleModel *sampleModel) const override;
};

} // namespace physicallayer
} // namespace inet

#endif

