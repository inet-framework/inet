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

#include "inet/visualizer/networklayer/RoutingTableCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(RoutingTableCanvasVisualizer);

RoutingTableCanvasVisualizer::RouteCanvasVisualization::RouteCanvasVisualization(LabeledLineFigure *figure, const Ipv4Route *route, int nodeModuleId, int nextHopModuleId) :
    RouteVisualization(route, nodeModuleId, nextHopModuleId),
    figure(figure)
{
}

RoutingTableCanvasVisualizer::RouteCanvasVisualization::~RouteCanvasVisualization()
{
    delete figure;
}

void RoutingTableCanvasVisualizer::initialize(int stage)
{
    RoutingTableVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        auto canvas = visualizationTargetModule->getCanvas();
        lineManager = LineManager::getCanvasLineManager(canvas);
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);
        routeGroup = new cGroupFigure("routing tables");
        routeGroup->setZIndex(zIndex);
        canvas->addFigure(routeGroup);
    }
}

void RoutingTableCanvasVisualizer::refreshDisplay() const
{
    auto simulation = getSimulation();
    for (auto it : routeVisualizations) {
        auto routeVisualization = it.second;
        auto routeCanvasVisualization = static_cast<const RouteCanvasVisualization *>(routeVisualization);
        auto figure = routeCanvasVisualization->figure;
        auto sourceModule = simulation->getModule(routeVisualization->sourceModuleId);
        auto destinationModule = simulation->getModule(routeVisualization->destinationModuleId);
        auto sourcePosition = getContactPosition(sourceModule, getPosition(destinationModule), lineContactMode, lineContactSpacing);
        auto destinationPosition = getContactPosition(destinationModule, getPosition(sourceModule), lineContactMode, lineContactSpacing);
        auto shift = lineManager->getLineShift(routeVisualization->sourceModuleId, routeVisualization->destinationModuleId, sourcePosition, destinationPosition, lineShiftMode, routeVisualization->shiftOffset) * lineShift;
        figure->setStart(canvasProjection->computeCanvasPoint(sourcePosition + shift));
        figure->setEnd(canvasProjection->computeCanvasPoint(destinationPosition + shift));
    }
}

const RoutingTableVisualizerBase::RouteVisualization *RoutingTableCanvasVisualizer::createRouteVisualization(Ipv4Route *route, cModule *node, cModule *nextHop) const
{
    auto figure = new LabeledLineFigure("routing entry");
    figure->setTags((std::string("route ") + tags).c_str());
    figure->setTooltip("This arrow represents a route in a routing table");
    figure->setAssociatedObject(route);
    auto lineFigure = figure->getLineFigure();
    lineFigure->setEndArrowhead(cFigure::ARROW_TRIANGLE);
    lineFigure->setLineWidth(lineWidth);
    lineFigure->setLineColor(lineColor);
    lineFigure->setLineStyle(lineStyle);
    auto labelFigure = figure->getLabelFigure();
    labelFigure->setFont(labelFont);
    labelFigure->setColor(labelColor);
    labelFigure->setVisible(displayLabels);
    auto routeVisualization = new RouteCanvasVisualization(figure, route, node->getId(), nextHop->getId());
    routeVisualization->shiftPriority = 0.5;
    refreshRouteVisualization(routeVisualization);
    return routeVisualization;
}

void RoutingTableCanvasVisualizer::addRouteVisualization(const RouteVisualization *routeVisualization)
{
    RoutingTableVisualizerBase::addRouteVisualization(routeVisualization);
    auto routeCanvasVisualization = static_cast<const RouteCanvasVisualization *>(routeVisualization);
    lineManager->addModuleLine(routeVisualization);
    routeGroup->addFigure(routeCanvasVisualization->figure);
}

void RoutingTableCanvasVisualizer::removeRouteVisualization(const RouteVisualization *routeVisualization)
{
    RoutingTableVisualizerBase::removeRouteVisualization(routeVisualization);
    auto routeCanvasVisualization = static_cast<const RouteCanvasVisualization *>(routeVisualization);
    lineManager->removeModuleLine(routeVisualization);
    routeGroup->removeFigure(routeCanvasVisualization->figure);
}

void RoutingTableCanvasVisualizer::refreshRouteVisualization(const RouteVisualization *routeVisualization) const
{
    auto routeCanvasVisualization = static_cast<const RouteCanvasVisualization *>(routeVisualization);
    auto labelFigure = routeCanvasVisualization->figure->getLabelFigure();
    auto text = displayRoutesIndividually ? getRouteVisualizationText(routeCanvasVisualization->route) : std::to_string(routeVisualization->numRoutes) + (routeVisualization->numRoutes > 1 ? " routes" : " route");
    labelFigure->setText(text.c_str());
}

} // namespace visualizer

} // namespace inet

