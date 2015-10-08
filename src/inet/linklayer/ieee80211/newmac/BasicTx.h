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

#ifndef __INET_BASICTX_H
#define __INET_BASICTX_H

#include "MacPlugin.h"
#include "ITx.h"

namespace inet {
namespace ieee80211 {

class IContentionTx;
class IImmediateTx;

#define MAX_NUM_CONTENTIONTX 4

class BasicTx : public cSimpleModule, public ITx
{
    protected:
        int numContentionTx;
        IContentionTx *contentionTx[MAX_NUM_CONTENTIONTX];
        IImmediateTx *immediateTx = nullptr;

    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage *msg) override;

    public:
        BasicTx();
        virtual ~BasicTx();

        virtual void transmitContentionFrame(int txIndex, Ieee80211Frame *frame, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount, ICallback *completionCallback) override;
        virtual void transmitImmediateFrame(Ieee80211Frame *frame, simtime_t ifs, ICallback *completionCallback) override;

        virtual void mediumStateChanged(bool mediumFree) override;
        virtual void radioTransmissionFinished() override;
        virtual void lowerFrameReceived(bool isFcsOk) override;
};

} // namespace ieee80211
} // namespace inet

#endif

