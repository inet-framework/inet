//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DATALINKVSGVISUALIZER_H
#define __INET_DATALINKVSGVISUALIZER_H

#include "inet/visualizer/vsg/base/LinkVsgVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API DataLinkVsgVisualizer : public LinkVsgVisualizerBase
{
  protected:
    virtual bool isLinkStart(cModule *module) const override;
    virtual bool isLinkEnd(cModule *module) const override;
};

} // namespace visualizer

} // namespace inet

#endif
