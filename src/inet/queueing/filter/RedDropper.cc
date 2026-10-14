//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/filter/RedDropper.h"

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/marker/EcnMarker.h"

namespace inet {
namespace queueing {

Define_Module(RedDropper);

void RedDropper::initialize(int stage)
{
    PacketFilterBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        wq = par("wq");
        if (wq < 0.0 || wq > 1.0)
            throw cRuntimeError("Invalid value for wq parameter: %g", wq);
        minth = par("minth");
        maxth = par("maxth");
        maxp = par("maxp");
        pkrate = par("pkrate");
        count = -1;
        randomValue = dblrand();
        WATCH(avg);
        WATCH(count);
        WATCH(markNext);
        WATCH(q_time);
        if (minth < 0.0)
            throw cRuntimeError("minth parameter must not be negative");
        if (maxth < 0.0)
            throw cRuntimeError("maxth parameter must not be negative");
        if (minth >= maxth)
            throw cRuntimeError("minth must be smaller than maxth");
        if (maxp < 0.0 || maxp > 1.0)
            throw cRuntimeError("Invalid value for maxp parameter: %g", maxp);
        if (pkrate < 0.0)
            throw cRuntimeError("Invalid value for pkrate parameter: %g", pkrate);
        useEcn = par("useEcn");
        packetCapacity = par("packetCapacity");
        if (maxth > packetCapacity)
            throw cRuntimeError("Warning: packetCapacity < maxth. Setting capacity to maxth");
        auto outputGate = gate("out");
        collection.reference(outputGate, false);
        if (!collection)
            collection.reference(this, "collectionModule", true);
        cModule * cm = check_and_cast<cModule *>(collection.get());
        cm->subscribe(packetPushedSignal, this);
        cm->subscribe(packetPulledSignal, this);
        cm->subscribe(packetRemovedSignal, this);
        cm->subscribe(packetDroppedSignal, this);
    }
}

RedDropper::RedDecision RedDropper::doRandomEarlyDetection(const Packet *packet) const
{
    RedDecision decision;
    decision.avg = avg;
    decision.count = count;
    decision.q_time = q_time;
    int queueLength = collection->getNumPackets();

    if (queueLength > 0) {
        // TD: This following calculation is only useful when the queue is not empty!
        decision.avg = (1 - wq) * avg + wq * queueLength;
    }
    else {
        // TD: Added behaviour for empty queue.
        const double m = SIMTIME_DBL(simTime() - q_time) * pkrate;
        decision.avg = pow(1 - wq, m) * avg;
        decision.q_time = simTime();
    }

    if (queueLength >= packetCapacity) { // maxth is also the "hard" limit
        decision.count = 0;
        decision.result = QUEUE_FULL;
    }
    else if (minth <= decision.avg && decision.avg < maxth) {
        decision.count = count + 1;
        const double pb = maxp * (decision.avg - minth) / (maxth - minth);
        const double pa = pb / (1 - decision.count * pb); // TD: Adapted to work as in [Floyd93].
        decision.usesRandomValue = true;
        if (randomValue < pa) {
            decision.count = 0;
            decision.result = RANDOMLY_ABOVE_LIMIT;
        }
        else
            decision.result = RANDOMLY_BELOW_LIMIT;
    }
    else if (decision.avg >= maxth) {
        decision.count = 0;
        decision.result = ABOVE_MAX_LIMIT;
    }
    else {
        decision.count = -1;
        decision.result = BELOW_MIN_LIMIT;
    }
    return decision;
}

bool RedDropper::matchesResult(RedResult result, const Packet *packet) const
{
    switch (result) {
        case RANDOMLY_ABOVE_LIMIT:
        case ABOVE_MAX_LIMIT:
            return useEcn && EcnMarker::getEcn(packet) != IP_ECN_NOT_ECT;
        case RANDOMLY_BELOW_LIMIT:
        case BELOW_MIN_LIMIT:
            return true;
        case QUEUE_FULL:
            return false;
        default:
            throw cRuntimeError("Unknown RED result");
    }
}

bool RedDropper::matchesPacket(const Packet *packet) const
{
    return matchesResult(doRandomEarlyDetection(packet).result, packet);
}

void RedDropper::handlePacketFiltered(Packet *packet, bool matches)
{
    // the same state as the query, so the same decision
    auto decision = doRandomEarlyDetection(packet);
    avg = decision.avg;
    count = decision.count;
    q_time = decision.q_time;
    lastResult = decision.result;
    switch (decision.result) {
        case QUEUE_FULL:
            EV_DEBUG << "Queue length >= capacity" << EV_FIELD(packetCapacity) << EV_ENDL;
            break;
        case RANDOMLY_ABOVE_LIMIT:
            EV_DEBUG << "Random early packet detected" << EV_FIELD(averageQueueLength, avg) << EV_ENDL;
            break;
        case ABOVE_MAX_LIMIT:
            EV_DEBUG << "Average queue length >= maxth" << EV_FIELD(averageQueueLength, avg) << EV_FIELD(maxth) << EV_ENDL;
            break;
        default:
            break;
    }
    if (decision.usesRandomValue)
        randomValue = dblrand();
    // a packet above the limit passes only to be marked
    if (matches && (decision.result == RANDOMLY_ABOVE_LIMIT || decision.result == ABOVE_MAX_LIMIT) && useEcn) {
        IpEcnCode ecn = EcnMarker::getEcn(packet);
        if (ecn != IP_ECN_NOT_ECT) {
            // if next packet should be marked and it is not
            if (markNext && ecn != IP_ECN_CE) {
                EcnMarker::setEcn(packet, IP_ECN_CE);
                markNext = false;
            }
            else {
                if (ecn == IP_ECN_CE)
                    markNext = true;
                else
                    EcnMarker::setEcn(packet, IP_ECN_CE);
            }
        }
    }
}

void RedDropper::dropPacket(Packet *packet)
{
    // an early drop is the congestion signal RED exists to give, a drop at the packet
    // capacity is a buffer overflow, as in PacketQueue
    if (lastResult == QUEUE_FULL)
        PacketFilterBase::dropPacket(packet, QUEUE_OVERFLOW);
    else if (lastResult == RANDOMLY_ABOVE_LIMIT || lastResult == ABOVE_MAX_LIMIT)
        PacketFilterBase::dropPacket(packet, CONGESTION);
    else
        PacketFilterBase::dropPacket(packet, OTHER_PACKET_DROP);
}

void RedDropper::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (signalID == packetPushedSignal || signalID == packetPulledSignal || signalID == packetRemovedSignal || signalID == packetDroppedSignal)
        q_time = simTime();
}

} // namespace queueing
} // namespace inet

