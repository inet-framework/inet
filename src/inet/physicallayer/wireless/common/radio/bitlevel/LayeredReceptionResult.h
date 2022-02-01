//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LAYEREDRECEPTIONRESULT_H
#define __INET_LAYEREDRECEPTIONRESULT_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalBitModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalPacketModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSampleModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSymbolModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReceptionResult.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ReceptionResult.h"

namespace inet {
namespace physicallayer {

class INET_API LayeredReceptionResult : public ReceptionResult
{
  protected:
    const IReceptionPacketModel *packetModel;
    const IReceptionBitModel *bitModel;
    const IReceptionSymbolModel *symbolModel;
    const IReceptionSampleModel *sampleModel;
    const IReceptionAnalogModel *analogModel;

  public:
    LayeredReceptionResult(const IReception *reception, const std::vector<const IReceptionDecision *> *decisions, const IReceptionPacketModel *packetModel, const IReceptionBitModel *bitModel, const IReceptionSymbolModel *symbolModel, const IReceptionSampleModel *sampleModel, const IReceptionAnalogModel *analogModel);
    virtual ~LayeredReceptionResult();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IReceptionPacketModel *getPacketModel() const { return packetModel; }
    virtual const IReceptionBitModel *getBitModel() const { return bitModel; }
    virtual const IReceptionSymbolModel *getSymbolModel() const { return symbolModel; }
    virtual const IReceptionSampleModel *getSampleModel() const { return sampleModel; }
    virtual const IReceptionAnalogModel *getAnalogModel() const { return analogModel; }

    virtual const Packet *getPacket() const override;
};

} // namespace physicallayer
} // namespace inet

#endif

