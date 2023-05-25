//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IDIMENSIONALSIGNAL_H
#define __INET_IDIMENSIONALSIGNAL_H

#include "inet/common/IPrintableObject.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/math/Functions.h"

namespace inet {
namespace physicallayer {

class INET_API IDimensionalSignal : public virtual ISignalAnalogModel
{
  public:
    virtual const Ptr<const math::IFunction<WpHz, math::Domain<simsec, Hz>>>& getPower() const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

