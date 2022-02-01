//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCENEOSGVISUALIZER_H
#define __INET_SCENEOSGVISUALIZER_H

#include <osg/Geode>

#include "inet/visualizer/osg/base/SceneOsgVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API SceneOsgVisualizer : public SceneOsgVisualizerBase
{
  protected:
    virtual void initialize(int stage) override;
    virtual void initializeScene() override;
    virtual void initializeViewpoint();
};

} // namespace visualizer

} // namespace inet

#endif

