//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRECEPTIONDECISION_H
#define __INET_IRECEPTIONDECISION_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"

namespace inet {

namespace physicallayer {

/**
 * This interface represents the decisions of a receiver's reception process.
 *
 * This interface is strictly immutable to safely support parallel computation.
 */
class INET_API IReceptionDecision : public IPrintableObject
{
  public:
    /**
     * Returns the corresponding reception that also specifies the receiver
     * and the received transmission. This function never returns nullptr.
     */
    virtual const IReception *getReception() const = 0;

    /**
     * Returns the signal part of this decision.
     */
    virtual IRadioSignal::SignalPart getSignalPart() const = 0;

    /**
     * Returns whether reception was possible according to the physical
     * properties of the received radio signal.
     */
    virtual bool isReceptionPossible() const = 0;

    /**
     * Returns whether the receiver decided to attempt the reception or
     * it decided to ignore it.
     */
    virtual bool isReceptionAttempted() const = 0;

    /**
     * Returns whether the reception was completely successful or not.
     */
    virtual bool isReceptionSuccessful() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

