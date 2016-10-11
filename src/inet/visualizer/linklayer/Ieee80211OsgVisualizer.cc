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

#include "inet/visualizer/linklayer/Ieee80211OsgVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(Ieee80211OsgVisualizer);

Ieee80211OsgVisualizer::Ieee80211OsgVisualization::Ieee80211OsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Node *node, int networkNodeId, int interfaceId) :
    Ieee80211Visualization(networkNodeId, interfaceId),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

Ieee80211VisualizerBase::Ieee80211Visualization *Ieee80211OsgVisualizer::createIeee80211Visualization(cModule *networkNode, InterfaceEntry *interfaceEntry)
{
    return new Ieee80211OsgVisualization(nullptr, nullptr, networkNode->getId(), interfaceEntry->getInterfaceId());
}

} // namespace visualizer

} // namespace inet

