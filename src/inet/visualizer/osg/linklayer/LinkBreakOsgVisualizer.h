//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LINKBREAKOSGVISUALIZER_H
#define __INET_LINKBREAKOSGVISUALIZER_H

#include <osg/ref_ptr>

#include "inet/visualizer/base/LinkBreakVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API LinkBreakOsgVisualizer : public LinkBreakVisualizerBase
{
  protected:
    class INET_API LinkBreakOsgVisualization : public LinkBreakVisualization {
      public:
        osg::ref_ptr<osg::Node> node;

      public:
        LinkBreakOsgVisualization(osg::Node *node, int transmitterModuleId, int receiverModuleId);
    };

  protected:
    virtual void refreshDisplay() const override;

    virtual const LinkBreakVisualization *createLinkBreakVisualization(cModule *transmitter, cModule *receiver) const override;
    virtual void addLinkBreakVisualization(const LinkBreakVisualization *linkBreakVisualization) override;
    virtual void removeLinkBreakVisualization(const LinkBreakVisualization *linkBreakVisualization) override;
    virtual void setAlpha(const LinkBreakVisualization *linkBreakVisualization, double alpha) const override;
};

} // namespace visualizer

} // namespace inet

#endif

