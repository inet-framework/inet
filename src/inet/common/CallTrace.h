//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CALLTRACE_H
#define __INET_CALLTRACE_H

#if defined(__GNUC__) && !defined(_WIN32)

#include "inet/common/INETDefs.h"

#include <regex>

namespace inet {

/**
 * This class is capable of generating a function call traces on the standard output.
 * The output contains one line for entering and one line for leaving a function.
 *
 * Important: compile INET using GCC with '-finstrument-functions' into an executable.
 *
 * For example, the following produces a function call trace for a maximum depth
 * limit of 10 levels including only INET functions:
 * CALL_TRACE(true, 10, "inet::.*");
 *
 * Here is another example focusing on certain classes and ignoring some functions:
 * CALL_TRACE(true, 20, "^inet::(ScenarioManager|LifecycleController|NodeStatus|NetworkInterface|InterfaceTable|Ipv4RoutingTable|Ipv4NetworkConfigurator|Ipv4NodeConfigurator|EthernetMac|EthernetMacBase)::(?!(get|is|has|compute)).*");
 *
 * It's also possible to disable call tracing for the lexical scope:
 * CALL_TRACE(false, 0, "");
 */
class INET_API CallTrace
{
  public:
    struct State
    {
      public:
        bool enabled = false;
        int level = 0;

        const char *name = nullptr;
        int maxLevel = -1;
        std::regex filter;
    };

  public:
    static State state;

  private:
    State oldState;

  public:
    CallTrace(bool enabled, const char *name, int maxLevel, const char *filter);
    ~CallTrace();

    static std::string demangle(const char* mangledName);
};

#define CALL_TRACE(enabled, maxLevel, filter) std::string name = CallTrace::demangle(typeid(*this).name()) + "::" + __func__ + "()"; CallTrace callTrace(enabled, name.c_str(), maxLevel, filter)

}  // namespace inet

#else

#define CALL_TRACE(enabled, maxLevel, filter)

#endif

#endif
