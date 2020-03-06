//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_APSKLAYEREDRECEIVER_H
#define __INET_APSKLAYEREDRECEIVER_H

#include "inet/physicallayer/wireless/apsk/packetlevel/ApskPhyHeader_m.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/SnirReceiverBase.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IAnalogDigitalConverter.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IDecoder.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IDemodulator.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ILayeredErrorModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IPulseFilter.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IErrorModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/ConvolutionalCode.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalPacketModel.h"

namespace inet {

namespace physicallayer {

class INET_API ApskLayeredReceiver : public SnirReceiverBase
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
    const ILayeredErrorModel *errorModel;
    const IDecoder *decoder;
    const IDemodulator *demodulator;
    const IPulseFilter *pulseFilter;
    const IAnalogDigitalConverter *analogDigitalConverter;
    W energyDetection;
    W sensitivity;
    Hz centerFrequency;
    Hz bandwidth;
    double snirThreshold;

  protected:
    virtual void initialize(int stage) override;

    virtual const IReceptionAnalogModel *createAnalogModel(const LayeredTransmission *transmission, const ISnir *snir) const;
    virtual const IReceptionSampleModel *createSampleModel(const LayeredTransmission *transmission, const ISnir *snir, const IReceptionAnalogModel *analogModel) const;
    virtual const IReceptionSymbolModel *createSymbolModel(const LayeredTransmission *transmission, const ISnir *snir, const IReceptionSampleModel *sampleModel) const;
    virtual const IReceptionBitModel *createBitModel(const LayeredTransmission *transmission, const ISnir *snir, const IReceptionSymbolModel *symbolModel) const;
    virtual const IReceptionPacketModel *createPacketModel(const LayeredTransmission *transmission, const ISnir *snir, const IReceptionBitModel *bitModel) const;

  public:
    ApskLayeredReceiver();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const IDecoder *getDecoder() const { return decoder; }
    virtual const IDemodulator *getModulator() const { return demodulator; }
    virtual const IPulseFilter *getPulseFilter() const { return pulseFilter; }
    virtual const IAnalogDigitalConverter *getAnalogDigitalConverter() const { return analogDigitalConverter; }
    virtual bool computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const override;
    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const override;
    virtual const IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition) const override;
    virtual const IListeningDecision *computeListeningDecision(const IListening *listening, const IInterference *interference) const override;
    virtual const IReceptionResult *computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

