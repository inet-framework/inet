//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCALARSNIR_H
#define __INET_SCALARSNIR_H

#include "inet/physicallayer/wireless/common/base/packetlevel/SnirBase.h"

namespace inet {

namespace physicallayer {

class INET_API ScalarSnir : public SnirBase
{
  protected:
    mutable double minSNIR;
    mutable double maxSNIR;
    mutable double meanSNIR;

  protected:
    virtual double computeMin() const;
    virtual double computeMax() const;

  public:
    ScalarSnir(const IReception *reception, const INoise *noise);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual double getMin() const override;
    virtual double getMax() const override;
    virtual double getMean() const override;

    virtual double computeMean(simtime_t startTime, simtime_t endTime) const;
};

} // namespace physicallayer

} // namespace inet

#endif

