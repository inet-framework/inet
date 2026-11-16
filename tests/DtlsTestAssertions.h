// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TESTS_DTLS_TEST_ASSERTIONS_H
#define INET_TESTS_DTLS_TEST_ASSERTIONS_H

#include <omnetpp.h>

// Test checks and their state transitions must execute with either model library.
#define DTLS_TEST_CHECK(expr) \
    ((void)((expr) ? 0 : (throw omnetpp::cRuntimeError("DTLS test condition '%s' failed at %s:%d", \
        #expr, __FILE__, __LINE__), 0)))

#endif
