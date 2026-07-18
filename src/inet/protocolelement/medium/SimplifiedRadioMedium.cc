//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/medium/SimplifiedRadioMedium.h"

#include "inet/common/PacketEventTag.h"
#include "inet/common/Simsignals.h"
#include "inet/common/TimeTag.h"

namespace inet {

Define_Module(SimplifiedRadioMedium);

void SimplifiedRadioMedium::initialize()
{
    numRadios = gateSize("radio");
    inputGateBaseId = gateBaseId("radio$i");
    outputGateBaseId = gateBaseId("radio$o");
    packetLossProbability = par("packetLossProbability");
    parseRanges(par("ranges"));

    WATCH(numRadios);
    WATCH(numMessages);

    subscribe(POST_MODEL_CHANGE, this);
    setTxUpdateSupport(true);
    setGateModes();
    setChannelModes();
}

// "i j; k l; ..." symmetric adjacency; empty => fully connected (every pair in range).
void SimplifiedRadioMedium::parseRanges(const char *rangesString)
{
    inRange.assign(numRadios, std::set<int>());
    if (strlen(rangesString) == 0) {
        for (int i = 0; i < numRadios; i++)
            for (int j = 0; j < numRadios; j++)
                if (i != j)
                    inRange[i].insert(j);
        return;
    }
    cStringTokenizer entryTokenizer(rangesString, ";");
    while (entryTokenizer.hasMoreTokens()) {
        std::vector<std::string> fields = cStringTokenizer(entryTokenizer.nextToken()).asVector();
        if (fields.empty())
            continue;
        if (fields.size() != 2)
            throw cRuntimeError("%s: invalid range entry, expected 'radioIndex radioIndex'", getFullPath().c_str());
        int a = atoi(fields[0].c_str());
        int b = atoi(fields[1].c_str());
        if (a < 0 || a >= numRadios || b < 0 || b >= numRadios)
            throw cRuntimeError("%s: range entry radio index out of bounds", getFullPath().c_str());
        inRange[a].insert(b);
        inRange[b].insert(a);
    }
}

void SimplifiedRadioMedium::setGateModes()
{
    for (int i = 0; i < numRadios; i++)
        gate(inputGateBaseId + i)->setDeliverImmediately(true);
}

void SimplifiedRadioMedium::setChannelModes()
{
    for (int i = 0; i < numRadios; i++) {
        cGate *outGate = gate(outputGateBaseId + i);
        if (outGate->isConnected()) {
            cDatarateChannel *outTxChannel = check_and_cast<cDatarateChannel *>(outGate->getTransmissionChannel());
            outTxChannel->setMode(cDatarateChannel::MULTI);
        }
    }
}

void SimplifiedRadioMedium::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));
    ASSERT(signalID == POST_MODEL_CHANGE);
    if (auto notif = dynamic_cast<cPostPathCreateNotification *>(obj)) {
        if ((this == notif->pathStartGate->getOwnerModule()) || (this == notif->pathEndGate->getOwnerModule()))
            setChannelModes();
    }
}

void SimplifiedRadioMedium::handleMessage(cMessage *msg)
{
    cPacket *signal = check_and_cast<cPacket *>(msg);
    int arrivalRadio = signal->getArrivalGate()->getIndex();

    numMessages++;
    emit(packetReceivedSignal, signal);

    long incomingTxId = signal->getTransmissionId();
    for (int i = 0; i < numRadios; i++) {
        if (i == arrivalRadio || !areInRange(arrivalRadio, i))
            continue;
        cGate *ogate = gate(outputGateBaseId + i);
        if (!ogate->isConnected())
            continue;

        // simplified channel loss: drop the whole (non-update) transmission to this radio with the
        // configured probability. Updates of a dropped transmission then find no TxInfo and are skipped.
        if (!signal->isUpdate()) {
            if (packetLossProbability > 0 && dblrand() < packetLossProbability) {
                EV_INFO << "Dropping signal " << signal->getName() << " to radio " << i << " (channel loss)" << endl;
                continue;
            }
        }

        cPacket *outSignal = signal->dup();
        SendOptions sendOptions;
        sendOptions.duration(signal->getDuration());

        if (!signal->isUpdate()) {
            sendOptions.transmissionId(outSignal->getId());
            addTxInfo(signal->getTransmissionId(), i, outSignal->getId(), simTime() + signal->getDuration());
        }
        else {
            TxInfo *tx = findTxInfo(incomingTxId, i);
            if (!tx) { // start was dropped/missed -> ignore this update
                delete outSignal;
                continue;
            }
            updateTxInfo(tx, simTime() + signal->getRemainingDuration());
            sendOptions.updateTx(tx->outgoingTxId, signal->getRemainingDuration());
        }

        if (auto channel = dynamic_cast<cDatarateChannel *>(ogate->findTransmissionChannel())) {
            auto packet = check_and_cast_nullable<Packet *>(outSignal->getEncapsulatedPacket());
            if (packet != nullptr) {
                insertPacketEvent(this, packet, PEK_PROPAGATED, 0, channel->getDelay());
                increaseTimeTag<PropagationTimeTag>(packet, channel->getDelay(), channel->getDelay());
            }
        }
        send(outSignal, sendOptions, ogate);
    }
    delete signal;
}

void SimplifiedRadioMedium::addTxInfo(long incomingTxId, int port, long outgoingTxId, simtime_t finishTime)
{
    txList.push_back(TxInfo());
    TxInfo *tx = &txList.back();
    tx->incomingTxId = incomingTxId;
    tx->outgoingPort = port;
    tx->outgoingTxId = outgoingTxId;
    tx->finishTime = finishTime;
}

SimplifiedRadioMedium::TxInfo *SimplifiedRadioMedium::findTxInfo(long incomingTxId, int port)
{
    int txIndex = -1;
    simtime_t now = getSimulation()->getSimTime();
    for (size_t i = 0; i < txList.size(); i++) {
        if (txList[i].finishTime < now) {
            txList[i] = txList.back();
            txList.pop_back();
            i--;
        }
        else if (incomingTxId == txList[i].incomingTxId && port == txList[i].outgoingPort)
            txIndex = i;
    }
    return (txIndex == -1) ? nullptr : &txList[txIndex];
}

} // namespace inet
