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

#ifndef __INET_LAYEREDTRANSMITTER_H
#define __INET_LAYEREDTRANSMITTER_H

#include "inet/physicallayer/contract/IEncoder.h"
#include "inet/physicallayer/contract/IModulator.h"
#include "inet/physicallayer/contract/IPulseShaper.h"
#include "inet/physicallayer/contract/IDigitalAnalogConverter.h"
#include "inet/physicallayer/base/TransmitterBase.h"

namespace inet {

namespace physicallayer {

class INET_API LayeredTransmitter : public TransmitterBase
{
  protected:
    const IEncoder *encoder;
    const IModulator *modulator;
    const IPulseShaper *pulseShaper;
    const IDigitalAnalogConverter *digitalAnalogConverter;

  protected:
    virtual void initialize(int stage);
    virtual const ITransmissionPacketModel *createPacketModel(const cPacket *macFrame) const;

  public:
    LayeredTransmitter();

    virtual const ITransmission *createTransmission(const IRadio *radio, const cPacket *packet, const simtime_t startTime) const = 0;

    virtual const IEncoder *getEncoder() const { return encoder; }
    virtual const IModulator *getModulator() const { return modulator; }
    virtual const IPulseShaper *getPulseShaper() const{ return pulseShaper; }
    virtual const IDigitalAnalogConverter *getDigitalAnalogConverter() const { return digitalAnalogConverter; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_LAYEREDTRANSMITTER_H
