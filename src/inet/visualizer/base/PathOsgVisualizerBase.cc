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
#include "inet/visualizer/base/PathOsgVisualizerBase.h"

#ifdef WITH_OSG
#include <osg/LineWidth>
#endif // ifdef WITH_OSG

namespace inet {

namespace visualizer {

#ifdef WITH_OSG

PathOsgVisualizerBase::OsgPath::OsgPath(const std::vector<int>& path, osg::Node *node) :
    Path(path),
    node(node)
{
}

PathOsgVisualizerBase::OsgPath::~OsgPath()
{
    // TODO: delete node;
}

void PathOsgVisualizerBase::addPath(std::pair<int, int> sourceAndDestination, const Path *path)
{
    PathVisualizerBase::addPath(sourceAndDestination, path);
    auto osgPath = static_cast<const OsgPath *>(path);
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizerTargetModule);
    scene->addChild(osgPath->node);
}

void PathOsgVisualizerBase::removePath(std::pair<int, int> sourceAndDestination, const Path *path)
{
    PathVisualizerBase::removePath(sourceAndDestination, path);
    auto osgPath = static_cast<const OsgPath *>(path);
    auto node = osgPath->node;
    node->getParent(0)->removeChild(node);
}

const PathVisualizerBase::Path *PathOsgVisualizerBase::createPath(const std::vector<int>& path) const
{
    std::vector<Coord> points;
    for (auto id : path) {
        auto node = getSimulation()->getModule(id);
        points.push_back(getPosition(node));
    }
    auto node = inet::osg::createPolyline(points, cFigure::ARROW_NONE, cFigure::ARROW_BARBED);
    auto color = cFigure::GOOD_DARK_COLORS[paths.size() % (sizeof(cFigure::GOOD_DARK_COLORS) / sizeof(cFigure::Color))];
    auto stateSet = inet::osg::createStateSet(color, 1, false);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    auto lineWidth = new osg::LineWidth();
    lineWidth->setWidth(this->lineWidth);
    stateSet->setAttributeAndModes(lineWidth, osg::StateAttribute::ON);
    node->setStateSet(stateSet);
    return new OsgPath(path, node);
}

void PathOsgVisualizerBase::setAlpha(const Path *path, double alpha) const
{
    auto osgPath = static_cast<const OsgPath *>(path);
    auto node = osgPath->node;
    auto stateSet = node->getOrCreateStateSet();
    auto material = static_cast<osg::Material *>(stateSet->getAttribute(osg::StateAttribute::MATERIAL));
    material->setAlpha(osg::Material::FRONT_AND_BACK, alpha);
}

void PathOsgVisualizerBase::setPosition(cModule *node, const Coord& position) const
{
    for (auto it : paths) {
        auto path = static_cast<const OsgPath *>(it.second);
        auto group = static_cast<osg::Group *>(path->node);
        auto geode = static_cast<osg::Geode *>(group->getChild(0));
        auto geometry = static_cast<osg::Geometry *>(geode->getDrawable(0));
        auto vertices = static_cast<osg::Vec3Array *>(geometry->getVertexArray());
        auto autoTransform = dynamic_cast<osg::AutoTransform *>(group->getChild(1));
        for (int i = 0; i < path->moduleIds.size(); i++) {
            if (node->getId() == path->moduleIds[i]) {
                osg::Vec3d p(position.x, position.y, position.z + path->offset);
                vertices->at(i) = p;
                if (autoTransform != nullptr && i == path->moduleIds.size() - 1)
                    autoTransform->setPosition(p);
            }
        }
        geometry->dirtyBound();
        geometry->dirtyDisplayList();
    }
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

