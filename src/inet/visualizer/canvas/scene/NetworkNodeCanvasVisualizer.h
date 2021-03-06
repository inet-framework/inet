//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_NETWORKNODECANVASVISUALIZER_H
#define __INET_NETWORKNODECANVASVISUALIZER_H

#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/NetworkNodeVisualizerBase.h"
#include "inet/visualizer/canvas/scene/NetworkNodeCanvasVisualization.h"

namespace inet {

namespace visualizer {

class INET_API NetworkNodeCanvasVisualizer : public NetworkNodeVisualizerBase
{
  protected:
    const CanvasProjection *canvasProjection = nullptr;
    double zIndex = NaN;
    std::map<int, NetworkNodeCanvasVisualization *> networkNodeVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual NetworkNodeCanvasVisualization *createNetworkNodeVisualization(cModule *networkNode) const override;
    virtual void addNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization) override;
    virtual void removeNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization) override;
    virtual void destroyNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization) override { delete networkNodeVisualization; }

  public:
    virtual ~NetworkNodeCanvasVisualizer();
    virtual NetworkNodeCanvasVisualization *findNetworkNodeVisualization(const cModule *networkNode) const override;
    virtual NetworkNodeCanvasVisualization *getNetworkNodeVisualization(const cModule *networkNode) const override;
};

} // namespace visualizer

} // namespace inet

#endif

