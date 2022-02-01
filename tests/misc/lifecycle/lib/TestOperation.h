//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TESTOPERATION_H_
#define __INET_TESTOPERATION_H_

#include "inet/common/lifecycle/NodeOperations.h"

namespace inet {

class INET_API TestNodeStartOperation : public NodeStartOperation {
  public:
    virtual int getNumStages() const { return 4; }
};

class INET_API TestNodeShutdownOperation : public NodeShutdownOperation {
  public:
    virtual int getNumStages() const { return 4; }
};

}
#endif
