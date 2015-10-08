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

#ifndef __MAC_IIEEE80211MACTX_H_
#define __MAC_IIEEE80211MACTX_H_

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
               virtual void transmissionComplete(IIeee80211MacTx *tx) = 0; // tx=nullptr if frame was transmitted by MAC itself (immediate frame!), not a tx process
        };

        virtual void transmitContentionFrame(Ieee80211Frame *frame, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, int retryCount, ICallback *completionCallback) = 0;
        virtual void mediumStateChanged(bool mediumFree) = 0;
        virtual void transmissionFinished() = 0;
        virtual void lowerFrameReceived(bool isFcsOk) = 0;
};

}

} //namespace

#endif
