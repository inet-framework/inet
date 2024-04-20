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
    virtual ~ExternalEnvironment();

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
};

}

#endif
