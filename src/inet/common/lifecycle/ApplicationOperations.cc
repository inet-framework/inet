//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/lifecycle/ApplicationOperations.h"

namespace inet {

Register_Class(ApplicationStartOperation);
Register_Class(ApplicationStopOperation);

void ApplicationOperationBase::initialize(cModule *module, StringMap& params)
{
    LifecycleOperation::initialize(module, params);
}

} // namespace inet

