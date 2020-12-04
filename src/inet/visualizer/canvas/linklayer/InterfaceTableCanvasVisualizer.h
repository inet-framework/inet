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

#ifndef __INET_INTERFACETABLECANVASVISUALIZER_H
#define __INET_INTERFACETABLECANVASVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/figures/BoxedLabelFigure.h"
#include "inet/visualizer/base/InterfaceTableVisualizerBase.h"
#include "inet/visualizer/scene/NetworkNodeCanvasVisualization.h"
#include "inet/visualizer/scene/NetworkNodeCanvasVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API InterfaceTableCanvasVisualizer : public InterfaceTableVisualizerBase
{
  protected:
    class INET_API InterfaceCanvasVisualization : public InterfaceVisualization {
      public:
        NetworkNodeCanvasVisualization *networkNodeVisualization = nullptr;
        BoxedLabelFigure *figure = nullptr;

      public:
        InterfaceCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, BoxedLabelFigure *figure, int networkNodeId, int networkNodeGateId, int interfaceId);
        virtual ~InterfaceCanvasVisualization();
    };

  protected:
    double zIndex = NaN;
    ModuleRefByPar<NetworkNodeCanvasVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual NetworkInterface *getNetworkInterface(const InterfaceVisualization *interfaceVisualization);
    virtual InterfaceVisualization *createInterfaceVisualization(cModule *networkNode, NetworkInterface *networkInterface) override;
    virtual void addInterfaceVisualization(const InterfaceVisualization *interfaceVisualization) override;
    virtual void removeInterfaceVisualization(const InterfaceVisualization *interfaceVisualization) override;
    virtual void refreshInterfaceVisualization(const InterfaceVisualization *interfaceVisualization, const NetworkInterface *networkInterface) override;
};

} // namespace visualizer

} // namespace inet

#endif

