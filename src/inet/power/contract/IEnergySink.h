//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IENERGYSINK_H
#define __INET_IENERGYSINK_H

#include "inet/power/contract/IEnergyGenerator.h"

namespace inet {

namespace power {

/**
 * This class is a base interface that must be implemented by energy sink
 * models to integrate with other parts of the power model. Energy generators
 * connect to an energy sink, and they notify the energy sink when their energy
 * generation changes. This interface is extended by various energy sink
 * interfaces. Actual energy sink implementations should implement one of the
 * derived interfaces.
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API IEnergySink
{
  public:
    virtual ~IEnergySink() {}

    /**
     * Returns the number of energy generators in the range [0, +infinity).
     */
    virtual int getNumEnergyGenerators() const = 0;

    /**
     * Returns the energy generator for the provided index. This functions throws
     * an exception if the index is out of range, and it never returns nullptr.
     */
    virtual const IEnergyGenerator *getEnergyGenerator(int index) const = 0;

    /**
     * Adds a new energy generator to the energy sink. The energyGenerator
     * parameter must not be nullptr.
     */
    virtual void addEnergyGenerator(const IEnergyGenerator *energyGenerator) = 0;

    /**
     * Removes a previously added energy generator from this energy sink.
     * This functions throws an exception if the generator is not found.
     */
    virtual void removeEnergyGenerator(const IEnergyGenerator *energyGenerator) = 0;
};

} // namespace power

} // namespace inet

#endif

