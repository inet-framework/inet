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

#ifndef __MAC_IEEE80211MACTX_H_
#define __MAC_IEEE80211MACTX_H_

#include "Ieee80211MacPlugin.h"
#include "IIeee80211MacTx.h"

namespace inet {

namespace ieee80211 {

class IIeee80211MacContentionTx;
class IIeee80211MacImmediateTx;

#define MAX_NUM_CONTENTIONTX 4

class Ieee80211MacTx : public Ieee80211MacPlugin, public IIeee80211MacTx
{
    protected:
        int numContentionTx;
        IIeee80211MacContentionTx *contentionTx[MAX_NUM_CONTENTIONTX];
        IIeee80211MacImmediateTx *immediateTx = nullptr;

    protected:
        virtual void handleMessage(cMessage *msg) override {}

    public:
        Ieee80211MacTx(Ieee80211NewMac *mac, int numContentionTx);
        virtual ~Ieee80211MacTx();

        virtual void transmitContentionFrame(int txIndex, Ieee80211Frame *frame, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount, ICallback *completionCallback) override;
        virtual void transmitImmediateFrame(Ieee80211Frame *frame, simtime_t ifs, ICallback *completionCallback) override;

        virtual void mediumStateChanged(bool mediumFree) override;
        virtual void radioTransmissionFinished() override;
        virtual void lowerFrameReceived(bool isFcsOk) override;
};

}

} //namespace

#endif
