//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ENERGYSTORAGEOSGVISUALIZER_H
#define __INET_ENERGYSTORAGEOSGVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/visualizer/base/EnergyStorageVisualizerBase.h"
#include "inet/visualizer/osg/scene/NetworkNodeOsgVisualizer.h"
#include "inet/visualizer/osg/util/OsgUtils.h"

namespace inet {

namespace visualizer {

class INET_API EnergyStorageOsgVisualizer : public EnergyStorageVisualizerBase
{
  protected:
    class INET_API EnergyStorageOsgVisualization : public EnergyStorageVisualization {
      public:
        NetworkNodeOsgVisualization *networkNodeVisualization = nullptr;
        osg::Geode *node = nullptr;

      public:
        EnergyStorageOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Geode *figure, const power::IEnergyStorage *energyStorage);
    };

  protected:
    // parameters
    ModuleRefByPar<NetworkNodeOsgVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual EnergyStorageVisualization *createEnergyStorageVisualization(const power::IEnergyStorage *energyStorage) const override;
    virtual void refreshEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization) const override;
};

} // namespace visualizer

} // namespace inet

#endif

