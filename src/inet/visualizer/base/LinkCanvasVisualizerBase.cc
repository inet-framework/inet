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

#include "inet/common/geometry/object/LineSegment.h"
#include "inet/common/geometry/shape/Cuboid.h"
#include "inet/common/ModuleAccess.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/base/LinkCanvasVisualizerBase.h"

namespace inet {

namespace visualizer {

LinkCanvasVisualizerBase::LinkCanvasVisualization::LinkCanvasVisualization(LabeledLineFigure *figure, int sourceModuleId, int destinationModuleId) :
    LinkVisualization(sourceModuleId, destinationModuleId),
    figure(figure)
{
}

LinkCanvasVisualizerBase::LinkCanvasVisualization::~LinkCanvasVisualization()
{
    delete figure;
}

LinkCanvasVisualizerBase::~LinkCanvasVisualizerBase()
{
    if (displayLinks)
        removeAllLinkVisualizations();
}

void LinkCanvasVisualizerBase::initialize(int stage)
{
    LinkVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        auto canvas = visualizationTargetModule->getCanvas();
        lineManager = LineManager::getCanvasLineManager(canvas);
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);
        linkGroup = new cGroupFigure("links");
        linkGroup->setZIndex(zIndex);
        canvas->addFigure(linkGroup);
    }
}

void LinkCanvasVisualizerBase::refreshDisplay() const
{
    LinkVisualizerBase::refreshDisplay();
    auto simulation = getSimulation();
    for (auto it : linkVisualizations) {
        auto linkVisualization = it.second;
        auto linkCanvasVisualization = static_cast<const LinkCanvasVisualization *>(linkVisualization);
        auto figure = linkCanvasVisualization->figure;
        auto sourceModule = simulation->getModule(linkVisualization->sourceModuleId);
        auto destinationModule = simulation->getModule(linkVisualization->destinationModuleId);
        if (sourceModule != nullptr && destinationModule != nullptr) {
            auto sourcePosition = getContactPosition(sourceModule, getPosition(destinationModule), lineContactMode, lineContactSpacing);
            auto destinationPosition = getContactPosition(destinationModule, getPosition(sourceModule), lineContactMode, lineContactSpacing);
            auto shift = lineManager->getLineShift(linkVisualization->sourceModuleId, linkVisualization->destinationModuleId, sourcePosition, destinationPosition, lineShiftMode, linkVisualization->shiftOffset) * lineShift;
            figure->setStart(canvasProjection->computeCanvasPoint(sourcePosition + shift));
            figure->setEnd(canvasProjection->computeCanvasPoint(destinationPosition + shift));
        }
    }
    visualizationTargetModule->getCanvas()->setAnimationSpeed(linkVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const LinkVisualizerBase::LinkVisualization *LinkCanvasVisualizerBase::createLinkVisualization(cModule *source, cModule *destination, cPacket *packet) const
{
    auto figure = new LabeledLineFigure("link");
    auto lineFigure = figure->getLineFigure();
    lineFigure->setEndArrowhead(cFigure::ARROW_BARBED);
    lineFigure->setLineWidth(lineWidth);
    lineFigure->setLineColor(lineColor);
    lineFigure->setLineStyle(lineStyle);
    auto labelFigure = figure->getLabelFigure();
    labelFigure->setFont(labelFont);
    labelFigure->setColor(labelColor);
    auto text = getLinkVisualizationText(packet);
    labelFigure->setText(text.c_str());
    return new LinkCanvasVisualization(figure, source->getId(), destination->getId());
}

void LinkCanvasVisualizerBase::addLinkVisualization(std::pair<int, int> sourceAndDestination, const LinkVisualization *linkVisualization)
{
    LinkVisualizerBase::addLinkVisualization(sourceAndDestination, linkVisualization);
    auto linkCanvasVisualization = static_cast<const LinkCanvasVisualization *>(linkVisualization);
    auto figure = linkCanvasVisualization->figure;
    lineManager->addModuleLine(linkVisualization);
    linkGroup->addFigure(figure);
}

void LinkCanvasVisualizerBase::removeLinkVisualization(const LinkVisualization *linkVisualization)
{
    LinkVisualizerBase::removeLinkVisualization(linkVisualization);
    auto linkCanvasVisualization = static_cast<const LinkCanvasVisualization *>(linkVisualization);
    lineManager->removeModuleLine(linkVisualization);
    linkGroup->removeFigure(linkCanvasVisualization->figure);
}

void LinkCanvasVisualizerBase::setAlpha(const LinkVisualization *linkVisualization, double alpha) const
{
    auto linkCanvasVisualization = static_cast<const LinkCanvasVisualization *>(linkVisualization);
    auto figure = linkCanvasVisualization->figure;
    figure->getLineFigure()->setLineOpacity(alpha);
}

void LinkCanvasVisualizerBase::refreshLinkVisualization(const LinkVisualization *linkVisualization, cPacket *packet)
{
    LinkVisualizerBase::refreshLinkVisualization(linkVisualization, packet);
    auto linkCanvasVisualization = static_cast<const LinkCanvasVisualization *>(linkVisualization);
    auto text = getLinkVisualizationText(packet);
    linkCanvasVisualization->figure->getLabelFigure()->setText(text.c_str());
}

} // namespace visualizer

} // namespace inet

