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

#include "inet/common/INETMath.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ethernet/EtherBus.h"

namespace inet {

Define_Module(EtherBus);

inline std::ostream& operator<<(std::ostream& os, cMessage *msg)
{
    os << "(" << msg->getClassName() << ")" << msg->getFullName();
    return os;
}

static std::ostream& operator<<(std::ostream& out, cPacket *msg)
{
    out << "(" << msg->getClassName() << ")" << msg->getFullName()
        << (msg->isReceptionStart() ? "-start" : msg->isReceptionEnd() ? "-end" : "")
        << " ("
        << (msg->getArrivalTime() + msg->getRemainingDuration() - msg->getDuration()).ustr()
        << ","
        << msg->getArrivalTime().ustr()
        << ","
        << (msg->getArrivalTime() + msg->getRemainingDuration()).ustr()
        << ")"
        ;
    return out;
}

inline std::ostream& operator<<(std::ostream& os, const EtherBus::BusTap& tr)
{
    os << "outId:" << tr.outgoingOrigId
       << ", inputs: {";
    for (auto& s: tr.outgoingSignals)
        os << s.first << ": " << s.second << ". ";
    os << "}"
       << ", collision:" << tr.outgoingCollision
       << ", start:" << tr.outgoingStartTime;
    return os;
}

EtherBus::EtherBus()
{
}

EtherBus::~EtherBus()
{
    for (auto& t: tap) {
        for (auto& s: t.outgoingSignals)
            delete s.second;
        delete t.incomingSignal;
    }
}

void EtherBus::initialize()
{
    numMessages = 0;
    WATCH(numMessages);
    setTxUpdateSupport(true);

    propagationSpeed = par("propagationSpeed");  //TODO there is a hardcoded propagation speed in EtherMACBase.cc -- use that?

    // initialize the positions where the hosts connects to the bus
    numTaps = gateSize("ethg");
    inputGateBaseId = gateBaseId("ethg$i");
    outputGateBaseId = gateBaseId("ethg$o");

    // read positions and check if positions are defined in order (we're lazy to sort...)
    std::vector<double> pos = cStringTokenizer(par("positions")).asDoubleVector();
    int numPos = pos.size();

    if (numPos > numTaps)
        EV << "Note: `positions' parameter contains more values (" << numPos << ") than "
                                                                                "the number of taps (" << numTaps << "), ignoring excess values.\n";
    else if (numPos < numTaps && numPos >= 2)
        EV << "Note: `positions' parameter contains less values (" << numPos << ") than "
                                                                                "the number of taps (" << numTaps << "), repeating distance between last 2 positions.\n";
    else if (numPos < numTaps && numPos < 2)
        EV << "Note: `positions' parameter contains too few values, using 5m distances.\n";

    tap.resize(numTaps);

    WATCH_VECTOR(tap);

    int i;
    double distance = numPos >= 2 ? pos[numPos - 1] - pos[numPos - 2] : 5;

    for (i = 0; i < numTaps; i++) {
        tap[i].id = i;
        tap[i].position = i < numPos ? pos[i] : i == 0 ? 5 : tap[i - 1].position + distance;
    }

    for (i = 0; i < numTaps - 1; i++) {
        if (tap[i].position > tap[i + 1].position)
            throw cRuntimeError("Tap positions must be ordered in ascending fashion, modify 'positions' parameter and rerun\n");
    }

    // Prints out data of parameters for parameter checking...
    EV << "Parameters of (" << getClassName() << ") " << getFullPath() << "\n";
    EV << "propagationSpeed: " << propagationSpeed << "\n";

    // Calculate propagation of delays between tap points on the bus
    for (i = 0; i < numTaps; i++) {
        // Propagation delay between adjacent tap points
        tap[i].propagationDelay[UPSTREAM] = (i > 0) ? tap[i - 1].propagationDelay[DOWNSTREAM] : 0;
        tap[i].propagationDelay[DOWNSTREAM] = (i + 1 < numTaps) ? (tap[i + 1].position - tap[i].position) / propagationSpeed : 0;
        EV << "tap[" << i << "] pos: " << tap[i].position
           << "  upstream delay: " << tap[i].propagationDelay[UPSTREAM]
           << "  downstream delay: " << tap[i].propagationDelay[DOWNSTREAM] << endl;
    }
    EV << "\n";

    // ensure we receive frames when their first bits arrive
    for (int i = 0; i < numTaps; i++)
        gate(inputGateBaseId + i)->setDeliverImmediately(true);
    subscribe(PRE_MODEL_CHANGE, this);    // for cPrePathCutNotification signal
    subscribe(POST_MODEL_CHANGE, this);    // we'll need to do the same for dynamically added gates as well

    checkConnections(true);
}

void EtherBus::checkConnections(bool errorWhenAsymmetric)
{
    int numActiveTaps = 0;
    datarate = 0.0;
    dataratesDiffer = false;

    for (int i = 0; i < numTaps; i++) {
        cGate *igate = gate(inputGateBaseId + i);
        cGate *ogate = gate(outputGateBaseId + i);
        if (!igate->isConnected() && !ogate->isConnected())
            continue;

        if (!igate->isConnected() || !ogate->isConnected()) {
            // half connected gate
            if (errorWhenAsymmetric)
                throw cRuntimeError("The input or output gate not connected at tap %i", i);
            dataratesDiffer = true;
            EV << "The input or output gate not connected at tap " << i << ".\n";
            continue;
        }

        numActiveTaps++;
        double drate = igate->getIncomingTransmissionChannel()->getNominalDatarate();

        if (numActiveTaps == 1)
            datarate = drate;
        else if (datarate != drate) {
            if (errorWhenAsymmetric)
                throw cRuntimeError("The input datarate at tap %i differs from datarates of previous taps", i);
            dataratesDiffer = true;
            EV << "The input datarate at tap " << i << " differs from datarates of previous taps.\n";
        }

        cChannel *outTrChannel = ogate->getTransmissionChannel();
        drate = outTrChannel->getNominalDatarate();

        if (datarate != drate) {
            if (errorWhenAsymmetric)
                throw cRuntimeError("The output datarate at tap %i differs from datarates of previous taps", i);
            dataratesDiffer = true;
            EV << "The output datarate at tap " << i << " differs from datarates of previous taps.\n";
        }

        if (!outTrChannel->isSubscribed(POST_MODEL_CHANGE, this))
            outTrChannel->subscribe(POST_MODEL_CHANGE, this);
    }
}

void EtherBus::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();

