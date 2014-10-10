//
// Copyright (C) 2014 OpenSim Ltd.
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

#ifndef __INET_IEEE80211LAYEREDTRANSMITTER_H
#define __INET_IEEE80211LAYEREDTRANSMITTER_H

#include "inet/physicallayer/contract/IEncoder.h"
#include "inet/physicallayer/contract/IModulator.h"
#include "inet/physicallayer/contract/IPulseShaper.h"
#include "inet/physicallayer/contract/IDigitalAnalogConverter.h"
#include "inet/physicallayer/layered/LayeredTransmitter.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211LayeredTransmitter : public LayeredTransmitter
{
    protected:
        const IEncoder *encoder;
        const IModulator *modulator;
        const IPulseShaper *pulseShaper;
        const IDigitalAnalogConverter *digitalAnalogConverter;
        int codeRateK;
        int codeRateN;

    protected:
        virtual void initialize(int stage);
        virtual const ITransmissionPacketModel *createPacketModel(const cPacket *macFrame) const;
        void setCodeRateParameters(const char *codeRate);

    public:
        Ieee80211LayeredTransmitter();
        virtual const ITransmission *createTransmission(const IRadio *radio, const cPacket *packet, const simtime_t startTime) const = 0;
        virtual const IEncoder *getEncoder() const { return encoder; }
        virtual const IModulator *getModulator() const { return modulator; }
        virtual const IPulseShaper *getPulseShaper() const{ return pulseShaper; }
        virtual const IDigitalAnalogConverter *getDigitalAnalogConverter() const { return digitalAnalogConverter; }
};

} // namespace physicallayer
} // namespace inet

#endif /* __INET_IEEE80211LAYEREDTRANSMITTER_H */
