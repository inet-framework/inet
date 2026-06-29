//
// Protocol Test Framework for INET -- Phase 1: program registry.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "ProtocolTest.h"

#include <sstream>

namespace inet {
namespace protocoltest {

Injection inject(const char *nodeName)
{
    Injection injection;
    injection.nodeName = nodeName;
    return injection;
}

Interception intercept(const char *tapName)
{
    Interception interception;
    interception.tapName = tapName;
    return interception;
}

StatePattern state(const char *modulePath, const char *signalName)
{
    StatePattern pattern;
    pattern.modulePath = modulePath;
    pattern.signalName = signalName;
    return pattern;
}

std::string StatePattern::str() const
{
    std::ostringstream os;
    os << modulePath << " " << signalName;
    if (hasValue)
        os << "==" << value;
    if (selHasWithin)
        os << " within=" << selWithin;
    if (selHasNotBefore)
        os << " notBefore=" << selNotBefore;
    return os.str();
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

} // namespace protocoltest
} // namespace inet
