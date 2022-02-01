//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NETWORKROUTECANVASVISUALIZER_H
#define __INET_NETWORKROUTECANVASVISUALIZER_H

#include "inet/visualizer/canvas/base/PathCanvasVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API NetworkRouteCanvasVisualizer : public PathCanvasVisualizerBase
{
  protected:
    virtual bool isPathStart(cModule *module) const override;
    virtual bool isPathEnd(cModule *module) const override;
    virtual bool isPathElement(cModule *module) const override;

    virtual const PathVisualization *createPathVisualization(const char *label, const std::vector<int>& path, cPacket *packet) const override;
};

} // namespace visualizer

} // namespace inet

#endif

