//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CCENERGYSOURCEBASE_H
#define __INET_CCENERGYSOURCEBASE_H

#include "inet/power/base/EnergySourceBase.h"
#include "inet/power/contract/ICcEnergyConsumer.h"
#include "inet/power/contract/ICcEnergySource.h"

namespace inet {

namespace power {

class INET_API CcEnergySourceBase : public EnergySourceBase, public virtual ICcEnergySource, public cListener
{
  protected:
    A totalCurrentConsumption = A(NaN);

  protected:
    virtual A computeTotalCurrentConsumption() const;
    virtual void updateTotalCurrentConsumption();

  public:
    virtual A getTotalCurrentConsumption() const override { return totalCurrentConsumption; }

    virtual void addEnergyConsumer(const IEnergyConsumer *energyConsumer) override;
    virtual void removeEnergyConsumer(const IEnergyConsumer *energyConsumer) override;
};

} // namespace power

} // namespace inet

#endif

