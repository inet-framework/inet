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
#include "inet/visualizer/base/StatisticVisualizerBase.h"

namespace inet {

namespace visualizer {

StatisticVisualizerBase::StatisticVisualization::StatisticVisualization(int moduleId, simsignal_t signal, const char *unit) :
    moduleId(moduleId),
    signal(signal),
    unit(unit)
{
    recorder = new LastValueRecorder();
}

StatisticVisualizerBase::~StatisticVisualizerBase()
{
    if (displayStatistics)
        unsubscribe();
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
        prefix = par("prefix");
        unit = par("unit");
        minValue = par("minValue");
        maxValue = par("maxValue");
        font = cFigure::parseFont(par("font"));
        textColor = cFigure::Color(par("textColor"));
        backgroundColor = cFigure::Color(par("backgroundColor"));
        opacity = par("opacity");
        if (displayStatistics)
            subscribe();
    }
}

void StatisticVisualizerBase::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "sourceFilter"))
            sourceFilter.setPattern(par("sourceFilter"));
        removeAllStatisticVisualizations();
    }
}

void StatisticVisualizerBase::subscribe()
{
    auto subscriptionModule = getModuleFromPar<cModule>(par("subscriptionModule"), this);
    if (*signalName != '\0')
        subscriptionModule->subscribe(registerSignal(signalName), this);
}

void StatisticVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto subscriptionModule = getModuleFromPar<cModule>(par("subscriptionModule"), this, false);
    if (subscriptionModule != nullptr) {
        if (*signalName != '\0')
            subscriptionModule->unsubscribe(registerSignal(signalName), this);
    }
}

cResultFilter *StatisticVisualizerBase::findResultFilter(cComponent *source, simsignal_t signal)
{
    auto listeners = source->getLocalSignalListeners(signal);
    for (auto listener : listeners) {
        if (auto resultListener = dynamic_cast<cResultListener *>(listener)) {
            auto foundResultFilter = findResultFilter(nullptr, resultListener);
            if (foundResultFilter != nullptr)
                return foundResultFilter;
        }
    }
    return nullptr;
}

cResultFilter *StatisticVisualizerBase::findResultFilter(cResultFilter *parentResultFilter, cResultListener *resultListener)
{
    if (auto resultRecorder = dynamic_cast<cResultRecorder *>(resultListener)) {
        if (resultRecorder->getStatisticName() == nullptr || !strcmp(statisticName, resultRecorder->getStatisticName()))
            return parentResultFilter;
    }
    else if (auto resultFilter = dynamic_cast<cResultFilter *>(resultListener)) {
        auto delegates = resultFilter->getDelegates();
        for (auto delegate : delegates) {
            auto foundResultFilter = findResultFilter(resultFilter, delegate);
            if (foundResultFilter != nullptr)
                return foundResultFilter;
        }
    }
    return nullptr;
}

std::string StatisticVisualizerBase::getText(const StatisticVisualization *statisticVisualization)
{
    char temp[128];
    double value = statisticVisualization->recorder->getLastValue();
    if (std::isnan(value))
        sprintf(temp, "%s: -", prefix);
    else {
        auto valueUnit = statisticVisualization->unit;
        if (*unit != '\0') {
            cStringTokenizer tokenizer(unit);
            while (tokenizer.hasMoreTokens()) {
                auto printUnit = tokenizer.nextToken();
                double printValue = cNEDValue::convertUnit(value, valueUnit, printUnit);
                if (printValue > 1 || !tokenizer.hasMoreTokens()) {
                    sprintf(temp, "%s: %.2g %s", prefix, printValue, printUnit);
                    break;
                }
            }
        }
        else
            sprintf(temp, "%s: %.2g %s", prefix, value, valueUnit == nullptr ? "" : valueUnit);
    }
    return temp;
}

const char *StatisticVisualizerBase::getUnit(cComponent *source)
{
    auto properties = source->getProperties();
    for (int i = 0; i < properties->getNumProperties(); i++) {
        auto property = properties->get(i);
        if (!strcmp(property->getName(), "statistic") && !strcmp(property->getIndex(), statisticName))
            return property->getValue("unit", 0);
    }
    return nullptr;
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

void StatisticVisualizerBase::processSignal(cComponent *source, simsignal_t signal, double value)
{
    auto statisticVisualization = getStatisticVisualization(source, signal);
    if (statisticVisualization != nullptr)
        refreshStatisticVisualization(statisticVisualization);
    else {
        if (sourceFilter.matches(check_and_cast<cModule *>(source))) {
            statisticVisualization = createStatisticVisualization(source, signal);
            auto resultFilter = findResultFilter(source, signal);
            statisticVisualization->recorder->setLastValue(value);
            if (resultFilter == nullptr)
                source->subscribe(registerSignal(signalName), statisticVisualization->recorder);
            else
                resultFilter->addDelegate(statisticVisualization->recorder);
            addStatisticVisualization(statisticVisualization);
            refreshStatisticVisualization(statisticVisualization);
        }
    }
}

} // namespace visualizer

} // namespace inet

