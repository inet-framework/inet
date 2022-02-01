//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IINTERFACEREGISTRATIONLISTENER_H
#define __INET_IINTERFACEREGISTRATIONLISTENER_H

#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

INET_API void registerInterface(const NetworkInterface& interface, cGate *in, cGate *out);

class INET_API IInterfaceRegistrationListener
{
  public:
    virtual void handleRegisterInterface(const NetworkInterface& interface, cGate *out, cGate *in) = 0;
};

} // namespace inet

#endif

