//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/common/StatisticCanvasVisualizer.h"

#include <algorithm>
#include <cmath>

#include "inet/common/ModuleAccess.h"
#include "inet/common/figures/BoxedLabelFigure.h"
#include "inet/common/figures/IIndicatorFigure.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

namespace visualizer {

Define_Module(StatisticCanvasVisualizer);

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

StatisticCanvasVisualizer::BarSetCanvasVisualization::BarSetCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, cGroupFigure *figure, int networkNodeId, int moduleId) :
    BarSetVisualization(networkNodeId, moduleId),
    networkNodeVisualization(networkNodeVisualization),
    figure(figure)
{
}

StatisticCanvasVisualizer::BarSetCanvasVisualization::~BarSetCanvasVisualization()
{
    delete figure;
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

StatisticVisualizerBase::BarSetVisualization *StatisticCanvasVisualizer::createBarSetVisualization(cComponent *source)
{
    auto module = check_and_cast<cModule *>(source);
    auto figure = new cGroupFigure("statisticBars");
    figure->setTags((std::string("statistic_bars ") + tags).c_str());
    figure->setZIndex(zIndex);
    auto networkNode = getContainingNode(module);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    auto barSetVisualization = new BarSetCanvasVisualization(networkNodeVisualization, figure, networkNode->getId(), module->getId());
    // title: the containing network interface name if the source is under one (e.g. a rate control),
    // else the source module's own name (e.g. a network node in sources mode)
    barSetVisualization->title = module->getFullName();
    for (cModule *m = module; m != nullptr && !isNetworkNode(m); m = m->getParentModule()) {
        if (auto networkInterface = dynamic_cast<NetworkInterface *>(m)) {
            barSetVisualization->title = networkInterface->getInterfaceName();
            break;
        }
    }
    return barSetVisualization;
}

void StatisticCanvasVisualizer::addBarSetVisualization(BarSetVisualization *barSetVisualization)
{
    StatisticVisualizerBase::addBarSetVisualization(barSetVisualization);
    auto barSetCanvasVisualization = static_cast<BarSetCanvasVisualization *>(barSetVisualization);
    if (barSetCanvasVisualization->networkNodeVisualization != nullptr)
        barSetCanvasVisualization->networkNodeVisualization->addAnnotation(barSetCanvasVisualization->figure, cFigure::Rectangle(0, 0, 0, 0), placementHint, placementPriority);
}

void StatisticCanvasVisualizer::removeBarSetVisualization(BarSetVisualization *barSetVisualization)
{
    StatisticVisualizerBase::removeBarSetVisualization(barSetVisualization);
    auto barSetCanvasVisualization = static_cast<BarSetCanvasVisualization *>(barSetVisualization);
    if (networkNodeVisualizer != nullptr && barSetCanvasVisualization->networkNodeVisualization != nullptr)
        barSetCanvasVisualization->networkNodeVisualization->removeAnnotation(barSetCanvasVisualization->figure);
}

void StatisticCanvasVisualizer::refreshDisplay() const
{
    VisualizerBase::refreshDisplay();
    if (!barChartMode)
        return;
    if (barSeriesBySources)
        const_cast<StatisticCanvasVisualizer *>(this)->refreshGroupedBarValues();
    else if (barSeriesByFlow)
        const_cast<StatisticCanvasVisualizer *>(this)->refreshFlowBarValues();
    for (auto& it : barSetVisualizations) {
        auto barSetCanvasVisualization = static_cast<BarSetCanvasVisualization *>(it.second);
        refreshChart(barSetCanvasVisualization);
        if (barSetCanvasVisualization->networkNodeVisualization != nullptr)
            barSetCanvasVisualization->networkNodeVisualization->setAnnotationSize(barSetCanvasVisualization->figure, barSetCanvasVisualization->bounds.getSize());
    }
}

void StatisticCanvasVisualizer::refreshChart(BarSetCanvasVisualization *barSetVisualization) const
{
    auto figure = barSetVisualization->figure;
    // clear previous content
    while (figure->getNumFigures() > 0)
        delete figure->removeFigure(0);

    // order series by label for a stable layout
    std::vector<std::pair<std::string, double>> entries(barSetVisualization->values.begin(), barSetVisualization->values.end());
    std::sort(entries.begin(), entries.end(), [](const std::pair<std::string, double>& a, const std::pair<std::string, double>& b) {
        return a.first < b.first;
    });

    // reserve vertical space for the title and the value labels, scaled by their font size
    double titleHeight = displayTitle ? (titleFont.pointSize > 0 ? titleFont.pointSize : 8) + 5 : 0;
    double valueLabelHeight = (valueLabelFont.pointSize > 0 ? valueLabelFont.pointSize : 8) + 4;
    double baselineY = titleHeight + valueLabelHeight + maxBarHeight;
    double chartWidth = entries.empty() ? 0 : (entries.size() * barWidth + (entries.size() - 1) * barSpacing);

    if (displayTitle) {
        auto titleFigure = new cLabelFigure("title");
        titleFigure->setText(barSetVisualization->title.c_str());
        titleFigure->setFont(titleFont);
        titleFigure->setColor(titleColor);
        titleFigure->setAnchor(cFigure::ANCHOR_N);
        titleFigure->setPosition(cFigure::Point(chartWidth / 2, 0));
        figure->addFigure(titleFigure);
    }

    if (!entries.empty()) {
        auto baseline = new cLineFigure("baseline");
        baseline->setStart(cFigure::Point(-1, baselineY));
        baseline->setEnd(cFigure::Point(chartWidth + 1, baselineY));
        baseline->setLineColor(cFigure::Color("grey50"));
        figure->addFigure(baseline);
    }

    // Fixed scale when maxValue is set (> minValue); otherwise autoscale each chart to its largest value.
    double effectiveMax = maxValue;
    if (!(effectiveMax > minValue)) {
        effectiveMax = minValue;
        for (auto& entry : entries)
            if (!std::isnan(entry.second) && entry.second > effectiveMax)
                effectiveMax = entry.second;
    }
    double range = effectiveMax - minValue;
    for (size_t i = 0; i < entries.size(); i++) {
        double value = entries[i].second;
        double x = i * (barWidth + barSpacing);
        double fraction = range > 0 ? (value - minValue) / range : 0;
        if (fraction < 0) fraction = 0;
        if (fraction > 1) fraction = 1;
        double h = maxBarHeight * fraction;

        auto bar = new cRectangleFigure("bar");
        bar->setBounds(cFigure::Rectangle(x, baselineY - h, barWidth, h));
        bar->setFilled(true);
        bar->setFillColor(getBarColor(value, effectiveMax));
        bar->setLineColor(cFigure::Color("grey30"));
        bar->setTooltip((entries[i].first + ": " + formatBarValue(value)).c_str());
        figure->addFigure(bar);

        // value label centered above the bar
        auto valueLabel = new cLabelFigure("value");
        valueLabel->setText(formatBarValue(value).c_str());
        valueLabel->setFont(valueLabelFont);
        valueLabel->setColor(valueLabelColor);
        valueLabel->setAnchor(cFigure::ANCHOR_S);
        valueLabel->setPosition(cFigure::Point(x + barWidth / 2, baselineY - h - 1));
        figure->addFigure(valueLabel);

        // series label below the baseline, rotated
        auto seriesLabel = new cLabelFigure("series");
        seriesLabel->setText(entries[i].first.c_str());
        seriesLabel->setFont(seriesLabelFont);
        seriesLabel->setColor(seriesLabelColor);
        seriesLabel->setAnchor(cFigure::ANCHOR_NE);
        seriesLabel->setAngle(nameRotation);
        seriesLabel->setPosition(cFigure::Point(x + barWidth / 2, baselineY + 2));
        figure->addFigure(seriesLabel);
    }

    // Estimate the bounding box (in the group's local coordinates). The rotated series labels hang
    // below and to the left of the baseline; reserve space for them.
    double longestName = 0;
    for (auto& entry : entries)
        longestName = std::max(longestName, (double)entry.first.length());
    double nameExtent = longestName * 4 + 4; // rough per-character estimate for the small font
    double bottom = baselineY + 4 + std::max(6.0, nameExtent * std::abs(std::sin(nameRotation)));
    double left = -std::max(2.0, nameExtent * std::abs(std::cos(nameRotation)));
    barSetVisualization->bounds = cFigure::Rectangle(left, 0, (chartWidth - left) + 2, bottom);
}

} // namespace visualizer

} // namespace inet

