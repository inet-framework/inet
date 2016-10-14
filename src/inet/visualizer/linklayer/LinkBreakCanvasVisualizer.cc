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

LinkBreakCanvasVisualizer::LinkBreakCanvasVisualization::LinkBreakCanvasVisualization(cIconFigure *figure, int transmitterModuleId, int receiverModuleId) :
    LinkBreakVisualization(transmitterModuleId, receiverModuleId),
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

const LinkBreakVisualizerBase::LinkBreakVisualization *LinkBreakCanvasVisualizer::createLinkBreakVisualization(cModule *transmitter, cModule *receiver) const
{
    auto transmitterPosition = canvasProjection->computeCanvasPoint(getPosition(getContainingNode(transmitter)));
    auto receiverPosition = canvasProjection->computeCanvasPoint(getPosition(getContainingNode(receiver)));
    auto figure = new cIconFigure();
    figure->setImageName(icon);
    figure->setTintAmount(iconTintAmount);
    figure->setTintColor(iconTintColor);
    figure->setPosition((transmitterPosition + receiverPosition) / 2);
    return new LinkBreakCanvasVisualization(figure, transmitter->getId(), receiver->getId());
}

void LinkBreakCanvasVisualizer::addLinkBreakVisualization(const LinkBreakVisualization *linkBreakVisualization)
{
    LinkBreakVisualizerBase::addLinkBreakVisualization(linkBreakVisualization);
    auto linkBreakCanvasVisualization = static_cast<const LinkBreakCanvasVisualization *>(linkBreakVisualization);
    linkBreakGroup->addFigure(linkBreakCanvasVisualization->figure);
}

void LinkBreakCanvasVisualizer::removeLinkBreakVisualization(const LinkBreakVisualization *linkBreakVisualization)
{
    LinkBreakVisualizerBase::removeLinkBreakVisualization(linkBreakVisualization);
    auto linkBreakCanvasVisualization = static_cast<const LinkBreakCanvasVisualization *>(linkBreakVisualization);
    linkBreakGroup->removeFigure(linkBreakCanvasVisualization->figure);
}

void LinkBreakCanvasVisualizer::setPosition(cModule *node, const Coord& position) const
{
    for (auto it : linkBreakVisualizations) {
        auto linkBreak = static_cast<const LinkBreakCanvasVisualization *>(it.second);
        auto transmitter = getSimulation()->getModule(linkBreak->transmitterModuleId);
        auto receiver = getSimulation()->getModule(linkBreak->receiverModuleId);
        auto transmitterPosition = canvasProjection->computeCanvasPoint(getPosition(getContainingNode(transmitter)));
        auto receiverPosition = canvasProjection->computeCanvasPoint(getPosition(getContainingNode(receiver)));
        auto figure = linkBreak->figure;
        figure->setPosition((transmitterPosition + receiverPosition) / 2);
    }
}

void LinkBreakCanvasVisualizer::setAlpha(const LinkBreakVisualization *linkBreakVisualization, double alpha) const
{
    auto linkBreakCanvasVisualization = static_cast<const LinkBreakCanvasVisualization *>(linkBreakVisualization);
    auto figure = linkBreakCanvasVisualization->figure;
    figure->setOpacity(alpha);
}

} // namespace visualizer

} // namespace inet

