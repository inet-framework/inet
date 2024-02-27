//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_EXTERNALPROCESS_H
#define __INET_EXTERNALPROCESS_H

#include "inet/common/INETDefs.h"

using namespace omnetpp;

namespace inet {

class ExternalProcess : public cSimpleModule
{
  private:
    std::string command;
    std::string pidFile;
    std::string cleanupCommand;
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual ~ExternalProcess();

    void startProcess();
    void stopProcess();
};

}

#endif
