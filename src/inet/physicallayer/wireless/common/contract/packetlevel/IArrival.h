//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IARRIVAL_H
#define __INET_IARRIVAL_H

#include "inet/common/IPrintableObject.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioSignal.h"

namespace inet {

namespace physicallayer {

class IRadio;

/**
 * This interface represents the space and time coordinates of a transmission
 * arriving at a receiver.
 *
 * This interface is strictly immutable to safely support parallel computation.
 */
class INET_API IArrival : public virtual IPrintableObject
{
  public:
    /**
     * Returns the radio this arrival was computed for. Together with the
     * transmission's transmitter radio it identifies the link, which is what
     * a model keeping per-link state (e.g. a correlated shadowing) is keyed
     * by. This function never returns nullptr.
     */
    virtual const IRadio *getReceiverRadio() const = 0;

    virtual const simtime_t getStartPropagationTime() const = 0;
    virtual const simtime_t getEndPropagationTime() const = 0;

    virtual const simtime_t getStartTime() const = 0;
    virtual const simtime_t getEndTime() const = 0;

    virtual const simtime_t getStartTime(IRadioSignal::SignalPart part) const = 0;
    virtual const simtime_t getEndTime(IRadioSignal::SignalPart part) const = 0;

    virtual const simtime_t getPreambleStartTime() const = 0;
    virtual const simtime_t getPreambleEndTime() const = 0;
    virtual const simtime_t getHeaderStartTime() const = 0;
    virtual const simtime_t getHeaderEndTime() const = 0;
    virtual const simtime_t getDataStartTime() const = 0;
    virtual const simtime_t getDataEndTime() const = 0;

    virtual const Coord& getStartPosition() const = 0;
    virtual const Coord& getEndPosition() const = 0;

    virtual const Quaternion& getStartOrientation() const = 0;
    virtual const Quaternion& getEndOrientation() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

