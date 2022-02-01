//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LINKOSGVISUALIZERBASE_H
#define __INET_LINKOSGVISUALIZERBASE_H

#include <osg/ref_ptr>

#include "inet/visualizer/base/LinkVisualizerBase.h"
#include "inet/visualizer/osg/util/OsgUtils.h"

namespace inet {

namespace visualizer {

class INET_API LinkOsgVisualizerBase : public LinkVisualizerBase
{
  protected:
    class INET_API LinkOsgVisualization : public LinkVisualization {
      public:
        osg::ref_ptr<inet::osg::LineNode> node;

      public:
        LinkOsgVisualization(inet::osg::LineNode *node, int sourceModuleId, int destinationModuleId);
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

