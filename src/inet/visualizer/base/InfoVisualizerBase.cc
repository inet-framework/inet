//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/InfoVisualizerBase.h"

#include <algorithm>

namespace inet {

namespace visualizer {

InfoVisualizerBase::InfoVisualization::InfoVisualization(int moduleId) :
    moduleId(moduleId)
{
}

const char *InfoVisualizerBase::DirectiveResolver::resolveDirective(char directive) const
{
    static std::string result;
    switch (directive) {
        case 'n':
            result = module->getFullName();
            break;
        case 'p':
            result = module->getFullPath();
            break;
        case 't':
            result = module->getDisplayString().getTagArg("t", 0);
            break;
        case 's':
            result = module->str();
            break;
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
    return result.c_str();
}

void InfoVisualizerBase::preDelete(cComponent *root)
{
    if (displayInfos)
        removeAllInfoVisualizations();
}

void InfoVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayInfos = par("displayInfos");
        modules.setPattern(par("modules"));
        format.parseFormat(par("format"));
        font = cFigure::parseFont(par("font"));
        textColor = cFigure::Color(par("textColor"));
        backgroundColor = cFigure::Color(par("backgroundColor"));
        opacity = par("opacity");
        placementHint = parsePlacement(par("placementHint"));
        placementPriority = par("placementPriority");
    }
    else if (stage == INITSTAGE_LAST) {
        if (displayInfos)
            addInfoVisualizations();
    }
}

void InfoVisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (name != nullptr) {
        if (!strcmp(name, "modules"))
            modules.setPattern(par("modules"));
        else if (!strcmp(name, "format"))
            format.parseFormat(par("format"));
        removeAllInfoVisualizations();
        addInfoVisualizations();
    }
}

void InfoVisualizerBase::refreshDisplay() const
{
    auto simulation = getSimulation();
    for (auto infoVisualization : infoVisualizations) {
        auto module = simulation->getModule(infoVisualization->moduleId);
        if (module != nullptr)
            refreshInfoVisualization(infoVisualization, getInfoVisualizationText(module));
    }
}

void InfoVisualizerBase::addInfoVisualization(const InfoVisualization *infoVisualization)
{
    infoVisualizations.push_back(infoVisualization);
}

void InfoVisualizerBase::removeInfoVisualization(const InfoVisualization *infoVisualization)
{
    infoVisualizations.erase(std::remove(infoVisualizations.begin(), infoVisualizations.end(), infoVisualization), infoVisualizations.end());
}

void InfoVisualizerBase::addInfoVisualizations()
{
    auto simulation = getSimulation();
    for (int id = 0; id < simulation->getLastComponentId(); id++) {
        auto component = simulation->getComponent(id);
        if (component != nullptr && component->isModule() && modules.matches(static_cast<cModule *>(component))) {
            auto infoVisualization = createInfoVisualization(static_cast<cModule *>(component));
            addInfoVisualization(infoVisualization);
        }
    }
}

void InfoVisualizerBase::removeAllInfoVisualizations()
{
    for (auto infoVisualization : std::vector<const InfoVisualization *>(infoVisualizations)) {
        removeInfoVisualization(infoVisualization);
        delete infoVisualization;
    }
}

const char *InfoVisualizerBase::getInfoVisualizationText(cModule *module) const
{
    DirectiveResolver directiveResolver(module);
    return format.formatString(&directiveResolver);
}

} // namespace visualizer

} // namespace inet

