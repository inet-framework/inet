//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UNITDISKANALOGMODEL_H
#define __INET_UNITDISKANALOGMODEL_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IAnalogModel.h"

namespace inet {

namespace physicallayer {

/**
 * Implements the UnitDiskAnalogModel model, see the NED file for details.
 */
class INET_API UnitDiskAnalogModel : public cModule, public IAnalogModel
{
  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const IReception *computeReception(const IRadio *radio, const ITransmission *transmission, const IArrival *arrival) const override;
    virtual const INoise *computeNoise(const IListening *listening, const IInterference *interference) const override;
    virtual const INoise *computeNoise(const IReception *reception, const INoise *noise) const override;
    virtual const ISnir *computeSNIR(const IReception *reception, const INoise *noise) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

