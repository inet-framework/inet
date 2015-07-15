//
// Copyright (C) 2015 OpenSim Ltd.
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

#include "SdlProcess.h"

namespace inet {
namespace ieee80211 {

void SdlProcess::insertSignal(cMessage* signal)
{
    signals.push_back(signal);
}

void SdlProcess::run()
{
    EV_DEBUG << "Looking for signal in state " << currentState->stateId << endl;
    nextSignal:
    // a) if the input port contains a signal matching a priority input of the current state, the first such
    // signal is consumed (see 11.4); otherwise
    for (auto & transition : currentState->transitions)
    {
        if (transition.priority)
        {
            for (auto it = signals.begin(); it != signals.end(); )
            {
                auto cur = it++;
                cMessage *signal = *cur;
                if (transition.signalId == signal->getKind())
                {
                    signals.erase(cur);
                    EV_DEBUG << "Priority signal found, signal name is " << signal->getName() << endl;
                    if (transition.processSignal)
                        transition.processSignal(signal);
                    else
                        defaultHandler(signal);
                    goto nextSignal;
                }
            }
        }
    }

    // b) in the order of the signals on the input port:
    //  1) the Provided-expressions of the Input-node corresponding to the current signal are
    //     interpreted in arbitrary order, if any;
    //  2) if the current signal is enabled, this signal is consumed (see 11.6); otherwise
    //  3) the next signal on the input port is selected.
    for (auto it = signals.begin(); it != signals.end(); )
    {
        auto cur = it++;
        cMessage *signal = *cur;
        bool hasMatchingInputNode = false;
        for (auto & transition : currentState->transitions)
        {
            // ii) A signal in the input port is enabled, if all the Provided-expressions of an
            //     Input-node return the predefined Boolean value true, or if the Input-node does not
            //     have a Provided-expression.
            hasMatchingInputNode |= transition.signalId == signal->getKind();
            if (transition.signalId == signal->getKind() &&
               (transition.enablingCondition == nullptr || transition.enablingCondition()))
            {
                if (transition.enablingCondition)
                    EV_DEBUG << "Signal = " << signal->getName() << " with enabling condition" << endl;
                else
                    EV_DEBUG << "Signal = " << signal->getName() << endl;
                signals.erase(cur);
                if (transition.processSignal)
                    transition.processSignal(signal);
                else
                    defaultHandler(signal);
                goto nextSignal;
            }
        }
        // i) A signal in a Save-signalset is not enabled.
        if (!hasMatchingInputNode && !currentState->isSaved(signal))
        {
            // Implicit transition
            EV_DEBUG << "Implicit transition signal = " << signal->getName() << endl;
            signals.erase(cur);
            goto nextSignal;
        }
    }

    // c) if no enabled signal was found, in priority order of the Continuous-signals, if any, with
    // Continuous-signals of equal priority being considered in an arbitrary order and no priority
    // being treated as the lowest priority:
    //  1) the Continuous-expression contained in the current Continuous-signal is interpreted;
    //  2) if the current continuous signal is enabled, this signal is consumed (see 11.5); otherwise
    //  3) the next continuous signal is selected.

    for (auto & transition : currentState->transitions)
    {
        // iii) If the Continuous-expression returns the predefined Boolean value true, the
        //      continuous signal is enabled.
        if (transition.continuousSignal != nullptr && transition.continuousSignal())
        {
            EV_DEBUG << "Continuous transition" << endl;
            transition.processSignal(nullptr);
            goto nextSignal;
        }
    }
    // d) if no enabled signal was found, the state machine waits in the state until another signal
    // instance is received. If the state has enabling conditions or continuous signals, these steps
    // are repeated even if no signal is received.
    EV_DEBUG << "End" << endl;
}

void SdlProcess::setCurrentState(int stateId)
{
    if (stateId != this->states[stateId].stateId)
        throw cRuntimeError("State id mismatch");
    if (currentState)
        EV_DEBUG << "State transition from " << currentState->stateId << " to " << stateId << endl;
    this->currentState = &this->states[stateId];
}

} /* namespace inet */
} /* namespace ieee80211 */

