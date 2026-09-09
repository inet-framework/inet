//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INFOVSGVISUALIZER_H
#define __INET_INFOVSGVISUALIZER_H

#include <string>

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

#include "inet/common/ModuleRefByPar.h"
#include "inet/visualizer/base/InfoVisualizerBase.h"
#include "inet/visualizer/vsg/scene/NetworkNodeVsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API InfoVsgVisualizer : public InfoVisualizerBase
{
  protected:
    class INET_API InfoVsgVisualization : public InfoVisualization {
      public:
        NetworkNodeVsgVisualization *networkNodeVisualization = nullptr;
        ::vsg::ref_ptr<::vsg::Group> node;  // annotation container; holds the current info-text label
        mutable std::string lastInfo;       // avoid rebuilding the text when the string is unchanged

      public:
        InfoVsgVisualization(NetworkNodeVsgVisualization *networkNodeVisualization, ::vsg::ref_ptr<::vsg::Group> node, int moduleId);
    };

  protected:
    ModuleRefByPar<NetworkNodeVsgVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual InfoVisualization *createInfoVisualization(cModule *module) const override;
    virtual void refreshInfoVisualization(const InfoVisualization *infoVisualization, const char *info) const override;
};

} // namespace visualizer

} // namespace inet

#endif
