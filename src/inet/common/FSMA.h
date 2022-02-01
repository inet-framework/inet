//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FSMA_H
#define __INET_FSMA_H

#include "inet/common/INETDefs.h"

namespace inet {

/*
    This is an alternative FSM implementation.

    Here is an example:

    FSMA_Switch(fsm)
    {
        FSMA_State(X)
        {
            FSMA_Event_Transition(XY, isFoo, Y,
                doFoo);
            FSMA_No_Event_Transition(XZ, isFooBar, Z,
                doFooBar);
        }
        FSMA_State(Y)
        {
            FSMA_Event_Transition(YX, isBar, X,
                doBar);
        }
        FSMA_State(Z)
        {
            FSMA_Event_Transition(ZX, isBaz, X,
                doBaz);
        }
    }

    After macro expansion, a state machine code looks like something along
    these lines:

    bool ___is_event = true;
    bool ___exit = false;
    int ___c = 0;
    cFSM& ___fsm = fsm;
    while (!___exit && (___c++ < FSM_MAXT || (throw cRuntimeError(E_INFLOOP, ___fsm.getStateName())))
    {
        if (transition_seen = false, ___exit = true, ___fsm.getState() == X)
        {
            if (!___is_event)
            {
                if (transition_seen)
                    throw cRuntimeError("...");
                // enter code
            }
            transition_seen = true; if (isFoo && ___is_event)
            {
                EV_DEBUG << "firing " << XY << " transition for " << ___fsm.getName() << endl;
                doFoo;
                ___fsm.setState(Y, "Y");
                ___is_event = false;
                ___exit = false;
                continue;
            }
            transition_seen = true; if (isFooBar && !___is_event)
            {
                EV_DEBUG << "firing " << XZ << " transition for " << ___fsm.getName() << endl;
                doFooBar;
                ___fsm.setState(Z, "Z");
                ___exit = false;
                continue;
            }
        }
        if (transition_seen = false, ___exit = true, ___fsm.getState() == Y)
        {
            transition_seen = true; if (isBar && ___is_event)
            {
                EV_DEBUG << "firing " << YX << " transition for " << ___fsm.getName() << endl;
                doVar;
                ___fsm.setState(X, "X");
                ___is_event = false;
                ___exit = false;
                continue;
            }
        }
    }
 */

#define FSMA_Switch(fsm) \
    bool ___is_event = true; \
    bool ___exit = false; \
    bool ___transition_seen = false; \
    int ___c = 0; \
    cFSM& ___fsm = (fsm); \
    bool ___logging = true; \
    if (___logging) EV_DEBUG << "FSM " << ___fsm.getName() << ": processing event in state " << ___fsm.getStateName() << "\n"; \
    while (!___exit && (___c++ < FSM_MAXT || (throw cRuntimeError(E_INFLOOP, ___fsm.getStateName()), 0)))

#define FSMA_SetLogging(enabled) \
        ___logging = enabled;

#define FSMA_Print(exiting) \
    if (___logging) EV_DEBUG << "FSM " << ___fsm.getName() << ((exiting) ? ": leaving state " : ": entering state ") << ___fsm.getStateName() << endl

#define FSMA_State(s)    if (___transition_seen = false, ___exit = true, ___fsm.getState() == (s))

#define FSMA_Enter(action) \
    if (!___is_event) \
    { \
        if (___transition_seen) \
            throw cRuntimeError(&___fsm, "FSMA_Enter() must precede all FSMA_*_Transition()'s in the code"); \
        action; \
    }

#define FSMA_Event_Transition(transition, condition, target, action) \
    ___transition_seen = true; if ((condition) && ___is_event) \
    { \
        ___is_event = false; \
        FSMA_Transition(transition, (condition), target, action)

#define FSMA_No_Event_Transition(transition, condition, target, action) \
    ___transition_seen = true; if ((condition) && !___is_event) \
    { \
        FSMA_Transition(transition, (condition), target, action)

#define FSMA_Transition(transition, condition, target, action) \
    FSMA_Print(true); \
    if (___logging) EV_DEBUG << "FSM " << ___fsm.getName() << ": condition \"" << #condition << "\" holds, taking transition \"" << #transition << "\" to state " << #target << endl; \
    action; \
    ___fsm.setState(target, #target); \
    if (___logging) EV_DEBUG << "FSM " << ___fsm.getName() << ": done processing associated actions\n"; \
    FSMA_Print(false); \
    ___exit = false; \
    continue; \
    }

#define FSMA_Ignore_Event(condition) \
    ___transition_seen = true; if ((condition) && ___is_event) \
    { \
        if (___logging) EV_DEBUG << "FSM " << ___fsm.getName() << ": condition \"" << #condition << "\" holds, staying in current state" << endl; \
        ___is_event = false; \
    }

#define FSMA_Fail_On_Unhandled_Event() \
    ___transition_seen = true; if (___is_event) \
    { \
        throw cRuntimeError(&___fsm, "Unhandled event"); \
    }

} // namespace inet

#endif

