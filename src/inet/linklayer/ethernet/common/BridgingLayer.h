//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BRIDGINGLAYER_H
#define __INET_BRIDGINGLAYER_H

#include "inet/common/INETDefs.h"
#include "inet/common/IModuleInterfaceLookup.h"

namespace inet {

class INET_API BridgingLayer : public cModule, public IModuleInterfaceLookup
{
  public:
    virtual cGate *lookupModuleInterface(cGate* gate, const std::type_info& type, const cObject* arguments, int direction) override;
};

} // namespace inet

#endif

