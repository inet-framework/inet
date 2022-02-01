//
// Copyright (C) 2020 Opensim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/physicallayer/wired/common/WireJunction.h"

#include "inet/common/Simsignals.h"

namespace inet {
namespace physicallayer {

Define_Module(WireJunction);

inline std::ostream& operator<<(std::ostream& os, cMessage *msg)
{
    os << "(" << msg->getClassName() << ")" << msg->getFullName();
    return os;
}

void WireJunction::initialize()
{
    auto gateNames = getGateNames();
    if (gateNames.size() != 1)
        throw cRuntimeError("Cannot find the gate vector");
    std::string gateName = gateNames[0];
    numPorts = gateSize(gateName.c_str());
    inputGateBaseId = gateBaseId((gateName + "$i").c_str());
    outputGateBaseId = gateBaseId((gateName + "$o").c_str());

    numMessages = 0;
    WATCH(numMessages);

    subscribe(POST_MODEL_CHANGE, this);

    setTxUpdateSupport(true);
    setGateModes();
    setChannelModes();
}

void WireJunction::setGateModes()
{
    for (int i = 0; i < numPorts; i++)
        gate(inputGateBaseId + i)->setDeliverImmediately(true);
}

void WireJunction::setChannelModes()
{
    for (int i = 0; i < numPorts; i++) {
        cGate *outGate = gate(outputGateBaseId + i);
        if (outGate->isConnected()) {
            cDatarateChannel *outTxChannel = check_and_cast<cDatarateChannel *>(outGate->getTransmissionChannel());
            outTxChannel->setMode(cDatarateChannel::MULTI);
        }
    }
}

void WireJunction::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    ASSERT(signalID == POST_MODEL_CHANGE);

    if (auto notif = dynamic_cast<cPostGateVectorResizeNotification *>(obj)) {
        if (notif->module == this)
            setGateModes();
        return;
    }

    if (auto notif = dynamic_cast<cPostPathCreateNotification *>(obj)) {
        if ((this == notif->pathStartGate->getOwnerModule()) || (this == notif->pathEndGate->getOwnerModule()))
            setChannelModes();
        return;
    }
}

void WireJunction::handleMessage(cMessage *msg)
{
    cPacket *signal = check_and_cast<cPacket *>(msg);

    // Handle frame sent down from the network entity: send out on every other port
    int arrivalPort = signal->getArrivalGate()->getIndex();
    EV << "Frame " << signal << " arrived on port " << arrivalPort << ", broadcasting on all other ports\n";

    numMessages++;
    emit(packetReceivedSignal, signal);

    if (numPorts <= 1) {
        delete signal;
        return;
    }

    long incomingTxId = signal->getTransmissionId();
    for (int i = 0; i < numPorts; i++) {
        if (i != arrivalPort) {
            cGate *ogate = gate(outputGateBaseId + i);
            if (!ogate->isConnected())
                continue;

            cPacket *outSignal = signal->dup();
            SendOptions sendOptions;
            sendOptions.duration(signal->getDuration());

            if (!signal->isUpdate()) {
                sendOptions.transmissionId(outSignal->getId());
                addTxInfo(signal->getTransmissionId(), i, outSignal->getId(), simTime() + signal->getDuration());
            }
            else {
                TxInfo *tx = findTxInfo(incomingTxId, i);
                if (!tx) { // we missed the original signal, ignore the update
                    delete outSignal;
                    continue;
                }
                updateTxInfo(tx, simTime() + signal->getRemainingDuration());
                sendOptions.updateTx(tx->outgoingTxId, signal->getRemainingDuration());
            }

            // send
            send(outSignal, sendOptions, ogate);
        }
    }
    delete signal;
}

void WireJunction::addTxInfo(long incomingTxId, int port, long outgoingTxId, simtime_t finishTime)
{
    txList.push_back(TxInfo());
    TxInfo *tx = &txList.back();
    tx->incomingTxId = incomingTxId;
    tx->outgoingPort = port;
    tx->outgoingTxId = outgoingTxId;
    tx->finishTime = finishTime;
}

WireJunction::TxInfo *WireJunction::findTxInfo(long incomingTxId, int port)
{
    // find transmission, purge expired ones
    int txIndex = -1;
    simtime_t now = getSimulation()->getSimTime();
    for (int i = 0; i < (int)txList.size(); i++) {
        if (txList[i].finishTime < now) {
            txList[i] = txList.back(); // no-op if txList[i] is the last item (i.e. txList.back())
            txList.pop_back();
            i--;
        }
        else if (incomingTxId == txList[i].incomingTxId && port == txList[i].outgoingPort)
            txIndex = i;
    }
    return (txIndex == -1) ? nullptr : &txList[txIndex];
}

} // namespace physicallayer
} // namespace inet

