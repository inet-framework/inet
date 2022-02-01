//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPROPAGATION_H
#define __INET_IPROPAGATION_H

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IArrival.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmission.h"

namespace inet {

namespace physicallayer {

/**
 * This interface models how a radio signal propagates through space over time.
 */
class INET_API IPropagation : public IPrintableObject
{
  public:
    /**
     * Returns the theoretical propagation speed of radio signals in the range
     * (0, +infinity). The value might be different from the approximation
     * provided by the actual computation of arrival times.
     */
    virtual mps getPropagationSpeed() const = 0;

    /**
     * Returns the time and space coordinates when the transmission arrives
     * at the object that moves with the provided mobility. The result might
     * be an approximation only, because there's a tradeoff between precision
     * and performance. This function never returns nullptr.
     */
    virtual const IArrival *computeArrival(const ITransmission *transmission, IMobility *mobility) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

