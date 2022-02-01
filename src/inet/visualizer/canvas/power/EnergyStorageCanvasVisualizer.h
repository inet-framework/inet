//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ENERGYSTORAGECANVASVISUALIZER_H
#define __INET_ENERGYSTORAGECANVASVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/figures/BarFigure.h"
#include "inet/visualizer/base/EnergyStorageVisualizerBase.h"
#include "inet/visualizer/canvas/scene/NetworkNodeCanvasVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API EnergyStorageCanvasVisualizer : public EnergyStorageVisualizerBase
{
  protected:
    class INET_API EnergyStorageCanvasVisualization : public EnergyStorageVisualization {
      public:
        NetworkNodeCanvasVisualization *networkNodeVisualization = nullptr;
        BarFigure *figure = nullptr;

      public:
        EnergyStorageCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, BarFigure *figure, const power::IEnergyStorage *energyStorage);
        virtual ~EnergyStorageCanvasVisualization();
    };

  protected:
    // parameters
    double zIndex = NaN;
    ModuleRefByPar<NetworkNodeCanvasVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual EnergyStorageVisualization *createEnergyStorageVisualization(const power::IEnergyStorage *energyStorage) const override;
    virtual void addEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization) override;
    virtual void removeEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization) override;
    virtual void refreshEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization) const override;
};

} // namespace visualizer

} // namespace inet

#endif

