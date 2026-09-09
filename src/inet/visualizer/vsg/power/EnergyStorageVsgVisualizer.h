//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ENERGYSTORAGEVSGVISUALIZER_H
#define __INET_ENERGYSTORAGEVSGVISUALIZER_H

#include <string>

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

#include "inet/common/ModuleRefByPar.h"
#include "inet/visualizer/base/EnergyStorageVisualizerBase.h"
#include "inet/visualizer/vsg/scene/NetworkNodeVsgVisualization.h"
#include "inet/visualizer/vsg/scene/NetworkNodeVsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API EnergyStorageVsgVisualizer : public EnergyStorageVisualizerBase
{
  protected:
    class INET_API EnergyStorageVsgVisualization : public EnergyStorageVisualization {
      public:
        NetworkNodeVsgVisualization *networkNodeVisualization = nullptr;
        ::vsg::ref_ptr<::vsg::Group> node;  // annotation container; rebuilt on refresh
        mutable std::string lastText;       // avoid rebuilding when unchanged

      public:
        EnergyStorageVsgVisualization(NetworkNodeVsgVisualization *networkNodeVisualization, ::vsg::ref_ptr<::vsg::Group> node, const power::IEnergyStorage *energyStorage);
    };

  protected:
    ModuleRefByPar<NetworkNodeVsgVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual EnergyStorageVisualization *createEnergyStorageVisualization(const power::IEnergyStorage *energyStorage) const override;
    virtual void refreshEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization) const override;
};

} // namespace visualizer

} // namespace inet

#endif
