//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEPENERGYMANAGEMENT_H
#define __INET_IEPENERGYMANAGEMENT_H

#include "inet/power/contract/IEnergyManagement.h"

namespace inet {

namespace power {

/**
 * TODO
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API IEpEnergyManagement : public IEnergyManagement
{
  public:
    /**
     * Returns the estimated energy capacity in the range [0, +infinity).
     * It specifies the amount of energy that the energy storage contains at
     * the moment according to the estimation of the management.
     */
    virtual J getEstimatedEnergyCapacity() const = 0;
};

} // namespace power

} // namespace inet

#endif

