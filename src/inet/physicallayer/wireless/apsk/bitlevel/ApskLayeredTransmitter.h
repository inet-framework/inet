//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_APSKLAYEREDTRANSMITTER_H
#define __INET_APSKLAYEREDTRANSMITTER_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/TransmitterBase.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IDigitalAnalogConverter.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IEncoder.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IModulator.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IPulseShaper.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmitter.h"

namespace inet {

namespace physicallayer {

class INET_API ApskLayeredTransmitter : public TransmitterBase
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
    Hz centerFrequency;

  protected:
    virtual void initialize(int stage) override;

    virtual const ITransmissionPacketModel *createPacketModel(const Packet *packet) const;
    virtual const ITransmissionBitModel *createBitModel(const ITransmissionPacketModel *packetModel) const;
    virtual const ITransmissionSymbolModel *createSymbolModel(const ITransmissionBitModel *bitModel) const;
    virtual const ITransmissionSampleModel *createSampleModel(const ITransmissionSymbolModel *symbolModel) const;
    virtual const ITransmissionAnalogModel *createAnalogModel(const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel, const ITransmissionSampleModel *sampleModel) const;

  public:
    ApskLayeredTransmitter();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const IEncoder *getEncoder() const { return encoder; }
    virtual const IModulator *getModulator() const { return modulator; }
    virtual const IPulseShaper *getPulseShaper() const { return pulseShaper; }
    virtual const IDigitalAnalogConverter *getDigitalAnalogConverter() const { return digitalAnalogConverter; }
    virtual W getPower() const { return power; }
    virtual bps getBitrate() const { return bitrate; }
    virtual const Hz getBandwidth() const { return bandwidth; }
    virtual const Hz getCenterFrequency() const { return centerFrequency; }
    virtual W getMaxPower() const override { return power; }
    virtual m getMaxCommunicationRange() const override { return m(NaN); }
    virtual m getMaxInterferenceRange() const override { return m(NaN); }
    virtual const ITransmission *createTransmission(const IRadio *radio, const Packet *packet, const simtime_t startTime) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

