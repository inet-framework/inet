//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/scene/NetworkConnectionCanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

Define_Module(NetworkConnectionCanvasVisualizer);

void NetworkConnectionCanvasVisualizer::initialize(int stage)
{
    NetworkConnectionVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        canvasProjection = CanvasProjection::getCanvasProjection(visualizationTargetModule->getCanvas());
    }
}

void NetworkConnectionCanvasVisualizer::createNetworkConnectionVisualization(cModule *startNetworkNode, cModule *endNetworkNode)
{
    auto lineFigure = new cLineFigure("connection");
    lineFigure->setLineColor(lineColor);
    lineFigure->setLineStyle(lineStyle);
    lineFigure->setLineWidth(lineWidth);
    lineFigure->setStart(canvasProjection->computeCanvasPoint(getPosition(startNetworkNode)));
    lineFigure->setEnd(canvasProjection->computeCanvasPoint(getPosition(endNetworkNode)));
    lineFigure->setEndArrowhead(cFigure::ARROW_BARBED);
    lineFigure->setZIndex(zIndex);
    visualizationTargetModule->getCanvas()->addFigure(lineFigure);
}

} // namespace visualizer

} // namespace inet

