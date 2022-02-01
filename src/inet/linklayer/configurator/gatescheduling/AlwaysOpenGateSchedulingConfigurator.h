//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ALWAYSOPENGATESCHEDULINGCONFIGURATOR_H
#define __INET_ALWAYSOPENGATESCHEDULINGCONFIGURATOR_H

#include "inet/linklayer/configurator/gatescheduling/base/GateSchedulingConfiguratorBase.h"

namespace inet {

class INET_API AlwaysOpenGateSchedulingConfigurator : public GateSchedulingConfiguratorBase
{
  protected:
    virtual Output *computeGateScheduling(const Input& input) const override;
};

} // namespace inet

#endif

