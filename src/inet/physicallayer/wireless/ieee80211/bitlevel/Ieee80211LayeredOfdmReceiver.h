//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211LAYEREDOFDMRECEIVER_H
#define __INET_IEEE80211LAYEREDOFDMRECEIVER_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/SnirReceiverBase.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IAnalogDigitalConverter.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IDecoder.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IDemodulator.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ILayeredErrorModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IPulseFilter.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IErrorModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmMode.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211LayeredOfdmReceiver : public SnirReceiverBase
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
    mutable const Ieee80211OfdmMode *mode = nullptr;
    const ILayeredErrorModel *errorModel = nullptr;
    const IDecoder *dataDecoder = nullptr;
    const IDecoder *signalDecoder = nullptr;
    const IDemodulator *dataDemodulator = nullptr;
    const IDemodulator *signalDemodulator = nullptr;
    const IPulseFilter *pulseFilter = nullptr;
    const IAnalogDigitalConverter *analogDigitalConverter = nullptr;

    W energyDetection;
    W sensitivity;
    Hz centerFrequency;
    Hz bandwidth;
    Hz channelSpacing;
    double snirThreshold;
    bool isCompliant;

  protected:
    virtual void initialize(int stage) override;

    /* Analog and sample domain */
    const IReceptionAnalogModel *createAnalogModel(const LayeredTransmission *transmission, const ISnir *snir) const;
    const IReceptionSampleModel *createSampleModel(const LayeredTransmission *transmission, const ISnir *snir) const;

    /* Symbol domain */
    const IReceptionSymbolModel *createSignalFieldSymbolModel(const IReceptionSymbolModel *symbolModel) const;
    const IReceptionSymbolModel *createDataFieldSymbolModel(const IReceptionSymbolModel *symbolModel) const;
    const IReceptionSymbolModel *createSymbolModel(const LayeredTransmission *transmission, const ISnir *snir) const;
    const IReceptionSymbolModel *createCompleteSymbolModel(const IReceptionSymbolModel *signalFieldSymbolModel, const IReceptionSymbolModel *dataFieldSymbolModel) const;

    /* Bit domain */
    const IReceptionBitModel *createSignalFieldBitModel(const IReceptionBitModel *bitModel, const IReceptionSymbolModel *symbolModel) const;
    const IReceptionBitModel *createDataFieldBitModel(const IReceptionBitModel *bitModel, const IReceptionSymbolModel *symbolModel, const IReceptionPacketModel *signalFieldPacketModel, const IReceptionBitModel *signalFieldBitModel) const;
    const IReceptionBitModel *createBitModel(const LayeredTransmission *transmission, const ISnir *snir) const;
    const IReceptionBitModel *createCompleteBitModel(const IReceptionBitModel *signalFieldBitModel, const IReceptionBitModel *dataFieldBitModel) const;

    /* Packet domain */
    const IReceptionPacketModel *createSignalFieldPacketModel(const IReceptionBitModel *signalFieldBitModel) const;
    const IReceptionPacketModel *createDataFieldPacketModel(const IReceptionBitModel *signalFieldBitModel, const IReceptionBitModel *dataFieldBitModel, const IReceptionPacketModel *signalFieldPacketModel) const;
    const IReceptionPacketModel *createPacketModel(const LayeredTransmission *transmission, const ISnir *snir) const;
    const IReceptionPacketModel *createCompletePacketModel(const char *name, const IReceptionPacketModel *signalFieldPacketModel, const IReceptionPacketModel *dataFieldPacketModel) const;

    const Ieee80211OfdmMode *computeMode(Hz bandwidth) const;
    unsigned int getSignalFieldLength(const BitVector *signalField) const;
    unsigned int calculatePadding(unsigned int dataFieldLengthInBits, const ApskModulationBase *modulation, double codeRate) const;
    double getCodeRateFromDecoderModule(const IDecoder *decoder) const;

  public:
    virtual ~Ieee80211LayeredOfdmReceiver();

    virtual const Ieee80211OfdmMode *getMode(const Packet *packet) const;
    virtual bool computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const override;
    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const override;
    virtual const IListeningDecision *computeListeningDecision(const IListening *listening, const IInterference *interference) const override;
    virtual const IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition) const override;
    virtual const IReceptionResult *computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const override;
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
};

} /* namespace physicallayer */
} /* namespace inet */

#endif

