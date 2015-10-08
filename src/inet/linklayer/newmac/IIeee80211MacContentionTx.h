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

#ifndef __MAC_IIEEE80211MACCONTENTIONTX_H_
#define __MAC_IIEEE80211MACCONTENTIONTX_H_

#include "inet/common/INETDefs.h"
#include "IIeee80211MacTx.h"

namespace inet {

namespace ieee80211 {

class Ieee80211Frame;

class IIeee80211MacContentionTx
{
    public:
        typedef IIeee80211MacTx::ICallback ICallback;
        virtual ~IIeee80211MacContentionTx() {}
        virtual void transmitContentionFrame(Ieee80211Frame *frame, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount, ICallback *completionCallback) = 0;
        virtual void mediumStateChanged(bool mediumFree) = 0;
        virtual void radioTransmissionFinished() = 0;
        virtual void lowerFrameReceived(bool isFcsOk) = 0;
};

}

} //namespace

#endif
