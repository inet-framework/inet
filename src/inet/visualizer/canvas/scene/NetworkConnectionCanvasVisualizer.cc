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
        connectionGroup = new ProjectionClipFigure("connections", canvasProjection);
        connectionGroup->setZIndex(zIndex);
        visualizationTargetModule->getCanvas()->addFigure(connectionGroup);
    }
}

void NetworkConnectionCanvasVisualizer::createNetworkConnectionVisualization(cModule *startNetworkNode, cModule *endNetworkNode)
{
    auto lineFigure = new cLineFigure("connection");
    lineFigure->setLineColor(lineColor);
    lineFigure->setLineStyle(lineStyle);
    lineFigure->setLineWidth(lineWidth);
    lineFigure->setEndArrowhead(cFigure::ARROW_BARBED);
    connectionGroup->addFigure(lineFigure);
    // the endpoints are projected in refreshDisplay(), once the shared canvas projection is configured;
    // projecting them here would depend on the relative initialization order of this visualizer and
    // the map visualizer that configures the projection
    connectionVisualizations.push_back({lineFigure, startNetworkNode, endNetworkNode});
}

void NetworkConnectionCanvasVisualizer::refreshDisplay() const
{
    for (auto& connection : connectionVisualizations) {
        connection.figure->setStart(canvasProjection->computeCanvasPoint(getPosition(connection.startNetworkNode)));
        connection.figure->setEnd(canvasProjection->computeCanvasPoint(getPosition(connection.endNetworkNode)));
    }
}

} // namespace visualizer

} // namespace inet

