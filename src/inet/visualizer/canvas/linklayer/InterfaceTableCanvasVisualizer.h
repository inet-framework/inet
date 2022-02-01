//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INTERFACETABLECANVASVISUALIZER_H
#define __INET_INTERFACETABLECANVASVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/figures/BoxedLabelFigure.h"
#include "inet/visualizer/base/InterfaceTableVisualizerBase.h"
#include "inet/visualizer/canvas/scene/NetworkNodeCanvasVisualization.h"
#include "inet/visualizer/canvas/scene/NetworkNodeCanvasVisualizer.h"

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

