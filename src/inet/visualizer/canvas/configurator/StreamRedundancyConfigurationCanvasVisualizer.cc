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

#include "inet/visualizer/canvas/configurator/StreamRedundancyConfigurationCanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/configurator/StreamRedundancyConfigurator.h"

namespace inet {

namespace visualizer {

Define_Module(StreamRedundancyConfigurationCanvasVisualizer);

void StreamRedundancyConfigurationCanvasVisualizer::initialize(int stage) {
    TreeCanvasVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        streamFilter.setPattern(par("streamFilter"), false, true, true);
    }
    else if (stage == INITSTAGE_LAST) {
        if (displayTrees) {
            auto streamRedundancyConfigurator = getModuleFromPar<StreamRedundancyConfigurator>(par("streamRedundancyConfiguratorModule"), this);
            for (auto streamName : streamRedundancyConfigurator->getStreamNames()) {
                cMatchableString matchableString(streamName.c_str());
                if (streamFilter.matches(&matchableString)) {
                    std::vector<std::vector<int>> moduleIds;
                    auto pathFragments = streamRedundancyConfigurator->getPathFragments(streamName.c_str());
                    for (auto& path : pathFragments) {
                        moduleIds.push_back(std::vector<int>());
                        for (auto name : path) {
                            auto it = name.find('.');
                            auto node = it == std::string::npos ? name : name.substr(0, it);
                            auto module = getModuleByPath(node.c_str());
                            moduleIds[moduleIds.size() - 1].push_back(module->getId());
                        }
                    }
                    auto treeVisualization = createTreeVisualization(moduleIds);
                    addTreeVisualization(treeVisualization);
                }
            }
        }
    }
}

const TreeCanvasVisualizerBase::TreeVisualization *StreamRedundancyConfigurationCanvasVisualizer::createTreeVisualization(const std::vector<std::vector<int>>& tree) const
{
    auto treeCanvasVisualization = static_cast<const TreeCanvasVisualization *>(TreeCanvasVisualizerBase::createTreeVisualization(tree));
    for (auto figure : treeCanvasVisualization->figures) {
        figure->setTags((std::string("stream_redundancy ") + tags).c_str());
        figure->setTooltip("This polyline arrow represents a redundant stream");
    }
    for (auto& path : treeCanvasVisualization->paths)
        path.shiftPriority = 3;
    return treeCanvasVisualization;
}

} // namespace visualizer

} // namespace inet

