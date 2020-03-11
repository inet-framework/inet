//
// Copyright (C) OpenSim Ltd.
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

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
#include "inet/physicallayer/ethernet/EtherPhy.h"

namespace inet {
namespace physicallayer {

Define_Module(EtherPhy);

simsignal_t EtherPhy::txStateChangedSignal = registerSignal("txStateChanged");
simsignal_t EtherPhy::txFinishedSignal = registerSignal("txFinished");
simsignal_t EtherPhy::txAbortedSignal = registerSignal("txAborted");
simsignal_t EtherPhy::rxStateChangedSignal = registerSignal("rxStateChanged");

std::ostream& operator <<(std::ostream& o, EtherPhy::RxState s)
{
    switch (s) {
        case EtherPhy::RX_OFF_STATE: o << "OFF"; break;
        case EtherPhy::RX_IDLE_STATE: o << "IDLE"; break;
        case EtherPhy::RX_RECEIVING_STATE: o << "RX"; break;
        default: o << (int)s;
    }
    return o;
}

std::ostream& operator <<(std::ostream& o, EtherPhy::TxState s)
{
    switch (s) {
        case EtherPhy::TX_OFF_STATE: o << "OFF"; break;
        case EtherPhy::TX_IDLE_STATE: o << "IDLE"; break;
        case EtherPhy::TX_TRANSMITTING_STATE: o << "TX"; break;
        default: o << (int)s;
    }
    return o;
}

EtherPhy::~EtherPhy()
{
    cancelAndDelete(endTxMsg);
}

void EtherPhy::initialize(int stage)
{
    cPhyModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        physInGate = gate("phys$i");
        physOutGate = gate("phys$o");
        upperLayerInGate = gate("upperLayerIn");
        displayStringTextFormat = par("displayStringTextFormat");
        sendRawBytes = par("sendRawBytes");

        // initialize connected flag
        connected = physOutGate->getPathEndGate()->isConnected() && physInGate->getPathStartGate()->isConnected();
        if (!connected)
            EV_WARN << "PHY not connected to a network.\n";

        // initialize states
        txState = connected ? TX_IDLE_STATE : TX_OFF_STATE;
        rxState = connected ? RX_IDLE_STATE : RX_OFF_STATE;
        lastRxStateChangeTime = simTime();
        emit(txStateChangedSignal, static_cast<intval_t>(txState));
        emit(rxStateChangedSignal, static_cast<intval_t>(rxState));

        // initialize self messages
        endTxMsg = new cMessage("EndTransmission", ENDTRANSMISSION);

        for (int i = 0; i <= RX_LAST; i++)
            totalRxStateTime[i] = SIMTIME_ZERO;

        WATCH(rxState);
        WATCH(txState);

        subscribe(PRE_MODEL_CHANGE, this);
        subscribe(POST_MODEL_CHANGE, this);
    }
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {
        interfaceEntry = getContainingNicModule(this);
        bitrate = interfaceEntry->par("bitrate");   //TODO add a NED parameter reference when will be supported in OMNeT++
        duplexMode = interfaceEntry->par("duplexMode");   //TODO add a NED parameter reference when will be supported in OMNeT++
        if (!duplexMode)
            throw cRuntimeError("half-duplex currently not supported.");
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        transmissionChannel = physOutGate->findTransmissionChannel();
        if (transmissionChannel) {
            transmissionChannel->subscribe(POST_MODEL_CHANGE, this);
            if (transmissionChannel->hasPar("datarate")) {
                bitrate = transmissionChannel->par("datarate");
                interfaceEntry->par("bitrate").setDoubleValue(bitrate);
            }
        }
    }
}

void EtherPhy::finish()
{
    changeRxState(RX_OFF_STATE);
    simtime_t t(SIMTIME_ZERO);
    for (int i = 0; i <= RX_LAST; i++)
        t += totalRxStateTime[i];
    if (t > SIMTIME_ZERO) {
        recordScalar("rx channel idle (%)", 100 * (totalRxStateTime[RX_IDLE_STATE] / t));
        recordScalar("rx channel utilization (%)", 100 * (totalRxStateTime[RX_RECEIVING_STATE] / t));
        recordScalar("rx channel off (%)", 100 * (totalRxStateTime[RX_OFF_STATE] / t));
    }
}

void EtherPhy::changeTxState(TxState newState)
{
    if (newState != txState) {
        txState = newState;
        emit(txStateChangedSignal, static_cast<intval_t>(newState));
    }
}

void EtherPhy::changeRxState(RxState newState)
{
    simtime_t t = simTime();
    changeRxState(newState, t);
}

void EtherPhy::changeRxState(RxState newState, simtime_t t)
{
    ASSERT(t >= lastRxStateChangeTime);

    totalRxStateTime[rxState] += t - lastRxStateChangeTime;
    lastRxStateChangeTime = t;
    if (newState != rxState) {
        rxState = newState;
        emit(rxStateChangedSignal, static_cast<intval_t>(newState));
    }
}

void EtherPhy::handleMessage(cMessage *message)
{
    if (message->isSelfMessage()) {
        if (message == endTxMsg)
            endTx();
        else
            throw cRuntimeError("Unknown self message received!");
    }
    else if (connected) {
        if (message->getArrivalGate() == upperLayerInGate) {
            auto packet = check_and_cast<Packet *>(message);
            auto signal = encapsulate(packet);
            startTx(signal);
        }
        else if (message->getArrivalGate() == physInGate) {
            auto signal = check_and_cast<EthernetSignalBase *>(message);
            endRx(signal);
        }
        else
            throw cRuntimeError("Received unknown message");
    }
    else {
        EV_ERROR << "Message " << message << " arrived when PHY disconnected, dropped\n";
        delete message;
    }
}

bool EtherPhy::checkConnected()
{
    bool newConn = physOutGate->getPathEndGate()->isConnected() && physInGate->getPathStartGate()->isConnected();

    if (newConn) {
        auto outChannel = physOutGate->findTransmissionChannel();
        auto inChannel = physInGate->findIncomingTransmissionChannel();
        newConn = inChannel && outChannel && !inChannel->isDisabled() && !outChannel->isDisabled();
    }
    return newConn;
}

void EtherPhy::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();

//    MacProtocolBase::receiveSignal(source, signalID, obj, details);

