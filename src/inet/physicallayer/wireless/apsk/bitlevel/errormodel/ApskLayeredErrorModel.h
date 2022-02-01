//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_APSKLAYEREDERRORMODEL_H
#define __INET_APSKLAYEREDERRORMODEL_H

#include "inet/physicallayer/wireless/common/base/bitlevel/LayeredErrorModelBase.h"

namespace inet {

namespace physicallayer {

class INET_API ApskLayeredErrorModel : public LayeredErrorModelBase
{
  public:
    ApskLayeredErrorModel();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IReceptionPacketModel *computePacketModel(const LayeredTransmission *transmission, const ISnir *snir) const override;
    virtual const IReceptionBitModel *computeBitModel(const LayeredTransmission *transmission, const ISnir *snir) const override;
    virtual const IReceptionSymbolModel *computeSymbolModel(const LayeredTransmission *transmission, const ISnir *snir) const override;
    virtual const IReceptionSampleModel *computeSampleModel(const LayeredTransmission *transmission, const ISnir *snir) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

