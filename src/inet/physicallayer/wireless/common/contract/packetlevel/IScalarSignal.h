//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ISCALARSIGNAL_H
#define __INET_ISCALARSIGNAL_H

#include "inet/common/IPrintableObject.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/math/Functions.h"

namespace inet {
namespace physicallayer {

class INET_API IScalarSignal : public virtual ISignalAnalogModel
{
  public:
    virtual W getPower() const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

