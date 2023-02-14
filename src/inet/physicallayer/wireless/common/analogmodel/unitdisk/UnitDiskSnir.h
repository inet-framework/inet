//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UNITDISKSNIR_H
#define __INET_UNITDISKSNIR_H

#include "inet/physicallayer/wireless/common/base/packetlevel/SnirBase.h"

namespace inet {

namespace physicallayer {

class INET_API UnitDiskSnir : public SnirBase
{
    bool isSnirInfinite;

  public:
    UnitDiskSnir(const IReception *reception, const INoise *noise);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual double getMin() const override { return isSnirInfinite ? INFINITY : 0; }
    virtual double getMax() const override { return isSnirInfinite ? INFINITY : 0; }
    virtual double getMean() const override { return isSnirInfinite ? INFINITY : 0; }
};

} // namespace physicallayer

} // namespace inet

#endif

