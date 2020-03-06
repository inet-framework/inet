//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DIMENSIONALSNIR_H
#define __INET_DIMENSIONALSNIR_H

#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalNoise.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalReception.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/SnirBase.h"

namespace inet {

namespace physicallayer {

class INET_API DimensionalSnir : public SnirBase
{
  protected:
    mutable double minSNIR;
    mutable double maxSNIR;
    mutable double meanSNIR;

  protected:
    virtual double computeMin() const;
    virtual double computeMax() const;
    virtual double computeMean() const;

  public:
    DimensionalSnir(const DimensionalReception *reception, const DimensionalNoise *noise);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual double getMin() const override;
    virtual double getMax() const override;
    virtual double getMean() const override;

    virtual const Ptr<const IFunction<double, Domain<simsec, Hz>>> getSnir() const;
};

} // namespace physicallayer

} // namespace inet

#endif

