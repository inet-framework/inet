//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TRANSPORTPROTOCOLBASE_H
#define __INET_TRANSPORTPROTOCOLBASE_H

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/lifecycle/ModuleOperations.h"

namespace inet {

class INET_API TransportProtocolBase : public LayeredProtocolBase
{
  public:
    TransportProtocolBase() {};

  protected:
    virtual bool isUpperMessage(cMessage *message) const override;
    virtual bool isLowerMessage(cMessage *message) const override;

    virtual bool isInitializeStage(int stage) const override;
    virtual bool isModuleStartStage(int stage) const override;
    virtual bool isModuleStopStage(int stage) const override;
};

} // namespace inet

#endif

