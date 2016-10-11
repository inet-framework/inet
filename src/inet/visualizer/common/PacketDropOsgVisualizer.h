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
    class INET_API OsgPacketDrop : public PacketDropVisualization
    {
      public:
        osg::Node *node = nullptr;

      public:
        OsgPacketDrop(osg::Node *node, int moduleId, cPacket *packet, simtime_t dropSimulationTime, double dropAnimationTime, int dropRealTime);
        virtual ~OsgPacketDrop();
    };

  protected:
    virtual const PacketDropVisualization *createPacketDropVisualization(cModule *module, cPacket *packet) const override;
    virtual void setAlpha(const PacketDropVisualization *packetDrop, double alpha) const override;
    virtual void addPacketDropVisualization(const PacketDropVisualization *packetDrop) override;
    virtual void removePacketDropVisualization(const PacketDropVisualization *packetDrop) override;

#else // ifdef WITH_OSG

  protected:
    virtual const PacketDropVisualization *createPacketDropVisualization(cModule *module, cPacket *packet) const override { return nullptr; }
    virtual void setAlpha(const PacketDropVisualization *packetDrop, double alpha) const override { }

#endif // ifdef WITH_OSG
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_PACKETDROPOSGVISUALIZER_H

