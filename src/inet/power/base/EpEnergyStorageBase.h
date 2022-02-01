//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_EPENERGYSTORAGEBASE_H
#define __INET_EPENERGYSTORAGEBASE_H

#include "inet/power/base/EpEnergySinkBase.h"
#include "inet/power/base/EpEnergySourceBase.h"
#include "inet/power/contract/IEpEnergyStorage.h"

namespace inet {

namespace power {

class INET_API EpEnergyStorageBase : public cSimpleModule, public EpEnergySourceBase, public EpEnergySinkBase, public virtual IEpEnergyStorage
{
  protected:
    virtual void initialize(int stage) override;

    virtual void updateTotalPowerConsumption() override;
    virtual void updateTotalPowerGeneration() override;

  public:
    virtual void addEnergyConsumer(const IEnergyConsumer *energyConsumer) override;
    virtual void removeEnergyConsumer(const IEnergyConsumer *energyConsumer) override;

    virtual void addEnergyGenerator(const IEnergyGenerator *energyGenerator) override;
    virtual void removeEnergyGenerator(const IEnergyGenerator *energyGenerator) override;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, double value, cObject *details) override;
};

} // namespace power

} // namespace inet

#endif

