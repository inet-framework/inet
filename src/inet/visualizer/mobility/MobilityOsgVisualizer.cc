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

#include "inet/visualizer/mobility/MobilityOsgVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/OsgScene.h"
#include "inet/common/OsgUtils.h"

#ifdef WITH_OSG
#include <osg/AutoTransform>
#include <osg/Material>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgText/Text>
#endif // ifdef WITH_OSG

namespace inet {

namespace visualizer {

Define_Module(MobilityOsgVisualizer);

#ifdef WITH_OSG

MobilityOsgVisualizer::MobilityOsgVisualization::MobilityOsgVisualization(osg::Geode *trail, IMobility *mobility) :
    MobilityVisualization(mobility),
    trail(trail)
{
}

void MobilityOsgVisualizer::initialize(int stage)
{
    MobilityVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
}

void MobilityOsgVisualizer::refreshDisplay() const
{
    MobilityVisualizerBase::refreshDisplay();
    for (auto it : mobilityVisualizations) {
        auto mobilityVisualization = static_cast<MobilityOsgVisualization *>(it.second);
        auto mobility = mobilityVisualization->mobility;
        auto position = mobility->getCurrentPosition();
        if (displayMovementTrails)
            extendMovementTrail(mobilityVisualization->trail, position);
    }
    // TODO switch to osg Osg when API is extended
    visualizationTargetModule->getCanvas()->setAnimationSpeed(mobilityVisualizations.empty() ? 0 : animationSpeed, this);
}

MobilityOsgVisualizer::MobilityVisualization *MobilityOsgVisualizer::createMobilityVisualization(IMobility *mobility)
{
    auto module = const_cast<cModule *>(check_and_cast<const cModule *>(mobility));
    auto trail = new osg::Geode();
    trail->setStateSet(inet::osg::createStateSet(movementTrailLineColorSet.getColor(module->getId()), 1.0));
    trail->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    return new MobilityOsgVisualization(trail, mobility);
}

void MobilityOsgVisualizer::addMobilityVisualization(const IMobility *mobility, MobilityVisualization *mobilityVisualization)
{
    MobilityVisualizerBase::addMobilityVisualization(mobility, mobilityVisualization);
    auto mobilityOsgVisualization = static_cast<MobilityOsgVisualization *>(mobilityVisualization);
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    if (displayMovementTrails)
        scene->addChild(mobilityOsgVisualization->trail);
}

void MobilityOsgVisualizer::removeMobilityVisualization(const MobilityVisualization *mobilityVisualization)
{
    auto mobilityOsgVisualization = static_cast<const MobilityOsgVisualization *>(mobilityVisualization);
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    if (displayMovementTrails)
        scene->removeChild(mobilityOsgVisualization->trail);
    MobilityVisualizerBase::removeMobilityVisualization(mobilityVisualization);
}

void MobilityOsgVisualizer::extendMovementTrail(osg::Geode *trail, const Coord& position) const
{
    if (trail->getNumDrawables() == 0)
        trail->addDrawable(inet::osg::createLineGeometry(position, position));
    else {
        auto drawable = static_cast<osg::Geometry *>(trail->getDrawable(0));
        auto vertices = static_cast<osg::Vec3Array *>(drawable->getVertexArray());
        auto lastPosition = vertices->at(vertices->size() - 1);
        auto dx = lastPosition.x() - position.x;
        auto dy = lastPosition.y() - position.y;
        auto dz = lastPosition.z() - position.z;
        // TODO 1?
        if (dx * dx + dy * dy + dz * dz > 1) {
            vertices->push_back(osg::Vec3d(position.x, position.y, position.z));
            if ((int)vertices->size() > trailLength)
                vertices->erase(vertices->begin(), vertices->begin() + 1);
            auto drawArrays = static_cast<osg::DrawArrays *>(drawable->getPrimitiveSet(0));
            drawArrays->setFirst(0);
            drawArrays->setCount(vertices->size());
            drawable->dirtyBound();
            drawable->dirtyDisplayList();
        }
    }
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

