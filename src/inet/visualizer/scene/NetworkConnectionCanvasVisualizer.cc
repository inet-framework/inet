//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/scene/NetworkConnectionCanvasVisualizer.h"

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

