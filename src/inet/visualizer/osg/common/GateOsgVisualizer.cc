//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#include "inet/visualizer/osg/common/GateOsgVisualizer.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

Define_Module(GateOsgVisualizer);

GateOsgVisualizer::GateOsgVisualization::GateOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Geode *node, queueing::IPacketGate *gate) :
    GateVisualization(gate),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

void GateOsgVisualizer::initialize(int stage)
{
    GateVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    }
    else if (stage == INITSTAGE_LAST) {
        for (auto gateVisualization : gateVisualizations) {
            auto gateOsgVisualization = static_cast<const GateOsgVisualization *>(gateVisualization);
            auto node = gateOsgVisualization->node;
            gateOsgVisualization->networkNodeVisualization->addAnnotation(node, osg::Vec3d(0, 0, 0), 0);
        }
    }
}

GateVisualizerBase::GateVisualization *GateOsgVisualizer::createGateVisualization(queueing::IPacketGate *gate) const
{
    auto ownedObject = check_and_cast<cOwnedObject *>(gate);
    auto module = check_and_cast<cModule *>(ownedObject->getOwner());
    auto geode = new osg::Geode();
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    auto networkNode = getContainingNode(module);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new GateOsgVisualization(networkNodeVisualization, geode, gate);
}

void GateOsgVisualizer::refreshGateVisualization(const GateVisualization *gateVisualization) const
{
    // TODO
//    auto infoOsgVisualization = static_cast<const GateOsgVisualization *>(gateVisualization);
//    auto node = infoOsgVisualization->node;
}

} // namespace visualizer

} // namespace inet

