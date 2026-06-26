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
    lineFigure->setEndArrowhead(cFigure::ARROW_BARBED);
    lineFigure->setZIndex(zIndex);
    visualizationTargetModule->getCanvas()->addFigure(lineFigure);
    // the endpoints are projected and clipped to the map area in refreshDisplay(), once the shared
    // canvas projection (and its clip rect) is configured; clipping here would depend on the relative
    // initialization order of this visualizer and the map visualizer that installs the clip rect
    connectionVisualizations.push_back({lineFigure, startNetworkNode, endNetworkNode});
}

void NetworkConnectionCanvasVisualizer::refreshDisplay() const
{
    for (auto& connection : connectionVisualizations) {
        cFigure::Point start = canvasProjection->computeCanvasPoint(getPosition(connection.startNetworkNode));
        cFigure::Point end = canvasProjection->computeCanvasPoint(getPosition(connection.endNetworkNode));
        bool visible = canvasProjection->clipLine(start, end); // clip to the map area, hide if fully outside
        connection.figure->setStart(start);
        connection.figure->setEnd(end);
        connection.figure->setVisible(visible);
    }
}

} // namespace visualizer

} // namespace inet

