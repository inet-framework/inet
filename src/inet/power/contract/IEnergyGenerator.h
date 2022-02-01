//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IENERGYGENERATOR_H
#define __INET_IENERGYGENERATOR_H

#include "inet/power/base/PowerDefs.h"

namespace inet {

namespace power {

class IEnergySink;

/**
 * This class is a base interface that must be implemented by energy generator
 * models to integrate with other parts of the power model. Energy generators
 * connect to an energy sink that absorbs the generated energy. Energy
 * generators are required to notify their energy sink when their energy
 * generation changes. This interface is extended by various energy generator
 * interfaces. Actual energy generator implementations should implement one of
 * the derived interfaces.
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API IEnergyGenerator
{
  public:
    virtual ~IEnergyGenerator() {}

    /**
     * Returns the energy sink that absorbs energy from this energy generator.
     * This function never returns nullptr.
     */
    virtual IEnergySink *getEnergySink() const = 0;
};

} // namespace power

} // namespace inet

#endif

