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

#ifndef IEEE80211UPPERMACCONTEXT_H_
#define IEEE80211UPPERMACCONTEXT_H_

#include "IIeee80211UpperMacContext.h"
#include "Ieee80211NewMac.h"

namespace inet {

namespace physicallayer {
class Ieee80211ModeSet;
class Ieee80211Mode;
}

namespace ieee80211 {

class IIeee80211MacTx;

class INET_API Ieee80211UpperMacContext : public IIeee80211UpperMacContext
{
    private:
        MACAddress address;
        const Ieee80211ModeSet *modeSet = nullptr;
        const IIeee80211Mode *dataFrameMode = nullptr;
        const IIeee80211Mode *basicFrameMode = nullptr;
        const IIeee80211Mode *controlFrameMode = nullptr;
        int shortRetryLimit;
        int rtsThreshold;
        IIeee80211MacTx *tx;

    public:
        Ieee80211UpperMacContext(const MACAddress& address, const IIeee80211Mode *dataFrameMode,
                const IIeee80211Mode *basicFrameMode, const IIeee80211Mode *controlFrameMode,
                int shortRetryLimit, int rtsThreshold, IIeee80211MacTx *tx);
        virtual ~Ieee80211UpperMacContext() {}

        virtual const MACAddress& getAddress() const override;

        virtual simtime_t getSlotTime() const override;
        virtual simtime_t getAIFS() const override;
        virtual simtime_t getSIFS() const override;
        virtual simtime_t getDIFS() const override;
        virtual simtime_t getEIFS() const override;
        virtual simtime_t getPIFS() const override;
        virtual simtime_t getRIFS() const override;

        virtual int getMinCW() const override;
        virtual int getMaxCW() const override;
        virtual int getShortRetryLimit() const override;
        virtual int getRtsThreshold() const override;

        virtual simtime_t getAckTimeout() const override;
        virtual simtime_t getCtsTimeout() const override;

        virtual Ieee80211RTSFrame *buildRtsFrame(Ieee80211DataOrMgmtFrame *frame) const override;
        virtual Ieee80211CTSFrame *buildCtsFrame(Ieee80211RTSFrame *frame) const override;
        virtual Ieee80211ACKFrame *buildAckFrame(Ieee80211DataOrMgmtFrame *frameToACK) const override;
        virtual Ieee80211DataOrMgmtFrame *buildBroadcastFrame(Ieee80211DataOrMgmtFrame *frameToSend) const override;

        virtual double computeFrameDuration(Ieee80211Frame *msg) const override;
        virtual double computeFrameDuration(int bits, double bitrate) const override;
        virtual Ieee80211Frame *setBasicBitrate(Ieee80211Frame *frame) const override;
        virtual void setDataFrameDuration(Ieee80211DataOrMgmtFrame *frame) const override;

        virtual bool isForUs(Ieee80211Frame *frame) const override;
        virtual bool isBroadcast(Ieee80211Frame *frame) const override;
        virtual bool isCts(Ieee80211Frame *frame) const override;
        virtual bool isAck(Ieee80211Frame *frame) const override;

        virtual void transmitContentionFrame(int txIndex, Ieee80211Frame *frame, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount, IIeee80211MacTx::ICallback *completionCallback) const override;
        virtual void transmitImmediateFrame(Ieee80211Frame *frame, simtime_t ifs, IIeee80211MacTx::ICallback *completionCallback) const override;
};

}
} /* namespace inet */

#endif
