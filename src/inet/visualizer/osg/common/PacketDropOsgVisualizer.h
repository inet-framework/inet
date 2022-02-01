//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETDROPOSGVISUALIZER_H
#define __INET_PACKETDROPOSGVISUALIZER_H

#include <osg/ref_ptr>

#include "inet/visualizer/base/PacketDropVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PacketDropOsgVisualizer : public PacketDropVisualizerBase
{
  protected:
    class INET_API PacketDropOsgVisualization : public PacketDropVisualization {
      public:
        osg::ref_ptr<osg::Node> node;

      public:
        PacketDropOsgVisualization(osg::Node *node, const PacketDrop *packetDrop);
    };

  protected:
    virtual void refreshDisplay() const override;

    virtual const PacketDropVisualization *createPacketDropVisualization(PacketDrop *packetDrop) const override;
    virtual void addPacketDropVisualization(const PacketDropVisualization *packetDropVisualization) override;
    virtual void removePacketDropVisualization(const PacketDropVisualization *packetDropVisualization) override;
    virtual void setAlpha(const PacketDropVisualization *packetDropVisualization, double alpha) const override;
};

} // namespace visualizer

} // namespace inet

#endif

