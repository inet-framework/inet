//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ICCENERGYGENERATOR_H
#define __INET_ICCENERGYGENERATOR_H

#include "inet/power/contract/IEnergyGenerator.h"

namespace inet {

namespace power {

/**
 * TODO
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API ICcEnergyGenerator : public virtual IEnergyGenerator
{
  public:
    /**
     * The signal that is used to publish current generation changes.
     */
    static simsignal_t currentGenerationChangedSignal;

  public:
    /**
     * Returns the current generation in the range [0, +infinity).
     */
    virtual A getCurrentGeneration() const = 0;
};

} // namespace power

} // namespace inet

#endif

