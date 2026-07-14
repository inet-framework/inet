//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_COROUTINEEVENTEXECUTION_H
#define __INET_COROUTINEEVENTEXECUTION_H

#include "inet/common/INETDefs.h"

namespace inet {

struct INET_API CoroutineSlot {
    cCoroutine coroutine;
    cEvent *event = nullptr;
    bool completed = false;
    bool yielded = false;
    std::exception_ptr exception;
};

extern INET_API thread_local CoroutineSlot *currentCoroutineSlot;

INET_API void installCoroutineEventExecution();
INET_API void uninstallCoroutineEventExecution();

} // namespace inet

#endif
