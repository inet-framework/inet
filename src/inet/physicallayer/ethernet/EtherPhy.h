//
// Copyright (C) OpenSimLtd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_ETHERPHY_H
#define __INET_ETHERPHY_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {
namespace physicallayer {

/*
 * Mac <-> Phy communication:
 *
 * Mac.send(cPacket) to Phy (start sending)
 * Phy.emit(txStarted)  // kell ez, vagy elég a TX_TRANSMITTING_STATE ?
 * Phy.emit(txFinished) // ebbol tudja a Mac, hogy kuldheti a kovetkezot. vagy elég a TX_IDLE_STATE?
 * Phy.emit(txAborted)  // honnan tudja majd a MAC, hogy dropolja, vagy ujrakuldje?
 *
 * Phy.send(cPacket) to Mac (data arrived)
 * Phy.emit(rxStarted)
 * Phy.emit(rxFinished)
 * Phy.emit(rxAborted)
 *
 * how to change Mac module the outgoing cPacket: abort now; for preemption; JAM, ... ?
 */

class INET_API EtherPhy : public cPhyModule, public cListener
{
  public:
    enum TxState : short{
        TX_INVALID_STATE = -1,
        TX_OFF_STATE = 0,
        TX_IDLE_STATE,
        TX_TRANSMITTING_STATE,
        TX_LAST = TX_TRANSMITTING_STATE
    };

    enum RxState : short{
        RX_INVALID_STATE = -1,
        RX_OFF_STATE = 0,
        RX_IDLE_STATE,
        RX_RECEIVING_STATE,
        RX_LAST = RX_RECEIVING_STATE
    };

  protected:
    // Self-message kind values
    enum SelfMsgKindValues {
        ENDTRANSMISSION = 101,
    };

    const char *displayStringTextFormat = nullptr;
    InterfaceEntry *interfaceEntry = nullptr;   // NIC module
    cChannel *rxTransmissionChannel = nullptr;    // rx transmission channel
    cChannel *txTransmissionChannel = nullptr;    // tx transmission channel
    cGate *physInGate = nullptr;    // pointer to the "phys$i" gate
    cGate *physOutGate = nullptr;    // pointer to the "phys$o" gate
    cGate *upperLayerInGate = nullptr;
    cMessage *endTxMsg = nullptr;
    EthernetSignalBase *curTx = nullptr;
    EthernetSignalBase *curRx = nullptr;
    double bitrate = NaN;
    bool   sendRawBytes = false;
    bool   duplexMode = true;
    bool   connected = false;    // true if connected to a network, set automatically by exploring the network configuration
    TxState txState = TX_INVALID_STATE;    // "transmit state" of the MAC
    RxState rxState = RX_INVALID_STATE;    // "receive state" of the MAC

    // statistics
    simtime_t lastRxStateChangeTime;
    simtime_t totalRxStateTime[RX_LAST + 1];    // total times by RxState
  public:
    static simsignal_t txStateChangedSignal;
    static simsignal_t txFinishedSignal;
    static simsignal_t txAbortedSignal;
    static simsignal_t rxStateChangedSignal;
    static simsignal_t connectionStateChangedSignal;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void handleParameterChange(const char *parname) override;

    void changeTxState(TxState newState);
    void changeRxState(RxState newState);

    EthernetSignal *encapsulate(Packet *packet);
    virtual simtime_t calculateDuration(EthernetSignalBase *signal);
    virtual void startTx(EthernetSignalBase *signal);
    virtual void endTx();
    virtual void abortTx();

    // overridden cPhyModule functions
    virtual void receivePacketStart(cPacket *packet) override;
    virtual void receivePacketProgress(cPacket *packet, int bitPosition, simtime_t timePosition, int extraProcessableBitLength, simtime_t extraProcessableDuration) override;
    virtual void receivePacketEnd(cPacket *packet) override;

    Packet *decapsulate(EthernetSignal *signal);
    virtual void startRx(EthernetSignalBase *signal);
    virtual void endRx(EthernetSignalBase *signal);
    virtual void abortRx();

    virtual void receiveSignal(cComponent *src, simsignal_t signalId, cObject *obj, cObject *details) override;
    bool checkConnected();
    virtual void handleConnected();
    virtual void handleDisconnected();

  public:
    virtual ~EtherPhy();
};

std::ostream& operator <<(std::ostream& o, EtherPhy::RxState);
std::ostream& operator <<(std::ostream& o, EtherPhy::TxState);

} // namespace physicallayer
} // namespace inet

#endif // ifndef __INET_ETHERPHY_H

