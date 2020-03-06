//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LAYEREDDIMENSIONALANALOGMODEL_H
#define __INET_LAYEREDDIMENSIONALANALOGMODEL_H

#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/DimensionalSignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredReception.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/DimensionalAnalogModelBase.h"

namespace inet {

namespace physicallayer {

class INET_API LayeredDimensionalAnalogModel : public DimensionalAnalogModelBase
{
  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IReception *computeReception(const IRadio *radio, const ITransmission *transmission, const IArrival *arrival) const override;
    virtual const ISnir *computeSNIR(const IReception *reception, const INoise *noise) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

