//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LIFECYCLEUNSUPPORTED_H
#define __INET_LIFECYCLEUNSUPPORTED_H

#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"

namespace inet {

class INET_API LifecycleUnsupported : public ILifecycle
{
  public:
    virtual bool handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback) override
    {
        omnetpp::cMethodCallContextSwitcher __ctx(check_and_cast<cModule *>(this));
        __ctx.methodCallSilent(); // Enter_Method_Silent();
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
        return true;
    }
};

} // namespace inet

#endif

