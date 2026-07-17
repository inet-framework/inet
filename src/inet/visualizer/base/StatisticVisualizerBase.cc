//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/StatisticVisualizerBase.h"

#include <cmath>

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/util/ColorSet.h"
#include "omnetpp/cstatisticbuilder.h"

namespace inet {

namespace visualizer {

Register_ResultRecorder("statisticVisualizerLastValueRecorder", StatisticVisualizerBase::LastValueRecorder);

StatisticVisualizerBase::StatisticVisualization::StatisticVisualization(int moduleId, simsignal_t signal, const char *unit) :
    moduleId(moduleId),
    signal(signal),
    unit(unit)
{
}

StatisticVisualizerBase::BarSetVisualization::BarSetVisualization(int networkNodeId, int moduleId) :
    networkNodeId(networkNodeId),
    moduleId(moduleId)
{
}

void StatisticVisualizerBase::preDelete(cComponent *root)
{
    if (displayStatistics) {
        unsubscribe();
        removeAllStatisticVisualizations();
        removeAllBarSetVisualizations();
    }
}

std::string StatisticVisualizerBase::DirectiveResolver::resolveDirective(char directive) const
{
    switch (directive) {
        case 's':
            return visualizer->signalName;
        case 'n':
            return visualizer->statisticName;
        case 'v':
            if (std::isnan(visualization->printValue))
                return "-";
            else {
                char temp[32];
                snprintf(temp, sizeof(temp), "%.4g", visualization->printValue);
                return temp;
            }
            break;
        case 'u':
            return visualization->printUnit;
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
}

void StatisticVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayStatistics = par("displayStatistics");
        sourceFilter.setPattern(par("sourceFilter"));
        signalName = par("signalName");
        statisticName = par("statisticName");
        statisticUnit = par("statisticUnit");
        statisticExpression = par("statisticExpression");
        format.parseFormat(par("format"));
        cStringTokenizer tokenizer(par("unit"));
        while (tokenizer.hasMoreTokens())
            units.push_back(tokenizer.nextToken());
        font = cFigure::parseFont(par("font"));
        textColor = cFigure::Color(par("textColor"));
        backgroundColor = cFigure::Color(par("backgroundColor"));
        opacity = par("opacity");
        placementHint = parsePlacement(par("placementHint"));
        placementPriority = par("placementPriority");
        barChartMode = !strcmp(par("displayMode"), "bars");
        if (barChartMode) {
            maxValue = par("maxValue");
            minValue = par("minValue");
            barWidth = par("barWidth");
            barSpacing = par("barSpacing");
            maxBarHeight = par("maxBarHeight");
            ColorSet barColorSet;
            barColorSet.parseColors(par("barColor"));
            for (size_t i = 0; i < barColorSet.getSize(); i++)
                barColors.push_back(barColorSet.getColor(i));
            valueFormat = par("valueFormat").stringValue();
            valueLabelFont = cFigure::parseFont(par("valueLabelFont"));
            valueLabelColor = cFigure::Color(par("valueLabelColor"));
            seriesLabelFont = cFigure::parseFont(par("seriesLabelFont"));
            seriesLabelColor = cFigure::Color(par("seriesLabelColor"));
            nameRotation = par("nameRotation").doubleValueInUnit("rad");
            displayTitle = par("displayTitle");
            titleFont = cFigure::parseFont(par("titleFont"));
            titleColor = cFigure::Color(par("titleColor"));
        }
        if (displayStatistics) {
            if (opp_isempty(signalName))
                throw cRuntimeError("The signalName parameter must be not empty");
            subscribe();
        }
    }
}

void StatisticVisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (!strcmp(name, "sourceFilter"))
        sourceFilter.setPattern(par("sourceFilter"));
    else if (!strcmp(name, "format"))
        format.parseFormat(par("format"));
    removeAllStatisticVisualizations();
    removeAllBarSetVisualizations();
}

void StatisticVisualizerBase::subscribe()
{
    visualizationSubjectModule->subscribe(registerSignal(signalName), this);
}

void StatisticVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto visualizationSubjectModule = findModuleFromPar<cModule>(par("visualizationSubjectModule"), this);
    if (visualizationSubjectModule != nullptr)
        visualizationSubjectModule->unsubscribe(registerSignal(signalName), this);
}

