//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETDROPVSGVISUALIZER_H
#define __INET_PACKETDROPVSGVISUALIZER_H

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>

#include "inet/visualizer/base/PacketDropVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PacketDropVsgVisualizer : public PacketDropVisualizerBase
{
  protected:
    class INET_API PacketDropVsgVisualization : public PacketDropVisualization {
      public:
        // Root transform that positions the marker in world space.
        // The child group holds the coloured sphere (the actual marker).
        ::vsg::ref_ptr<::vsg::MatrixTransform> node;

      public:
        PacketDropVsgVisualization(::vsg::ref_ptr<::vsg::MatrixTransform> node, const PacketDrop *packetDrop);
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
