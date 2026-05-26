//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/ipv4/Ipv4NetworkLayer.h"

#include "inet/common/StringFormat.h"

namespace inet {

Define_Module(Ipv4NetworkLayer);

void Ipv4NetworkLayer::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        WATCH_EXPR("ipText", getSubmodule("ip")->getDisplayString().getTagArg("t", 0));
}

std::string Ipv4NetworkLayer::resolveDirective(char directive) const
{
    switch (directive) {
        case 'i':
            return getSubmodule("ip")->getDisplayString().getTagArg("t", 0);
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
}

} // namespace inet

