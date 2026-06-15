//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RADIOVSGVISUALIZER_H
#define __INET_RADIOVSGVISUALIZER_H

#include <vsg/nodes/Group.h>

#include "inet/visualizer/base/RadioVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API RadioVsgVisualizer : public RadioVisualizerBase
{
  protected:
    class INET_API RadioVsgVisualization : public RadioVisualization {
      public:
        // Scene-level group node holding the radio markers (sphere + optional label)
        ::vsg::ref_ptr<::vsg::Group> node;

      public:
        RadioVsgVisualization(::vsg::ref_ptr<::vsg::Group> node, const int radioModuleId);
    };

  protected:
    virtual void initialize(int stage) override;

    virtual RadioVisualization *createRadioVisualization(const physicallayer::IRadio *radio) const override;
    virtual void addRadioVisualization(const RadioVisualization *radioVisualization) override;
    virtual void removeRadioVisualization(const RadioVisualization *radioVisualization) override;
    virtual void refreshRadioVisualization(const RadioVisualization *radioVisualization) const override;
};

} // namespace visualizer

} // namespace inet

#endif
