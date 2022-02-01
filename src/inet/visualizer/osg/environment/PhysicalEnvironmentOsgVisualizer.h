//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PHYSICALENVIRONMENTOSGVISUALIZER_H
#define __INET_PHYSICALENVIRONMENTOSGVISUALIZER_H

#include "inet/environment/common/PhysicalEnvironment.h"
#include "inet/visualizer/base/PhysicalEnvironmentVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PhysicalEnvironmentOsgVisualizer : public PhysicalEnvironmentVisualizerBase
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

