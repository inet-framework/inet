//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ICCENERGYSINK_H
#define __INET_ICCENERGYSINK_H

#include "inet/power/contract/IEnergySink.h"

namespace inet {

namespace power {

/**
 * TODO
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API ICcEnergySink : public virtual IEnergySink
{
  public:
    /**
     * The signal that is used to publish current generation changes.
     */
    static simsignal_t currentGenerationChangedSignal;

  public:
    /**
     * Returns the total current generation in the range [0, +infinity).
     */
    virtual A getTotalCurrentGeneration() const = 0;
};

} // namespace power

} // namespace inet

#endif