    if (signalID == PRE_MODEL_CHANGE) {
        if (auto *notification = dynamic_cast<cPrePathCutNotification *>(obj)) {
            if ((this == notification->pathStartGate->getOwnerModule()) && (notification->pathStartGate->getBaseId() == outputGateBaseId)) {
                cutActiveTxOnTap(notification->pathStartGate->getId() - notification->pathStartGate->getBaseId());
            }
            else if ((this == notification->pathEndGate->getOwnerModule()) && (notification->pathEndGate->getBaseId() == inputGateBaseId)) {
                rxCutOnTap(notification->pathEndGate->getId() - notification->pathEndGate->getBaseId());
            }
            return;
        }
    }
    else if (signalID == POST_MODEL_CHANGE) {
        // throw error if new gates have been added
        if (auto notif = dynamic_cast<cPostGateVectorResizeNotification *>(obj)) {
            if (strcmp(notif->gateName, "ethg") == 0)
                throw cRuntimeError("EtherBus does not allow adding/removing links dynamically");
        }

        if (auto notification = dynamic_cast<cPostPathCreateNotification *>(obj)) {
            if ((this == notification->pathStartGate->getOwnerModule()) || (this == notification->pathEndGate->getOwnerModule()))
                checkConnections(false);
            if (this == notification->pathStartGate->getOwnerModule()) {
                int tapIdx = notification->pathStartGate->getId() - notification->pathStartGate->getBaseId();
                copyIncomingsToTap(tapIdx);
            }
            return;
        }

        if (auto cutNotif = dynamic_cast<cPostPathCutNotification *>(obj)) {
            if ((this == cutNotif->pathStartGate->getOwnerModule()) || (this == cutNotif->pathEndGate->getOwnerModule()))
                checkConnections(false);
            return;
        }

        // note: we are subscribed to the channel object too
        if (auto parNotif = dynamic_cast<cPostParameterChangeNotification *>(obj)) {
            cChannel *channel = dynamic_cast<cDatarateChannel *>(parNotif->par->getOwner());
            if (channel) {
                cGate *gate = channel->getSourceGate();
                if (gate->pathContains(this))
                    checkConnections(false);
                //TODO: call copyIncomingsToTap(tapIdx) when the channel parameter 'disable' changed from true to false
            }
            return;
        }
    }
}

