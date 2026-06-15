//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PATHVSGVISUALIZERBASE_H
#define __INET_PATHVSGVISUALIZERBASE_H

#include <vector>

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

#include "inet/visualizer/base/PathVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PathVsgVisualizerBase : public PathVisualizerBase
{
  protected:
    class INET_API PathVsgVisualization : public PathVisualization {
      public:
        ::vsg::ref_ptr<::vsg::Group> node;   // container holding the current polyline (rebuilt to fade)
        std::vector<Coord> points;           // path point positions (cached so setAlpha can rebuild)
        cFigure::Color color;

      public:
        PathVsgVisualization(const char *label, const std::vector<int>& path, ::vsg::ref_ptr<::vsg::Group> node, const std::vector<Coord>& points, const cFigure::Color& color);
    };

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual const PathVisualization *createPathVisualization(const char *label, const std::vector<int>& path, cPacket *packet) const override;
    virtual void addPathVisualization(const PathVisualization *pathVisualization) override;
    virtual void removePathVisualization(const PathVisualization *pathVisualization) override;
    virtual void setAlpha(const PathVisualization *pathVisualization, double alpha) const override;
};

} // namespace visualizer

} // namespace inet

#endif
