//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IDIMENSIONALSIGNAL_H
#define __INET_IDIMENSIONALSIGNAL_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalAnalogModel.h"

namespace inet {
namespace physicallayer {

class INET_API IDimensionalSignalAnalogModel : public virtual ISignalAnalogModel
{
  public:
    virtual const Ptr<const math::IFunction<WpHz, math::Domain<simsec, Hz>>>& getPower() const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

