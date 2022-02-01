//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_EPENERGYSINKBASE_H
#define __INET_EPENERGYSINKBASE_H

#include "inet/power/base/EnergySinkBase.h"
#include "inet/power/contract/IEpEnergyGenerator.h"
#include "inet/power/contract/IEpEnergySink.h"

namespace inet {

namespace power {

class INET_API EpEnergySinkBase : public EnergySinkBase, public virtual IEpEnergySink, public cListener
{
  protected:
    W totalPowerGeneration = W(NaN);

  protected:
    virtual W computeTotalPowerGeneration() const;
    virtual void updateTotalPowerGeneration();

  public:
    virtual W getTotalPowerGeneration() const override { return totalPowerGeneration; }

    virtual void addEnergyGenerator(const IEnergyGenerator *energyGenerator) override;
    virtual void removeEnergyGenerator(const IEnergyGenerator *energyGenerator) override;
};

} // namespace power

} // namespace inet

#endif

