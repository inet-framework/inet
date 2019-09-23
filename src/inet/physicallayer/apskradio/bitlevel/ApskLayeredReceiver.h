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

#ifndef __INET_APSKLAYEREDRECEIVER_H
#define __INET_APSKLAYEREDRECEIVER_H

#include "inet/physicallayer/apskradio/packetlevel/ApskPhyHeader_m.h"
#include "inet/physicallayer/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/base/packetlevel/SnirReceiverBase.h"
#include "inet/physicallayer/common/bitlevel/ConvolutionalCode.h"
#include "inet/physicallayer/common/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/contract/bitlevel/IAnalogDigitalConverter.h"
#include "inet/physicallayer/contract/bitlevel/IDecoder.h"
#include "inet/physicallayer/contract/bitlevel/IDemodulator.h"
#include "inet/physicallayer/contract/bitlevel/ILayeredErrorModel.h"
#include "inet/physicallayer/contract/bitlevel/IPulseFilter.h"
#include "inet/physicallayer/contract/packetlevel/IErrorModel.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"

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

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual const IDecoder *getDecoder() const { return decoder; }
    virtual const IDemodulator *getModulator() const { return demodulator; }
    virtual const IPulseFilter *getPulseFilter() const { return pulseFilter; }
    virtual const IAnalogDigitalConverter *getAnalogDigitalConverter() const { return analogDigitalConverter; }
    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const override;
    virtual const IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const override;
    virtual const IListeningDecision *computeListeningDecision(const IListening *listening, const IInterference *interference) const override;
    virtual const IReceptionResult *computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_APSKLAYEREDRECEIVER_H

