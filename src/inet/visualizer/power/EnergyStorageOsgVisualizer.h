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

#ifndef __INET_ENERGYSTORAGEOSGVISUALIZER_H
#define __INET_ENERGYSTORAGEOSGVISUALIZER_H

#include "inet/common/OsgUtils.h"
#include "inet/visualizer/base/EnergyStorageVisualizerBase.h"
#include "inet/visualizer/scene/NetworkNodeOsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API EnergyStorageOsgVisualizer : public EnergyStorageVisualizerBase
{
#ifdef WITH_OSG

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
    NetworkNodeOsgVisualizer *networkNodeVisualizer = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual EnergyStorageVisualization *createEnergyStorageVisualization(const power::IEnergyStorage *energyStorage) const override;
    virtual void refreshEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization) const override;

  public:
    virtual ~EnergyStorageOsgVisualizer();

#else // ifdef WITH_OSG

  protected:
    virtual void initialize(int stage) override {}

    virtual EnergyStorageVisualization *createEnergyStorageVisualization(const power::IEnergyStorage *energyStorage) const override { return nullptr; }
    virtual void refreshEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization) const override { }

#endif // ifdef WITH_OSG
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_ENERGYSTORAGEOSGGVISUALIZER_H

