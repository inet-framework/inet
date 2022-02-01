//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCENEOSGVISUALIZERBASE_H
#define __INET_SCENEOSGVISUALIZERBASE_H

#include <osg/Geode>
#include <osg/Group>

#include "inet/visualizer/base/SceneVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API SceneOsgVisualizerBase : public SceneVisualizerBase
{
  protected:
    virtual void initializeScene();
    virtual void initializeAxis(double axisLength);
    virtual void initializeSceneFloor();
    virtual osg::Geode *createSceneFloor(const Coord& min, const Coord& max, cFigure::Color& color, osg::Image *image, double imageSize, double opacity, bool shading) const;
    virtual osg::BoundingSphere getNetworkBoundingSphere();
};

} // namespace visualizer

} // namespace inet

#endif

