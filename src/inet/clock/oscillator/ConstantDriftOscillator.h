//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CONSTANTDRIFTOSCILLATOR_H
#define __INET_CONSTANTDRIFTOSCILLATOR_H

#include "inet/clock/base/DriftingOscillatorBase.h"

namespace inet {

class INET_API ConstantDriftOscillator : public DriftingOscillatorBase
{
  protected:
    virtual void initialize(int stage) override;
};

} // namespace inet

#endif