void EtherBus::handleMessage(cMessage *msg)
{
//    throw cRuntimeError("ETHERBUS not work");
    if (dataratesDiffer)
        checkConnections(true);

    auto signal = check_and_cast<EthernetSignalBase *>(msg);

    if (!signal->isSelfMessage()) {
        if (signal->getSrcMacFullDuplex() != false)
            throw cRuntimeError("Ethernet misconfiguration: MACs on the Ethernet BUS must be all in half-duplex mode, check it in module '%s'", signal->getSenderModule()->getFullPath().c_str());

        // Handle frame sent down from the network entity
        int tapPoint = signal->getArrivalGate()->getIndex();
        EV << "Frame " << signal << " arrived on tap " << tapPoint << endl;
        if (numTaps > 1) {
            delete tap[tapPoint].incomingSignal;
            tap[tapPoint].incomingSignal = signal;
            forwardSignalFrom(tapPoint);
            if (signal->isReceptionEnd()) {
                delete tap[tapPoint].incomingSignal;
                tap[tapPoint].incomingSignal = nullptr;
            }
        }
        else {
            // if there's only one tap, there's nothing to do
            delete signal;
        }
    }
    else {
        // handle upstream and downstream events
        int direction = signal->getKind();
        BusTap *thistap = (BusTap *)signal->getContextPointer();
        int tapPoint = thistap->id;
        signal->setContextPointer(nullptr);

        EV << "Event " << signal << " on tap " << tapPoint << ", sending out frame\n";

        // send out on gate
        bool isLast = (direction == UPSTREAM) ? (tapPoint == 0) : (tapPoint == numTaps - 1);
        EthernetSignalBase *signalCopy = isLast ? signal : signal->dup();
        long origId = signal->getOrigPacketId();
        if (origId == -1)
            origId = signal->getId();
        auto it = thistap->outgoingSignals.find(origId);
        if (it != thistap->outgoingSignals.end()) {
            delete it->second;
            it->second = signalCopy;
        }
        else {
            thistap->outgoingSignals.insert(std::pair<long, EthernetSignalBase*>(origId, signalCopy));
        }

        sendSignalOnTap(tapPoint, origId);

        // if not end of the bus, schedule for next tap
        if (isLast) {
            EV << "End of bus reached\n";
        }
        else {
            EV << "Scheduling for next tap\n";
            int nextTap = (direction == UPSTREAM) ? (tapPoint - 1) : (tapPoint + 1);
            signal->setContextPointer(&tap[nextTap]);
            scheduleAfter(tap[tapPoint].propagationDelay[direction], signal);
        }
    }
}

