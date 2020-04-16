//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_PACKETDROPOSGVISUALIZER_H
#define __INET_PACKETDROPOSGVISUALIZER_H

#include "inet/visualizer/base/PacketDropVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PacketDropOsgVisualizer : public PacketDropVisualizerBase
{
#ifdef WITH_OSG

  protected:
    class INET_API PacketDropOsgVisualization : public PacketDropVisualization
    {
      public:
        osg::Node *node = nullptr;

      public:
        PacketDropOsgVisualization(osg::Node* node, const PacketDrop* packetDrop);
        virtual ~PacketDropOsgVisualization();
    };

  protected:
    virtual void refreshDisplay() const override;

    virtual const PacketDropVisualization *createPacketDropVisualization(PacketDrop *packetDrop) const override;
    virtual void addPacketDropVisualization(const PacketDropVisualization *packetDropVisualization) override;
    virtual void removePacketDropVisualization(const PacketDropVisualization *packetDropVisualization) override;
    virtual void setAlpha(const PacketDropVisualization *packetDropVisualization, double alpha) const override;

  public:
    virtual ~PacketDropOsgVisualizer();

#else // ifdef WITH_OSG

  protected:
    virtual void initialize(int stage) override {}

    virtual const PacketDropVisualization *createPacketDropVisualization(PacketDrop *packetDrop) const override { return nullptr; }
    virtual void setAlpha(const PacketDropVisualization *packetDropVisualization, double alpha) const override { }

#endif // ifdef WITH_OSG
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_PACKETDROPOSGVISUALIZER_H

