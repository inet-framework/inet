//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PHYSICALLINKCANVASVISUALIZER_H
#define __INET_PHYSICALLINKCANVASVISUALIZER_H

#include "inet/visualizer/canvas/base/LinkCanvasVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PhysicalLinkCanvasVisualizer : public LinkCanvasVisualizerBase
{
  protected:
    virtual bool isLinkStart(cModule *module) const override;
    virtual bool isLinkEnd(cModule *module) const override;

    virtual const LinkVisualization *createLinkVisualization(cModule *source, cModule *destination, cPacket *packet) const override;
};

} // namespace visualizer

} // namespace inet

#endif

