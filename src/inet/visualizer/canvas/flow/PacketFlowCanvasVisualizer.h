//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETFLOWCANVASVISUALIZER_H
#define __INET_PACKETFLOWCANVASVISUALIZER_H

#include "inet/visualizer/canvas/base/PathCanvasVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PacketFlowCanvasVisualizer : public PathCanvasVisualizerBase
{
  protected:
    virtual bool isPathStart(cModule *module) const override;
    virtual bool isPathEnd(cModule *module) const override;
    virtual bool isPathElement(cModule *module) const override;

    virtual const PathVisualization *createPathVisualization(const char *label, const std::vector<int>& path, cPacket *packet) const override;

    virtual void processPathElement(cModule *networkNode, const char *label, Packet *packet) override;
};

} // namespace visualizer

} // namespace inet

#endif

