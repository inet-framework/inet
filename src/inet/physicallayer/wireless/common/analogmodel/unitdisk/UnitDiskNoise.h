//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UNITDISKNOISE_H
#define __INET_UNITDISKNOISE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/NoiseBase.h"
#include "inet/physicallayer/wireless/common/analogmodel/unitdisk/UnitDiskReceptionAnalogModel.h"

namespace inet {

namespace physicallayer {

class INET_API UnitDiskNoise : public NoiseBase
{
  public:
    using Power = UnitDiskReceptionAnalogModel::Power;

  protected:
    Power minPower;
    Power maxPower;

  public:

    UnitDiskNoise(simtime_t startTime, simtime_t endTime, Power minPower, Power maxPower);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual bool isInterfering() const { return maxPower >= Power::POWER_INTERFERING; }

    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const override;
    virtual W computeMaxPower(simtime_t startTime, simtime_t endTime) const override;

    Power getMinPower() const { return minPower; }
    Power getMaxPower() const { return maxPower; }
};

} // namespace physicallayer

} // namespace inet

#endif

