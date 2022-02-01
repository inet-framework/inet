//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCENEVISUALIZERBASE_H
#define __INET_SCENEVISUALIZERBASE_H

#include "inet/common/geometry/object/Box.h"
#include "inet/visualizer/base/VisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API SceneVisualizerBase : public VisualizerBase
{
  protected:
    Coord sceneMin;
    Coord sceneMax;

  protected:
    virtual void initialize(int stage) override;
    virtual Box getSceneBounds();
};

} // namespace visualizer

} // namespace inet

#endif

