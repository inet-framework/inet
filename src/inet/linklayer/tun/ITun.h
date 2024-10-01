//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ITUN_H
#define __INET_ITUN_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API ITun
{
  public:
    virtual void open(int socketId) = 0;
};

} // namespace inet

#endif

