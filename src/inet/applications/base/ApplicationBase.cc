//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/applications/base/ApplicationBase.h"

namespace inet {

ApplicationBase::ApplicationBase() = default;

ApplicationBase::~ApplicationBase() = default;

void ApplicationBase::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    // custom initialization code here
}

void ApplicationBase::handleMessage(cMessage *msg)
{
    // message handling code here
    delete msg;
}

} // namespace inet

