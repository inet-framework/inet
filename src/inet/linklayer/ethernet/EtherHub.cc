/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "inet/common/Simsignals.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ethernet/EtherHub.h"

namespace inet {

Define_Module(EtherHub);

inline std::ostream& operator<<(std::ostream& os, cMessage *msg)
{
    os << "(" << msg->getClassName() << ")" << msg->getFullName();
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const EtherHub::PortInfo& tr)
{
    os << "outId:" << tr.outgoingOrigId << ", numInIds:" << tr.forwardFromPorts.size() << ", collision:" << tr.outgoingCollision << ", start:" << tr.outgoingStartTime;
    return os;
}

EtherHub::~EtherHub()
{
    for (auto& gateInfo: portInfos) {
        delete gateInfo.incomingSignal;
    }
}

void EtherHub::initialize()
{
    numPorts = gateSize("ethg");
    inputGateBaseId = gateBaseId("ethg$i");
    outputGateBaseId = gateBaseId("ethg$o");
    portInfos.resize(numPorts);

    setTxUpdateSupport(true);

    numMessages = 0;
    WATCH(numMessages);
    WATCH_VECTOR(portInfos);

    // ensure we receive frames when their first bits arrive
    for (int i = 0; i < numPorts; i++)
        gate(inputGateBaseId + i)->setDeliverImmediately(true);
    subscribe(PRE_MODEL_CHANGE, this);    // for cPrePathCutNotification signal
    subscribe(POST_MODEL_CHANGE, this);    // we'll need to do the same for dynamically added gates as well

    checkConnections(true);
}

void EtherHub::checkConnections(bool errorWhenAsymmetric)
{
    int numActivePorts = 0;
    datarate = 0.0;
    dataratesDiffer = false;

    for (int i = 0; i < numPorts; i++) {
        cGate *igate = gate(inputGateBaseId + i);
        cGate *ogate = gate(outputGateBaseId + i);
        if (!igate->isConnected() && !ogate->isConnected())
            continue;

        if (!igate->isConnected() || !ogate->isConnected()) {
            // half connected gate
            if (errorWhenAsymmetric)
                throw cRuntimeError("The input or output gate not connected at port %i", i);
            dataratesDiffer = true;
            EV << "The input or output gate not connected at port " << i << ".\n";
            continue;
        }

        numActivePorts++;
        double drate = igate->getIncomingTransmissionChannel()->getNominalDatarate();

        if (numActivePorts == 1)
            datarate = drate;
        else if (datarate != drate) {
            if (errorWhenAsymmetric)
                throw cRuntimeError("The input datarate at port %i differs from datarates of previous ports", i);
            dataratesDiffer = true;
            EV << "The input datarate at port " << i << " differs from datarates of previous ports.\n";
        }

        cChannel *outTrChannel = ogate->getTransmissionChannel();
        drate = outTrChannel->getNominalDatarate();

        if (datarate != drate) {
            if (errorWhenAsymmetric)
                throw cRuntimeError("The output datarate at port %i differs from datarates of previous ports", i);
            dataratesDiffer = true;
            EV << "The output datarate at port " << i << " differs from datarates of previous ports.\n";
        }

        if (!outTrChannel->isSubscribed(POST_MODEL_CHANGE, this))
            outTrChannel->subscribe(POST_MODEL_CHANGE, this);
    }
}

void EtherHub::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();

