//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PATHOSGVISUALIZERBASE_H
#define __INET_PATHOSGVISUALIZERBASE_H

#include <osg/ref_ptr>
#include <osg/Node>

#include "inet/visualizer/base/PathVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PathOsgVisualizerBase : public PathVisualizerBase
{
  protected:
    class INET_API PathOsgVisualization : public PathVisualization {
      public:
        osg::ref_ptr<osg::Node> node;

      public:
        PathOsgVisualization(const char *label, const std::vector<int>& path, osg::Node *node);
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

