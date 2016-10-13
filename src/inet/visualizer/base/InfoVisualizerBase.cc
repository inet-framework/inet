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
        fontColor = cFigure::Color(par("fontColor"));
        backgroundColor = cFigure::Color(par("backgroundColor"));
        moduleMatcher.setPattern(par("modules"), true, true, true);
        opacity = par("opacity");
    }
    else if (stage == INITSTAGE_LAST) {
        auto simulation = getSimulation();
        for (int id = 0; id < simulation->getLastComponentId(); id++) {
            auto component = simulation->getComponent(id);
            if (component != nullptr && component->isModule() && moduleMatcher.matches(component->getFullPath().c_str()))
                infoVisualizations.push_back(createInfoVisualization(static_cast<cModule *>(component)));
        }
    }
}

void InfoVisualizerBase::refreshDisplay() const
{
    auto simulation = getSimulation();
    for (auto infoVisualization : infoVisualizations) {
        auto module = simulation->getModule(infoVisualization->moduleId);
        if (module != nullptr) {
            const char *content = par("content");
            const char *info;
            if (!strcmp(content, "info"))
                info = module->info().c_str();
            else if (!strcmp(content, "displayStringText"))
                info = module->getDisplayString().getTagArg("t", 0);
            else
                throw cRuntimeError("Unknown content parameter value: %s", content);
            refreshInfoVisualization(infoVisualization, info);
        }
    }
}

} // namespace visualizer

} // namespace inet

