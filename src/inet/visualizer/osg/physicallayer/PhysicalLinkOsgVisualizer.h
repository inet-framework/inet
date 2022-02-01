//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PHYSICALLINKOSGVISUALIZER_H
#define __INET_PHYSICALLINKOSGVISUALIZER_H

#include "inet/visualizer/osg/base/LinkOsgVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PhysicalLinkOsgVisualizer : public LinkOsgVisualizerBase
{
  protected:
    virtual bool isLinkStart(cModule *module) const override;
    virtual bool isLinkEnd(cModule *module) const override;
};

} // namespace visualizer

} // namespace inet

#endif

