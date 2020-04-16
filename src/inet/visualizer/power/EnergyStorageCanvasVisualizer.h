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

#ifndef __INET_ENERGYSTORAGECANVASVISUALIZER_H
#define __INET_ENERGYSTORAGECANVASVISUALIZER_H

#include "inet/common/figures/BarFigure.h"
#include "inet/visualizer/base/EnergyStorageVisualizerBase.h"
#include "inet/visualizer/scene/NetworkNodeCanvasVisualizer.h"

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
    NetworkNodeCanvasVisualizer *networkNodeVisualizer = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual EnergyStorageVisualization *createEnergyStorageVisualization(const power::IEnergyStorage *energyStorage) const override;
    virtual void addEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization) override;
    virtual void removeEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization) override;
    virtual void refreshEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization) const override;

  public:
    virtual ~EnergyStorageCanvasVisualizer();
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_ENERGYSTORAGECANVASVISUALIZER_H

