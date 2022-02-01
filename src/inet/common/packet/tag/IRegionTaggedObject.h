//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IREGIONTAGGEDOBJECT_H
#define __INET_IREGIONTAGGEDOBJECT_H

#include "inet/common/packet/tag/SharingRegionTagSet.h"

namespace inet {

class INET_API IRegionTaggedObject
{
  public:
    virtual SharingRegionTagSet& getRegionTags() = 0;
};

} // namespace inet

#endif