    if (signalID == PRE_MODEL_CHANGE) {
        if (auto gcobj = dynamic_cast<cPrePathCutNotification *>(obj)) {
            if (connected && ((physOutGate == gcobj->pathStartGate) || (physInGate == gcobj->pathEndGate))) {
                disconnect();
            }
        }
        else if (auto gcobj = dynamic_cast<cPreParameterChangeNotification *>(obj)) {
            if (connected
                    && (gcobj->par->getOwner() == transmissionChannel || gcobj->par->getOwner() == physInGate->findIncomingTransmissionChannel())
                    && gcobj->par->getType() == cPar::BOOL
                    && strcmp(gcobj->par->getName(), "disabled") == 0
                    /* && gcobj->newValue == true */ //TODO the new value of parameter currently unavailable
                    ) {
                disconnect();
            }
        }
    }
    else if (signalID == POST_MODEL_CHANGE) {
        if (auto gcobj = dynamic_cast<cPostPathCreateNotification *>(obj)) {
            if ((physOutGate == gcobj->pathStartGate) || (physInGate == gcobj->pathEndGate)) {
                if (checkConnected())
                    connect();
            }
        }
        else if (auto gcobj = dynamic_cast<cPostParameterChangeNotification *>(obj)) {
            (void)gcobj;
            if (checkConnected())
                connect();
            else
                disconnect();
        }
    }
}

void EtherPhy::connect()
{
    if (!connected) {
        connected = true;
        transmissionChannel = physOutGate->getTransmissionChannel();
        if (!transmissionChannel->isSubscribed(POST_MODEL_CHANGE, this))
            transmissionChannel->subscribe(POST_MODEL_CHANGE, this);
        changeTxState(TX_IDLE_STATE);
        changeRxState(RX_IDLE_STATE);
        interfaceEntry->setCarrier(true);
        if (transmissionChannel->hasPar("datarate")) {
            bitrate = transmissionChannel->par("datarate");
            interfaceEntry->par("bitrate").setDoubleValue(bitrate);
        }
    }
}

