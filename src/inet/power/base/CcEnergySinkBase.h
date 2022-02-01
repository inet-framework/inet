//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CCENERGYSINKBASE_H
#define __INET_CCENERGYSINKBASE_H

#include "inet/power/base/EnergySinkBase.h"
#include "inet/power/contract/ICcEnergyGenerator.h"
#include "inet/power/contract/ICcEnergySink.h"

namespace inet {

namespace power {

class INET_API CcEnergySinkBase : public EnergySinkBase, public virtual ICcEnergySink, public cListener
{
  protected:
    A totalCurrentGeneration = A(NaN);

  protected:
    virtual A computeTotalCurrentGeneration() const;
    virtual void updateTotalCurrentGeneration();

  public:
    virtual A getTotalCurrentGeneration() const override { return totalCurrentGeneration; }

    virtual void addEnergyGenerator(const IEnergyGenerator *energyGenerator) override;
    virtual void removeEnergyGenerator(const IEnergyGenerator *energyGenerator) override;
};

} // namespace power

} // namespace inet

#endif

