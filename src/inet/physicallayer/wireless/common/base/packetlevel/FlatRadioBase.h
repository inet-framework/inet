//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FLATRADIOBASE_H
#define __INET_FLATRADIOBASE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandRadioBase.h"

namespace inet {

namespace physicallayer {

// REFACTOR TODO
class INET_API FlatRadioBase : public NarrowbandRadioBase
{
  public:
    FlatRadioBase();

    virtual void setPower(W newPower);
    virtual void setBitrate(bps newBitrate);
};

} // namespace physicallayer

} // namespace inet

#endif

