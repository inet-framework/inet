//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/common/StatisticCanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/figures/BoxedLabelFigure.h"
#include "inet/common/figures/IIndicatorFigure.h"

namespace inet {

namespace visualizer {

Define_Module(StatisticCanvasVisualizer);

StatisticCanvasVisualizer::StatisticCanvasVisualization::StatisticCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, cFigure *figure, int moduleId, simsignal_t signal, const char *unit) :
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
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
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
    if (networkNodeVisualizer != nullptr)
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

