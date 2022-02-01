//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEPENERGYCONSUMER_H
#define __INET_IEPENERGYCONSUMER_H

#include "inet/power/contract/IEnergyConsumer.h"

namespace inet {

namespace power {

/**
 * This class is an interface that should be implemented by power based energy
 * consumer models to integrate with other parts of the power based energy model.
 * Such an energy consumer model describes the energy consumption over time by
 * providing a function that computes the power consumption in watts.
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API IEpEnergyConsumer : public virtual IEnergyConsumer
{
  public:
    /**
     * The signal that is used to publish power consumption changes.
     */
    static simsignal_t powerConsumptionChangedSignal;

  public:
    /**
     * Returns the power consumption in the range [0, +infinity).
     */
    virtual W getPowerConsumption() const = 0;
};

} // namespace power

} // namespace inet

#endif