void StatisticVisualizerBase::addResultRecorder(cComponent *source, simsignal_t signal)
{
    cStatisticBuilder statisticBuilder(getEnvir()->getConfig());
    cProperty statisticTemplateProperty;
    auto recordingMode = getRecordingMode();
    statisticTemplateProperty.addKey("record");
    statisticTemplateProperty.setValue("record", 0, recordingMode.c_str());
    statisticTemplateProperty.setIndex("visualizer");
    statisticBuilder.addResultRecorders(source, signal, statisticName, &statisticTemplateProperty);
}

StatisticVisualizerBase::LastValueRecorder *StatisticVisualizerBase::getResultRecorder(cComponent *source, simsignal_t signal)
{
    auto listeners = source->getLocalSignalListeners(signal);
    for (auto listener : listeners) {
        if (auto resultListener = dynamic_cast<cResultListener *>(listener)) {
            auto foundResultFilter = findResultRecorder(resultListener);
            if (foundResultFilter != nullptr)
                return foundResultFilter;
        }
    }
    throw cRuntimeError("Recorder not found for signal '%s'", signalName);
}

StatisticVisualizerBase::LastValueRecorder *StatisticVisualizerBase::findResultRecorder(cResultListener *resultListener)
{
    if (auto resultRecorder = dynamic_cast<StatisticVisualizerBase::LastValueRecorder *>(resultListener)) {
        if (getRecordingMode() == resultRecorder->getRecordingMode() && !strcmp(statisticName, resultRecorder->getStatisticName()))
            return resultRecorder;
        else
            return nullptr;
    }
    else if (auto resultFilter = dynamic_cast<cResultFilter *>(resultListener)) {
        auto delegates = resultFilter->getDelegates();
        for (auto delegate : delegates) {
            auto foundResultFilter = findResultRecorder(delegate);
            if (foundResultFilter != nullptr)
                return foundResultFilter;
        }
    }
    return nullptr;
}

std::string StatisticVisualizerBase::getText(const StatisticVisualization *statisticVisualization)
{
    DirectiveResolver directiveResolver(this, statisticVisualization);
    return format.formatString(&directiveResolver);
}

const char *StatisticVisualizerBase::getUnit(cComponent *source)
{
    auto properties = source->getProperties();
    for (int i = 0; i < properties->getNumProperties(); i++) {
        auto property = properties->get(i);
        if (!strcmp(property->getName(), "statistic") && !strcmp(property->getIndex(), statisticName)) {
            auto unit = property->getValue("unit", 0);
            if (unit != nullptr)
                return unit;
        }
    }
    return statisticUnit;
}

std::string StatisticVisualizerBase::getRecordingMode()
{
    if (*statisticExpression == '\0')
        return "statisticVisualizerLastValueRecorder";
    else
        return std::string("statisticVisualizerLastValueRecorder(") + statisticExpression + std::string(")");
}

const StatisticVisualizerBase::StatisticVisualization *StatisticVisualizerBase::getStatisticVisualization(cComponent *source, simsignal_t signal)
{
    auto key = std::pair<int, simsignal_t>(source->getId(), signal);
    auto it = statisticVisualizations.find(key);
    return (it == statisticVisualizations.end()) ? nullptr : it->second;
}

void StatisticVisualizerBase::addStatisticVisualization(const StatisticVisualization *statisticVisualization)
{
    auto key = std::pair<int, simsignal_t>(statisticVisualization->moduleId, statisticVisualization->signal);
    statisticVisualizations[key] = statisticVisualization;
}

void StatisticVisualizerBase::removeStatisticVisualization(const StatisticVisualization *statisticVisualization)
{
    auto key = std::pair<int, simsignal_t>(statisticVisualization->moduleId, statisticVisualization->signal);
    statisticVisualizations.erase(statisticVisualizations.find(key));
}

void StatisticVisualizerBase::removeAllStatisticVisualizations()
{
    std::vector<const StatisticVisualization *> removedStatisticVisualizations;
    for (auto it : statisticVisualizations)
        removedStatisticVisualizations.push_back(it.second);
    for (auto it : removedStatisticVisualizations) {
        removeStatisticVisualization(it);
        delete it;
    }
}

