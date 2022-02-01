//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ICCENERGYSOURCE_H
#define __INET_ICCENERGYSOURCE_H

#include "inet/power/contract/IEnergySource.h"

namespace inet {

namespace power {

/**
 * TODO
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API ICcEnergySource : public virtual IEnergySource
{
  public:
    /**
     * The signal that is used to publish current consumption changes.
     */
    static simsignal_t currentConsumptionChangedSignal;

  public:
    /**
     * Returns the open circuit voltage in the range [0, +infinity).
     */
    virtual V getNominalVoltage() const = 0;

    /**
     * Returns the output voltage in the ragne [0, +infinity).
     */
    virtual V getOutputVoltage() const = 0;

    /**
     * Returns the total current consumption in the range [0, +infinity).
     */
    virtual A getTotalCurrentConsumption() const = 0;
};

} // namespace power

} // namespace inet

#endif

