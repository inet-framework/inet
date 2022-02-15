//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ROUTINGPROTOCOLBASE_H
#define __INET_ROUTINGPROTOCOLBASE_H

#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"

namespace inet {

class INET_API RoutingProtocolBase : public OperationalBase
{
  public:
    RoutingProtocolBase() {}

  protected:
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_ROUTING_PROTOCOLS; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_ROUTING_PROTOCOLS; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_ROUTING_PROTOCOLS; }
};

} // namespace inet

#endif

