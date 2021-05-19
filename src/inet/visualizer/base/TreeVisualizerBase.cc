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

#include "inet/visualizer/base/TreeVisualizerBase.h"

#include "inet/common/FlowTag.h"
#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/mobility/contract/IMobility.h"

namespace inet {

namespace visualizer {

TreeVisualizerBase::TreeVisualization::TreeVisualization(const std::vector<std::vector<int>>& tree)
{
    for (auto& path : tree)
        paths.push_back(LineManager::ModulePath(path));
}

void TreeVisualizerBase::preDelete(cComponent *root)
{
    if (displayTrees)
        removeAllTreeVisualizations();
}

void TreeVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayTrees = par("displayTrees");
        lineColorSet.parseColors(par("lineColor"));
        lineStyle = cFigure::parseLineStyle(par("lineStyle"));
        lineWidth = par("lineWidth");
        lineSmooth = par("lineSmooth");
        lineShift = par("lineShift");
        lineShiftMode = par("lineShiftMode");
        lineContactSpacing = par("lineContactSpacing");
        lineContactMode = par("lineContactMode");
    }
}

void TreeVisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (name != nullptr)
        removeAllTreeVisualizations();
}

const TreeVisualizerBase::TreeVisualization *TreeVisualizerBase::createTreeVisualization(const std::vector<std::vector<int>>& tree) const
{
    return new TreeVisualization(tree);
}

void TreeVisualizerBase::addTreeVisualization(const TreeVisualization *treeVisualization)
{
    auto sourceAndDestination = std::pair<int, int>(treeVisualization->paths.front().moduleIds.front(), treeVisualization->paths.back().moduleIds.back());
    treeVisualizations.insert(std::pair<std::pair<int, int>, const TreeVisualization *>(sourceAndDestination, treeVisualization));
}

void TreeVisualizerBase::removeTreeVisualization(const TreeVisualization *treeVisualization)
{
    auto sourceAndDestination = std::pair<int, int>(treeVisualization->paths.front().moduleIds.front(), treeVisualization->paths.back().moduleIds.back());
    auto range = treeVisualizations.equal_range(sourceAndDestination);
    for (auto it = range.first; it != range.second; it++) {
        if (it->second == treeVisualization) {
            treeVisualizations.erase(it);
            break;
        }
    }
}

void TreeVisualizerBase::removeAllTreeVisualizations()
{
    numTrees.clear();
    std::vector<const TreeVisualization *> removedTreeVisualizations;
    for (auto it : treeVisualizations)
        removedTreeVisualizations.push_back(it.second);
    for (auto it : removedTreeVisualizations) {
        removeTreeVisualization(it);
        delete it;
    }
}

} // namespace visualizer

} // namespace inet

