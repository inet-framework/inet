//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NETWORKBASE_H
#define __INET_NETWORKBASE_H

#include "inet/common/INETDefs.h"
#include "inet/common/Module.h"

namespace inet {

class INET_API NetworkBase : public Module
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
};

} // namespace inet

#endif

