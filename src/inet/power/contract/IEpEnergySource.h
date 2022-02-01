//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEPENERGYSOURCE_H
#define __INET_IEPENERGYSOURCE_H

#include "inet/power/contract/IEnergySource.h"

namespace inet {

namespace power {

/**
 * This class is an interface that should be implemented by power based energy
 * source models to integrate with other parts of the power based model. Such
 * an energy source model describes its total energy consumption in terms of
 * power consumption
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API IEpEnergySource : public virtual IEnergySource
{
  public:
    /**
     * The signal that is used to publish power consumption changes.
     */
    static simsignal_t powerConsumptionChangedSignal;

  public:
    /**
     * Returns the total power consumption in the range [0, +infinity).
     */
    virtual W getTotalPowerConsumption() const = 0;
};

} // namespace power

} // namespace inet

#endif

