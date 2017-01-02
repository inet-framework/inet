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

#include <algorithm>
#include "inet/visualizer/base/InfoVisualizerBase.h"

namespace inet {

namespace visualizer {

InfoVisualizerBase::InfoVisualization::InfoVisualization(int moduleId) :
    moduleId(moduleId)
{
}

void InfoVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayInfos = par("displayInfos");
        modules.setPattern(par("modules"));
        content = par("content");
        font = cFigure::parseFont(par("font"));
        textColor = cFigure::Color(par("textColor"));
        backgroundColor = cFigure::Color(par("backgroundColor"));
        opacity = par("opacity");
    }
    else if (stage == INITSTAGE_LAST) {
        if (displayInfos)
            addInfoVisualizations();
    }
}

void InfoVisualizerBase::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "modules"))
            modules.setPattern(par("modules"));
        else if (!strcmp(name, "content"))
            content = par("content");
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
            refreshInfoVisualization(infoVisualization, getInfoText(module));
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
        if (component != nullptr && component->isModule() && modules.matches(static_cast<cModule*>(component))) {
            auto infoVisualization = createInfoVisualization(static_cast<cModule*>(component));
            addInfoVisualization(infoVisualization);
        }
    }
}

void InfoVisualizerBase::removeAllInfoVisualizations()
{
    for (auto infoVisualization : infoVisualizations) {
        removeInfoVisualization(infoVisualization);
        delete infoVisualization;
    }
}

const char* InfoVisualizerBase::getInfoText(omnetpp::cModule* module) const
{
    const char* info;
    if (!strcmp(content, "str"))
        info = module->str().c_str();
    else if (!strcmp(content, "displayStringText"))
        info = module->getDisplayString().getTagArg("t", 0);
    else
        throw cRuntimeError("Unknown content parameter value: %s", content);
    return info;
}

} // namespace visualizer

} // namespace inet

