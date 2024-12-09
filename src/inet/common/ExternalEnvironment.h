//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_EXTERNALENVIRONMENT_H
#define __INET_EXTERNALENVIRONMENT_H

#include "inet/common/INETDefs.h"

namespace inet {

class ExternalEnvironment : public cSimpleModule
{
  protected:
    const char *networkNamespace = nullptr;
    const char *setupCommand = nullptr;
    const char *teardownCommand = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void preDelete(cComponent *root) override;
    virtual void handleMessage(cMessage *msg) override;
};

}

#endif
