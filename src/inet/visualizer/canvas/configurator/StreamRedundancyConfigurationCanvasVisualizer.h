//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STREAMREDUNDANCYCONFIGURATIONCANVASVISUALIZER_H
#define __INET_STREAMREDUNDANCYCONFIGURATIONCANVASVISUALIZER_H

#include "inet/visualizer/canvas/base/TreeCanvasVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API StreamRedundancyConfigurationCanvasVisualizer : public TreeCanvasVisualizerBase
{
  protected:
    cMatchExpression streamFilter;

  protected:
    virtual void initialize(int stage) override;
    virtual const TreeVisualization *createTreeVisualization(const std::vector<std::vector<int>>& tree) const override;
};

} // namespace visualizer

} // namespace inet

#endif

