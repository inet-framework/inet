//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEPENERGYSINK_H
#define __INET_IEPENERGYSINK_H

#include "inet/power/contract/IEnergySink.h"

namespace inet {

namespace power {

/**
 * TODO
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API IEpEnergySink : public virtual IEnergySink
{
  public:
    /**
     * The signal that is used to publish power generation changes.
     */
    static simsignal_t powerGenerationChangedSignal;

  public:
    /**
     * Returns the total power generation in the range [0, +infinity).
     */
    virtual W getTotalPowerGeneration() const = 0;
};

} // namespace power

} // namespace inet

#endif

