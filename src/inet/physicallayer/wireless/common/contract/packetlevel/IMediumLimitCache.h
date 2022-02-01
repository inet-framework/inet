//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IMEDIUMLIMITCACHE_H
#define __INET_IMEDIUMLIMITCACHE_H

#include "inet/common/IPrintableObject.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {

namespace physicallayer {

/**
 * This interface is used to cache various medium limits among the radios.
 */
class INET_API IMediumLimitCache : public IPrintableObject
{
  public:
    /**
     * Notifies the cache when a radio is added to the medium.
     */
    virtual void addRadio(const IRadio *radio) = 0;

    /**
     * Notifies the cache when a radio is removed from the medium.
     */
    virtual void removeRadio(const IRadio *radio) = 0;

    /**
     * Returns the minimum possible position among the radios, the coordinate
     * values are in the range (-infinity, +infinity) or NaN if unspecified.
     */
    virtual Coord getMinConstraintArea() const = 0;

    /**
     * Returns the maximum possible position among the radios, the coordinate
     * values are in the range (-infinity, +infinity) or NaN if unspecified.
     */
    virtual Coord getMaxConstraintArea() const = 0;

    /**
     * Returns the maximum possible speed among the radios, the value is
     * in the range [0, +infinity) or NaN if unspecified.
     */
    virtual mps getMaxSpeed() const = 0;

    /**
     * Returns the maximum possible transmission power among the radios, the
     * value is in the range [0, +infinity) or NaN if unspecified.
     */
    virtual W getMaxTransmissionPower() const = 0;

    /**
     * Returns the minimum possible interference power among the radios, the
     * value is in the range [0, +infinity) or NaN if unspecified.
     */
    virtual W getMinInterferencePower() const = 0;

    /**
     * Returns the minimum possible reception power among the radios, the
     * value is in the range [0, +infinity) or NaN if unspecified.
     */
    virtual W getMinReceptionPower() const = 0;

    /**
     * Returns the maximum possible antenna gain among the radios, the value
     * is in the range [1, +infinity) or NaN if unspecified.
     */
    virtual double getMaxAntennaGain() const = 0;

    /**
     * Returns the minimum required signal interference time among the radios,
     * the value is in the range [0, +infinity) or NaN if unspecified.
     */
    virtual const simtime_t& getMinInterferenceTime() const = 0;

    /**
     * Returns the maximum possible transmission durations among the radios,
     * the value is in the range [0, +infinity) or NaN if unspecified.
     */
    virtual const simtime_t& getMaxTransmissionDuration() const = 0;

    /**
     * Returns the maximum possible communication range among the radios, the
     * value is in the range [0, +infinity) or NaN if unspecified.
     */
    virtual m getMaxCommunicationRange() const = 0;

    /**
     * Returns the maximum possible interference range among the radios, the
     * value is in the range [0, +infinity) or NaN if unspecified.
     */
    virtual m getMaxInterferenceRange() const = 0;

    /**
     * Returns the maximum possible communication range for a given radio, the
     * value is in the range [0, +infinity) or NaN if unspecified.
     */
    virtual m getMaxCommunicationRange(const IRadio *radio) const = 0;

    /**
     * Returns the maximum possible interference range for a given radio, the
     * value is in the range [0, +infinity) or NaN if unspecified.
     */
    virtual m getMaxInterferenceRange(const IRadio *radio) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

