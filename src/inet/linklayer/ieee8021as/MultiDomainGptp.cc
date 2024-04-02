//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021as/MultiDomainGptp.h"

namespace inet {

Define_Module(MultiDomainGptp);

void MultiDomainGptp::initialize(int stage)
{
    if (stage == INITSTAGE_APPLICATION_LAYER)
        registerProtocol(Protocol::gptp, gate("socketOut"), gate("socketIn"));
}

} // namespace inet

