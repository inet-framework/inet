//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/common/Sink.h"

namespace inet {

Define_Module(Sink);

void Sink::handleMessage(cMessage *msg)
{
    delete msg;
}

} // namespace inet

