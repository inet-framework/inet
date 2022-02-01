//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LAYEREDSCALARANALOGMODEL_H
#define __INET_LAYEREDSCALARANALOGMODEL_H

#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredReception.h"
#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/ScalarSignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/ScalarAnalogModelBase.h"

namespace inet {

namespace physicallayer {

class INET_API LayeredScalarAnalogModel : public ScalarAnalogModelBase
{
  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IReception *computeReception(const IRadio *radio, const ITransmission *transmission, const IArrival *arrival) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

