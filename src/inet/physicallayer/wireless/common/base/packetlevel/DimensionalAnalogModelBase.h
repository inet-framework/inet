//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DIMENSIONALANALOGMODELBASE_H
#define __INET_DIMENSIONALANALOGMODELBASE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/AnalogModelBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"

namespace inet {

namespace physicallayer {

using namespace inet::math;

class INET_API DimensionalAnalogModelBase : public AnalogModelBase
{
  protected:
    bool attenuateWithCenterFrequency;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> computeReceptionPower(const IRadio *radio, const ITransmission *transmission, const IArrival *arrival) const;
    virtual const INoise *computeNoise(const IListening *listening, const IInterference *interference) const override;
    virtual const INoise *computeNoise(const IReception *reception, const INoise *noise) const override;
    virtual const ISnir *computeSNIR(const IReception *reception, const INoise *noise) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

