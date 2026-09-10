//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FUNCTIONALEVENTEXAMPLE_H
#define __INET_FUNCTIONALEVENTEXAMPLE_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API FunctionalEventExample : public cSimpleModule
{
  protected:
    virtual void initialize() override;
};

} // namespace inet

#endif
