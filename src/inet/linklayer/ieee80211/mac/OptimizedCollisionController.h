//
// Copyright (C) 2015 Andras Varga
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// Author: Andras Varga
//

#ifndef __INET_OPTIMIZEDCOLLISIONCONTROLLER_H
#define __INET_OPTIMIZEDCOLLISIONCONTROLLER_H

#include "ICollisionController.h"

namespace inet {
namespace ieee80211 {

/**
 * A collision controller optimized for runtime efficiency. This practically
 * means that it uses as few timers as possible.
 */
class INET_API OptimizedCollisionController : public cSimpleModule, public ICollisionController
{
    private:
        static simtime_t MAX_TIME;   // used as the "invalid" value
        static const int MAX_NUM_TX = 4;
        int txCount = 0;
        simtime_t txTime[MAX_NUM_TX];
        ICallback *callback[MAX_NUM_TX];
        cMessage *timer = nullptr;
        simtime_t timeLastProcessed;
    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage* msg) override;
        void reschedule();
    public:
        OptimizedCollisionController();
        ~OptimizedCollisionController();
        virtual void scheduleTransmissionRequest(int txIndex, simtime_t txStartTime, ICallback *callback) override;
        virtual void cancelTransmissionRequest(int txIndex) override;
};

} // namespace ieee80211
} // namespace inet

#endif

