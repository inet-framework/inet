//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TRANSPORTROUTEVSGVISUALIZER_H
#define __INET_TRANSPORTROUTEVSGVISUALIZER_H

#include "inet/visualizer/vsg/base/PathVsgVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API TransportRouteVsgVisualizer : public PathVsgVisualizerBase
{
  protected:
    virtual bool isPathStart(cModule *module) const override;
    virtual bool isPathEnd(cModule *module) const override;
    virtual bool isPathElement(cModule *module) const override;
};

} // namespace visualizer

} // namespace inet

#endif