void StatisticVisualizerBase::processSignal(cComponent *source, simsignal_t signal, std::function<void(cIListener *)> receiveSignal)
{
    auto statisticVisualization = getStatisticVisualization(source, signal);
    if (statisticVisualization != nullptr)
        refreshStatisticVisualization(statisticVisualization);
    else {
        if (sourceFilter.matches(check_and_cast<cModule *>(source))) {
            auto statisticVisualization = createStatisticVisualization(source, signal);
            addResultRecorder(source, signal);
            statisticVisualization->recorder = getResultRecorder(source, signal);
            auto listeners = source->getLocalSignalListeners(signal);
            receiveSignal(listeners[listeners.size() - 1]);
            addStatisticVisualization(statisticVisualization);
            refreshStatisticVisualization(statisticVisualization);
        }
    }
}

void StatisticVisualizerBase::refreshStatisticVisualization(const StatisticVisualization *statisticVisualization)
{
    double value = statisticVisualization->recorder->getLastValue();
    if (std::isnan(value) || units.empty()) {
        statisticVisualization->printValue = value;
        statisticVisualization->printUnit = statisticVisualization->unit == nullptr ? "" : statisticVisualization->unit;
    }
    else {
        for (auto& unit : units) {
            statisticVisualization->printUnit = unit.c_str();
            statisticVisualization->printValue = cNEDValue::convertUnit(value, statisticVisualization->unit, statisticVisualization->printUnit);
            if (statisticVisualization->printValue > 1)
                break;
        }
    }
}

StatisticVisualizerBase::BarSetVisualization *StatisticVisualizerBase::getBarSetVisualization(int moduleId)
{
    auto it = barSetVisualizations.find(moduleId);
    return it == barSetVisualizations.end() ? nullptr : it->second;
}

void StatisticVisualizerBase::addBarSetVisualization(BarSetVisualization *barSetVisualization)
{
    barSetVisualizations[barSetVisualization->moduleId] = barSetVisualization;
}

void StatisticVisualizerBase::removeBarSetVisualization(BarSetVisualization *barSetVisualization)
{
    barSetVisualizations.erase(barSetVisualization->moduleId);
}

void StatisticVisualizerBase::removeAllBarSetVisualizations()
{
    std::vector<BarSetVisualization *> removedBarSetVisualizations;
    for (auto it : barSetVisualizations)
        removedBarSetVisualizations.push_back(it.second);
    for (auto barSetVisualization : removedBarSetVisualizations) {
        removeBarSetVisualization(barSetVisualization);
        delete barSetVisualization;
    }
}

void StatisticVisualizerBase::processBarValue(cComponent *source, double value, cObject *details)
{
    // Each bar is keyed on a demux label (the details full name); a value emitted without a label
    // (e.g. a group-addressed frame's rate) has no bar of its own and is ignored in bar chart mode.
    std::string label = details != nullptr ? details->getFullName() : "";
    if (label.empty())
        return;
    auto module = check_and_cast<cModule *>(source);
    auto barSetVisualization = getBarSetVisualization(module->getId());
    if (barSetVisualization == nullptr) {
        if (!sourceFilter.matches(module))
            return;
        barSetVisualization = createBarSetVisualization(source);
        if (barSetVisualization == nullptr)
            return; // bar charts not supported by this concrete visualizer (e.g. osg)
        addBarSetVisualization(barSetVisualization);
    }
    double displayValue = value;
    if (!units.empty() && statisticUnit != nullptr && *statisticUnit != '\0')
        displayValue = cNEDValue::convertUnit(value, statisticUnit, units[0].c_str());
    barSetVisualization->values[label] = displayValue;
}

std::string StatisticVisualizerBase::formatBarValue(double value) const
{
    char buf[64];
    snprintf(buf, sizeof(buf), valueFormat.c_str(), value);
    return buf;
}

cFigure::Color StatisticVisualizerBase::getBarColor(double value) const
{
    if (barColors.empty())
        return cFigure::Color("grey");
    if (barColors.size() == 1)
        return barColors[0];
    double range = maxValue - minValue;
    double fraction = range > 0 ? (value - minValue) / range : 0;
    if (fraction < 0) fraction = 0;
    if (fraction > 1) fraction = 1;
    double pos = fraction * (barColors.size() - 1);
    int index = (int)std::floor(pos);
    if (index >= (int)barColors.size() - 1)
        return barColors.back();
    double t = pos - index;
    const auto& c0 = barColors[index];
    const auto& c1 = barColors[index + 1];
    auto lerp = [](uint8_t a, uint8_t b, double t) { return (uint8_t)std::round(a + (b - a) * t); };
    return cFigure::Color(lerp(c0.red, c1.red, t), lerp(c0.green, c1.green, t), lerp(c0.blue, c1.blue, t));
}

} // namespace visualizer

} // namespace inet

