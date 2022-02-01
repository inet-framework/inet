//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IFRAGMENTERPOLICY_H
#define __INET_IFRAGMENTERPOLICY_H

#include "inet/common/packet/Packet.h"

namespace inet {

class INET_API IFragmenterPolicy
{
  public:
    virtual std::vector<b> computeFragmentLengths(Packet *packet) const = 0;
};

} // namespace inet

#endif

