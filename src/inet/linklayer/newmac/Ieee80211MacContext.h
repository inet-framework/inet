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

#ifndef IEEE80211MACCONTEXT_H_
#define IEEE80211MACCONTEXT_H_

#include "IIeee80211MacContext.h"
#include "Ieee80211NewMac.h"

namespace inet {

namespace physicallayer {
class Ieee80211ModeSet;
class Ieee80211Mode;
}

namespace ieee80211 {

class INET_API Ieee80211MacContext : public IIeee80211MacContext
{
    private:
        MACAddress address;
        const Ieee80211ModeSet *modeSet = nullptr;
        const IIeee80211Mode *dataFrameMode = nullptr;
        const IIeee80211Mode *basicFrameMode = nullptr;
        const IIeee80211Mode *controlFrameMode = nullptr;
        int shortRetryLimit;
        int rtsThreshold;

    public:
        Ieee80211MacContext(const MACAddress& address, const IIeee80211Mode *dataFrameMode,
                const IIeee80211Mode *basicFrameMode, const IIeee80211Mode *controlFrameMode,
                int shortRetryLimit, int rtsThreshold);
        virtual ~Ieee80211MacContext() {}

        virtual const MACAddress& getAddress() const;

        virtual simtime_t getSlotTime() const;
        virtual simtime_t getAIFS() const;
        virtual simtime_t getSIFS() const;
        virtual simtime_t getDIFS() const;
        virtual simtime_t getEIFS() const;
        virtual simtime_t getPIFS() const;
        virtual simtime_t getRIFS() const;

        virtual int getMinCW() const;
        virtual int getMaxCW() const;
        virtual int getShortRetryLimit();
        virtual int getRtsThreshold();

        virtual simtime_t getAckTimeout() const;
        virtual simtime_t getCtsTimeout() const;

        virtual Ieee80211RTSFrame *buildRtsFrame(Ieee80211DataOrMgmtFrame *frame);
        virtual Ieee80211CTSFrame *buildCtsFrame(Ieee80211RTSFrame *frame);
        virtual Ieee80211ACKFrame *buildAckFrame(Ieee80211DataOrMgmtFrame *frameToACK);
        virtual Ieee80211DataOrMgmtFrame *buildBroadcastFrame(Ieee80211DataOrMgmtFrame *frameToSend);

        virtual double computeFrameDuration(Ieee80211Frame *msg) const;
        virtual double computeFrameDuration(int bits, double bitrate) const;
        virtual Ieee80211Frame *setBasicBitrate(Ieee80211Frame *frame);
        virtual void setDataFrameDuration(Ieee80211DataOrMgmtFrame *frame);

        virtual bool isForUs(Ieee80211Frame *frame) const;
        virtual bool isBroadcast(Ieee80211Frame *frame) const;
        virtual bool isCts(Ieee80211Frame *frame) const;
        virtual bool isAck(Ieee80211Frame *frame) const;
};

}
} /* namespace inet */

#endif
