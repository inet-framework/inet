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

#ifndef __INET_TRANSPORTCONNECTIONCANVASVISUALIZER_H
#define __INET_TRANSPORTCONNECTIONCANVASVISUALIZER_H

#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/TransportConnectionVisualizerBase.h"
#include "inet/visualizer/networknode/NetworkNodeCanvasVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API TransportConnectionCanvasVisualizer : public TransportConnectionVisualizerBase
{
  protected:
    class INET_API TransportConnectionCanvasVisualization : public TransportConnectionVisualization {
      public:
        cFigure *sourceFigure = nullptr;
        cFigure *destinationFigure = nullptr;

      public:
        TransportConnectionCanvasVisualization(cFigure *sourceFigure, cFigure *destinationFigure, int sourceModuleId, int destinationModuleId, int count);
    };

  protected:
    double zIndex = NaN;
    const CanvasProjection *canvasProjection = nullptr;
    NetworkNodeCanvasVisualizer *networkNodeVisualizer = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual cFigure *createConnectionEndFigure(tcp::TCPConnection *connection) const;
    virtual const TransportConnectionVisualization *createConnectionVisualization(cModule *source, cModule *destination, tcp::TCPConnection *tcpConnection) const override;
    virtual void addConnectionVisualization(const TransportConnectionVisualization *connection) override;
    virtual void removeConnectionVisualization(const TransportConnectionVisualization *connection) override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_TRANSPORTCONNECTIONCANVASVISUALIZER_H

