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

#ifndef __INET_UPPERMACCONTEXT_H
#define __INET_UPPERMACCONTEXT_H

#include "IUpperMacContext.h"
#include "IUpperMacContext.h"
#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

class IImmediateTx;
class IContentionTx;

class INET_API UpperMacContext : public cOwnedObject, public IUpperMacContext
{
    private:
        MACAddress address;
        const Ieee80211ModeSet *modeSet = nullptr;
        const IIeee80211Mode *dataFrameMode = nullptr;
        const IIeee80211Mode *basicFrameMode = nullptr;
        const IIeee80211Mode *controlFrameMode = nullptr;
        int shortRetryLimit;
        int rtsThreshold;
        IImmediateTx *immediateTx;
        IContentionTx **contentionTx;

    protected:
        Ieee80211Frame *setBitrate(Ieee80211Frame *frame, const IIeee80211Mode *mode) const;

    public:
        UpperMacContext(const MACAddress& address, const IIeee80211Mode *dataFrameMode,
                const IIeee80211Mode *basicFrameMode, const IIeee80211Mode *controlFrameMode,
                int shortRetryLimit, int rtsThreshold, IImmediateTx *immediateTx, IContentionTx **contentionTx);
        virtual ~UpperMacContext() {}

        virtual const char *getName() const override; // cObject
        virtual std::string info() const override; // cObject

        virtual const MACAddress& getAddress() const override;

        virtual simtime_t getSlotTime() const override;
        virtual simtime_t getAifsTime(int accessCategory) const override;
        virtual simtime_t getSifsTime() const override;
        virtual simtime_t getDifsTime() const override;
        virtual simtime_t getEifsTime(int accessCategory) const override;
        virtual simtime_t getPifsTime() const override;
        virtual simtime_t getRifsTime() const override;

        virtual int getCwMin(int accessCategory) const override;
        virtual int getCwMax(int accessCategory) const override;
        virtual int getShortRetryLimit() const override;
        virtual int getRtsThreshold() const override;
        virtual simtime_t getTxopLimit(int accessCategory) const override;

        virtual simtime_t getAckTimeout() const override;
        virtual simtime_t getAckDuration() const override;
        virtual simtime_t getCtsTimeout() const override;
        virtual simtime_t getCtsDuration() const override;

        virtual Ieee80211RTSFrame *buildRtsFrame(Ieee80211DataOrMgmtFrame *frame) const override;
        virtual Ieee80211CTSFrame *buildCtsFrame(Ieee80211RTSFrame *frame) const override;
        virtual Ieee80211ACKFrame *buildAckFrame(Ieee80211DataOrMgmtFrame *frameToACK) const override;
        virtual Ieee80211DataOrMgmtFrame *buildBroadcastFrame(Ieee80211DataOrMgmtFrame *frameToSend) const override;

        virtual double computeFrameDuration(int bits, double bitrate) const override;
        virtual Ieee80211Frame *setBasicBitrate(Ieee80211Frame *frame) const override;
        virtual Ieee80211Frame *setDataBitrate(Ieee80211Frame *frame) const override;
        virtual Ieee80211Frame *setControlBitrate(Ieee80211Frame *frame) const override;

        virtual bool isForUs(Ieee80211Frame *frame) const override;
        virtual bool isMulticast(Ieee80211Frame *frame) const override;
        virtual bool isBroadcast(Ieee80211Frame *frame) const override;
        virtual bool isCts(Ieee80211Frame *frame) const override;
        virtual bool isAck(Ieee80211Frame *frame) const override;

        virtual void transmitContentionFrame(int txIndex, Ieee80211Frame *frame, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount, ITxCallback *completionCallback) const override;
        virtual void transmitImmediateFrame(Ieee80211Frame *frame, simtime_t ifs, ITxCallback *completionCallback) const override;
};

} // namespace ieee80211
} // namespace inet

#endif

