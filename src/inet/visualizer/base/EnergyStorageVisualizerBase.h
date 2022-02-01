//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ENERGYSTORAGEVISUALIZERBASE_H
#define __INET_ENERGYSTORAGEVISUALIZERBASE_H

#include "inet/power/contract/IEnergyStorage.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/ModuleFilter.h"
#include "inet/visualizer/util/Placement.h"

namespace inet {
namespace visualizer {

class INET_API EnergyStorageVisualizerBase : public VisualizerBase
{
  protected:
    class INET_API EnergyStorageVisualization {
      public:
        const power::IEnergyStorage *energyStorage;

      public:
        EnergyStorageVisualization(const power::IEnergyStorage *energyStorage);
        virtual ~EnergyStorageVisualization() {}
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayEnergyStorages = false;
    ModuleFilter energyStorageFilter;
    cFigure::Color color;
    double width;
    double height;
    double spacing;
    Placement placementHint;
    double placementPriority;
    //@}

    std::vector<const EnergyStorageVisualization *> energyStorageVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void refreshDisplay() const override;
    virtual void preDelete(cComponent *root) override;

    virtual double getNominalCapacity(const power::IEnergyStorage *energyStorage) const;
    virtual double getResidualCapacity(const power::IEnergyStorage *energyStorage) const;

    virtual EnergyStorageVisualization *createEnergyStorageVisualization(const power::IEnergyStorage *energyStorage) const = 0;
    virtual void addEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization);
    virtual void addEnergyStorageVisualizations();
    virtual void removeEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization);
    virtual void refreshEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization) const = 0;
    virtual void removeAllEnergyStorageVisualizations();
};

} // namespace visualizer
} // namespace inet

#endif

