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

#ifndef __INET_IUPPERMAC_H
#define __INET_IUPPERMAC_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

class IContentionCallback;
class ITxCallback;
class IUpperMacContext;
class Ieee80211Frame;
class Ieee80211DataOrMgmtFrame;

/**
 * Abstract class interface for UpperMacs. UpperMacs deal with exchanging
 * frames, and build on the services of IContention, ITx and IRx
 * for accessing the channel. Responsibilities of UpperMacs include queueing,
 * ACK generation and retransmissions, RTS/CTS, duplicate detection,
 * fragmentation, aggregation, block acknowledgment, rate selection, and more.
 */
class INET_API IUpperMac
{
    public:
        // from container MAC module:
        virtual void upperFrameReceived(Ieee80211DataOrMgmtFrame *frame) = 0;

        // from Rx:
        virtual void lowerFrameReceived(Ieee80211Frame *frame) = 0;
        virtual void corruptedFrameReceived() = 0;

        // from Tx:
        virtual void channelAccessGranted(IContentionCallback *callback, int txIndex) = 0;
        virtual void internalCollision(IContentionCallback *callback, int txIndex) = 0;
        virtual void transmissionComplete(ITxCallback *callback) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

