//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ISCALARSIGNAL_H
#define __INET_ISCALARSIGNAL_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalAnalogModel.h"

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

