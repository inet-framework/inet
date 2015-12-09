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

#ifndef __INET_ICONTENTION_H
#define __INET_ICONTENTION_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

class Ieee80211Frame;
class IContentionCallback;

/**
 * Abstract interface for processes that implement contention-based channel access.
 * For each frame, Contention listens on the channel for a DIFS (AIFS) period
 * then for a random backoff period before transmitting the frame, and defers when
 * busy channel is sensed. After receiving a corrupted frame, EIFS is used instead
 * of the original DIFS (AIFS).
 *
 * Note that waiting for an ACK (or CTS) and initiating the retransmission if
 * it does not arrive is not handled by this process. Instead, that is typically
 * performed by a frame exchange class INET_API inside UpperMac (see IFrameExchange, IUpperMac).
 */
class INET_API IContention
{
    public:
        virtual ~IContention() {}
        virtual void startContention(simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount, IContentionCallback *callback) = 0;

        // notifications
        virtual void mediumStateChanged(bool mediumFree) = 0;
        virtual void corruptedFrameReceived() = 0;
        virtual void channelReleased() = 0;
        virtual bool isOwning() = 0;
};

void collectContentionModules(cModule *firstContentionModule, IContention **& contention);

} // namespace ieee80211
} // namespace inet

#endif

