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
#include "inet/visualizer/common/InfoCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(InfoCanvasVisualizer);

InfoCanvasVisualizer::InfoCanvasVisualization::InfoCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, BoxedLabelFigure *figure, int moduleId) :
    InfoVisualization(moduleId),
    networkNodeVisualization(networkNodeVisualization),
    figure(figure)
{
}

void InfoCanvasVisualizer::initialize(int stage)
{
    InfoVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        networkNodeVisualizer = getModuleFromPar<NetworkNodeCanvasVisualizer>(par("networkNodeVisualizerModule"), this);
    }
    else if (stage == INITSTAGE_LAST) {
        for (auto infoVisualization : infoVisualizations) {
            auto infoCanvasVisualization = static_cast<const InfoCanvasVisualization *>(infoVisualization);
            auto figure = infoCanvasVisualization->figure;
            infoCanvasVisualization->networkNodeVisualization->addAnnotation(figure, figure->getBounds().getSize());
        }
    }
}

InfoVisualizerBase::InfoVisualization *InfoCanvasVisualizer::createInfoVisualization(cModule *module) const
{
    auto figure = new BoxedLabelFigure();
    figure->setTags("info");
    figure->setTooltip("This label represents a module");
    figure->setAssociatedObject(module);
    figure->setZIndex(zIndex);
    figure->setFontColor(fontColor);
    figure->setBackgroundColor(backgroundColor);
    figure->setOpacity(opacity);
    auto visualization = networkNodeVisualizer->getNeworkNodeVisualization(getContainingNode(module));
    return new InfoCanvasVisualization(visualization, figure, module->getId());
}

void InfoCanvasVisualizer::refreshInfoVisualization(const InfoVisualization *infoVisualization, const char *info) const
{
    auto infoCanvasVisualization = static_cast<const InfoCanvasVisualization *>(infoVisualization);
    auto figure = infoCanvasVisualization->figure;
    figure->setText(info);
    infoCanvasVisualization->networkNodeVisualization->setAnnotationSize(figure, figure->getBounds().getSize());
}

} // namespace visualizer

} // namespace inet

