//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/configurator/FailureProtectionConfigurationCanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/configurator/FailureProtectionConfigurator.h"

namespace inet {

namespace visualizer {

Define_Module(FailureProtectionConfigurationCanvasVisualizer);

void FailureProtectionConfigurationCanvasVisualizer::initialize(int stage) {
    TreeCanvasVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        streamFilter.setPattern(par("streamFilter"), false, true, true);
    }
    else if (stage == INITSTAGE_LAST) {
        if (displayTrees) {
            auto failureProtectionConfigurator = getModuleFromPar<FailureProtectionConfigurator>(par("failureProtectionConfiguratorModule"), this);
            for (auto& stream : failureProtectionConfigurator->getStreams()) {
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

const TreeCanvasVisualizerBase::TreeVisualization *FailureProtectionConfigurationCanvasVisualizer::createTreeVisualization(const std::vector<std::vector<int>>& tree) const
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

