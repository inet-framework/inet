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

#ifndef __MAC_IEEE80211MACTRANSMISSION_H_
#define __MAC_IEEE80211MACTRANSMISSION_H_

#include "Ieee80211MacPlugin.h"
#include "inet/common/FSMA.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {

namespace ieee80211 {

class Ieee80211MacTransmission;

class ITransmissionCompleteCallback {  //or ITransmissionListener?
    public:
       virtual void transmissionComplete(Ieee80211MacTransmission *tx) = 0; //tx=nullptr if frame was transmitted by MAC itself (immediate frame!), not a tx process
};

class Ieee80211MacTransmission : public Ieee80211MacPlugin
{
    public:
        enum State {
            IDLE,
            DEFER,
            BACKOFF,
            WAIT_IFS,
            TRANSMIT
        };
        enum EventType { LOWER_FRAME, MEDIUM_STATE_CHANGED, TIMER, START_TRANSMISSION };

    protected:
        Ieee80211Frame *frame = nullptr;
        ITransmissionCompleteCallback *transmissionCompleteCallback = nullptr;
        simtime_t deferDuration = SIMTIME_ZERO;
        simtime_t eifs = SIMTIME_ZERO;
        int backoffSlots = 0;
        bool mediumFree = false;
        bool useEIFS = false;
        IRadio::TransmissionState transmissionState = IRadio::TRANSMISSION_STATE_UNDEFINED;
        cFSM fsm;
        /** End of the backoff period */
        cMessage *endBackoff = nullptr;
        /** End of the Data Inter-Frame Time period */
        cMessage *endIFS = nullptr;
        cMessage *endEIFS = nullptr;
        /** Remaining backoff period in seconds */
        simtime_t backoffPeriod = SIMTIME_ZERO;
        cMessage *frameDuration = nullptr;

    protected:
        void handleWithFSM(EventType event, cMessage *msg);
        void scheduleIFSPeriod(simtime_t deferDuration);
        void scheduleEIFSPeriod(simtime_t deferDuration);
        void updateBackoffPeriod();
        void scheduleBackoffPeriod(int backoffPeriod);
        void logState();
        void handleMessage(cMessage *msg);

    public:
        void transmitContentionFrame(Ieee80211Frame *frame, simtime_t ifs, simtime_t eifs, int cw, ITransmissionCompleteCallback *transmissionCompleteCallback);
        void mediumStateChanged(bool mediumFree);
        void transmissionStateChanged(IRadio::TransmissionState transmissionState);
        void lowerFrameReceived(bool isFcsOk); //TODO on receiving a frame with wrong FCS, we need to switch from DIFS to EIFS (ie. from ifs parameter to eifs parameter)!
        Ieee80211MacTransmission(Ieee80211NewMac *mac);
        ~Ieee80211MacTransmission();
};

}

} //namespace

#endif
