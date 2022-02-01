//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INTERACTIVEGATE_H
#define __INET_INTERACTIVEGATE_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketGateBase.h"

namespace inet {
namespace queueing {

class INET_API InteractiveGate : public PacketGateBase
{
  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
};

} // namespace queueing
} // namespace inet

#endif

