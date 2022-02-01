//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ICCENERGYMANAGEMENT_H
#define __INET_ICCENERGYMANAGEMENT_H

#include "inet/power/contract/IEnergyManagement.h"

namespace inet {

namespace power {

/**
 * TODO
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API ICcEnergyManagement : public IEnergyManagement
{
  public:
    /**
     * Returns the estimated charge capacity in the range [0, +infinity).
     * It specifies the amount of charge that the energy storage contains at
     * the moment according to the estimation of the management.
     */
    virtual C getEstimatedChargeCapacity() const = 0;
};

} // namespace power

} // namespace inet

#endif

