//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IMODULEINTERFACELOOKUP_H
#define __INET_IMODULEINTERFACELOOKUP_H

#include "inet/common/ProtocolTag_m.h"

namespace inet {

class INET_API IModuleInterfaceLookup
{
  public:
    virtual cGate *lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction) = 0;
};

INET_API cGate *findModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments = nullptr, int direction = 0);

} // namespace inet

#endif

