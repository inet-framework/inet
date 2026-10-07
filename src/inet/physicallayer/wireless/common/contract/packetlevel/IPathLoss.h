//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPATHLOSS_H
#define __INET_IPATHLOSS_H

#include "inet/common/math/IFunction.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IArrival.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmission.h"

namespace inet {

namespace physicallayer {

class IRadio;

/**
 * This interface models path loss (or path attenuation) that is the reduction
 * in power density of a radio signal as it propagates through space.
 */
class INET_API IPathLoss : public virtual IPrintableObject
{
  public:
    /**
     * Returns the loss factor between the transmitter of the provided
     * transmission and the provided receiver radio, for the provided arrival,
     * as a function of frequency. The values are in the range [0, 1] where 1
     * means no loss at all and 0 means all power is lost.
     *
     * The medium analog model calls this once per reception: the scalar one
     * evaluates the function at the center frequency, the dimensional one
     * across the band, possibly several times and at any later time. A model
     * that keeps per-link state therefore resolves that state in this call,
     * and the function it returns only depends on the frequency.
     */
    virtual Ptr<const math::IFunction<double, math::Domain<Hz>>> computeReceptionPathLoss(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const = 0;

    /**
     * Returns the loss factor as a function of propagation speed, carrier
     * frequency and distance. The value is in the range [0, 1] where 1 means
     * no loss at all and 0 means all power is lost.
     */
    virtual double computePathLoss(mps propagationSpeed, Hz frequency, m distance) const = 0;

    /**
     * Returns the range for the given loss factor. The value is in the range
     * [0, +infinity) or NaN if unspecified.
     */
    virtual m computeRange(mps propagationSpeed, Hz frequency, double loss) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

