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
#include "inet/visualizer/linklayer/LinkBreakCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(LinkBreakCanvasVisualizer);

LinkBreakCanvasVisualizer::CanvasLinkBreak::CanvasLinkBreak(cIconFigure *figure, int transmitterModuleId, int receiverModuleId, simtime_t breakSimulationTime, double breakAnimationTime, double breakRealTime) :
    LinkBreakVisualization(transmitterModuleId, receiverModuleId, breakSimulationTime, breakAnimationTime, breakRealTime),
    figure(figure)
{
}

void LinkBreakCanvasVisualizer::initialize(int stage)
{
    LinkBreakVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        auto canvas = visualizerTargetModule->getCanvas();
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);
        linkBreakGroup = new cGroupFigure();
        linkBreakGroup->setZIndex(zIndex);
        canvas->addFigure(linkBreakGroup);
    }
}

void LinkBreakCanvasVisualizer::setPosition(cModule *node, const Coord& position) const
{
    for (auto it : linkBreakVisualizations) {
        auto linkBreak = static_cast<const CanvasLinkBreak *>(it.second);
        auto transmitter = getSimulation()->getModule(linkBreak->transmitterModuleId);
        auto receiver = getSimulation()->getModule(linkBreak->receiverModuleId);
        auto transmitterPosition = canvasProjection->computeCanvasPoint(getPosition(getContainingNode(transmitter)));
        auto receiverPosition = canvasProjection->computeCanvasPoint(getPosition(getContainingNode(receiver)));
        auto figure = linkBreak->figure;
        figure->setPosition((transmitterPosition + receiverPosition) / 2);
    }
}

void LinkBreakCanvasVisualizer::setAlpha(const LinkBreakVisualization *linkBreak, double alpha) const
{
    auto canvasLinkBreak = static_cast<const CanvasLinkBreak *>(linkBreak);
    auto figure = canvasLinkBreak->figure;
    figure->setOpacity(alpha);
}

const LinkBreakVisualizerBase::LinkBreakVisualization *LinkBreakCanvasVisualizer::createLinkBreakVisualization(cModule *transmitter, cModule *receiver) const
{
    auto transmitterPosition = canvasProjection->computeCanvasPoint(getPosition(getContainingNode(transmitter)));
    auto receiverPosition = canvasProjection->computeCanvasPoint(getPosition(getContainingNode(receiver)));
    auto figure = new cIconFigure();
    figure->setImageName(icon);
    figure->setTintAmount(iconTintAmount);
    figure->setTintColor(iconTintColor);
    figure->setPosition((transmitterPosition + receiverPosition) / 2);
    return new CanvasLinkBreak(figure, transmitter->getId(), receiver->getId(), simTime(), getSimulation()->getEnvir()->getAnimationTime(), getRealTime());
}

void LinkBreakCanvasVisualizer::addLinkBreakVisualization(const LinkBreakVisualization *linkBreak)
{
    LinkBreakVisualizerBase::addLinkBreakVisualization(linkBreak);
    auto canvasLinkBreak = static_cast<const CanvasLinkBreak *>(linkBreak);
    linkBreakGroup->addFigure(canvasLinkBreak->figure);
}

void LinkBreakCanvasVisualizer::removeLinkBreakVisualization(const LinkBreakVisualization *linkBreak)
{
    LinkBreakVisualizerBase::removeLinkBreakVisualization(linkBreak);
    auto canvasLinkBreak = static_cast<const CanvasLinkBreak *>(linkBreak);
    linkBreakGroup->removeFigure(canvasLinkBreak->figure);
}

} // namespace visualizer

} // namespace inet

