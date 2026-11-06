//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCENEVSGVISUALIZERBASE_H
#define __INET_SCENEVSGVISUALIZERBASE_H

#include <vsg/nodes/Group.h>
#include <vsg/nodes/Node.h>

#include "inet/visualizer/base/SceneVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API SceneVsgVisualizerBase : public SceneVisualizerBase
{
  protected:
    // Keeps the scene alive: cEnvir::refVsgObject() is a no-op outside Qtenv, so the cVsgCanvas alone does not own it there.
    ::vsg::ref_ptr<::vsg::Group> topLevelSceneHolder;

    virtual void initializeScene();
    virtual void initializeAxis(double axisLength);
    virtual void initializeSceneFloor();
    virtual ::vsg::ref_ptr<::vsg::Node> createSceneFloor(const Coord& min, const Coord& max, const cFigure::Color& color, double opacity) const;
    // (center, radius) bounding sphere of the network nodes, used to frame the initial viewpoint
    virtual std::pair<Coord, double> getNetworkBoundingSphere();
};

} // namespace visualizer

} // namespace inet

#endif
