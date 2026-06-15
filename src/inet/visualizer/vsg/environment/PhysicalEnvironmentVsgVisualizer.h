//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PHYSICALENVIRONMENTVSGVISUALIZER_H
#define __INET_PHYSICALENVIRONMENTVSGVISUALIZER_H

#include "inet/visualizer/base/PhysicalEnvironmentVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PhysicalEnvironmentVsgVisualizer : public PhysicalEnvironmentVisualizerBase
{
  protected:
    bool enableObjectOpacity = false;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;
};

} // namespace visualizer

} // namespace inet

#endif
