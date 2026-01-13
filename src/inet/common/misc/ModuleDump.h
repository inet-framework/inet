//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/ModuleAccess.h"

namespace inet {

class ModuleDump : public cSimpleModule
{
  protected:
    bool printClassNames = false;
    bool printProperties = false;
    bool printParamAssignmentLocations = false;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void dump(cModule *mod, std::ostream& out, std::string indent);
    virtual std::string formatProperties(cProperties *props);
};

}
