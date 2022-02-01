//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEPENERGYSTORAGE_H
#define __INET_IEPENERGYSTORAGE_H

#include "inet/power/contract/IEnergyStorage.h"
#include "inet/power/contract/IEpEnergySink.h"
#include "inet/power/contract/IEpEnergySource.h"

namespace inet {

namespace power {

/**
 * TODO
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API IEpEnergyStorage : public virtual IEpEnergySource, public virtual IEpEnergySink, public virtual IEnergyStorage
{
  public:
    /**
     * The signal that is used to publish residual energy capacity changes also
     * including when the energy storage becomes completely depleted or charged.
     */
    static simsignal_t residualEnergyCapacityChangedSignal;

  public:
    /**
     * Returns the nominal energy capacity in the range [0, +infinity]. It
     * specifies the maximum amount of energy that the energy storage can
     * contain.
     */
    virtual J getNominalEnergyCapacity() const = 0;

    /**
     * Returns the residual energy capacity in the range [0, nominalCapacity].
     * It specifies the amount of energy that the energy storage contains at
     * the moment.
     */
    virtual J getResidualEnergyCapacity() const = 0;
};

} // namespace power

} // namespace inet

#endif

