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

#ifndef __INET_ICOLLISIONCONTROLLER_H
#define __INET_ICOLLISIONCONTROLLER_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

/**
 * Abstract interface for collision controllers. A collision controller is
 * used with EDCA, and it detects and reports internal collisions between
 * Contention instances.
 */
class INET_API ICollisionController
{
    public:
        class INET_API ICallback {
            public:
                virtual ~ICallback() {}
                virtual void transmissionGranted(int txIndex) = 0;
                virtual void internalCollision(int txIndex) = 0;
        };
        virtual void scheduleTransmissionRequest(int txIndex, simtime_t txStartTime, ICallback *callback) = 0;
        virtual void cancelTransmissionRequest(int txIndex) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

