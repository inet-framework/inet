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

#ifndef __INET_SDLPROCESS_H
#define __INET_SDLPROCESS_H

#include "inet/common/INETDefs.h"
#include <functional>
#include <set>
#include <list>

namespace inet {
namespace ieee80211 {

class INET_API SdlProcess
{
    public:
        struct SdlTransition
        {
            SdlTransition() {}

            SdlTransition(int signalId, std::function<void(cMessage *)> processSignal = nullptr, bool priority = false, std::function<bool()> enablingCondition = nullptr, std::function<bool()> continuousSignal = nullptr) :
                signalId(signalId),
                processSignal(processSignal),
                priority(priority),
                enablingCondition(enablingCondition),
                continuousSignal(continuousSignal) {}

            int signalId = -1;
            std::function<void(cMessage *)> processSignal = nullptr;
            bool priority = false;
            std::function<bool()> enablingCondition = nullptr;
            std::function<bool()> continuousSignal = nullptr;
        };

        struct SdlState
        {
            int stateId;
            std::vector<SdlTransition> transitions;
            std::set<int> saveSignalSet;

            SdlState(int stateId, std::vector<SdlTransition> transitions, std::set<int> saveSignalSet) :
                stateId(stateId),
                transitions(transitions),
                saveSignalSet(saveSignalSet) {}

            bool isSaved(cMessage *signal) const { return saveSignalSet.count(signal->getKind()) == 1 || saveSignalSet.count(-1) == 1; }
        };

        SdlProcess(std::function<void(cMessage *)> defaultHandler, std::vector<SdlState> states) :
            defaultHandler(defaultHandler),
            states(states)
        {}
    protected:
        std::list<cMessage *> signals;
        std::function<void(cMessage *)> defaultHandler = nullptr;
        std::vector<SdlState> states;
        SdlState *currentState = nullptr;

    public:
        void insertSignal(cMessage *signal);
        void run();

        void setCurrentState(int stateId);
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_SDLPROCESS_H
