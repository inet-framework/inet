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

#ifndef __INET_BASICCONTENTIONTX_H
#define __INET_BASICCONTENTIONTX_H

#include "IContentionTx.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

class IUpperMac;
class IMacRadioInterface;

//TODO EDCA internal collisions should trigger retry (exp.backoff) in the lower pri tx process(es)
//TODO why do we need to schedule IFS and Backoff separately? why not compute waittime = ifs+backoff upfront, and schedule that?
//TODO fsm is wrong wrt channelLastBusyTime (not all cases handled) -- FIGURE OUT HOW TO HANDLE IT WHEN CHANNEL IS LONG FREE WHEN WE WANT TO TRANSMIT!
//TODO needs to be decided whether slots for different ACs are aligned; and if not, whether "within a slot time" is a good definition for collision!
//TODO NOTE: latter is interlinked with the prev question -- what if we could transmit immediately? we should wait for 1 slot time to make sure we don't collide with a higher priority process??
class BasicContentionTx : public cSimpleModule, public IContentionTx
{
    public:
        enum State {
            IDLE,
            DEFER,
            BACKOFF,
            WAIT_IFS,
            TRANSMIT
        };
        enum EventType { START, MEDIUM_STATE_CHANGED, TRANSMISSION_FINISHED, TIMER, FRAME_ARRIVED };

    protected:
        IMacRadioInterface *mac;
        IUpperMac *upperMac;
        int txIndex;

        // current transmission's parameters
        Ieee80211Frame *frame = nullptr;
        simtime_t ifs = SIMTIME_ZERO;
        simtime_t eifs = SIMTIME_ZERO;
        int cwMin = 0;
        int cwMax = 0;
        simtime_t slotTime;
        int retryCount = 0;
        ITxCallback *completionCallback = nullptr;

        simtime_t channelLastBusyTime = SIMTIME_ZERO;  //TODO lastChannelStateChangeTime?
        int backoffSlots = 0;
        bool mediumFree = false;
        bool useEIFS = false;
        IRadio::TransmissionState transmissionState = IRadio::TRANSMISSION_STATE_UNDEFINED;

        cFSM fsm;
        cMessage *endBackoff = nullptr;
        cMessage *endIFS = nullptr;
        cMessage *endEIFS = nullptr;

    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage *msg) override;

        virtual int computeCW(int cwMin, int cwMax, int retryCount);
        virtual void handleWithFSM(EventType event, cMessage *msg);
        virtual bool isIFSNecessary();
        virtual void scheduleIFS();
        virtual void scheduleIFSPeriod(simtime_t deferDuration);
        virtual void scheduleEIFSPeriod(simtime_t deferDuration);
        virtual void updateBackoffPeriod();
        virtual void scheduleBackoffPeriod(int backoffPeriod);
        virtual void transmissionComplete();
        virtual void logState();
        virtual void updateDisplayString();

    public:
        BasicContentionTx() {}
        ~BasicContentionTx();

        //TODO also add a switchToReception() method? because switching takes time, so we dont automatically switch to tx after completing a transmission! (as we may want to transmit immediate frames afterwards)
        virtual void transmitContentionFrame(Ieee80211Frame *frame, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount, ITxCallback *completionCallback) override;

        virtual void mediumStateChanged(bool mediumFree) override;
        virtual void radioTransmissionFinished() override;
        virtual void lowerFrameReceived(bool isFcsOk) override;
};

} // namespace ieee80211
} // namespace inet

#endif

