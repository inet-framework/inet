//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LINKBREAKVSGVISUALIZER_H
#define __INET_LINKBREAKVSGVISUALIZER_H

#include <algorithm>

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>

#include "inet/visualizer/base/LinkBreakVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API LinkBreakVsgVisualizer : public LinkBreakVisualizerBase
{
  protected:
    class INET_API LinkBreakVsgVisualization : public LinkBreakVisualization {
      public:
        // Root transform that positions the marker at the link midpoint in world space.
        // The marker group (child) holds the sphere (and optional label).
        ::vsg::ref_ptr<::vsg::MatrixTransform> node;

      public:
        LinkBreakVsgVisualization(::vsg::ref_ptr<::vsg::MatrixTransform> node, int transmitterModuleId, int receiverModuleId);
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
