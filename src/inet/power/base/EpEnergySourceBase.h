//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_EPENERGYSOURCEBASE_H
#define __INET_EPENERGYSOURCEBASE_H

#include "inet/power/base/EnergySourceBase.h"
#include "inet/power/contract/IEpEnergyConsumer.h"
#include "inet/power/contract/IEpEnergySource.h"

namespace inet {

namespace power {

class INET_API EpEnergySourceBase : public EnergySourceBase, public virtual IEpEnergySource, public cListener
{
  protected:
    W totalPowerConsumption = W(NaN);

  protected:
    virtual W computeTotalPowerConsumption() const;
    virtual void updateTotalPowerConsumption();

  public:
    virtual W getTotalPowerConsumption() const override { return totalPowerConsumption; }

    virtual void addEnergyConsumer(const IEnergyConsumer *energyConsumer) override;
    virtual void removeEnergyConsumer(const IEnergyConsumer *energyConsumer) override;
};

} // namespace power

} // namespace inet

#endif

