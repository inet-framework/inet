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

#include "inet/common/figures/BoxedLabelFigure.h"
#include "inet/common/figures/IIndicatorFigure.h"
#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/common/StatisticCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(StatisticCanvasVisualizer);

StatisticCanvasVisualizer::StatisticCanvasVisualization::StatisticCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, cFigure *figure, int moduleId, simsignal_t signal, const char *unit) :
    StatisticVisualization(moduleId, signal, unit),
    networkNodeVisualization(networkNodeVisualization),
    figure(figure)
{
}

StatisticCanvasVisualizer::~StatisticCanvasVisualizer()
{
    if (displayStatistics)
        removeAllStatisticVisualizations();
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
    cFigure *figure = nullptr;
    const char *propertyName = par("propertyName");
    const char *propertyIndex = par("propertyIndex");
    if (!strcmp(propertyName, "")) {
        auto boxedLabelFigure = new BoxedLabelFigure("statistic");
        boxedLabelFigure->setFont(font);
        boxedLabelFigure->setText("");
        boxedLabelFigure->setLabelColor(textColor);
        boxedLabelFigure->setBackgroundColor(backgroundColor);
        boxedLabelFigure->setOpacity(opacity);
        figure = boxedLabelFigure;
        figure->setTooltip("This label represents the current value of a statistic");
    }
    else {
        cProperty *property = nullptr;
        cModule *current = this;
        while (current != nullptr) {
            property = current->getProperties()->get(propertyName, strcmp(propertyIndex, "") ? propertyIndex : nullptr);
            if (property != nullptr)
                break;
            current = current->getParentModule();
        }
        if (property == nullptr)
            throw cRuntimeError("Cannot find property: @%s[%s] on the module path: %s", propertyName, propertyIndex, getFullPath().c_str());
        std::string classname = property->getValue("type");
        classname[0] = toupper(classname[0]);
        classname = "inet::" + classname + "Figure";
        figure = check_and_cast<cFigure *>(createOneIfClassIsKnown(classname.c_str()));
        if (figure == nullptr)
            throw cRuntimeError("Cannot create figure with type: %s", property->getValue("type"));
        figure->parse(property);
        figure->setName("statistic");
        figure->setTooltip("This figure represents the value of a statistic");
    }
    figure->setTags((std::string("statistic ") + tags).c_str());
    figure->setAssociatedObject(source);
    figure->setZIndex(zIndex);
    auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    if (networkNodeVisualization == nullptr)
        throw cRuntimeError("Cannot create statistic visualization for '%s', because network node visualization is not found for '%s'", source->getFullPath().c_str(), networkNode->getFullPath().c_str());
    return new StatisticCanvasVisualization(networkNodeVisualization, figure, source->getId(), signal, getUnit(source));
}

void StatisticCanvasVisualizer::addStatisticVisualization(const StatisticVisualization *statisticVisualization)
{
    StatisticVisualizerBase::addStatisticVisualization(statisticVisualization);
    auto statisticCanvasVisualization = static_cast<const StatisticCanvasVisualization *>(statisticVisualization);
    auto figure = statisticCanvasVisualization->figure;
    if (auto indicatorFigure = dynamic_cast<IIndicatorFigure *>(figure))
        statisticCanvasVisualization->networkNodeVisualization->addAnnotation(statisticCanvasVisualization->figure, indicatorFigure->getSize(), placementHint, placementPriority);
    else if (auto boxedLabelFigure = check_and_cast<BoxedLabelFigure *>(figure))
        statisticCanvasVisualization->networkNodeVisualization->addAnnotation(statisticCanvasVisualization->figure, boxedLabelFigure->getBounds().getSize(), placementHint, placementPriority);
}

void StatisticCanvasVisualizer::removeStatisticVisualization(const StatisticVisualization *statisticVisualization)
{
    StatisticVisualizerBase::removeStatisticVisualization(statisticVisualization);
    auto statisticCanvasVisualization = static_cast<const StatisticCanvasVisualization *>(statisticVisualization);
    statisticCanvasVisualization->networkNodeVisualization->removeAnnotation(statisticCanvasVisualization->figure);
}

void StatisticCanvasVisualizer::refreshStatisticVisualization(const StatisticVisualization *statisticVisualization)
{
    StatisticVisualizerBase::refreshStatisticVisualization(statisticVisualization);
    auto statisticCanvasVisualization = static_cast<const StatisticCanvasVisualization *>(statisticVisualization);
    auto figure = statisticCanvasVisualization->figure;
    if (auto indicatorFigure = dynamic_cast<IIndicatorFigure *>(figure))
        indicatorFigure->setValue(0, simTime(), statisticVisualization->recorder->getLastValue());
    else {
        auto boxedLabelFigure = check_and_cast<BoxedLabelFigure *>(figure);
        boxedLabelFigure->setText(getText(statisticVisualization).c_str());
        statisticCanvasVisualization->networkNodeVisualization->setAnnotationSize(figure, boxedLabelFigure->getBounds().getSize());
    }
}

} // namespace visualizer

} // namespace inet

