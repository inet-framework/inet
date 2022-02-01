//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPHYSICALLAYERFRAME_H
#define __INET_IPHYSICALLAYERFRAME_H

#include "inet/physicallayer/wireless/common/base/packetlevel/PhysicalLayerDefs.h"

namespace inet {

/**
 * This purely virtual interface provides an abstraction for different physical layer frames.
 */
class INET_API IPhysicalLayerFrame
{
  public:
    virtual ~IPhysicalLayerFrame() {}
};

} // namespace inet

#endif

