//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PHYSICALENVIRONMENTVISUALIZERBASE_H
#define __INET_PHYSICALENVIRONMENTVISUALIZERBASE_H

#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/visualizer/base/VisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PhysicalEnvironmentVisualizerBase : public VisualizerBase
{
  protected:
    /** @name Parameters */
    //@{
    const physicalenvironment::IPhysicalEnvironment *physicalEnvironment = nullptr;
    bool displayObjects = false;
    //@}

  protected:
    virtual void initialize(int stage) override;
};

} // namespace visualizer

} // namespace inet

#endif

