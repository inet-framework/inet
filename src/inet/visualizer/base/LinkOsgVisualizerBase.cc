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

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/OSGScene.h"
#include "inet/common/OSGUtils.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/base/LinkOsgVisualizerBase.h"

#ifdef WITH_OSG
#include <osg/Geode>
#include <osg/LineWidth>
#endif // ifdef WITH_OSG

namespace inet {

namespace visualizer {

#ifdef WITH_OSG

LinkOsgVisualizerBase::LinkOsgVisualization::LinkOsgVisualization(osg::Node *node, int sourceModuleId, int destinationModuleId) :
    LinkVisualization(sourceModuleId, destinationModuleId),
    node(node)
{
}

LinkOsgVisualizerBase::LinkOsgVisualization::~LinkOsgVisualization()
{
    // TODO: delete node;
}


void LinkOsgVisualizerBase::refreshDisplay() const
{
    LinkVisualizerBase::refreshDisplay();
    // TODO: switch to osg canvas when API is extended
    visualizerTargetModule->getCanvas()->setAnimationSpeed(linkVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const LinkVisualizerBase::LinkVisualization *LinkOsgVisualizerBase::createLinkVisualization(cModule *source, cModule *destination, cPacket *packet) const
{
    auto sourcePosition = getPosition(source);
    auto destinationPosition = getPosition(destination);
    auto node = inet::osg::createLine(sourcePosition, destinationPosition, cFigure::ARROW_NONE, cFigure::ARROW_BARBED);
    node->setStateSet(inet::osg::createLineStateSet(lineColor, lineStyle, lineWidth));
    return new LinkOsgVisualization(node, source->getId(), destination->getId());
}

void LinkOsgVisualizerBase::addLinkVisualization(std::pair<int, int> sourceAndDestination, const LinkVisualization *link)
{
    LinkVisualizerBase::addLinkVisualization(sourceAndDestination, link);
    auto linkOsgVisualization = static_cast<const LinkOsgVisualization *>(link);
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizerTargetModule);
    scene->addChild(linkOsgVisualization->node);
}

void LinkOsgVisualizerBase::removeLinkVisualization(const LinkVisualization *link)
{
    LinkVisualizerBase::removeLinkVisualization(link);
    auto linkOsgVisualization = static_cast<const LinkOsgVisualization *>(link);
    auto node = linkOsgVisualization->node;
    node->getParent(0)->removeChild(node);
}

void LinkOsgVisualizerBase::setAlpha(const LinkVisualization *link, double alpha) const
{
    auto linkOsgVisualization = static_cast<const LinkOsgVisualization *>(link);
    auto node = linkOsgVisualization->node;
    auto material = static_cast<osg::Material *>(node->getOrCreateStateSet()->getAttribute(osg::StateAttribute::MATERIAL));
    material->setAlpha(osg::Material::FRONT_AND_BACK, alpha);
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

