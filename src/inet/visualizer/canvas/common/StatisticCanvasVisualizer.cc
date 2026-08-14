//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/common/StatisticCanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"
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

StatisticCanvasVisualizer::StatisticCanvasVisualization::StatisticCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, cFigure *figure, int moduleId, simsignal_t signal, const char *unit) :
    StatisticVisualization(moduleId, signal, unit),
    networkNodeVisualization(networkNodeVisualization),
    figure(figure)
{
}

StatisticCanvasVisualizer::StatisticCanvasVisualization::~StatisticCanvasVisualization()
{
    delete figure;
    figure = nullptr;
}

StatisticCanvasVisualizer::GroupCanvasVisualization::GroupCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, cFigure *figure, IIndicatorFigure *indicatorFigure, int moduleId) :
    GroupVisualization(moduleId),
    networkNodeVisualization(networkNodeVisualization),
    figure(figure),
    indicatorFigure(indicatorFigure)
{
}

StatisticCanvasVisualizer::GroupCanvasVisualization::~GroupCanvasVisualization()
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

StatisticVisualizerBase::StatisticVisualization *StatisticCanvasVisualizer::createStatisticVisualization(cComponent *source, simsignal_t signal)
{
    cFigure *figure = createIndicatorFigure();
    if (figure == nullptr) {
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
        figure->setName("statistic");
        std::string tooltip = std::string("This figure represents the value of ") + statisticName + " in " + source->getFullPath();
        figure->setTooltip(tooltip.c_str());
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
    if (auto indicatorFigure = dynamic_cast<IIndicatorFigure *>(figure)) {
        auto size = indicatorFigure->getSize();
        statisticCanvasVisualization->networkNodeVisualization->addAnnotation(statisticCanvasVisualization->figure, cFigure::Rectangle(0.0, 0.0, size.x, size.y), placementHint, placementPriority);
    }
    else if (auto boxedLabelFigure = check_and_cast<BoxedLabelFigure *>(figure))
        statisticCanvasVisualization->networkNodeVisualization->addAnnotation(statisticCanvasVisualization->figure, boxedLabelFigure->getBounds(), placementHint, placementPriority);
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
    auto statisticCanvasVisualization = static_cast<StatisticCanvasVisualization *>(const_cast<StatisticVisualization *>(statisticVisualization));
    auto figure = statisticCanvasVisualization->figure;
    if (auto indicatorFigure = dynamic_cast<IIndicatorFigure *>(figure)) {
        // the value in the display unit, the same value the text label would display; the
        // item is the only one of the figure, created here if the figure has dynamic items
        indicatorFigure->setValue(indicatorFigure->getItemIndex("", true), simTime(), statisticVisualization->printValue);
        // the size of a figure may depend on its value, e.g. that of a counter
        setAnnotationSize(statisticCanvasVisualization->networkNodeVisualization, figure, indicatorFigure->getSize(), statisticCanvasVisualization->annotationSize);
    }
    else {
        auto boxedLabelFigure = check_and_cast<BoxedLabelFigure *>(figure);
        boxedLabelFigure->setText(getText(statisticVisualization).c_str());
        statisticCanvasVisualization->networkNodeVisualization->setAnnotationSize(figure, boxedLabelFigure->getBounds().getSize());
    }
}

StatisticVisualizerBase::GroupVisualization *StatisticCanvasVisualizer::createGroupVisualization(cComponent *module)
{
    auto networkNode = getContainingNode(check_and_cast<cModule *>(module));
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    if (networkNodeVisualization == nullptr)
        return nullptr; // the network node is not visualized
    cFigure *figure = createIndicatorFigure();
    if (figure == nullptr) {
        // a bar chart is the default figure for displaying several items
        cProperty property("figure");
        property.addKey("type");
        property.setValue("type", 0, "barChart");
        figure = createFigure(&property);
    }
    auto indicatorFigure = dynamic_cast<IIndicatorFigure *>(figure);
    if (indicatorFigure == nullptr) {
        std::string className = figure->getClassName();
        delete figure;
        throw cRuntimeError("Cannot display statistic values with a figure of class %s, because it is not an indicator figure", className.c_str());
    }
    figure->setName("statisticGroup");
    figure->setTags((std::string("statistic ") + tags).c_str());
    figure->setAssociatedObject(module);
    figure->setZIndex(zIndex);
    return new GroupCanvasVisualization(networkNodeVisualization, figure, indicatorFigure, module->getId());
}

void StatisticCanvasVisualizer::addGroupVisualization(GroupVisualization *groupVisualization)
{
    StatisticVisualizerBase::addGroupVisualization(groupVisualization);
    auto groupCanvasVisualization = static_cast<GroupCanvasVisualization *>(groupVisualization);
    auto size = groupCanvasVisualization->indicatorFigure->getSize();
    groupCanvasVisualization->annotationSize = size;
    groupCanvasVisualization->networkNodeVisualization->addAnnotation(groupCanvasVisualization->figure, cFigure::Rectangle(0.0, 0.0, size.x, size.y), placementHint, placementPriority);
}

void StatisticCanvasVisualizer::removeGroupVisualization(GroupVisualization *groupVisualization)
{
    StatisticVisualizerBase::removeGroupVisualization(groupVisualization);
    auto groupCanvasVisualization = static_cast<GroupCanvasVisualization *>(groupVisualization);
    if (networkNodeVisualizer != nullptr)
        groupCanvasVisualization->networkNodeVisualization->removeAnnotation(groupCanvasVisualization->figure);
}

void StatisticCanvasVisualizer::refreshDisplay() const
{
    VisualizerBase::refreshDisplay();
    if (groupMode == GROUP_NONE)
        return;
    if (groupMode == GROUP_NETWORK_NODE)
        refreshSourceItemValues();
    else if (splitMode == SPLIT_FLOW)
        refreshFlowItemValues();
    for (auto& it : groupVisualizations)
        refreshGroupVisualization(static_cast<GroupCanvasVisualization *>(it.second));
}

void StatisticCanvasVisualizer::refreshGroupVisualization(GroupCanvasVisualization *groupVisualization) const
{
    auto indicatorFigure = groupVisualization->indicatorFigure;
    for (auto& value : groupVisualization->values) {
        const char *label = value.first.c_str();
        int itemIndex = indicatorFigure->getItemIndex(label, true);
        if (itemIndex == -1)
            throw cRuntimeError("Cannot display the item '%s' with a figure of class %s, because it does not support several named items", label, groupVisualization->figure->getClassName());
        indicatorFigure->setValue(itemIndex, simTime(), value.second);
    }
    indicatorFigure->refreshDisplay();
    setAnnotationSize(groupVisualization->networkNodeVisualization, groupVisualization->figure, indicatorFigure->getSize(), groupVisualization->annotationSize);
}

} // namespace visualizer

} // namespace inet

