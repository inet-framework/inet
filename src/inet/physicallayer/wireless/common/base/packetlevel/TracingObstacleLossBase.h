//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TRACINGOBSTACLELOSSBASE_H
#define __INET_TRACINGOBSTACLELOSSBASE_H

#include "inet/environment/contract/IPhysicalObject.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITracingObstacleLoss.h"

namespace inet {

namespace physicallayer {

class INET_API TracingObstacleLossBase : public cModule, public ITracingObstacleLoss
{
};

} // namespace physicallayer

} // namespace inet

#endif

