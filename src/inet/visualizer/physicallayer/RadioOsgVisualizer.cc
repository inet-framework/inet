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
#include "inet/visualizer/physicallayer/RadioOsgVisualizer.h"

namespace inet {

namespace visualizer {

using namespace inet::physicallayer;

Define_Module(RadioOsgVisualizer);

#ifdef WITH_OSG

RadioOsgVisualizer::RadioOsgVisualization::RadioOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Geode *node, const int radioModuleId) :
    RadioVisualization(radioModuleId),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

void RadioOsgVisualizer::initialize(int stage)
{
    RadioVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        networkNodeVisualizer = getModuleFromPar<NetworkNodeOsgVisualizer>(par("networkNodeVisualizerModule"), this);
    }
    else if (stage == INITSTAGE_LAST) {
        for (auto it : radioVisualizations) {
            auto radioOsgVisualization = static_cast<const RadioOsgVisualization *>(it.second);
            auto node = radioOsgVisualization->node;
            radioOsgVisualization->networkNodeVisualization->addAnnotation(node, osg::Vec3d(0, 0, 0), 0);
        }
    }
}

RadioVisualizerBase::RadioVisualization *RadioOsgVisualizer::createRadioVisualization(const IRadio *radio) const
{
    auto module = check_and_cast<const cModule *>(radio);
    auto geode = new osg::Geode();
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    auto networkNode = getContainingNode(module);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    if (networkNodeVisualization == nullptr)
        throw cRuntimeError("Cannot create radio visualization for '%s', because network node visualization is not found for '%s'", module->getFullPath().c_str(), networkNode->getFullPath().c_str());
    return new RadioOsgVisualization(networkNodeVisualization, geode, module->getId());
}

void RadioOsgVisualizer::refreshRadioVisualization(const RadioVisualization *radioVisualization) const
{
    // TODO:
    // auto infoOsgVisualization = static_cast<const RadioOsgVisualization *>(radioVisualization);
    // auto node = infoOsgVisualization->node;
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

