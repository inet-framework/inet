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
    disabledFigure = new cIconFigure();
    disabledFigure->setTags("disabled cable");
    disabledFigure->setTooltip("This icon indicates that the cable is disabled, disconnected temporarily.");
    disabledFigure->setAnchor(cFigure::ANCHOR_CENTER);
    disabledFigure->setImageName(par("disabledIcon"));
    getParentModule()->getCanvas()->addFigure(disabledFigure);
}

void Cable::refreshDisplay() const
{
    cDatarateChannel::refreshDisplay();
    getDisplayString().setTagArg("ls", 2, isDisabled() ? disabledLineStyle : enabledLineStyle);
    std::vector<cFigure::Point> points = getEnvir()->getConnectionLine(getSourceGate());
    if (points.size() == 2) {
        disabledFigure->setVisible(isDisabled());
        disabledFigure->setPosition((points[0] + points[1]) / 2);
    }
}

}  // namespace inet
