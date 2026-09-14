//
// Protocol Test Framework for INET -- Phase 1: program registry.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "ProtocolTest.h"

#include <sstream>

namespace inet {
namespace protocoltest {

Injection at(const char *nodeName)
{
    Injection injection;
    injection.nodeName = nodeName;
    return injection;
}

Interception tap(const char *tapName)
{
    Interception interception;
    interception.tapName = tapName;
    return interception;
}

std::map<std::string, ProtocolTestBuilderFn>& ProtocolTestRegistry::all()
{
    // Function-local static: safe across static-initializer registration order.
    static std::map<std::string, ProtocolTestBuilderFn> registry;
    return registry;
}

void ProtocolTestRegistry::add(const char *name, ProtocolTestBuilderFn builder)
{
    all()[name] = builder;
}

bool ProtocolTestRegistry::has(const char *name)
{
    return all().find(name) != all().end();
}

ProtocolTest ProtocolTestRegistry::build(const char *name)
{
    auto it = all().find(name);
    if (it == all().end())
        throw cRuntimeError("ProtocolTest '%s' is not registered", name);
    return (it->second)();
}

static ProtocolTestBuilderFn& defaultBuilder()
{
    static ProtocolTestBuilderFn builder = nullptr;
    return builder;
}

void ProtocolTestRegistry::setDefault(ProtocolTestBuilderFn builder)
{
    if (defaultBuilder() != nullptr)
        throw cRuntimeError("ProtocolTest: more than one Define_ProtocolTestProgram() in this build");
    defaultBuilder() = builder;
}

bool ProtocolTestRegistry::hasDefault()
{
    return defaultBuilder() != nullptr;
}

ProtocolTest ProtocolTestRegistry::buildDefault()
{
    if (defaultBuilder() == nullptr)
        throw cRuntimeError("ProtocolTest: no Define_ProtocolTestProgram() in this build");
    return defaultBuilder()();
}

Step never(EventPattern pattern)
{
    Step step;
    step.type = StepType::Never;
    step.pattern = std::move(pattern);
    return step;
}

Step atMostTimes(int n, EventPattern pattern)
{
    Step step;
    step.type = StepType::Count;
    step.cardMin = 0;
    step.cardMax = n;
    step.pattern = std::move(pattern);
    return step;
}

Step exactlyTimes(int n, EventPattern pattern)
{
    Step step;
    step.type = StepType::Count;
    step.cardMin = n;
    step.cardMax = n;
    step.pattern = std::move(pattern);
    return step;
}

Step atLeastTimes(int n, EventPattern pattern)
{
    Step step;
    step.type = StepType::Count;
    step.cardMin = n;
    step.cardMax = -1;
    step.pattern = std::move(pattern);
    return step;
}

} // namespace protocoltest
} // namespace inet
