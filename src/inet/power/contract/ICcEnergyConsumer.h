//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ICCENERGYCONSUMER_H
#define __INET_ICCENERGYCONSUMER_H

#include "inet/power/contract/IEnergyConsumer.h"

namespace inet {

namespace power {

/**
 * TODO
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API ICcEnergyConsumer : public virtual IEnergyConsumer
{
  public:
    /**
     * The signal that is used to publish current consumption changes.
     */
    static simsignal_t currentConsumptionChangedSignal;

  public:
    /**
     * Returns the current consumption in the range [0, +infinity).
     */
    virtual A getCurrentConsumption() const = 0;
};

} // namespace power

} // namespace inet

#endif

