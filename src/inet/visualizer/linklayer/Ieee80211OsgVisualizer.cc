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
#include "inet/common/OsgScene.h"
#include "inet/common/OsgUtils.h"
#include "inet/visualizer/linklayer/Ieee80211OsgVisualizer.h"

#ifdef WITH_OSG
#include <osg/Geode>
#endif // ifdef WITH_OSG

namespace inet {

namespace visualizer {

Define_Module(Ieee80211OsgVisualizer);

#ifdef WITH_OSG

Ieee80211OsgVisualizer::Ieee80211OsgVisualization::Ieee80211OsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Node *node, int networkNodeId, int interfaceId) :
    Ieee80211Visualization(networkNodeId, interfaceId),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

Ieee80211OsgVisualizer::~Ieee80211OsgVisualizer()
{
    if (displayAssociations)
        removeAllIeee80211Visualizations();
}

void Ieee80211OsgVisualizer::initialize(int stage)
{
    Ieee80211VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        networkNodeVisualizer = getModuleFromPar<NetworkNodeOsgVisualizer>(par("networkNodeVisualizerModule"), this);
    }
}

Ieee80211VisualizerBase::Ieee80211Visualization *Ieee80211OsgVisualizer::createIeee80211Visualization(cModule *networkNode, InterfaceEntry *interfaceEntry, std::string ssid, W power)
{
    auto path = resolveResourcePath((getIcon(power) + ".png").c_str());
    auto image = inet::osg::createImage(path.c_str());
    auto texture = new osg::Texture2D();
    texture->setImage(image);
    auto geometry = osg::createTexturedQuadGeometry(osg::Vec3(-image->s() / 2, 0.0, 0.0), osg::Vec3(image->s(), 0.0, 0.0), osg::Vec3(0.0, image->t(), 0.0), 0.0, 0.0, 1.0, 1.0);
    auto stateSet = geometry->getOrCreateStateSet();
    stateSet->setTextureAttributeAndModes(0, texture);
    stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    std::hash<std::string> hasher;
    auto color = iconColorSet.getColor(hasher(ssid));
    auto colorArray = new osg::Vec4Array();
    colorArray->push_back(osg::Vec4((double)color.red / 255.0, (double)color.green / 255.0, (double)color.blue / 255.0, 1));
    geometry->setColorArray(colorArray, osg::Array::BIND_OVERALL);
    auto geode = new osg::Geode();
    geode->addDrawable(geometry);
    // TODO: apply tinting
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    if (networkNodeVisualization == nullptr)
        throw cRuntimeError("Cannot create IEEE 802.11 visualization for '%s', because network node visualization is not found for '%s'", interfaceEntry->getInterfaceName(), networkNode->getFullPath().c_str());
    return new Ieee80211OsgVisualization(networkNodeVisualization, geode, networkNode->getId(), interfaceEntry->getInterfaceId());
}

void Ieee80211OsgVisualizer::addIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization)
{
    Ieee80211VisualizerBase::addIeee80211Visualization(ieee80211Visualization);
    auto ieee80211OsgVisualization = static_cast<const Ieee80211OsgVisualization *>(ieee80211Visualization);
    ieee80211OsgVisualization->networkNodeVisualization->addAnnotation(ieee80211OsgVisualization->node, osg::Vec3d(0, 0, 32), 0);
}

void Ieee80211OsgVisualizer::removeIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization)
{
    Ieee80211VisualizerBase::removeIeee80211Visualization(ieee80211Visualization);
    auto ieee80211OsgVisualization = static_cast<const Ieee80211OsgVisualization *>(ieee80211Visualization);
    ieee80211OsgVisualization->networkNodeVisualization->removeAnnotation(ieee80211OsgVisualization->node);
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

