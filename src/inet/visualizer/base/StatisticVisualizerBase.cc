//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/StatisticVisualizerBase.h"

#include <cmath>

#include "inet/common/ModuleAccess.h"
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

void StatisticVisualizerBase::preDelete(cComponent *root)
{
    if (displayStatistics) {
        unsubscribe();
        removeAllStatisticVisualizations();
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
        const char *splitBy = par("splitBy");
        if (!strcmp(splitBy, "none"))
            splitMode = SPLIT_NONE;
        else if (!strcmp(splitBy, "details"))
            splitMode = SPLIT_DETAILS;
        else if (!strcmp(splitBy, "flow"))
            splitMode = SPLIT_FLOW;
        else
            throw cRuntimeError("Unknown splitBy parameter value: '%s'", splitBy);
        if (displayStatistics) {
            if (opp_isempty(signalName))
                throw cRuntimeError("The signalName parameter must be not empty");
            subscribedSignal = registerSignal(signalName);
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
}

void StatisticVisualizerBase::subscribe()
{
    visualizationSubjectModule->subscribe(subscribedSignal, this);
}

void StatisticVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto visualizationSubjectModule = findModuleFromPar<cModule>(par("visualizationSubjectModule"), this);
    if (visualizationSubjectModule != nullptr)
        visualizationSubjectModule->unsubscribe(subscribedSignal, this);
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

std::string StatisticVisualizerBase::getText(const StatisticVisualization *statisticVisualization) const
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

std::string StatisticVisualizerBase::getRecordingMode() const
{
    if (*statisticExpression == '\0')
        return "statisticVisualizerLastValueRecorder";
    else
        return std::string("statisticVisualizerLastValueRecorder(") + statisticExpression + std::string(")");
}

StatisticVisualizerBase::StatisticVisualization *StatisticVisualizerBase::getStatisticVisualization(int moduleId)
{
    auto it = statisticVisualizations.find(moduleId);
    return it == statisticVisualizations.end() ? nullptr : it->second;
}

StatisticVisualizerBase::StatisticVisualization *StatisticVisualizerBase::getOrCreateStatisticVisualization(cComponent *module, simsignal_t signal)
{
    auto statisticVisualization = getStatisticVisualization(module->getId());
    if (statisticVisualization == nullptr) {
        statisticVisualization = createStatisticVisualization(module, signal);
        if (statisticVisualization == nullptr)
            return nullptr; // the module is not visualized
        addStatisticVisualization(statisticVisualization);
    }
    return statisticVisualization;
}

void StatisticVisualizerBase::addStatisticVisualization(StatisticVisualization *statisticVisualization)
{
    statisticVisualizations[statisticVisualization->moduleId] = statisticVisualization;
}

void StatisticVisualizerBase::removeStatisticVisualization(StatisticVisualization *statisticVisualization)
{
    statisticVisualizations.erase(statisticVisualization->moduleId);
}

void StatisticVisualizerBase::removeAllStatisticVisualizations()
{
    std::vector<StatisticVisualization *> removedStatisticVisualizations;
    for (auto it : statisticVisualizations)
        removedStatisticVisualizations.push_back(it.second);
    for (auto statisticVisualization : removedStatisticVisualizations) {
        removeStatisticVisualization(statisticVisualization);
        delete statisticVisualization;
    }
    registeredSourceIds.clear();
}

void StatisticVisualizerBase::processSignal(cComponent *source, simsignal_t signal, std::function<void(cIListener *)> receiveSignal)
{
    auto statisticVisualization = getStatisticVisualization(source->getId());
    if (statisticVisualization != nullptr)
        refreshStatisticVisualization(statisticVisualization);
    else {
        if (sourceFilter.matches(check_and_cast<cModule *>(source))) {
            statisticVisualization = createStatisticVisualization(source, signal);
            if (statisticVisualization == nullptr)
                return; // the module is not visualized
            addResultRecorder(source, signal);
            // the statistic is neither split nor grouped, so it has a single, unlabelled item
            statisticVisualization->items[""].recorder = getResultRecorder(source, signal);
            auto listeners = source->getLocalSignalListeners(signal);
            receiveSignal(listeners[listeners.size() - 1]);
            addStatisticVisualization(statisticVisualization);
            refreshStatisticVisualization(statisticVisualization);
        }
    }
}

void StatisticVisualizerBase::refreshStatisticVisualization(StatisticVisualization *statisticVisualization)
{
    auto& item = statisticVisualization->items[""];
    double value = item.recorder->getLastValue();
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
    item.value = statisticVisualization->printValue;
}

double StatisticVisualizerBase::convertToDisplayUnit(double value) const
{
    if (std::isnan(value) || units.empty() || opp_isempty(statisticUnit))
        return value;
    return cNEDValue::convertUnit(value, statisticUnit, units[0].c_str());
}

void StatisticVisualizerBase::processSplitValue(cComponent *source, double value, cObject *details)
{
    // the statistic is identified by the details object emitted with the value, the same way
    // as the demux() result filter identifies the statistics it demultiplexes a signal into;
    // a value emitted without a details object (e.g. the rate of a group addressed frame)
    // belongs to no statistic
    std::string label = details != nullptr ? details->getFullName() : "";
    if (label.empty())
        return;
    auto module = check_and_cast<cModule *>(source);
    if (!sourceFilter.matches(module))
        return;
    auto statisticVisualization = getOrCreateStatisticVisualization(module, subscribedSignal);
    if (statisticVisualization == nullptr)
        return;
    statisticVisualization->items[label].value = convertToDisplayUnit(value);
}

void StatisticVisualizerBase::registerSource(cComponent *source, simsignal_t signal)
{
    auto module = check_and_cast<cModule *>(source);
    if (registeredSourceIds.find(module->getId()) != registeredSourceIds.end())
        return; // already registered
    if (!sourceFilter.matches(module))
        return;
    registeredSourceIds.insert(module->getId());
    auto statisticVisualization = getOrCreateStatisticVisualization(module, signal);
    if (statisticVisualization == nullptr)
        return;
    // the values come from the result recorders built from statisticExpression, so that an
    // item can display e.g. a count or a throughput rather than the raw value of the signal
    addResultRecorder(source, signal);
    // when splitting by flow, statisticExpression contains demuxFlow(), so the recorder chain
    // creates a separate recorder per flow as the flows appear, see refreshFlowItemValues()
}

void StatisticVisualizerBase::refreshFlowItemValues() const
{
    for (auto& it : statisticVisualizations) {
        auto statisticVisualization = it.second;
        auto source = getSimulation()->getModule(statisticVisualization->moduleId);
        if (source == nullptr)
            continue;
        std::vector<LastValueRecorder *> recorders;
        for (auto listener : source->getLocalSignalListeners(subscribedSignal))
            if (auto resultListener = dynamic_cast<cResultListener *>(listener))
                collectResultRecorders(resultListener, recorders);
        for (auto recorder : recorders) {
            const char *label = recorder->getDemuxLabel();
            if (opp_isempty(label))
                continue; // the recorder of the undemultiplexed statistic is not an item
            auto& item = statisticVisualization->items[label];
            item.recorder = recorder;
            item.value = convertToDisplayUnit(recorder->getLastValue());
        }
    }
}

void StatisticVisualizerBase::collectResultRecorders(cResultListener *resultListener, std::vector<LastValueRecorder *>& recorders) const
{
    if (auto resultRecorder = dynamic_cast<LastValueRecorder *>(resultListener)) {
        if (getRecordingMode() == resultRecorder->getRecordingMode() && !strcmp(statisticName, resultRecorder->getStatisticName()))
            recorders.push_back(resultRecorder);
    }
    else if (auto resultFilter = dynamic_cast<cResultFilter *>(resultListener)) {
        for (auto delegate : resultFilter->getDelegates())
            collectResultRecorders(delegate, recorders);
    }
}

} // namespace visualizer

} // namespace inet

