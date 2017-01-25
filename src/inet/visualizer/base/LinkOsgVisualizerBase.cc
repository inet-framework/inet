//
// Copyright (C) 2016 OpenSim Ltd.
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

LinkOsgVisualizerBase::OsgLink::OsgLink(osg::Node *node, int sourceModuleId, int destinationModuleId) :
    Link(sourceModuleId, destinationModuleId),
    node(node)
{
}

LinkOsgVisualizerBase::OsgLink::~OsgLink()
{
    // TODO: delete node;
}

void LinkOsgVisualizerBase::addLink(std::pair<int, int> sourceAndDestination, const Link *link)
{
    LinkVisualizerBase::addLink(sourceAndDestination, link);
    auto osgLink = static_cast<const OsgLink *>(link);
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizerTargetModule);
    scene->addChild(osgLink->node);
}

void LinkOsgVisualizerBase::removeLink(const Link *link)
{
    LinkVisualizerBase::removeLink(link);
    auto osgLink = static_cast<const OsgLink *>(link);
    auto node = osgLink->node;
    node->getParent(0)->removeChild(node);
}

const LinkVisualizerBase::Link *LinkOsgVisualizerBase::createLink(cModule *source, cModule *destination) const
{
    auto sourcePosition = getPosition(source);
    auto destinationPosition = getPosition(destination);
    auto node = inet::osg::createLine(sourcePosition, destinationPosition, cFigure::ARROW_NONE, cFigure::ARROW_BARBED);
    auto stateSet = inet::osg::createStateSet(lineColor, 1.0, false);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    auto lineWidth = new osg::LineWidth();
    lineWidth->setWidth(this->lineWidth);
    stateSet->setAttributeAndModes(lineWidth, osg::StateAttribute::ON);
    node->setStateSet(stateSet);
    return new OsgLink(node, source->getId(), destination->getId());
}

void LinkOsgVisualizerBase::setAlpha(const Link *link, double alpha) const
{
    auto osgLink = static_cast<const OsgLink *>(link);
    auto node = osgLink->node;
    auto material = static_cast<osg::Material *>(node->getOrCreateStateSet()->getAttribute(osg::StateAttribute::MATERIAL));
    material->setAlpha(osg::Material::FRONT_AND_BACK, alpha);
}

void LinkOsgVisualizerBase::setPosition(cModule *node, const Coord& position) const
{
    for (auto it : links) {
        auto link = static_cast<const OsgLink *>(it.second);
        auto group = static_cast<osg::Group *>(link->node);
        auto geode = static_cast<osg::Geode *>(group->getChild(0));
        auto geometry = static_cast<osg::Geometry *>(geode->getDrawable(0));
        auto vertices = static_cast<osg::Vec3Array *>(geometry->getVertexArray());
        if (node->getId() == it.first.first)
            vertices->at(0) = osg::Vec3d(position.x, position.y, position.z);
        else if (node->getId() == it.first.second) {
            osg::Vec3d p(position.x, position.y, position.z);
            vertices->at(1) = p;
            auto autoTransform = static_cast<osg::AutoTransform *>(group->getChild(1));
            if (autoTransform != nullptr)
                autoTransform->setPosition(p);
        }
        geometry->dirtyBound();
        geometry->dirtyDisplayList();
    }
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

