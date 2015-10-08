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

#ifndef IIEEE80211MACCONTEXT_H_
#define IIEEE80211MACCONTEXT_H_

#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MACAddress.h"

namespace inet {

namespace ieee80211 {

class Ieee80211Frame;
class Ieee80211DataOrMgmtFrame;
class Ieee80211ACKFrame;
class Ieee80211RTSFrame;
class Ieee80211CTSFrame;

class IIeee80211UpperMacContext
{
    public:
        IIeee80211UpperMacContext() {}
        virtual ~IIeee80211UpperMacContext() {}

        virtual const MACAddress& getAddress() const = 0;

        virtual simtime_t getSlotTime() const = 0;
        virtual simtime_t getAIFS() const = 0;
        virtual simtime_t getSIFS() const = 0;
        virtual simtime_t getDIFS() const = 0;
        virtual simtime_t getEIFS() const = 0;
        virtual simtime_t getPIFS() const = 0;
        virtual simtime_t getRIFS() const = 0;

        virtual int getMinCW() const = 0;
        virtual int getMaxCW() const = 0;
        virtual int getShortRetryLimit() = 0;
        virtual int getRtsThreshold() = 0;

        virtual simtime_t getAckTimeout() const = 0;
        virtual simtime_t getCtsTimeout() const = 0;

        virtual Ieee80211RTSFrame *buildRtsFrame(Ieee80211DataOrMgmtFrame *frame) = 0;
        virtual Ieee80211CTSFrame *buildCtsFrame(Ieee80211RTSFrame *frame) = 0;
        virtual Ieee80211ACKFrame *buildAckFrame(Ieee80211DataOrMgmtFrame *frameToACK) = 0;
        virtual Ieee80211DataOrMgmtFrame *buildBroadcastFrame(Ieee80211DataOrMgmtFrame *frameToSend) = 0;

        virtual double computeFrameDuration(Ieee80211Frame *msg) const = 0;
        virtual double computeFrameDuration(int bits, double bitrate) const = 0;
        virtual Ieee80211Frame *setBasicBitrate(Ieee80211Frame *frame) = 0;
        virtual void setDataFrameDuration(Ieee80211DataOrMgmtFrame *frame) = 0;

        virtual bool isForUs(Ieee80211Frame *frame) const = 0;
        virtual bool isBroadcast(Ieee80211Frame *frame) const = 0;
        virtual bool isCts(Ieee80211Frame *frame) const = 0;
        virtual bool isAck(Ieee80211Frame *frame) const = 0;
};

}
} /* namespace inet */

#endif
