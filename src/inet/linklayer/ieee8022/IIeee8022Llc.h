//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IIEEE8022LLC_H
#define __INET_IIEEE8022LLC_H

#include "inet/common/Protocol.h"
#include "inet/linklayer/common/MacAddress.h"

namespace inet {

class INET_API IIeee8022Llc
{
  public:
    virtual void open(int socketId, int interfaceId, int localSap, int remoteSap) = 0;
};

} // namespace inet

#endif

