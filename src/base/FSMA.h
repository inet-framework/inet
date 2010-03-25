//
// Copyright (C) 2006 Andras Varga and Levente Meszaros
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_FSMA_H
#define __INET_FSMA_H

#include "INETDefs.h"

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
    cFSM ___fsm = fsm;
    while (!___exit && (___c++ < FSM_MAXT || opp_error(eINFLOOP, ___fsm->getStateName()))
    {
        if (condition_seen = false, ___exit = true, ___fsm->getState() == X)
        {
            if (!___is_event)
            {
                if (condition_seen)
                    error("...");
                // enter code
            }
            condition_seen = true; if (isFoo && ___is_event)
            {
                EV << "firing " << XY << " transition for " << ___fsm->getName() << endl;
                doFoo;
                ___fsm->setState(Y, "Y");
                ___is_event = false;
                ___exit = false;
                continue;
            }
            condition_seen = true; if (isFooBar && !___is_event)
            {
                EV << "firing " << XZ << " transition for " << ___fsm->getName() << endl;
                doFooBar;
                ___fsm->setState(Z, "Z");
                ___exit = false;
                continue;
            }
        }
        if (condition_seen = false, ___exit = true, ___fsm->getState() == Y)
        {
            condition_seen = true; if (isBar && ___is_event)
            {
                EV << "firing " << YX << " transition for " << ___fsm->getName() << endl;
                doVar;
                ___fsm->setState(X, "X");
                ___is_event = false;
                ___exit = false;
                continue;
            }
        }
    }
*/

#define FSMA_Switch(fsm)                                               \
    bool ___is_event = true;                                           \
    bool ___exit = false;                                              \
    bool ___condition_seen = false;                                    \
    int ___c = 0;                                                      \
    cFSM *___fsm = &fsm;                                               \
    EV << "processing event in state machine " << (fsm).getName() << endl;  \
    while (!___exit && (___c++ < FSM_MAXT || (opp_error(eINFLOOP, (fsm).getStateName()), 0)))

#define FSMA_Print(exiting)                                            \
    (ev << "FSM " << ___fsm->getName()                                    \
        << ((exiting) ? ": leaving state  " : ": entering state ")     \
        << ___fsm->getStateName() << endl)

#define FSMA_State(s)  if (___condition_seen = false, ___exit = true, ___fsm->getState() == (s))

#define FSMA_Event_Transition(transition, condition, target, action)                    \
    ___condition_seen = true; if ((condition) && ___is_event)                             \
    {                                                                                   \
        ___is_event = false;                                                            \
        FSMA_Transition(transition, (condition), target, action)

#define FSMA_No_Event_Transition(transition, condition, target, action)                 \
    ___condition_seen = true; if ((condition) && !___is_event)                            \
    {                                                                                   \
        FSMA_Transition(transition, (condition), target, action)

#define FSMA_Transition(transition, condition, target, action)                          \
        FSMA_Print(true);                                                               \
        EV << "firing " << #transition << " transition for " << ___fsm->getName() << endl; \
        action;                                                                         \
        ___fsm->setState(target, #target);                                              \
        FSMA_Print(false);                                                              \
        ___exit = false;                                                                \
        continue;                                                                       \
    }

#define FSMA_Enter(action)                                                              \
    if (!___is_event)                                                                   \
    {                                                                                   \
        if (___condition_seen)                                                          \
            error("FSMA_Enter() must precede all FSMA_*_Transition()'s in the code");   \
        action;                                                                         \
    }

#endif