void EtherBus::sendSignalOnTap(int tapPoint, int incomingOrigId)
{
    simtime_t now = simTime();
    cGate *ogate = gate(outputGateBaseId + tapPoint);
    if (ogate->isConnected()) {
        if (tap[tapPoint].outgoingOrigId == -1)
            tap[tapPoint].outgoingStartTime = now;
        EthernetSignalBase *outSignal = mergeSignals(tapPoint);
        bool isEnd = outSignal->getRemainingDuration() == SIMTIME_ZERO;
        // send out on gate
        if (!isEnd || tap[tapPoint].outgoingSignals.size() == 1) {
            if (tap[tapPoint].outgoingOrigId == -1) {
                // send a new signal
                tap[tapPoint].outgoingOrigId = outSignal->getId();
                EV_DEBUG << "Send a new signal " << outSignal << endl;
                send(outSignal, SendOptions().duration(outSignal->getDuration()), ogate);
            }
            else {
                // update signal
                EV_DEBUG << "Send an update signal " << outSignal << endl;
                send(outSignal, SendOptions().updateTx(tap[tapPoint].outgoingOrigId, outSignal->getRemainingDuration()).duration(outSignal->getDuration()), ogate);
            }
        }
        else {
            EV_DEBUG << "drop outsignal " << outSignal << endl;
            delete outSignal;   // send packet end when the all end arrived
        }
    }

    auto incomingSignal = tap[tapPoint].outgoingSignals.at(incomingOrigId);
    if (incomingSignal->getArrivalTime() + incomingSignal->getRemainingDuration() <= now) {
        delete incomingSignal;
        tap[tapPoint].outgoingSignals.erase(incomingOrigId);
    }
    if (tap[tapPoint].outgoingSignals.empty()) {
        tap[tapPoint].outgoingOrigId = -1;
        tap[tapPoint].outgoingStartTime = now;
        tap[tapPoint].outgoingCollision = false;
    }
}

void EtherBus::forwardSignalFrom(int tapPoint)
{
    long origPacketId = tap[tapPoint].incomingSignal->getOrigPacketId();
    if (origPacketId == -1)
        origPacketId = tap[tapPoint].incomingSignal->getId();
    // create upstream and downstream events
    if (tapPoint > 0) {
        // start UPSTREAM travel
        EthernetSignalBase *msg2 = tap[tapPoint].incomingSignal->dup();
        msg2->setOrigPacketId(origPacketId);
        msg2->setKind(UPSTREAM);
        msg2->setContextPointer(&tap[tapPoint - 1]);
        scheduleAfter(tap[tapPoint].propagationDelay[UPSTREAM], msg2);
    }

    if (tapPoint < numTaps - 1) {
        // start DOWNSTREAM travel
        EthernetSignalBase *msg2 = tap[tapPoint].incomingSignal->dup();
        msg2->setOrigPacketId(origPacketId);
        msg2->setKind(DOWNSTREAM);
        msg2->setContextPointer(&tap[tapPoint + 1]);
        scheduleAfter(tap[tapPoint].propagationDelay[DOWNSTREAM], msg2);
    }
}

EthernetSignalBase *EtherBus::mergeSignals(int tapIdx)
{
    simtime_t now = simTime();
    bool sendSimple = false;
    if (tap[tapIdx].outgoingSignals.size() == 1 && !tap[tapIdx].outgoingCollision) {
        EthernetSignalBase *s = tap[tapIdx].outgoingSignals.begin()->second;
        if (s->getArrivalTime() + s->getRemainingDuration() - s->getDuration() == tap[tapIdx].outgoingStartTime)
            sendSimple = true;
    }
    EthernetSignalBase *signal = nullptr;
    if (sendSimple) {
        signal = tap[tapIdx].outgoingSignals.begin()->second->dup();
        signal->setRemainingDuration(signal->getArrivalTime() + signal->getRemainingDuration() - now);
    }
    else {
        signal = new EthernetSignalBase();
        std::string name = "collision-" + std::to_string(tap[tapIdx].outgoingOrigId != -1 ? tap[tapIdx].outgoingOrigId : signal->getId());
        signal->setName(name.c_str());
        tap[tapIdx].outgoingCollision = true;
        simtime_t startTime = tap[tapIdx].outgoingStartTime;
        simtime_t endTime;
        for (auto elem: tap[tapIdx].outgoingSignals) {
            simtime_t t = elem.second->getArrivalTime() + elem.second->getRemainingDuration();
            ASSERT(t >= now);
            if (t > endTime)
                endTime = t;
        }
        signal->setArrivalTime(now);
        signal->setDuration(endTime - startTime);
        signal->setRemainingDuration(endTime - now);
        signal->setBitError(true);
        int64_t newBitLength = (endTime - startTime).dbl() * datarate;
        signal->setBitLength(newBitLength);
        signal->setBitrate(datarate);
    }
    // The merged signal generated: (inet::EthernetSignalBase)collision-95-end (9.996675ms,10.000075ms,10.000075ms)
    // from outId:95,
    // inputs: {82: (inet::EthernetSignal)pk-13-1-end (10.000075ms,10.003475ms,10.003475ms).
    //          88: (inet::EthernetSignal)pk-45-1 (10.000075ms,10.0002ms,10.003475ms). },
    // collision:1, start:0.010000075
    EV << "The merged signal generated: " << signal << " from " << tap[tapIdx] << endl;
    return signal;
}

