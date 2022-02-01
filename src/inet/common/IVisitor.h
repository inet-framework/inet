//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IVISITOR_H
#define __INET_IVISITOR_H

#include "inet/common/INETDefs.h"

namespace inet {

// This is the interface for data structure visitors
class INET_API IVisitor
{
  public:
    virtual void visit(const cObject *) const = 0;
    virtual ~IVisitor() {}
};

} /* namespace inet */

#endif