    if (signalID == PRE_MODEL_CHANGE) {
        if (auto *notification = dynamic_cast<cPrePathCutNotification *>(obj)) {
            if ((this == notification->pathStartGate->getOwnerModule()) && (notification->pathStartGate->getBaseId() == outputGateBaseId)) {
                cutActiveTxOnPort(notification->pathStartGate->getId() - notification->pathStartGate->getBaseId());
            }
            else if ((this == notification->pathEndGate->getOwnerModule()) && (notification->pathEndGate->getBaseId() == inputGateBaseId)) {
                rxCutOnPort(notification->pathEndGate->getId() - notification->pathEndGate->getBaseId());
            }
            return;
        }
    }
    else if (signalID == POST_MODEL_CHANGE) {
        // if new gates have been added, we need to call setDeliverImmediately(true) on them
        if (auto *notification = dynamic_cast<cPostGateVectorResizeNotification *>(obj)) {
            if (strcmp(notification->gateName, "ethg") == 0) {
                int newSize = gateSize("ethg");
                portInfos.resize(newSize);
                for (int i = notification->oldSize; i < newSize; i++) {
                    gate(inputGateBaseId + i)->setDeliverImmediately(true);
                    copyIncomingsToPort(i);
                }
            }
            return;
        }
        else if (auto *notification = dynamic_cast<cPostPathCreateNotification *>(obj)) {
            if (((this == notification->pathStartGate->getOwnerModule()) && (notification->pathStartGate->getBaseId() == outputGateBaseId))
                    || ((this == notification->pathEndGate->getOwnerModule()) && (notification->pathEndGate->getBaseId() == inputGateBaseId)))
                checkConnections(false);
            if (this == notification->pathStartGate->getOwnerModule()) {
                int gateIdx = notification->pathStartGate->getId() - notification->pathStartGate->getBaseId();
                copyIncomingsToPort(gateIdx);
            }
            return;
        }
        else if (cPostPathCutNotification *notification = dynamic_cast<cPostPathCutNotification *>(obj)) {
            if ((this == notification->pathStartGate->getOwnerModule()) || (this == notification->pathEndGate->getOwnerModule()))
                checkConnections(false);
            return;
        }
        else if (cPostParameterChangeNotification *notification = dynamic_cast<cPostParameterChangeNotification *>(obj)) {
            cChannel *channel = dynamic_cast<cDatarateChannel *>(notification->par->getOwner());
            if (channel) {
                cGate *gate = channel->getSourceGate();
                if (gate->pathContains(this))
                    checkConnections(false);
                //TODO: call copyIncomingsToPort(gateIdx) when the channel parameter 'disable' changed from true to false
            }
            return;
        }
    }
}

void EtherHub::handleMessage(cMessage *msg)
{
    if (dataratesDiffer)
        checkConnections(true);

    EthernetSignalBase *signal = check_and_cast<EthernetSignalBase *>(msg);
    if (signal->getSrcMacFullDuplex() != false)
        throw cRuntimeError("Ethernet misconfiguration: MACs on the Ethernet HUB must be all in half-duplex mode, check it in module '%s'", signal->getSenderModule()->getFullPath().c_str());

    // Handle frame sent down from the network entity: send out on every other port
    int arrivalPort = msg->getArrivalGate()->getIndex();
    EV << "Frame " << msg << " arrived on port " << arrivalPort << ", broadcasting on all other ports\n";

    if (signal->isReceptionEnd()) {
        numMessages++;
        emit(packetReceivedSignal, msg);
    }

    if (numPorts <= 1) {
        delete msg;
        return;
    }

    if (signal->isUpdate()) {
        ASSERT(portInfos[arrivalPort].incomingOrigId == signal->getOrigPacketId());
        ASSERT(portInfos[arrivalPort].incomingSignal != nullptr);
        delete portInfos[arrivalPort].incomingSignal;
    }
    else {
        ASSERT(portInfos[arrivalPort].incomingOrigId == -1);
        portInfos[arrivalPort].incomingOrigId = signal->getId();
        ASSERT(portInfos[arrivalPort].incomingSignal == nullptr);
    }

    portInfos[arrivalPort].incomingSignal = signal;

    forwardSignalFrom(arrivalPort);
}

