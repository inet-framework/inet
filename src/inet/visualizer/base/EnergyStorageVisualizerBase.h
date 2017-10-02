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

#ifndef __INET_ENERGYSTORAGEVISUALIZERBASE_H
#define __INET_ENERGYSTORAGEVISUALIZERBASE_H

#include "inet/power/contract/IEnergyStorage.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/Placement.h"
#include "inet/visualizer/util/ModuleFilter.h"

namespace inet {

namespace visualizer {

using namespace inet::power;

class INET_API EnergyStorageVisualizerBase : public VisualizerBase
{
  protected:
    class INET_API EnergyStorageVisualization
    {
      public:
        const IEnergyStorage *energyStorage;

      public:
        EnergyStorageVisualization(const IEnergyStorage *energyStorage);
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

    virtual double getNominalCapacity(const IEnergyStorage *energyStorage) const;
    virtual double getResidualCapacity(const IEnergyStorage *energyStorage) const;

    virtual EnergyStorageVisualization *createEnergyStorageVisualization(const IEnergyStorage *energyStorage) const = 0;
    virtual void addEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization);
    virtual void addEnergyStorageVisualizations();
    virtual void removeEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization);
    virtual void refreshEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization) const = 0;
    virtual void removeAllEnergyStorageVisualizations();
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_ENERGYSTORAGEVISUALIZERBASE_H

