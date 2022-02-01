//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ITAGGEDOBJECT_H
#define __INET_ITAGGEDOBJECT_H

#include "inet/common/packet/tag/SharingTagSet.h"

namespace inet {

class INET_API ITaggedObject
{
  public:
    virtual SharingTagSet& getTags() = 0;
};

} // namespace inet

#endif

