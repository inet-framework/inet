//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEPENERGYGENERATOR_H
#define __INET_IEPENERGYGENERATOR_H

#include "inet/power/contract/IEnergyGenerator.h"

namespace inet {

namespace power {

/**
 * TODO
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API IEpEnergyGenerator : public virtual IEnergyGenerator
{
  public:
    /**
     * The signal that is used to publish power generation changes.
     */
    static simsignal_t powerGenerationChangedSignal;

  public:
    /**
     * Returns the power generation in the range [0, +infinity).
     */
    virtual W getPowerGeneration() const = 0;
};

} // namespace power

} // namespace inet

#endif

