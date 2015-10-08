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

#ifndef __MAC_IEEE80211MACCONTENTIONTX_H_
#define __MAC_IEEE80211MACCONTENTIONTX_H_

#include "Ieee80211MacPlugin.h"
#include "IIeee80211MacContentionTx.h"

namespace inet {

namespace ieee80211 {

//TODO EDCA internal collisions should trigger retry (exp.backoff) in the lower pri tx process(es)
//TODO fsm is wrong wrt channelLastBusyTime (not all cases handled)
class Ieee80211MacContentionTx : public Ieee80211MacPlugin, public IIeee80211MacContentionTx
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
        int txIndex;

        // current transmission's parameters
        Ieee80211Frame *frame = nullptr;
        simtime_t ifs = SIMTIME_ZERO;
        simtime_t eifs = SIMTIME_ZERO;
        int cwMin = 0;
        int cwMax = 0;
        simtime_t slotTime;
        int retryCount = 0;
        ICallback *completionCallback = nullptr;

        simtime_t channelLastBusyTime = SIMTIME_ZERO;  //TODO lastChannelStateChangeTime?
        int backoffSlots = 0;
        bool mediumFree = false;
        bool useEIFS = false;
        IRadio::TransmissionState transmissionState = IRadio::TRANSMISSION_STATE_UNDEFINED;

        cFSM fsm;
        cMessage *endBackoff = nullptr;
        cMessage *endIFS = nullptr;
        cMessage *endEIFS = nullptr;
        simtime_t backoffPeriod = SIMTIME_ZERO;
        cMessage *frameDuration = nullptr;

    protected:
        virtual int computeCW(int cwMin, int cwMax, int retryCount);
        void handleWithFSM(EventType event, cMessage *msg);
        void scheduleIFS();
        void scheduleIFSPeriod(simtime_t deferDuration);
        void scheduleEIFSPeriod(simtime_t deferDuration);
        void updateBackoffPeriod();
        void scheduleBackoffPeriod(int backoffPeriod);
        void logState();
        void handleMessage(cMessage *msg);
        bool isIFSNecessary();

    public:
        Ieee80211MacContentionTx(Ieee80211NewMac *mac, int txIndex);
        ~Ieee80211MacContentionTx();

        //TODO also add a switchToReception() method? because switching takes time, so we dont automatically switch to tx after completing a transmission! (as we may want to transmit immediate frames afterwards)
        virtual void transmitContentionFrame(Ieee80211Frame *frame, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount, ICallback *completionCallback) override;

        virtual void mediumStateChanged(bool mediumFree) override;
        virtual void radioTransmissionFinished() override;
        virtual void lowerFrameReceived(bool isFcsOk) override;
};

}

} //namespace

#endif
