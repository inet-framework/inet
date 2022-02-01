//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RADIOOSGVISUALIZER_H
#define __INET_RADIOOSGVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/visualizer/base/RadioVisualizerBase.h"
#include "inet/visualizer/osg/scene/NetworkNodeOsgVisualizer.h"
#include "inet/visualizer/osg/util/OsgUtils.h"

namespace inet {

namespace visualizer {

class INET_API RadioOsgVisualizer : public RadioVisualizerBase
{
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
};

} // namespace visualizer

} // namespace inet

#endif

