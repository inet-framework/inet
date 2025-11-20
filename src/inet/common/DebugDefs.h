//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DEBUGDEFS_H
#define __INET_DEBUGDEFS_H

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "inet/common/INETDefs.h"

namespace inet {

#ifndef DEBUG_PRINT_LOG
#define DEBUG_PRINT_LOG 0
#endif

static bool _dbg_global_enabled = true;

struct NullStream {
    template<typename T> NullStream& operator<<(const T&) { return *this; }
    NullStream& operator<<(std::ostream& (*)(std::ostream&)) { return *this; }
};
inline NullStream nullStream;

#if DEBUG_PRINT_LOG

inline unsigned _dbg_indent = 0;

struct _DbgStream {
    bool on;
    unsigned indent;
    bool needIndent{true};

    explicit _DbgStream(bool enabled, unsigned level) : on(enabled), indent(level) {}

    void prefix_if_needed() {
        if (on && needIndent) {
            std::cout << std::setprecision(24)
                      << std::string(indent * 3, ' ');
            needIndent = false;
        }
    }

    template <typename T>
    _DbgStream& operator<<(const T& v) {
        if (on) { prefix_if_needed(); std::cout << v; }
        return *this;
    }

    _DbgStream& operator<<(std::ostream& (*manip)(std::ostream&)) {
        if (on) {
            prefix_if_needed();
            std::cout << manip;
            if (manip == static_cast<std::ostream& (*)(std::ostream&)>(std::endl))
                needIndent = true;
        }
        return *this;
    }
};

inline void print_args(std::ostream&) {}
template <typename T, typename... Ts>
inline void print_args(std::ostream& os, const T& t, const Ts&... ts) {
    os << t;
    (void)std::initializer_list<int>{ ( (void)(os << ", " << ts), 0 )... };
}
inline void print_ret_or_void(std::ostream& os) { os << "void"; }          // no retval
template <typename T>
inline void print_ret_or_void(std::ostream& os, const T& v) { os << v; }   // with retval

// 1-based index of the current call (valid after DEBUG_ENTER)
#define DEBUG_EXEC_INDEX   (_dbg_this_exec)

static inline bool _dbg_function_enabled(const char *function)
{
    return !strcmp("doComputeIntervalForTicks", function); // TODO: make this a command line argument regex, matching nothing by default
}

// Unified ENTER. Use: DEBUG_ENTER()  or  DEBUG_ENTER(a, b, c)
#define DEBUG_ENTER(...)                                                           \
    static bool _dbg_enabled = false;                                              \
    _dbg_enabled = _dbg_function_enabled(__func__);                                \
    static unsigned long long _dbg_exec_count = 0;                                 \
    unsigned long long _dbg_this_exec = 0;                                         \
    std::string _dbg_argstr;                                                       \
    {                                                                              \
        std::ostringstream _dbg_os;                                                \
        __VA_OPT__(print_args(_dbg_os, __VA_ARGS__);)                              \
        _dbg_argstr = _dbg_os.str();                                               \
    }                                                                              \
    const bool _dbg_on = _dbg_enabled && _dbg_global_enabled;                      \
    _DbgStream DEBUG_OUT(_dbg_on, _dbg_indent + 1);                                \
    if (_dbg_on) {                                                                 \
        ++_dbg_indent;                                                             \
        _dbg_this_exec = ++_dbg_exec_count;                                        \
        std::cout << std::setprecision(24)                                         \
                  << std::string((_dbg_indent - 1) * 3, ' ')                       \
                  << "[" << _dbg_this_exec << "] " << __func__ << "("              \
                  << _dbg_argstr << ")" << std::endl;                              \
    }

// Per-method toggle
#define DEBUG_ENABLE()       (_dbg_enabled = true)
#define DEBUG_DISABLE()      (_dbg_enabled = false)
#define DEBUG_IS_ENABLED()   (_dbg_enabled)

// Unified LEAVE. Use: DEBUG_LEAVE()  or  DEBUG_LEAVE(result)
#define DEBUG_LEAVE(...)                                                           \
    do {                                                                           \
        if (_dbg_on) {                                                             \
            std::cout << std::setprecision(24)                                     \
                      << std::string((_dbg_indent - 1) * 3, ' ')                   \
                      << "[" << _dbg_this_exec << "] " << __func__ << "("          \
                      << _dbg_argstr << ") = ";                                    \
            print_ret_or_void(std::cout __VA_OPT__(, __VA_ARGS__));                \
            std::cout << std::endl;                                                \
            --_dbg_indent;                                                         \
        }                                                                          \
    } while (0)

#else

#define DEBUG_ENTER(...)         do { } while (0)
#define DEBUG_ENABLE()           ((void)0)
#define DEBUG_DISABLE()          ((void)0)
#define DEBUG_IS_ENABLED()       (false)
#define DEBUG_LEAVE(...)         ((void)0)
#define DEBUG_EXEC_INDEX         (0ull)
#define DEBUG_OUT                nullStream

#endif

#define DEBUG_FIELD_1(field)                     DEBUG_FIELD_2(field, field)
#define DEBUG_FIELD_2(field, value)              #field << " = " << value << ", "
#define DEBUG_FIELD_CHOOSER(...)                 GET_3TH_ARG(__VA_ARGS__, DEBUG_FIELD_2, DEBUG_FIELD_1, )
#define DEBUG_FIELD(...)                         DEBUG_FIELD_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#ifndef DEBUG_CHECK_IMPLEMENTATION
#ifndef NDEBUG
#define DEBUG_CHECK_IMPLEMENTATION 1
#endif
#endif

#if DEBUG_CHECK_IMPLEMENTATION
#define DEBUG_CMP(o1, cmp, o2) { \
    bool _old_dbg_global_enabled = _dbg_global_enabled; _dbg_global_enabled = false; auto _o1 = (o1); auto _o2 = (o2); _dbg_global_enabled = _old_dbg_global_enabled; \
    if (!(_o1 cmp _o2)) { \
        _dbg_global_enabled = true; _o1 = (o1); _o2 = (o2); \
        std::ostringstream oss; oss << "ASSERT: Condition '" << #o1 << " " << #cmp << " " << #o2 << "' as '" << _o1 << " " << #cmp << " " << _o2 << "' does not hold"; \
        omnetpp::cRuntimeError error("in function '%s()' at %s:%d", __FUNCTION__, __FILE__, __LINE__); \
        error.prependMessage(oss.str().c_str()); \
        throw error; \
    } \
}
#else
#define DEBUG_CMP(o1, cmp, o2) { (void)(o1); (void)(o2); }
#endif

} // namespace inet

#endif

