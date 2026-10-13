//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPATHLOSS_H
#define __INET_IPATHLOSS_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IArrival.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReceptionPathLoss.h"
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
     * Returns the path loss of the reception of the provided transmission by
     * the provided receiver radio, for the provided arrival. The medium
     * analog model calls this once per reception, when it computes the
     * reception, and evaluates the result at one or more frequencies, at that
     * time or later. A model that keeps per-link state resolves it in this
     * call. The returned object may refer to the path loss model, but not to
     * the transmission, the arrival or the radios: the medium may delete them
     * while the reception's power is still evaluated.
     */
    virtual Ptr<const IReceptionPathLoss> computeReceptionPathLoss(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const = 0;

    /**
     * Returns the loss factor as a function of propagation speed, carrier
     * frequency and distance, independently of the transmitter and the
     * receiver. The value is in the range [0, 1] where 1 means no loss at all
     * and 0 means all power is lost, or NaN if the model cannot tell the loss
     * from the distance alone.
     */
    virtual double computeDistancePathLoss(mps propagationSpeed, Hz frequency, m distance) const = 0;

    /**
     * Returns the range for the given loss factor. The value is in the range
     * [0, +infinity) or NaN if unspecified.
     */
    virtual m computeRange(mps propagationSpeed, Hz frequency, double loss) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