void EtherPhy::disconnect()
{
    if (connected) {
        abortTx();
        abortRx();
        connected = false;
        transmissionChannel = nullptr;
        changeTxState(TX_OFF_STATE);
        changeRxState(RX_OFF_STATE);
        interfaceEntry->setCarrier(false);
    }
}

EthernetSignal *EtherPhy::encapsulate(Packet *packet)
{
    auto phyHeader = makeShared<EthernetPhyHeader>();
    packet->insertAtFront(phyHeader);
    packet->clearTags();
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ethernetPhy);
    auto signal = new EthernetSignal(packet->getName());
    signal->setSrcMacFullDuplex(duplexMode);
    signal->setBitrate(bitrate);
    signal->encapsulate(packet);
    return signal;
}

simtime_t EtherPhy::calculateDuration(EthernetSignalBase *signal)
{
    return signal->getBitLength() / signal->getBitrate();
}

void EtherPhy::startTx(EthernetSignalBase *signal)
{
    ASSERT(txState == TX_IDLE_STATE);
    ASSERT(curTx == nullptr);

    curTx = signal;
    auto duration = calculateDuration(signal);
    send(signal, physOutGate, duration);
    // sendPacketStart(signal, physOutGate, duration);
    ASSERT(transmissionChannel->getTransmissionFinishTime() == simTime() + duration);
    scheduleAt(simTime() + duration, endTxMsg);
    changeTxState(TX_TRANSMITTING_STATE);
}

void EtherPhy::endTx()
{
    ASSERT(txState == TX_TRANSMITTING_STATE);
    ASSERT(curTx != nullptr);
    emit(txFinishedSignal, 1);   //TODO
    curTx = nullptr;
    changeTxState(TX_IDLE_STATE);
}

void EtherPhy::abortTx()
{
    if (txState == TX_TRANSMITTING_STATE) {
        ASSERT(curTx != nullptr);
        ASSERT(endTxMsg->isScheduled());
        auto abortTime = simTime();
        transmissionChannel->forceTransmissionFinishTime(abortTime);
        cancelEvent(endTxMsg);
        emit(txAbortedSignal, 1);   //TODO
        curTx = nullptr;
    }
    else {
        ASSERT(curTx == nullptr);
        ASSERT(!endTxMsg->isScheduled());
    }
}

Packet *EtherPhy::decapsulate(EthernetSignal *signal)
{
    auto packet = check_and_cast<Packet *>(signal->decapsulate());
    delete signal;
    auto phyHeader = packet->popAtFront<EthernetPhyHeader>();
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
    return packet;
}

void EtherPhy::startRx(EthernetSignalBase *signal)
{
    // only the rx end received in full duplex mode
}

void EtherPhy::endRx(EthernetSignalBase *signal)
{
    if (signal->getSrcMacFullDuplex() != duplexMode)
        throw cRuntimeError("Ethernet misconfiguration: MACs on the same link must be all in full duplex mode, or all in half-duplex mode");
    if (signal->getBitrate() != bitrate)
        throw cRuntimeError("Ethernet misconfiguration: MACs on the same link must be same bitrate %f Mbps (sender:%s, %f Mbps)", bitrate/1e6, signal->getSenderModule()->getFullPath().c_str(), signal->getBitrate()/1e6);

    //KLUDGE: should set it with receptionOnStart or with receiveSignalStart
    if (rxState == RX_IDLE_STATE)
        changeRxState(RX_RECEIVING_STATE, simTime() - signal->getDuration());
    //KLUDGE end

    if (rxState == RX_RECEIVING_STATE) {
        auto packet = decapsulate(check_and_cast<EthernetSignal*>(signal));
        send(packet, "upperLayerOut");
        changeRxState(RX_IDLE_STATE);
    }
    else {
        //TODO
        delete signal;
    }
}

void EtherPhy::abortRx()
{
}

} // namespace physicallayer
} // namespace inet

