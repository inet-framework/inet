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

#ifndef __INET_APSKLAYEREDTRANSMITTER_H
#define __INET_APSKLAYEREDTRANSMITTER_H

#include "inet/physicallayer/contract/packetlevel/ITransmitter.h"
#include "inet/physicallayer/contract/bitlevel/IEncoder.h"
#include "inet/physicallayer/contract/bitlevel/IModulator.h"
#include "inet/physicallayer/contract/bitlevel/IPulseShaper.h"
#include "inet/physicallayer/contract/bitlevel/IDigitalAnalogConverter.h"
#include "inet/physicallayer/base/packetlevel/APSKModulationBase.h"
#include "inet/physicallayer/apskradio/bitlevel/APSKPhyFrame_m.h"

namespace inet {

namespace physicallayer {

class INET_API APSKLayeredTransmitter : public ITransmitter, public cSimpleModule
{
  public:
    enum LevelOfDetail {
        PACKET_DOMAIN,
        BIT_DOMAIN,
        SYMBOL_DOMAIN,
        SAMPLE_DOMAIN,
    };

  protected:
    LevelOfDetail levelOfDetail;
    const IEncoder *encoder;
    const IModulator *modulator;
    const IPulseShaper *pulseShaper;
    const IDigitalAnalogConverter *digitalAnalogConverter;
    W power;
    bps bitrate;
    Hz bandwidth;
    Hz carrierFrequency;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual int computePaddingLength(BitVector *bits) const;
    virtual const APSKPhyFrame *createPhyFrame(const cPacket *macFrame) const;
    virtual const ITransmissionPacketModel *createPacketModel(const APSKPhyFrame *phyFrame) const;
    virtual const ITransmissionBitModel *createBitModel(const ITransmissionPacketModel *packetModel) const;
    virtual const ITransmissionSymbolModel *createSymbolModel(const ITransmissionBitModel *bitModel) const;
    virtual const ITransmissionSampleModel *createSampleModel(const ITransmissionSymbolModel *symbolModel) const;
    virtual const ITransmissionAnalogModel *createAnalogModel(const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel, const ITransmissionSampleModel *sampleModel) const;

  public:
    APSKLayeredTransmitter();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual const IEncoder *getEncoder() const { return encoder; }
    virtual const IModulator *getModulator() const { return modulator; }
    virtual const IPulseShaper *getPulseShaper() const { return pulseShaper; }
    virtual const IDigitalAnalogConverter *getDigitalAnalogConverter() const { return digitalAnalogConverter; }
    virtual W getPower() const { return power; }
    virtual bps getBitrate() const { return bitrate; }
    virtual const Hz getBandwidth() const { return bandwidth; }
    virtual const Hz getCarrierFrequency() const { return carrierFrequency; }
    virtual W getMaxPower() const override { return power; }
    virtual m getMaxCommunicationRange() const override { return m(NaN); }
    virtual m getMaxInterferenceRange() const override { return m(NaN); }
    virtual const ITransmission *createTransmission(const IRadio *radio, const cPacket *packet, const simtime_t startTime) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_APSKLAYEREDTRANSMITTER_H

