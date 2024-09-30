//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_VIRTUALINTERFACE_H
#define __INET_VIRTUALINTERFACE_H

#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

class INET_API VirtualInterface : public NetworkInterface
{
  public:
    virtual cGate *lookupModuleInterface(cGate* gate, const std::type_info& type, const cObject* arguments, int direction) override;
};

} // namespace inet

#endif

