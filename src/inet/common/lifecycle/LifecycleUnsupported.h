//
// (C) 2013 Opensim Ltd.
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// Author: Andras Varga (andras@omnetpp.org)
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
        omnetpp::cMethodCallContextSwitcher __ctx(check_and_cast<cModule*>(this)); __ctx.methodCallSilent();   // Enter_Method_Silent();
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
        return true;
    }
};

} // namespace inet

#endif // ifndef __INET_LIFECYCLEUNSUPPORTED_H

