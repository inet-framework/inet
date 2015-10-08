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

#ifndef __INET_IIEEE80211MACTX_H
#define __INET_IIEEE80211MACTX_H

#include "Ieee80211MacPlugin.h"
#include "inet/common/FSMA.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class IIeee80211MacTx
{
    public:
        class ICallback {
            public:
               virtual void transmissionComplete(int txIndex) = 0; // -1: immediate tx
               virtual void internalCollision(int txIndex) = 0;  //TODO currently never called
        };

        virtual void transmitContentionFrame(int txIndex, Ieee80211Frame *frame, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount, ICallback *completionCallback) = 0;
        virtual void transmitImmediateFrame(Ieee80211Frame *frame, simtime_t ifs, ICallback *completionCallback) = 0;

        virtual void mediumStateChanged(bool mediumFree) = 0;
        virtual void radioTransmissionFinished() = 0;
        virtual void lowerFrameReceived(bool isFcsOk) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

