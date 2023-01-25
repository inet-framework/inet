//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211LAYEREDOFDMTRANSMITTER_H
#define __INET_IEEE80211LAYEREDOFDMTRANSMITTER_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/TransmitterBase.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IDigitalAnalogConverter.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IEncoder.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IModulator.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IPulseShaper.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmitter.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmMode.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211LayeredOfdmTransmitter : public TransmitterBase
{
    // TODO copy

  public:
    enum LevelOfDetail {
        PACKET_DOMAIN,
        BIT_DOMAIN,
        SYMBOL_DOMAIN,
        SAMPLE_DOMAIN,
    };

  protected:
    LevelOfDetail levelOfDetail;
    mutable const Ieee80211OfdmMode *mode = nullptr;
    const IEncoder *signalEncoder = nullptr;
    const IEncoder *dataEncoder = nullptr;
    const IModulator *signalModulator = nullptr;
    const IModulator *dataModulator = nullptr;
    const IPulseShaper *pulseShaper = nullptr;
    const IDigitalAnalogConverter *digitalAnalogConverter = nullptr;
    bool isCompliant;

    Hz bandwidth;
    Hz channelSpacing;
    Hz centerFrequency;
    W power;

  protected:
    virtual void initialize(int stage) override;

    /* Packet domain */
    const ITransmissionPacketModel *createSignalFieldPacketModel(const ITransmissionPacketModel *completePacketModel) const;
    const ITransmissionPacketModel *createDataFieldPacketModel(const ITransmissionPacketModel *completePacketModel) const;
    virtual const ITransmissionPacketModel *createPacketModel(const Packet *packet) const;

    /* Symbol and bit domain */
    const ITransmissionSymbolModel *createSymbolModel(const ITransmissionSymbolModel *signalFieldSymbolModel, const ITransmissionSymbolModel *dataFieldSymbolModel) const;
    const ITransmissionBitModel *createBitModel(const ITransmissionBitModel *signalFieldBitModel, const ITransmissionBitModel *dataFieldBitModel, const ITransmissionPacketModel *packetModel) const;
    void encodeAndModulate(const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *& fieldBitModel, const ITransmissionSymbolModel *& fieldSymbolModel, const IEncoder *encoder, const IModulator *modulator, bool isSignalField) const;

    /* Sample domain */
    const ITransmissionSampleModel *createSampleModel(const ITransmissionSymbolModel *symbolModel) const;

    /* Analog domain */
    const ITransmissionAnalogModel *createAnalogModel(simtime_t startTime, const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel, const ITransmissionSymbolModel *symbolModel, const ITransmissionSampleModel *sampleModel) const;
    const ITransmissionAnalogModel *createDimensionalAnalogModel(simtime_t startTime, const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel) const;

    const Ieee80211OfdmMode *computeMode(Hz bandwidth) const;

  public:
    virtual ~Ieee80211LayeredOfdmTransmitter();

    virtual const Ieee80211OfdmMode *getMode(const Packet *packet) const;
    virtual const ITransmission *createTransmission(const IRadio *radio, const Packet *packet, const simtime_t startTime) const override;
    virtual const IEncoder *getEncoder() const { return dataEncoder; }
    virtual const IModulator *getModulator() const { return dataModulator; }
    virtual const IPulseShaper *getPulseShaper() const { return pulseShaper; }
    virtual const IDigitalAnalogConverter *getDigitalAnalogConverter() const { return digitalAnalogConverter; }
    virtual W getMaxPower() const override { return power; }
    virtual m getMaxCommunicationRange() const override { return m(NaN); }
    virtual m getMaxInterferenceRange() const override { return m(NaN); }
    const Hz getBandwidth() const { return mode->getDataMode()->getBandwidth(); }
    const Hz getCenterFrequency() const { return centerFrequency; }
    const Hz getCarrierSpacing() const { return mode->getChannelSpacing(); }
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