void EtherBus::cutSignalEnd(EthernetSignalBase* signal, simtime_t duration)
{
    signal->setRemainingDuration(signal->getArrivalTime() + duration - simTime());
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

void EtherBus::rxCutOnTap(int inTapIdx)
{
    if (tap[inTapIdx].incomingSignal != nullptr) {
        simtime_t now = simTime();
        EthernetSignalBase *signal = tap[inTapIdx].incomingSignal;
        simtime_t duration = now - signal->getArrivalTime();
        cutSignalEnd(signal, duration);
        forwardSignalFrom(inTapIdx);
    }
}

void EtherBus::cutActiveTxOnTap(int outTapIdx)
{
    if (tap[outTapIdx].outgoingSignals.size() == 0) {
        // no active TX, do nothing
        ASSERT(tap[outTapIdx].outgoingOrigId == -1);
        return;
    }

    simtime_t now = simTime();
    cGate *ogate = gate(outputGateBaseId + outTapIdx);
    simtime_t duration = now - tap[outTapIdx].outgoingStartTime;
    EthernetSignalBase *signal = mergeSignals(outTapIdx);
    ASSERT(ogate->getTransmissionChannel()->isBusy());
    cutSignalEnd(signal, duration);
    EV_DEBUG << "Send a cut signal " << signal << endl;
    send(signal, SendOptions().updateTx(tap[outTapIdx].outgoingOrigId).duration(duration), ogate);

    // transmisssion finished, clear outgoing infos
    for (auto elem: tap[outTapIdx].outgoingSignals)
        delete elem.second;
    tap[outTapIdx].outgoingSignals.clear();
    tap[outTapIdx].outgoingOrigId = -1;
    tap[outTapIdx].outgoingStartTime = now;
    tap[outTapIdx].outgoingCollision = false;
}

void EtherBus::copyIncomingsToTap(int outTapIdx)
{
    if (tap[outTapIdx].outgoingSignals.size() == 0) {
        // no active TX, do nothing
        return;
    }

    simtime_t now = simTime();
    tap[outTapIdx].outgoingStartTime = now;
    ASSERT(tap[outTapIdx].outgoingOrigId == -1);
    tap[outTapIdx].outgoingCollision = (tap[outTapIdx].outgoingSignals.size() > 1);
    cGate *ogate = gate(outputGateBaseId + outTapIdx);
    EthernetSignalBase *signal = mergeSignals(outTapIdx);
    ASSERT(signal->getDuration() == signal->getRemainingDuration());
    ASSERT(!ogate->getTransmissionChannel()->isBusy());
    if (signal->getRemainingDuration() == SIMTIME_ZERO) {
        EV_DEBUG << "drop outgoing signal start " << signal << endl;
        delete signal;  // do not send signalEnd, signalEnd will be sent when the signalEnd arrived
    }
    else {
        EV_DEBUG << "Send a current signal " << signal << endl;
        tap[outTapIdx].outgoingOrigId = signal->getId();
        send(signal, SendOptions().duration(signal->getDuration()), ogate);
    }
}

void EtherBus::finish()
{
    for (int i = 0; i < numTaps; i++)
        cutActiveTxOnTap(i);
    simtime_t t = simTime();
    recordScalar("simulated time", t);
    recordScalar("messages handled", numMessages);

    if (t > 0)
        recordScalar("messages/sec", numMessages / t);
}

} // namespace inet

