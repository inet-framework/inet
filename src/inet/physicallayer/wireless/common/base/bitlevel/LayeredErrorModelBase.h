//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LAYEREDERRORMODELBASE_H
#define __INET_LAYEREDERRORMODELBASE_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ILayeredErrorModel.h"

namespace inet {

namespace physicallayer {

class INET_API LayeredErrorModelBase : public cModule, public ILayeredErrorModel
{
  protected:
    virtual const IReceptionPacketModel *computePacketModel(const LayeredTransmission *transmission, double packetErrorRate) const;
    virtual const IReceptionBitModel *computeBitModel(const LayeredTransmission *transmission, double bitErrorRate) const;
    virtual const IReceptionSymbolModel *computeSymbolModel(const LayeredTransmission *transmission, double symbolErrorRate) const;
};

} // namespace physicallayer

} // namespace inet

#endif