void EtherHub::cutSignalEnd(EthernetSignalBase* signal, simtime_t duration)
{
    signal->setDuration(duration);
    int64_t newBitLength = duration.dbl() * datarate;
    if (auto packet = check_and_cast_nullable<Packet*>(signal->decapsulate())) {
        //TODO: removed length calculation based on the PHY layer (parallel bits, bit order, etc.)
        if (newBitLength < packet->getBitLength()) {
            packet->trimFront();
            packet->setBackOffset(b(newBitLength));
            packet->trimBack();
            packet->setBitError(true);
        }
        signal->encapsulate(packet);
    }
    signal->setBitError(true);
    signal->setBitLength(newBitLength);
}

void EtherHub::rxCutOnPort(int inPort)
{
    if (portInfos[inPort].incomingSignal != nullptr) {
        simtime_t now = simTime();
        EthernetSignalBase *signal = portInfos[inPort].incomingSignal;
        simtime_t duration = now - signal->getArrivalTime();
        cutSignalEnd(signal, duration);
        signal->setRemainingDuration(SIMTIME_ZERO);
        forwardSignalFrom(inPort);
    }
}

void EtherHub::forwardSignalFrom(int arrivalPort)
{
    EthernetSignalBase *signal = portInfos[arrivalPort].incomingSignal;
    simtime_t now = simTime();

    for (int outPort = 0; outPort < numPorts; outPort++) {
        if (outPort != arrivalPort) {
            cGate *ogate = gate(outputGateBaseId + outPort);
            if (!ogate->isConnected())
                continue;

            if (portInfos[outPort].forwardFromPorts.empty()) {
                // new correct transmisssion started
                EthernetSignalBase *signalCopy = signal->dup();
                ASSERT(!ogate->getTransmissionChannel()->isBusy());
                ASSERT(signal->isReceptionStart());
                portInfos[outPort].forwardFromPorts.insert(arrivalPort);
                portInfos[outPort].outgoingOrigId = signalCopy->getId();
                portInfos[outPort].outgoingStartTime = now;
                portInfos[outPort].outgoingCollision = false;
                send(signalCopy, SendOptions().duration(signal->getDuration()), ogate);
            }
            else {
                portInfos[outPort].forwardFromPorts.insert(arrivalPort);
                if (!portInfos[outPort].outgoingCollision && portInfos[outPort].forwardFromPorts.size() == 1) {
                    ASSERT(now + signal->getRemainingDuration() - signal->getDuration() >= portInfos[outPort].outgoingStartTime);
                    // current single transmisssion updated
                    ASSERT(signal->isReceptionEnd() || ogate->getTransmissionChannel()->isBusy());
                    EthernetSignalBase *signalCopy = signal->dup();
                    send(signalCopy, SendOptions().updateTx(portInfos[outPort].outgoingOrigId).duration(signal->getDuration()), ogate);
                }
                else {
                    // collision
                    portInfos[outPort].outgoingCollision = true;
                    simtime_t newEnd = now;
                    for (auto inPort: portInfos[outPort].forwardFromPorts) {
                        simtime_t curEnd = portInfos[inPort].incomingSignal->getArrivalTime() + portInfos[inPort].incomingSignal->getRemainingDuration();
                        if (curEnd > newEnd)
                            newEnd = curEnd;
                    }
                    if (newEnd > now || portInfos[outPort].forwardFromPorts.size() == 1) {
                        // send when not signalEnd, or signalEnd and doesn't have any other signal (signal end send only when the all end arrived)
                        simtime_t duration = newEnd - portInfos[outPort].outgoingStartTime;
                        EthernetSignalBase *signalCopy = new EthernetSignalBase("collision");
                        signalCopy->setBitLength(duration.dbl() * datarate);
                        signalCopy->setBitrate(datarate);
                        signalCopy->setBitError(true);
                        send(signalCopy, SendOptions().updateTx(portInfos[outPort].outgoingOrigId).duration(duration), ogate);
                    }
                }
            }
            if (signal->isReceptionEnd()) {
                portInfos[outPort].forwardFromPorts.erase(arrivalPort);
                if (portInfos[outPort].forwardFromPorts.empty()) {
                    // transmisssion finished
                    portInfos[outPort].outgoingOrigId = -1;
                    portInfos[outPort].outgoingStartTime = now;
                    portInfos[outPort].outgoingCollision = false;
                }
            }
        }
    }
    if (signal->isReceptionEnd()) {
        portInfos[arrivalPort].incomingOrigId = -1;
        delete portInfos[arrivalPort].incomingSignal;
        portInfos[arrivalPort].incomingSignal = nullptr;
    }
}

