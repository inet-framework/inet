//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIMPLEMODULE_H
#define __INET_SIMPLEMODULE_H

#include <type_traits>

#include "inet/common/ModuleMixin.h"

namespace inet {

class INET_API SimpleModule : public ModuleMixin<cSimpleModule>
{
  public:
    virtual void initialize() override;
    virtual void initialize(int stage) override;
};

} // namespace inet

#endif
