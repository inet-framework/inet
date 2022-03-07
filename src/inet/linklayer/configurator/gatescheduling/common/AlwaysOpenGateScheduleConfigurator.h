//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ALWAYSOPENGATESCHEDULECONFIGURATOR_H
#define __INET_ALWAYSOPENGATESCHEDULECONFIGURATOR_H

#include "inet/linklayer/configurator/gatescheduling/base/GateScheduleConfiguratorBase.h"

namespace inet {

class INET_API AlwaysOpenGateScheduleConfigurator : public GateScheduleConfiguratorBase
{
  protected:
    virtual Output *computeGateScheduling(const Input& input) const override;
};

} // namespace inet

#endif

