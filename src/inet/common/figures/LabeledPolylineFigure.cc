//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/figures/LabeledPolylineFigure.h"

namespace inet {

LabeledPolylineFigure::LabeledPolylineFigure(const char *name) :
    cGroupFigure(name)
{
    polylineFigure = new cPolylineFigure("line");
    panelFigure = new cPanelFigure("panel");
    addFigure(polylineFigure);
    addFigure(panelFigure);
    labelFigure = new cTextFigure("label");
    labelFigure->setAnchor(cFigure::ANCHOR_S);
    labelFigure->setTags("label");
    labelFigure->setHalo(true);
    panelFigure->addFigure(labelFigure);
}

void LabeledPolylineFigure::setPoints(const std::vector<cFigure::Point>& points)
{
    polylineFigure->setPoints(points);
    updateLabelPosition();
}

void LabeledPolylineFigure::updateLabelPosition()
{
    auto points = polylineFigure->getPoints();
    int index = (points.size() - 1) / 2;
    auto position = (points[index] + points[index + 1]) / 2;
    auto direction = points[index + 1] - points[index];
    double alpha = atan2(-direction.y, direction.x);
    if (alpha > M_PI / 2 || alpha < -M_PI / 2)
        alpha += M_PI;
    panelFigure->setTransform(cFigure::Transform().rotate(-alpha));
    panelFigure->setPosition(position);
    labelFigure->setPosition(cFigure::Point(0, -polylineFigure->getLineWidth() / 2));
}

} // namespace inet

