//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MEDIUMLIMITCACHE_H
#define __INET_MEDIUMLIMITCACHE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IMediumLimitCache.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"

namespace inet {
namespace physicallayer {

class INET_API MediumLimitCache : public cModule, public IMediumLimitCache
{
  protected:
    /**
     * The corresponding radio medium is never nullptr.
     */
    const IRadioMedium *radioMedium;

    /**
     * The list of communicating radios on the medium.
     */
    std::vector<const IRadio *> radios;

    /** @name Various radio medium limits. */
    /**
     * The constraint area minimum among the radios is in the range [-infinity,
     * +infinity] or NaN if unspecified.
     */
    Coord minConstraintArea;
    /**
     * The constraint area maximum among the radios is in the range [-infinity,
     * +infinity] or NaN if unspecified.
     */
    Coord maxConstraintArea;
    /**
     * The maximum speed among the radios is in the range [0, +infinity) or
     * NaN if unspecified.
     */
    mps maxSpeed;
    /**
     * The maximum transmission power among the radio transmitters is in the
     * range [0, +infinity) or NaN if unspecified.
     */
    W maxTransmissionPower;
    /**
     * The minimum interference power among the radio receivers is in the
     * range [0, +infinity) or NaN if unspecified.
     */
    W minInterferencePower;
    /**
     * The minimum reception power among the radio receivers is in the range
     * [0, +infinity) or NaN if unspecified.
     */
    W minReceptionPower;
    /**
     * The maximum gain among the radio antennas is in the range [1, +infinity).
     */
    double maxAntennaGain;
    /**
     * The minimum overlapping in time needed to consider two transmissions
     * interfering.
     */
    // TODO compute from longest frame duration, maximum mobility speed and signal propagation time?
    simtime_t minInterferenceTime;
    /**
     * The maximum transmission duration of a radio signal.
     */
    // TODO compute from maximum bit length and minimum bitrate?
    simtime_t maxTransmissionDuration;
    /**
     * The maximum communication range where a transmission can still be
     * potentially successfully received is in the range [0, +infinity) or
     * NaN if unspecified.
     */
    m maxCommunicationRange;
    /**
     * The maximum interference range where a transmission is still considered
     * to some effect on other transmissions is in the range [0, +infinity)
     * or NaN if unspecified.
     */
    m maxInterferenceRange;
    //@}

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    /** @name Compute limits */
    //@{
    virtual Coord computeMinConstraintArea() const;
    virtual Coord computeMaxConstreaintArea() const;

    virtual mps computeMaxSpeed() const;

    virtual W computeMaxTransmissionPower() const;
    virtual W computeMinInterferencePower() const;
    virtual W computeMinReceptionPower() const;

    virtual double computeMaxAntennaGain() const;

    virtual const simtime_t computeMinInterferenceTime() const;
    virtual const simtime_t computeMaxTransmissionDuration() const;

    virtual m computeMaxRange(W maxTransmissionPower, W minReceptionPower) const;
    virtual m computeMaxCommunicationRange() const;
    virtual m computeMaxInterferenceRange() const;

    virtual void updateLimits();
    //@}

  public:
    MediumLimitCache();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual void addRadio(const IRadio *radio) override;
    virtual void removeRadio(const IRadio *radio) override;

    /** @name Query limits */
    //@{
    virtual Coord getMinConstraintArea() const override { return minConstraintArea; }
    virtual Coord getMaxConstraintArea() const override { return maxConstraintArea; }

    virtual mps getMaxSpeed() const override { return maxSpeed; }

    virtual W getMaxTransmissionPower() const override { return maxTransmissionPower; }
    virtual W getMinInterferencePower() const override { return minInterferencePower; }
    virtual W getMinReceptionPower() const override { return minReceptionPower; }

    virtual double getMaxAntennaGain() const override { return maxAntennaGain; }

    virtual const simtime_t& getMinInterferenceTime() const override { return minInterferenceTime; }
    virtual const simtime_t& getMaxTransmissionDuration() const override { return maxTransmissionDuration; }

    virtual m getMaxCommunicationRange() const override { return maxCommunicationRange; }
    virtual m getMaxInterferenceRange() const override { return maxInterferenceRange; }

    virtual m getMaxCommunicationRange(const IRadio *radio) const override;
    virtual m getMaxInterferenceRange(const IRadio *radio) const override;
    //@}
};

} // namespace physicallayer
} // namespace inet

#endif

