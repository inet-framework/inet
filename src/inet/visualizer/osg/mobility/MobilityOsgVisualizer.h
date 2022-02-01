//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MOBILITYOSGVISUALIZER_H
#define __INET_MOBILITYOSGVISUALIZER_H

#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/base/MobilityVisualizerBase.h"
#include "inet/visualizer/osg/scene/NetworkNodeOsgVisualization.h"
#include "inet/visualizer/osg/scene/NetworkNodeOsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API MobilityOsgVisualizer : public MobilityVisualizerBase
{
  protected:
    class INET_API MobilityOsgVisualization : public MobilityVisualization {
      public:
        osg::Geode *trail;

      public:
        MobilityOsgVisualization(osg::Geode *trail, IMobility *mobility);
    };

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual MobilityVisualization *createMobilityVisualization(IMobility *mobility) override;
    virtual void addMobilityVisualization(const IMobility *mobility, MobilityVisualization *mobilityVisualization) override;
    virtual void removeMobilityVisualization(const MobilityVisualization *mobilityVisualization) override;
    virtual void extendMovementTrail(osg::Geode *trail, const Coord& position) const;
};

} // namespace visualizer

} // namespace inet

#endif

