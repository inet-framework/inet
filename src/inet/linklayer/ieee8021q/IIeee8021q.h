//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IIEEE8021Q_H
#define __INET_IIEEE8021Q_H

#include "inet/common/Protocol.h"
#include "inet/linklayer/common/MacAddress.h"

namespace inet {

class INET_API IIeee8021q
{
  public:
    class ICallback {
      public:
        virtual ~ICallback() {}
    };

  public:
    virtual void bind(int socketId, const Protocol *protocol, int vlanId, bool steal) = 0;
};

} // namespace inet

#endif

