//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCENEVSGVISUALIZER_H
#define __INET_SCENEVSGVISUALIZER_H

#include "inet/visualizer/vsg/base/SceneVsgVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API SceneVsgVisualizer : public SceneVsgVisualizerBase
{
  protected:
    virtual void initialize(int stage) override;
    virtual void initializeScene() override;
    virtual void initializeViewpoint();
};

} // namespace visualizer

} // namespace inet

#endif
