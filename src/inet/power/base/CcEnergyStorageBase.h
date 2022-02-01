//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CCENERGYSTORAGEBASE_H
#define __INET_CCENERGYSTORAGEBASE_H

#include "inet/power/base/CcEnergySinkBase.h"
#include "inet/power/base/CcEnergySourceBase.h"
#include "inet/power/contract/ICcEnergyStorage.h"

namespace inet {

namespace power {

class INET_API CcEnergyStorageBase : public cSimpleModule, public CcEnergySourceBase, public CcEnergySinkBase, public virtual ICcEnergyStorage
{
  protected:
    virtual void initialize(int stage) override;

    virtual void updateTotalCurrentConsumption() override;
    virtual void updateTotalCurrentGeneration() override;

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

