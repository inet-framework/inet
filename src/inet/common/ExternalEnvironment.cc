//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/common/ExternalEnvironment.h"

#include "inet/common/NetworkNamespaceContext.h"

namespace inet {

Define_Module(ExternalEnvironment);

ExternalEnvironment::~ExternalEnvironment()
{
    if (initialized()) {
        if (!opp_isempty(networkNamespace) && existsNetworkNamespace(networkNamespace))
            deleteNetworkNamespace(networkNamespace);
    }
}

void ExternalEnvironment::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        networkNamespace = par("namespace");
        setupCommand = par("setupCommand");
        teardownCommand = par("teardownCommand");
    }
    if (stage == par("initStage").intValue()) {
        if (!opp_isempty(networkNamespace) && !existsNetworkNamespace(networkNamespace))
            createNetworkNamespace(networkNamespace, par("globalNamespace"));
        NetworkNamespaceContext context(networkNamespace);
        if (!opp_isempty(setupCommand)) {
            EV_INFO << "Executing setup command: " << setupCommand << std::endl;
            if (std::system(setupCommand) != 0)
                throw cRuntimeError("Failed to execute setup command: %s", setupCommand);
        }
    }
}

void ExternalEnvironment::preDelete(cComponent *root)
{
    if (initialized()) {
        NetworkNamespaceContext context(networkNamespace);
        if (!opp_isempty(teardownCommand)) {
            EV_INFO << "Executing teardown command: " << teardownCommand << std::endl;
            if (std::system(teardownCommand) != 0)
                // cannot throw an exception from the destructor
                EV_FATAL << "Failed to execute teardown command: " << teardownCommand << std::endl;
        }
    }
}

void ExternalEnvironment::handleMessage(cMessage *msg)
{
    throw cRuntimeError("Unknown message: %s", msg->getFullName());
}

} // namespace inet
