//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PRECOMPILED_H
#define __INET_PRECOMPILED_H

// NOTE: All macros that modify the behavior of an omnet or system header file
// MUST be defined inside this header otherwise the precompiled header will be incorrectly built

// use GNU compatibility (see: https://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html)
// for asprintf() and other functions
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <omnetpp.h>

#endif

