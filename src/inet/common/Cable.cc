//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/common/Cable.h"

namespace inet {

Define_Channel(Cable);

void Cable::initialize()
{
    cDatarateChannel::initialize();
    enabledLineStyle = par("enabledLineStyle");
    disabledLineStyle = par("disabledLineStyle");
}

void Cable::refreshDisplay() const
{
    cDatarateChannel::refreshDisplay();
    getDisplayString().setTagArg("ls", 2, isDisabled() ? disabledLineStyle : enabledLineStyle);
}

}  // namespace inet
