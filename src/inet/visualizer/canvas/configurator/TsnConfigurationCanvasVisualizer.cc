//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/visualizer/canvas/configurator/TsnConfigurationCanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/configurator/TsnConfigurator.h"

namespace inet {

namespace visualizer {

Define_Module(TsnConfigurationCanvasVisualizer);

void TsnConfigurationCanvasVisualizer::initialize(int stage) {
    TreeCanvasVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        streamFilter.setPattern(par("streamFilter"), false, true, true);
    }
    else if (stage == INITSTAGE_LAST) {
        if (displayTrees) {
            auto tsnConfigurator = getModuleFromPar<TsnConfigurator>(par("tsnConfiguratorModule"), this);
            for (auto& stream : tsnConfigurator->getStreams()) {
                cMatchableString matchableString(stream.name.c_str());
                if (streamFilter.matches(&matchableString)) {
                    for (auto& tree : stream.trees) {
                        std::vector<std::vector<int>> moduleIds;
                        for (auto& path : tree.paths) {
                            moduleIds.push_back(std::vector<int>());
                            for (auto interface : path.interfaces)
                                moduleIds[moduleIds.size() - 1].push_back(interface->node->module->getId());
                        }
                        auto treeVisualization = createTreeVisualization(moduleIds);
                        addTreeVisualization(treeVisualization);
                    }
                }
            }
        }
    }
}

const TreeCanvasVisualizerBase::TreeVisualization *TsnConfigurationCanvasVisualizer::createTreeVisualization(const std::vector<std::vector<int>>& tree) const
{
    auto treeCanvasVisualization = static_cast<const TreeCanvasVisualization *>(TreeCanvasVisualizerBase::createTreeVisualization(tree));
    for (auto figure : treeCanvasVisualization->figures) {
        figure->setTags((std::string("tsn_stream ") + tags).c_str());
        figure->setTooltip("This polyline arrow represents a TSN stream");
    }
    for (auto& path : treeCanvasVisualization->paths)
        path.shiftPriority = 3;
    return treeCanvasVisualization;
}

} // namespace visualizer

} // namespace inet

