//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_OMITTEDMODULE_H
#define __INET_OMITTEDMODULE_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API OmittedModule : public cModule
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual bool initializeModules(int stage) override;
};

} // namespace inet

#endif