void EtherHub::cutActiveTxOnPort(int outPort)
{
    if (portInfos[outPort].forwardFromPorts.size() == 0) {
        // no active TX, do nothing
        ASSERT(portInfos[outPort].outgoingOrigId == -1);
        return;
    }
    simtime_t now = simTime();
    cGate *ogate = gate(outputGateBaseId + outPort);
    simtime_t duration = now - portInfos[outPort].outgoingStartTime;
    EthernetSignalBase *signalCopy = nullptr;
    if (!portInfos[outPort].outgoingCollision && portInfos[outPort].forwardFromPorts.size() == 1) {
        // cut a single transmission:
        int arrivalPort = *(portInfos[outPort].forwardFromPorts.begin());
        ASSERT(ogate->getTransmissionChannel()->isBusy());
        signalCopy = portInfos[arrivalPort].incomingSignal->dup();
        cutSignalEnd(signalCopy, duration);
    }
    else {
        // collision
        ASSERT(ogate->getTransmissionChannel()->isBusy());
        ASSERT(portInfos[outPort].outgoingCollision);
        signalCopy = new EthernetSignalBase("collision");
        int64_t newBitLength = duration.dbl() * datarate;
        signalCopy->setBitLength(newBitLength);
    }
    signalCopy->setBitrate(datarate);
    signalCopy->setBitError(true);
    send(signalCopy, SendOptions().updateTx(portInfos[outPort].outgoingOrigId).duration(duration), ogate);
    // transmisssion finished
    portInfos[outPort].forwardFromPorts.clear();
    portInfos[outPort].outgoingOrigId = -1;
    portInfos[outPort].outgoingStartTime = now;
    portInfos[outPort].outgoingCollision = false;
}

void EtherHub::copyIncomingsToPort(int outPort)
{
    simtime_t now = simTime();
    simtime_t newEnd = now;
    ASSERT(portInfos[outPort].outgoingOrigId == -1);
    ASSERT(portInfos[outPort].outgoingStartTime <= now);
    for (int inPort = 0; inPort < numPorts; inPort++) {
        if (outPort != inPort && portInfos[inPort].incomingOrigId != -1) {
            simtime_t curEnd = portInfos[inPort].incomingSignal->getArrivalTime() + portInfos[inPort].incomingSignal->getRemainingDuration();
            if (curEnd > now)
                portInfos[outPort].forwardFromPorts.insert(inPort);
            if (curEnd > newEnd)
                newEnd = curEnd;
        }
    }
    if (newEnd > now) {
        //TODO if only one incoming exists, and it is started at current simtime, could forward it without error
        cGate *ogate = gate(outputGateBaseId + outPort);
        EthernetSignalBase *signalCopy = new EthernetSignalBase("noise");
        simtime_t duration = newEnd - now;
        portInfos[outPort].outgoingStartTime = now;
        portInfos[outPort].outgoingCollision = true;
        portInfos[outPort].outgoingOrigId = signalCopy->getId();
        signalCopy->setBitLength(duration.dbl() * datarate);
        signalCopy->setBitrate(datarate);
        signalCopy->setBitError(true);
        send(signalCopy, SendOptions().duration(duration), ogate);
    }
}

void EtherHub::finish()
{
    simtime_t t = simTime();
    recordScalar("simulated time", t);

    if (t > 0)
        recordScalar("messages/sec", numMessages / t);
}

} // namespace inet

