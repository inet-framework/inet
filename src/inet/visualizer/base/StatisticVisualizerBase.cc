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

#include "omnetpp/cstatisticbuilder.h"

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/base/StatisticVisualizerBase.h"

namespace inet {

namespace visualizer {

Register_ResultRecorder("statisticVisualizer", StatisticVisualizerBase::LastValueRecorder);

StatisticVisualizerBase::StatisticVisualization::StatisticVisualization(int moduleId, simsignal_t signal, const char *unit) :
    moduleId(moduleId),
    signal(signal),
    unit(unit)
{
}

StatisticVisualizerBase::~StatisticVisualizerBase()
{
    if (displayStatistics)
        unsubscribe();
}

const char *StatisticVisualizerBase::DirectiveResolver::resolveDirective(char directive) const
{
    static std::string result;
    switch (directive) {
        case 's':
            result = visualizer->signalName;
            break;
        case 'n':
            result = visualizer->statisticName;
            break;
        case 'v':
            if (std::isnan(visualization->printValue))
                result = "-";
            else {
                char temp[32];
                sprintf(temp, "%.4g", visualization->printValue);
                result = temp;
            }
            break;
        case 'u':
            result = visualization->printUnit;
            break;
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
    return result.c_str();
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
        if (displayStatistics)
            subscribe();
    }
}

void StatisticVisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (name != nullptr) {
        if (!strcmp(name, "sourceFilter"))
            sourceFilter.setPattern(par("sourceFilter"));
        else if (!strcmp(name, "format"))
            format.parseFormat(par("format"));
        removeAllStatisticVisualizations();
    }
}

void StatisticVisualizerBase::subscribe()
{
    if (*signalName != '\0')
        visualizationSubjectModule->subscribe(registerSignal(signalName), this);
}

void StatisticVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto visualizationSubjectModule = getModuleFromPar<cModule>(par("visualizationSubjectModule"), this, false);
    if (visualizationSubjectModule != nullptr) {
        if (*signalName != '\0')
            visualizationSubjectModule->unsubscribe(registerSignal(signalName), this);
    }
}

void StatisticVisualizerBase::addResultRecorder(cComponent *source, simsignal_t signal)
{
    cStatisticBuilder statisticBuilder(getEnvir()->getConfig());
    cProperty statisticTemplateProperty;
    std::string record;
    if (*statisticExpression == '\0')
        record = "statisticVisualizer";
    else
        record = std::string("statisticVisualizer(") + statisticExpression + std::string(")");
    statisticTemplateProperty.addKey("record");
    statisticTemplateProperty.setValue("record", 0, record.c_str());
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
    if (auto resultRecorder = dynamic_cast<StatisticVisualizerBase::LastValueRecorder *>(resultListener))
        return resultRecorder;
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

const StatisticVisualizerBase::StatisticVisualization *StatisticVisualizerBase::getStatisticVisualization(cComponent *source, simsignal_t signal)
{
    auto key = std::pair<int, simsignal_t>(source->getId(), signal);
    auto it = statisticVisualizations.find(key);
    if (it == statisticVisualizations.end())
        return nullptr;
    else
        return it->second;
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

void StatisticVisualizerBase::processSignal(cComponent *source, simsignal_t signal, std::function<void (cIListener *)> receiveSignal)
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

} // namespace visualizer

} // namespace inet

