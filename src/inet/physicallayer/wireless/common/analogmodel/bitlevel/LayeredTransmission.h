//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LAYEREDTRANSMISSION_H
#define __INET_LAYEREDTRANSMISSION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/TransmissionBase.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalBitModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalPacketModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSampleModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSymbolModel.h"

namespace inet {
namespace physicallayer {

class INET_API LayeredTransmission : public TransmissionBase
{
  protected:
    const ITransmissionPacketModel *packetModel;
    const ITransmissionBitModel *bitModel;
    const ITransmissionSymbolModel *symbolModel;
    const ITransmissionSampleModel *sampleModel;
    const ITransmissionAnalogModel *analogModel;

  public:
    LayeredTransmission(const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel, const ITransmissionSymbolModel *symbolModel, const ITransmissionSampleModel *sampleModel, const ITransmissionAnalogModel *analogModel, const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation);
    virtual ~LayeredTransmission();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const ITransmissionPacketModel *getPacketModel() const { return packetModel; }
    virtual const ITransmissionBitModel *getBitModel()    const { return bitModel; }
    virtual const ITransmissionSymbolModel *getSymbolModel() const { return symbolModel; }
    virtual const ITransmissionSampleModel *getSampleModel() const { return sampleModel; }
    virtual const ITransmissionAnalogModel *getAnalogModel() const override { return analogModel; }

    virtual const Packet *getPacket() const override { return packetModel->getPacket(); }
};

} // namespace physicallayer
} // namespace inet

#endif

