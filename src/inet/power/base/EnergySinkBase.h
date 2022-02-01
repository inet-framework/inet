//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ENERGYSINKBASE_H
#define __INET_ENERGYSINKBASE_H

#include "inet/power/contract/IEnergySink.h"

namespace inet {

namespace power {

class INET_API EnergySinkBase : public virtual IEnergySink
{
  protected:
    std::vector<const IEnergyGenerator *> energyGenerators;

  public:
    virtual int getNumEnergyGenerators() const override { return energyGenerators.size(); }
    virtual const IEnergyGenerator *getEnergyGenerator(int index) const override;
    virtual void addEnergyGenerator(const IEnergyGenerator *energyGenerator) override;
    virtual void removeEnergyGenerator(const IEnergyGenerator *energyGenerator) override;
};

} // namespace power

} // namespace inet

#endif

