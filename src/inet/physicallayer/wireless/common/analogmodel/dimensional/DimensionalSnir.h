//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DIMENSIONALSNIR_H
#define __INET_DIMENSIONALSNIR_H

#include "inet/physicallayer/wireless/common/analogmodel/dimensional/DimensionalNoise.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/SnirBase.h"

namespace inet {

namespace physicallayer {

class INET_API DimensionalSnir : public SnirBase
{
  protected:
    mutable double minSNIR;
    mutable double maxSNIR;
    mutable double meanSNIR;
    // the limits and the values of the last interval query; a value is computed when first asked
    mutable simtime_t lastStartTime = -1;
    mutable simtime_t lastEndTime = -1;
    mutable double lastMinSNIR = NaN;
    mutable double lastMaxSNIR = NaN;
    mutable double lastMeanSNIR = NaN;
    // the quotient of the reception power and the noise power, computed once
    mutable Ptr<const IFunction<double, Domain<simsec, Hz>>> snir;

  protected:
    virtual void updateLastInterval(simtime_t startTime, simtime_t endTime) const;
    virtual Interval<simsec, Hz> computeInterval(simtime_t startTime, simtime_t endTime) const;
    virtual double computeMin(simtime_t startTime, simtime_t endTime) const;
    virtual double computeMax(simtime_t startTime, simtime_t endTime) const;
    virtual double computeMean(simtime_t startTime, simtime_t endTime) const;

  public:
    DimensionalSnir(const IReception *reception, const INoise *noise);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual double getMin() const override;
    virtual double getMax() const override;
    virtual double getMean() const override;

    virtual double getMin(simtime_t startTime, simtime_t endTime) const override;
    virtual double getMax(simtime_t startTime, simtime_t endTime) const override;
    virtual double getMean(simtime_t startTime, simtime_t endTime) const override;

    virtual const Ptr<const IFunction<double, Domain<simsec, Hz>>> getSnir() const;
};

} // namespace physicallayer

} // namespace inet

#endif

