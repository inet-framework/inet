//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/common/SimpleModule.h"

namespace inet {

Define_Module(SimpleModule);

void SimpleModule::initialize()
{
    if (typeid(*this) == typeid(SimpleModule))
        throw cRuntimeError("%s is missing @class property - modules that extend SimpleModule must have a @class property to specify the C++ implementation class", getComponentType()->getFullName());
    ModuleMixin<cSimpleModule>::initialize();
}

void SimpleModule::initialize(int stage)
{
    if (typeid(*this) == typeid(SimpleModule))
        throw cRuntimeError("%s is missing @class property - modules that extend SimpleModule must have a @class property to specify the C++ implementation class", getComponentType()->getFullName());
    ModuleMixin<cSimpleModule>::initialize(stage);
}

} // namespace inet

