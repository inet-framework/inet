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

#ifndef __INET_RADIOOSGVISUALIZER_H
#define __INET_RADIOOSGVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/OsgUtils.h"
#include "inet/visualizer/base/RadioVisualizerBase.h"
#include "inet/visualizer/scene/NetworkNodeOsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API RadioOsgVisualizer : public RadioVisualizerBase
{
#ifdef WITH_OSG

  protected:
    class INET_API RadioOsgVisualization : public RadioVisualization {
      public:
        NetworkNodeOsgVisualization *networkNodeVisualization = nullptr;
        osg::Geode *node = nullptr;

      public:
        RadioOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Geode *figure, const int radioModuleId);
    };

  protected:
    // parameters
    ModuleRefByPar<NetworkNodeOsgVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual RadioVisualization *createRadioVisualization(const physicallayer::IRadio *radio) const override;
    virtual void refreshRadioVisualization(const RadioVisualization *radioVisualization) const override;

#else // ifdef WITH_OSG

  protected:
    virtual void initialize(int stage) override {}

    virtual RadioVisualization *createRadioVisualization(const physicallayer::IRadio *radio) const override { return nullptr; }
    virtual void refreshRadioVisualization(const RadioVisualization *radioVisualization) const override { }

#endif // ifdef WITH_OSG
};

} // namespace visualizer

} // namespace inet

#endif

