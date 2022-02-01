//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ICCENERGYSTORAGE_H
#define __INET_ICCENERGYSTORAGE_H

#include "inet/power/contract/ICcEnergySink.h"
#include "inet/power/contract/ICcEnergySource.h"
#include "inet/power/contract/IEnergyStorage.h"

namespace inet {

namespace power {

/**
 * TODO
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API ICcEnergyStorage : public virtual ICcEnergySource, public virtual ICcEnergySink, public virtual IEnergyStorage
{
  public:
    /**
     * The signal that is used to publish residual charge capacity changes also
     * including when the energy storage becomes completely depleted or charged.
     */
    static simsignal_t residualChargeCapacityChangedSignal;

  public:
    /**
     * Returns the nominal charge capacity in the range [0, +infinity). It
     * specifies the maximum amount of charge that the energy storage can
     * contain.
     */
    virtual C getNominalChargeCapacity() const = 0;

    /**
     * Returns the residual charge capacity in the range [0, nominalCapacity].
     * It specifies the amount of charge that the energy storage contains at
     * the moment.
     */
    virtual C getResidualChargeCapacity() const = 0;
};

} // namespace power

} // namespace inet

#endif

