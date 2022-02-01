//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/ipv4/Ipv4NetworkLayer.h"

#include "inet/common/StringFormat.h"

namespace inet {

Define_Module(Ipv4NetworkLayer);

void Ipv4NetworkLayer::refreshDisplay() const
{
    updateDisplayString();
}

void Ipv4NetworkLayer::updateDisplayString() const
{
    if (getEnvir()->isGUI()) {
        auto text = StringFormat::formatString(par("displayStringTextFormat"), this);
        getDisplayString().setTagArg("t", 0, text);
    }
}

const char *Ipv4NetworkLayer::resolveDirective(char directive) const
{
    static std::string result;
    switch (directive) {
        case 'i':
            result = getSubmodule("ip")->getDisplayString().getTagArg("t", 0);
            break;
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
    return result.c_str();
}

} // namespace inet

