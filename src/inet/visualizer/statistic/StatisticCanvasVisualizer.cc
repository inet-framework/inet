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
#include "inet/visualizer/statistic/StatisticCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(StatisticCanvasVisualizer);

StatisticCanvasVisualizer::StatisticCanvasVisualization::StatisticCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, BoxedLabelFigure *figure, int moduleId, simsignal_t signal, const char *unit) :
    StatisticVisualization(moduleId, signal, unit),
    networkNodeVisualization(networkNodeVisualization),
    figure(figure)
{
}

void StatisticCanvasVisualizer::initialize(int stage)
{
    StatisticVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        networkNodeVisualizer = getModuleFromPar<NetworkNodeCanvasVisualizer>(par("networkNodeVisualizerModule"), this);
    }
}

StatisticVisualizerBase::StatisticVisualization *StatisticCanvasVisualizer::createStatisticVisualization(cComponent *source, simsignal_t signal)
{
    auto labelFigure = new BoxedLabelFigure("statistic");
    labelFigure->setZIndex(zIndex);
    labelFigure->setFontColor(fontColor);
    labelFigure->setBackgroundColor(backgroundColor);
    labelFigure->setOpacity(opacity);
    labelFigure->setText("");
    auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
    auto networkNodeVisualization = networkNodeVisualizer->getNeworkNodeVisualization(networkNode);
    return new StatisticCanvasVisualization(networkNodeVisualization, labelFigure, source->getId(), signal, getUnit(source));
}

void StatisticCanvasVisualizer::addStatisticVisualization(StatisticVisualization *statisticVisualization)
{
    StatisticVisualizerBase::addStatisticVisualization(statisticVisualization);
    auto statisticCanvasVisualization = static_cast<StatisticCanvasVisualization *>(statisticVisualization);
    statisticCanvasVisualization->networkNodeVisualization->addAnnotation(statisticCanvasVisualization->figure, statisticCanvasVisualization->figure->getBounds().getSize());
}

void StatisticCanvasVisualizer::removeStatisticVisualization(StatisticVisualization *statisticVisualization)
{
    StatisticVisualizerBase::removeStatisticVisualization(statisticVisualization);
    auto statisticCanvasVisualization = static_cast<StatisticCanvasVisualization *>(statisticVisualization);
    statisticCanvasVisualization->networkNodeVisualization->removeAnnotation(statisticCanvasVisualization->figure);
}

void StatisticCanvasVisualizer::refreshStatisticVisualization(StatisticVisualization *statisticVisualization)
{
    auto statisticCanvasVisualization = static_cast<StatisticCanvasVisualization *>(statisticVisualization);
    statisticCanvasVisualization->figure->setText(getText(statisticVisualization).c_str());
}

} // namespace visualizer

} // namespace inet

