//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LINKVSGVISUALIZERBASE_H
#define __INET_LINKVSGVISUALIZERBASE_H

#include <vsg/core/ref_ptr.h>

#include "inet/visualizer/base/LinkVisualizerBase.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"   // inet::vsg::LineNode

namespace inet {

namespace visualizer {

class INET_API LinkVsgVisualizerBase : public LinkVisualizerBase
{
  protected:
    class INET_API LinkVsgVisualization : public LinkVisualization {
      public:
        ::vsg::ref_ptr<inet::vsg::LineNode> node;

      public:
        LinkVsgVisualization(inet::vsg::LineNode *node, int sourceModuleId, int destinationModuleId);
    };

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual const LinkVisualization *createLinkVisualization(cModule *source, cModule *destination, cPacket *packet) const override;
    virtual void addLinkVisualization(std::pair<int, int> sourceAndDestination, const LinkVisualization *linkVisualization) override;
    virtual void removeLinkVisualization(const LinkVisualization *linkVisualization) override;
    virtual void setAlpha(const LinkVisualization *linkVisualization, double alpha) const override;
};

} // namespace visualizer

} // namespace inet

#endif
