//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IDEALEPENERGYSTORAGE_H
#define __INET_IDEALEPENERGYSTORAGE_H

#include "inet/power/base/EpEnergyStorageBase.h"

namespace inet {

namespace power {

/**
 * This class implements an ideal energy storage.
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API IdealEpEnergyStorage : public EpEnergyStorageBase
{
  protected:
    J energyBalance = J(NaN);
    simtime_t lastEnergyBalanceUpdate = -1;

  protected:
    virtual void initialize(int stage) override;

    virtual void updateTotalPowerConsumption() override;
    virtual void updateTotalPowerGeneration() override;
    virtual void updateEnergyBalance();

  public:
    virtual J getNominalEnergyCapacity() const override { return J(INFINITY); }
    virtual J getResidualEnergyCapacity() const override { return J(INFINITY); }
    virtual J getEnergyBalance() { return energyBalance; }
};

} // namespace power

} // namespace inet

#endif

