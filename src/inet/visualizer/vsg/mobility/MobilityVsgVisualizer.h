//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MOBILITYVSGVISUALIZER_H
#define __INET_MOBILITYVSGVISUALIZER_H

#include <vector>

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/base/MobilityVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API MobilityVsgVisualizer : public MobilityVisualizerBase
{
  protected:
    class INET_API MobilityVsgVisualization : public MobilityVisualization {
      public:
        ::vsg::ref_ptr<::vsg::Group> trail; // container; holds the current movement-trail polyline
        std::vector<Coord> trailPoints;
        cFigure::Color color;

      public:
        MobilityVsgVisualization(::vsg::ref_ptr<::vsg::Group> trail, IMobility *mobility, const cFigure::Color& color);
    };

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual MobilityVisualization *createMobilityVisualization(IMobility *mobility) override;
    virtual void addMobilityVisualization(const IMobility *mobility, MobilityVisualization *mobilityVisualization) override;
    virtual void removeMobilityVisualization(const MobilityVisualization *mobilityVisualization) override;
    virtual void extendMovementTrail(MobilityVsgVisualization *visualization, const Coord& position) const;
};

} // namespace visualizer

} // namespace inet

#endif
