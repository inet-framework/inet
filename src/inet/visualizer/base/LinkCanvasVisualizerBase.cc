//
// Copyright (C) 2016 OpenSim Ltd.
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
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/base/LinkCanvasVisualizerBase.h"

namespace inet {

namespace visualizer {

LinkCanvasVisualizerBase::CanvasLink::CanvasLink(cLineFigure *figure, int sourceModuleId, int destinationModuleId) :
    Link(sourceModuleId, destinationModuleId),
    figure(figure)
{
}

LinkCanvasVisualizerBase::CanvasLink::~CanvasLink()
{
    delete figure;
}

void LinkCanvasVisualizerBase::initialize(int stage)
{
    LinkVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        auto canvas = visualizerTargetModule->getCanvas();
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);
        linkGroup = new cGroupFigure();
        canvas->addFigure(linkGroup);
    }
}

void LinkCanvasVisualizerBase::addLink(std::pair<int, int> sourceAndDestination, const Link *link)
{
    LinkVisualizerBase::addLink(sourceAndDestination, link);
    auto canvasLink = static_cast<const CanvasLink *>(link);
    linkGroup->addFigure(canvasLink->figure);
}

void LinkCanvasVisualizerBase::removeLink(const Link *link)
{
    LinkVisualizerBase::removeLink(link);
    auto canvasLink = static_cast<const CanvasLink *>(link);
    linkGroup->removeFigure(canvasLink->figure);
}

const LinkVisualizerBase::Link *LinkCanvasVisualizerBase::createLink(cModule *source, cModule *destination) const
{
    auto figure = new cLineFigure();
    figure->setStart(canvasProjection->computeCanvasPoint(getPosition(source)));
    figure->setEnd(canvasProjection->computeCanvasPoint(getPosition(destination)));
    figure->setEndArrowhead(cFigure::ARROW_BARBED);
    figure->setLineWidth(lineWidth);
    figure->setLineColor(lineColor);
    figure->setLineStyle(lineStyle);
    return new CanvasLink(figure, source->getId(), destination->getId());
}

void LinkCanvasVisualizerBase::setAlpha(const Link *link, double alpha) const
{
    auto canvasLink = static_cast<const CanvasLink *>(link);
    auto figure = canvasLink->figure;
    figure->setLineOpacity(alpha);
}

void LinkCanvasVisualizerBase::setPosition(cModule *node, const Coord& position) const
{
    for (auto it : links) {
        auto link = static_cast<const CanvasLink *>(it.second);
        auto figure = link->figure;
        if (node->getId() == link->sourceModuleId)
            figure->setStart(canvasProjection->computeCanvasPoint(position));
        else if (node->getId() == link->destinationModuleId)
            figure->setEnd(canvasProjection->computeCanvasPoint(position));
    }
}

} // namespace visualizer

} // namespace inet

