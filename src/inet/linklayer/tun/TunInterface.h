//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TUNINTERFACE_H
#define __INET_TUNINTERFACE_H

#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

class INET_API TunInterface : public NetworkInterface
{
  public:
    virtual cGate *lookupModuleInterface(cGate* gate, const std::type_info& type, const cObject* arguments, int direction) override;
};

} // namespace inet

#endif

