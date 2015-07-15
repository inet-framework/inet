//
// Copyright (C) 2015 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
// 

#ifndef __INET_IEEE80211MACDATAPUMP_H
#define __INET_IEEE80211MACDATAPUMP_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee80211/thenewmac/reception/Ieee80211MacChannelState.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"
#include "inet/linklayer/ieee80211/thenewmac/base/Ieee80211MacMacProcessBase.h"
#include "inet/common/RawPacket.h"
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacMacsorts.h"
#include "inet/linklayer/ieee80211/thenewmac/signals/Ieee80211MacSignals_m.h"
#include "inet/linklayer/ieee80211/thenewmac/Ieee80211NewFrame_m.h"

using namespace inet::physicallayer;

namespace inet {
namespace ieee80211 {

class INET_API IIeee80211MacDataPump
{
    protected:
        virtual void handleBusy() = 0;
        virtual void handleIdle() = 0;
        virtual void handleSlot() = 0;
        virtual void handleTxRequest(Ieee80211NewFrame *frame) = 0;
        virtual void handleResetMac() = 0;
        virtual void handlePhyTxStartConfirm() = 0;
        virtual void handlePhyTxEndConfirm() = 0;
        virtual void handlePhyDataConfirm() = 0;

        virtual void emitBusy() = 0;
        virtual void emitIdle() = 0;
        virtual void emitSlot() = 0;
        virtual void emitPhyTxStartRequest(int length, bps rate) = 0;

    public:
        virtual void emitResetMac() = 0;
};

/* This process sends an Mpdu to the Phy while generating &
 * appending the Fcs.
 */
class INET_API Ieee80211MacDataPump : public IIeee80211MacDataPump, public Ieee80211MacMacProcessBase
{
    protected:
        enum DataPumpState {
            DATA_PUMP_STATE_START,
            DATA_PUMP_STATE_TX_IDLE,
            DATA_PUMP_STATE_SEND_CRC,
            DATA_PUMP_STATE_SEND_FRAME,
            DATA_PUMP_STATE_WAIT_TX_END,
            DATA_PUMP_STATE_WAIT_TX_START,
            DATA_PUMP_STATE_INSERT_TIMESTAMP,
        };

    protected:
        DataPumpState state = DATA_PUMP_STATE_START;
        Ieee80211MacMacsorts *macsorts = nullptr;

        simtime_t dTx = SIMTIME_ZERO; // dcl dTx Duration
        const RawPacket *rawPdu = nullptr; // dcl pdu Frame
        Ieee80211NewFrame *pdu = nullptr;
        cGate *source = nullptr; // dcl source PId
        bps rate = bps(NaN); // dcl rate Octet
        int k = -1; // dcl k Integer
        int fcs = -1; // dcl fcs Crc
        int txLength = -1; // dcl txLength Integer
        cMessage *resetMac = nullptr; // TODO: move a base class?

        cGate *backoffProcedureGate = nullptr;

    public:
        virtual ~Ieee80211MacDataPump();

    protected:
        void processSignal(cMessage *msg);
        void initialize(int stage) override;

        virtual void sendOneByte();
        virtual void sendOneCrcByte();
        virtual void insertTimestamp();
        virtual int getFrameType() const;
        //virtual void handlePdu(const cPacket *pdu);

        // Incoming signals
        virtual void handleBusy() override;
        virtual void handleIdle() override;
        virtual void handleSlot() override;
        virtual void handleTxRequest(Ieee80211NewFrame *frame) override;
        virtual void handleResetMac() override;
        virtual void handlePhyTxStartConfirm() override;
        virtual void handlePhyDataConfirm() override;
        virtual void handlePhyTxEndConfirm() override;

        // Outgoing signals
        virtual void emitBusy() override;
        virtual void emitIdle() override;
        virtual void emitSlot() override;
        virtual void emitPhyTxStartRequest(int length, bps rate);

    public:
        virtual void emitResetMac() override { Ieee80211MacMacProcessBase::emitResetMac(); }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211MACDATAPUMP_H
