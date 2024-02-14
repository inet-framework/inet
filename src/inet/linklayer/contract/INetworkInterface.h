//
// Copyright (C) 2008 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INETWORKINTERFACE_H
#define __INET_INETWORKINTERFACE_H

#include "inet/common/INETDefs.h"

namespace inet {

class IInterfaceTable;

class INET_API INetworkInterface
{
  public:
    virtual ~INetworkInterface() {}

    virtual IInterfaceTable *getInterfaceTable() const = 0;

    virtual void startup() const = 0;
    virtual void shutdown() const = 0;
    virtual void crash() const = 0;
};

} // namespace inet

#endif

