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

/**
 * This interface represents the analog domain of a radio signal with a time and
 * frequency independent constant power.
 */
class INET_API IScalarSignalAnalogModel : public virtual ISignalAnalogModel
{
  public:
    virtual W getPower() const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

