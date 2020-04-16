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

#ifndef __INET_IEEE80211LAYEREDOFDMRECEIVER_H
#define __INET_IEEE80211LAYEREDOFDMRECEIVER_H

#include "inet/physicallayer/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/base/packetlevel/SnirReceiverBase.h"
#include "inet/physicallayer/common/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/contract/bitlevel/IAnalogDigitalConverter.h"
#include "inet/physicallayer/contract/bitlevel/IDecoder.h"
#include "inet/physicallayer/contract/bitlevel/IDemodulator.h"
#include "inet/physicallayer/contract/bitlevel/ILayeredErrorModel.h"
#include "inet/physicallayer/contract/bitlevel/IPulseFilter.h"
#include "inet/physicallayer/contract/packetlevel/IErrorModel.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmMode.h"

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

    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const override;
    virtual const IListeningDecision *computeListeningDecision(const IListening *listening, const IInterference *interference) const override;
    virtual const IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const override;
    virtual const IReceptionResult *computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const override;
    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
};

} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211LAYEREDOFDMRECEIVER_H

