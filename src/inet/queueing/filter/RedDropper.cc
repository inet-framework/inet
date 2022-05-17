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

RedDropper::RedResult RedDropper::doRandomEarlyDetection(const Packet *packet)
{
    int queueLength = collection->getNumPackets();

    if (queueLength > 0) {
        // TD: This following calculation is only useful when the queue is not empty!
        avg = (1 - wq) * avg + wq * queueLength;
    }
    else {
        // TD: Added behaviour for empty queue.
        const double m = SIMTIME_DBL(simTime() - q_time) * pkrate;
        avg = pow(1 - wq, m) * avg;
        q_time = simTime();
    }

    if (queueLength >= packetCapacity) { // maxth is also the "hard" limit
        EV_DEBUG << "Queue length >= capacity" << EV_FIELD(queueLength) << EV_FIELD(packetCapacity) << EV_ENDL;
        count = 0;
        return QUEUE_FULL;
    }
    else if (minth <= avg && avg < maxth) {
        count++;
        const double pb = maxp * (avg - minth) / (maxth - minth);
        const double pa = pb / (1 - count * pb); // TD: Adapted to work as in [Floyd93].
        if (dblrand() < pa) {
            EV_DEBUG << "Random early packet detected" << EV_FIELD(averageQueueLength, avg) << EV_FIELD(probability, pa) << EV_ENDL;
            count = 0;
            return RANDOMLY_ABOVE_LIMIT;
        }
        else
            return RANDOMLY_BELOW_LIMIT;
    }
    else if (avg >= maxth) {
        EV_DEBUG << "Average queue length >= maxth" << EV_FIELD(averageQueueLength, avg) << EV_FIELD(maxth) << EV_ENDL;
        count = 0;
        return ABOVE_MAX_LIMIT;
    }
    else {
        count = -1;
    }

    return BELOW_MIN_LIMIT;
}

bool RedDropper::matchesPacket(const Packet *packet) const
{
    lastResult = const_cast<RedDropper *>(this)->doRandomEarlyDetection(packet);
    switch (lastResult) {
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

void RedDropper::processPacket(Packet *packet)
{
    switch (lastResult) {
        case RANDOMLY_ABOVE_LIMIT:
        case ABOVE_MAX_LIMIT: {
            if (useEcn) {
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
        case RANDOMLY_BELOW_LIMIT:
        case BELOW_MIN_LIMIT:
        case QUEUE_FULL:
            break;
        default:
            throw cRuntimeError("Unknown RED result");
    }
}

void RedDropper::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (signalID == packetPushedSignal || signalID == packetPulledSignal || signalID == packetRemovedSignal || signalID == packetDroppedSignal)
        q_time = simTime();
}

} // namespace queueing
} // namespace inet

