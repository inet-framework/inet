//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPHYSICALLAYER_H
#define __INET_IPHYSICALLAYER_H

#include "inet/physicallayer/wireless/common/base/packetlevel/PhysicalLayerDefs.h"

namespace inet {

/**
 * This purely virtual interface provides an abstraction for different physical layers.
 */
class INET_API IPhysicalLayer
{
  public:
    virtual ~IPhysicalLayer() {}
};

} // namespace inet

#endif

