//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/common/StatisticCanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/figures/BarChartFigure.h"
#include "inet/common/figures/BoxedLabelFigure.h"

namespace inet {

namespace visualizer {

Define_Module(StatisticCanvasVisualizer);

static std::string getFigureAttributeValue(const char *key, const cValue& value)
{
    switch (value.getType()) {
        case cValue::BOOL:
            return value.boolValue() ? "true" : "false";
        case cValue::INT:
        case cValue::DOUBLE:
            if (!opp_isempty(value.getUnit()))
                throw cRuntimeError("Figure attribute '%s' must be dimensionless, but it has unit '%s'", key, value.getUnit());
            return value.getType() == cValue::INT ? std::to_string(value.intValue()) : opp_stringf("%.15g", value.doubleValue());
        case cValue::STRING:
            return value.stringValue();
        default:
            throw cRuntimeError("Invalid value for figure attribute '%s': %s", key, value.str().c_str());
    }
}

StatisticCanvasVisualizer::StatisticCanvasVisualization::StatisticCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, int networkNodeId, cFigure *figure, int moduleId, simsignal_t signal, const char *unit) :
    StatisticVisualization(moduleId, signal, unit),
    networkNodeVisualization(networkNodeVisualization),
    networkNodeId(networkNodeId),
    figure(figure)
{
}

StatisticCanvasVisualizer::StatisticCanvasVisualization::~StatisticCanvasVisualization()
{
    delete figure;
    figure = nullptr;
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

cProperty *StatisticCanvasVisualizer::findFigureTemplateProperty()
{
    const char *propertyName = par("propertyName");
    if (opp_isempty(propertyName))
        return nullptr;
    const char *propertyIndex = par("propertyIndex");
    for (cModule *module = this; module != nullptr; module = module->getParentModule()) {
        auto property = module->getProperties()->get(propertyName, opp_isempty(propertyIndex) ? nullptr : propertyIndex);
        if (property != nullptr)
            return property;
    }
    throw cRuntimeError("Cannot find property: @%s[%s] on the module path: %s", propertyName, propertyIndex, getFullPath().c_str());
}

cFigure *StatisticCanvasVisualizer::createIndicatorFigure()
{
    // the attributes of the figure are given either in a figure template property along the
    // module path of this visualizer, or directly in the figure parameter; the template takes
    // precedence, so that it can also replace the figure a derived visualizer defaults to
    if (auto property = findFigureTemplateProperty())
        return createFigure(property);
    auto figureParameter = par("figure").objectValue();
    auto figureAttributes = dynamic_cast<cValueMap *>(figureParameter);
    if (figureParameter != nullptr && figureAttributes == nullptr)
        throw cRuntimeError("The figure parameter must contain the attributes of a figure, e.g. {type: \"gauge\", maxValue: 100}");
    if (figureAttributes != nullptr && figureAttributes->size() != 0) {
        cProperty property("figure");
        for (auto& field : figureAttributes->getFields()) {
            const char *key = field.first.c_str();
            const cValue& value = field.second;
            property.addKey(key);
            // an attribute that takes several values, e.g. pos, is given as an array
            if (value.containsObject()) {
                auto values = dynamic_cast<cValueArray *>(value.objectValue());
                if (values == nullptr)
                    throw cRuntimeError("Invalid value for figure attribute '%s': %s", key, value.str().c_str());
                for (int i = 0; i < values->size(); i++)
                    property.setValue(key, i, getFigureAttributeValue(key, values->get(i)).c_str());
            }
            else
                property.setValue(key, 0, getFigureAttributeValue(key, value).c_str());
        }
        return createFigure(&property);
    }
    return nullptr;
}

cFigure *StatisticCanvasVisualizer::createFigure(cProperty *property) const
{
    const char *type = property->getValue("type");
    if (opp_isempty(type))
        throw cRuntimeError("The type of the figure is not specified");
    // for backward compatibility, a type also selects the inet::<Type>Figure class,
    // even if the figure class is not registered with Register_Figure()
    std::string className = type;
    className[0] = toupper(className[0]);
    className = "inet::" + className + "Figure";
    auto figure = dynamic_cast<cFigure *>(createOneIfClassIsKnown(className.c_str()));
    if (figure == nullptr)
        figure = getCanvas()->createFigure(type);
    // parse() validates the attribute keys against the figure's own getAllowedPropertyKeys(),
    // so a misspelled one is an error here just as it is in an @figure property
    figure->parse(property);
    return figure;
}

void StatisticCanvasVisualizer::setAnnotationSize(NetworkNodeCanvasVisualization *networkNodeVisualization, cFigure *figure, const cFigure::Point& size, cFigure::Point& lastSize) const
{
    // avoid invalidating the annotation layout of the network node when nothing changed
    if (size.x != lastSize.x || size.y != lastSize.y) {
        lastSize = size;
        networkNodeVisualization->setAnnotationSize(figure, size);
    }
}

StatisticVisualizerBase::StatisticVisualization *StatisticCanvasVisualizer::createStatisticVisualization(cComponent *module, simsignal_t signal)
{
    auto networkNode = getContainingNode(check_and_cast<cModule *>(module));
    // find, not get: get throws when the node has no visualization, which is a legitimate
    // configuration whenever the scene visualizer's nodeFilter is narrower than sourceFilter
    auto networkNodeVisualization = networkNodeVisualizer->findNetworkNodeVisualization(networkNode);
    if (networkNodeVisualization == nullptr)
        return nullptr; // the network node is not visualized
    cFigure *figure = createIndicatorFigure();
    if (figure == nullptr && splitMode == SPLIT_NONE) {
        // a single value is displayed with a text label by default
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
        if (figure == nullptr) {
            // several values need a figure that displays several items, a bar chart by default
            cProperty property("figure");
            property.addKey("type");
            property.setValue("type", 0, "barChart");
            figure = createFigure(&property);
        }
        if (dynamic_cast<IIndicatorFigure *>(figure) == nullptr) {
            std::string className = figure->getClassName();
            delete figure;
            throw cRuntimeError("Cannot display statistic values with a figure of class %s, because it is not an indicator figure", className.c_str());
        }
        figure->setName("statistic");
        std::string tooltip = std::string("This figure represents the value of ") + statisticName + " in " + module->getFullPath();
        figure->setTooltip(tooltip.c_str());
    }
    figure->setTags((std::string("statistic ") + tags).c_str());
    figure->setAssociatedObject(module);
    figure->setZIndex(zIndex);
    return new StatisticCanvasVisualization(networkNodeVisualization, networkNode->getId(), figure, module->getId(), signal, getUnit(module));
}

void StatisticCanvasVisualizer::addStatisticVisualization(StatisticVisualization *statisticVisualization)
{
    StatisticVisualizerBase::addStatisticVisualization(statisticVisualization);
    auto statisticCanvasVisualization = static_cast<StatisticCanvasVisualization *>(statisticVisualization);
    auto figure = statisticCanvasVisualization->figure;
    if (auto indicatorFigure = dynamic_cast<IIndicatorFigure *>(figure)) {
        auto size = indicatorFigure->getSize();
        statisticCanvasVisualization->annotationSize = size;
        statisticCanvasVisualization->networkNodeVisualization->addAnnotation(figure, cFigure::Rectangle(0.0, 0.0, size.x, size.y), placementHint, placementPriority);
    }
    else {
        auto boxedLabelFigure = check_and_cast<BoxedLabelFigure *>(figure);
        statisticCanvasVisualization->networkNodeVisualization->addAnnotation(figure, boxedLabelFigure->getBounds(), placementHint, placementPriority);
    }
}

void StatisticCanvasVisualizer::removeStatisticVisualization(StatisticVisualization *statisticVisualization)
{
    StatisticVisualizerBase::removeStatisticVisualization(statisticVisualization);
    auto statisticCanvasVisualization = static_cast<StatisticCanvasVisualization *>(statisticVisualization);
    // the cached network node visualization may already be gone: it is a figure group, and it
    // is destroyed with its network node, taking the statistic figure with it. Look it up
    // instead of trusting the pointer, and when it is gone leave the figure to it.
    if (networkNodeVisualizer != nullptr) {
        auto networkNode = getSimulation()->getModule(statisticCanvasVisualization->networkNodeId);
        auto networkNodeVisualization = networkNode != nullptr ? networkNodeVisualizer->findNetworkNodeVisualization(networkNode) : nullptr;
        if (networkNodeVisualization != nullptr)
            networkNodeVisualization->removeAnnotation(statisticCanvasVisualization->figure);
        else
            statisticCanvasVisualization->figure = nullptr;
    }
}

void StatisticCanvasVisualizer::refreshStatisticVisualization(StatisticVisualization *statisticVisualization)
{
    StatisticVisualizerBase::refreshStatisticVisualization(statisticVisualization);
    refreshFigure(static_cast<StatisticCanvasVisualization *>(statisticVisualization));
}

void StatisticCanvasVisualizer::refreshDisplay() const
{
    VisualizerBase::refreshDisplay();
    // the visualization of a deleted module must go before anything reads its figure; the
    // removal changes state, which is why refreshDisplay() being const is stepped around here
    const_cast<StatisticCanvasVisualizer *>(this)->removeVisualizationsOfDeletedModules();
    if (splitMode == SPLIT_NONE)
        return; // a single value is refreshed when its signal is received
    if (splitMode == SPLIT_FLOW)
        refreshFlowItemValues();
    for (auto& it : statisticVisualizations)
        refreshFigure(static_cast<StatisticCanvasVisualization *>(it.second));
}

void StatisticCanvasVisualizer::refreshFigure(StatisticCanvasVisualization *statisticCanvasVisualization) const
{
    auto figure = statisticCanvasVisualization->figure;
    auto& items = statisticCanvasVisualization->items;
    if (auto indicatorFigure = dynamic_cast<IIndicatorFigure *>(figure)) {
        if ((int)items.size() != indicatorFigure->getNumItems())
            setFigureItems(figure, items);
        int index = 0;
        for (auto& item : items)
            indicatorFigure->setValue(index++, simTime(), item.second.value);
        indicatorFigure->refreshDisplay();
        // the size of a figure may depend on its value, e.g. that of a counter, and on the
        // number of its items, as in a bar chart that gained a bar
        setAnnotationSize(statisticCanvasVisualization->networkNodeVisualization, figure, indicatorFigure->getSize(), statisticCanvasVisualization->annotationSize);
    }
    else {
        auto boxedLabelFigure = check_and_cast<BoxedLabelFigure *>(figure);
        boxedLabelFigure->setText(getText(statisticCanvasVisualization).c_str());
        statisticCanvasVisualization->networkNodeVisualization->setAnnotationSize(figure, boxedLabelFigure->getBounds().getSize());
    }
}

void StatisticCanvasVisualizer::setFigureItems(cFigure *figure, const std::map<std::string, StatisticVisualization::Item>& items) const
{
    // only a bar chart can be given a changing set of labelled items so far; another
    // indicator figure displays the single item of an unsplit, ungrouped statistic
    auto barChartFigure = dynamic_cast<BarChartFigure *>(figure);
    if (barChartFigure == nullptr) {
        if (items.size() > 1)
            throw cRuntimeError("Cannot display %d values with a figure of class %s, because only a bar chart displays a changing set of labelled items", (int)items.size(), figure->getClassName());
        return;
    }
    barChartFigure->setNumItems(items.size());
    int index = 0;
    for (auto& item : items)
        barChartFigure->setItemLabel(index++, item.first.c_str());
}

void StatisticCanvasVisualizer::removeVisualizationsOfDeletedModules()
{
    std::vector<StatisticVisualization *> removedStatisticVisualizations;
    for (auto& it : statisticVisualizations)
        if (getSimulation()->getModule(it.second->moduleId) == nullptr)
            removedStatisticVisualizations.push_back(it.second);
    for (auto statisticVisualization : removedStatisticVisualizations) {
        removeStatisticVisualization(statisticVisualization);
        delete statisticVisualization;
    }
}

} // namespace visualizer

} // namespace inet

