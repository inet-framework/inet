//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/common/FSMA.h"

namespace inet {

Register_Class(Fsm);

void Fsm::copy(const Fsm &other)
{
    stateName = other.stateName;
    state = other.state;
}

Fsm& Fsm::operator=(const Fsm &vs)
{
    if (this == &vs)
        return *this;
    cOwnedObject::operator=(vs);
    copy(vs);
    return *this;
}

void Fsm::setState(int state, const char *stateName)
{
    this->state = state;
    this->stateName = stateName;
    if (stateChangedSignal != -1)
        cSimulation::getActiveSimulation()->getContextModule()->emit(stateChangedSignal, state);
}

void Fsm::insertDelayedAction(std::function<void()> action)
{
    delayedActions.push_back(action);
}

void Fsm::executeDelayedActions()
{
    auto actions = delayedActions;
    delayedActions.clear();
    for (auto action : actions)
        action();
}

std::string Fsm::str() const
{
    std::stringstream out;
    if (!stateName)
        out << "state: " << state;
    else
        out << "state: " << stateName << " (" << state << ")";
    return out.str();
}

} // namespace inet
