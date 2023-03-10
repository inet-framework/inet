//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/gate/InteractiveGate.h"

namespace inet {
namespace queueing {

Define_Module(InteractiveGate);

void InteractiveGate::initialize(int stage)
{
    PacketGateBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        isOpen_ = par("open");
}

void InteractiveGate::handleParameterChange(const char *name)
{
    if (!strcmp(name, "open")) {
        if (par("open"))
            open();
        else
            close();
    }
}

} // namespace queueing
} // namespace inet

