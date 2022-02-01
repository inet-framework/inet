//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ENERGYSOURCEBASE_H
#define __INET_ENERGYSOURCEBASE_H

#include "inet/power/contract/IEnergySource.h"

namespace inet {

namespace power {

class INET_API EnergySourceBase : public virtual IEnergySource
{
  protected:
    std::vector<const IEnergyConsumer *> energyConsumers;

  public:
    virtual int getNumEnergyConsumers() const override { return energyConsumers.size(); }
    virtual const IEnergyConsumer *getEnergyConsumer(int index) const override;
    virtual void addEnergyConsumer(const IEnergyConsumer *energyConsumer) override;
    virtual void removeEnergyConsumer(const IEnergyConsumer *energyConsumer) override;
};

} // namespace power

} // namespace inet

#endif

