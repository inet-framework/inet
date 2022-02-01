//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IENERGYSOURCE_H
#define __INET_IENERGYSOURCE_H

#include "inet/power/contract/IEnergyConsumer.h"

namespace inet {

namespace power {

/**
 * This class is a base interface that must be implemented by energy source
 * models to integrate with other parts of the power model. This interface is
 * extended by various energy source interfaces. Actual energy source
 * implementations should implement one of the derived interfaces.
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API IEnergySource
{
  public:
    virtual ~IEnergySource() {}

    /**
     * Returns the number of energy consumers in the range [0, +infinity).
     */
    virtual int getNumEnergyConsumers() const = 0;

    /**
     * Returns the energy consumer for the provided index. This functions throws
     * an exception if the index is out of range, and it never returns nullptr.
     */
    virtual const IEnergyConsumer *getEnergyConsumer(int index) const = 0;

    /**
     * Adds a new energy consumer to the energy source. The energyConsumer
     * parameter must not be nullptr.
     */
    virtual void addEnergyConsumer(const IEnergyConsumer *energyConsumer) = 0;

    /**
     * Removes a previously added energy consumer from this energy source.
     * This functions throws an exception if the consumer is not found.
     */
    virtual void removeEnergyConsumer(const IEnergyConsumer *energyConsumer) = 0;
};

} // namespace power

} // namespace inet

#endif

