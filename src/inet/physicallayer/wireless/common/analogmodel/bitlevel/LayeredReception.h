//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LAYEREDRECEPTION_H
#define __INET_LAYEREDRECEPTION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ReceptionBase.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalBitModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalPacketModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSampleModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSymbolModel.h"

namespace inet {
namespace physicallayer {

class INET_API LayeredReception : public ReceptionBase
{
  protected:
    // TODO where are the other models?
//    const IReceptionPacketModel *packetModel;
//    const IReceptionBitModel    *bitModel;
//    const IReceptionSymbolModel *symbolModel;
//    const IReceptionSampleModel *sampleModel;
    const IReceptionAnalogModel *analogModel;

  public:
    LayeredReception(const IReceptionAnalogModel *analogModel, const IRadio *radio, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation);
    virtual ~LayeredReception();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IReceptionAnalogModel *getAnalogModel() const override { return analogModel; }
};

} // namespace physicallayer
} // namespace inet

#endif

