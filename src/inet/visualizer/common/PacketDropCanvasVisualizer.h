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

#ifndef __INET_PACKETDROPCANVASVISUALIZER_H
#define __INET_PACKETDROPCANVASVISUALIZER_H

#include "inet/common/figures/LabeledIconFigure.h"
#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/PacketDropVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PacketDropCanvasVisualizer : public PacketDropVisualizerBase
{
  protected:
    class INET_API PacketDropCanvasVisualization : public PacketDropVisualization {
      public:
        LabeledIconFigure *figure;

      public:
        PacketDropCanvasVisualization(LabeledIconFigure *figure, int moduleId, const cPacket *packet, const Coord& position);
        virtual ~PacketDropCanvasVisualization() { delete figure; }
    };

  protected:
    double zIndex = NaN;
    const CanvasProjection *canvasProjection = nullptr;
    cGroupFigure *packetDropGroup = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual const PacketDropVisualization *createPacketDropVisualization(cModule *module, cPacket *packet) const override;
    virtual void addPacketDropVisualization(const PacketDropVisualization *packetDrop) override;
    virtual void removePacketDropVisualization(const PacketDropVisualization *packetDrop) override;
    virtual void setAlpha(const PacketDropVisualization *packetDrop, double alpha) const override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_PACKETDROPCANVASVISUALIZER_H

