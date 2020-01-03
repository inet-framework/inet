//
// Copyright (C) 2011 OpenSim Ltd.
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
// @author Zoltan Bojthe
//

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/ResultFilters.h"
#include "inet/common/ResultRecorders.h"
#include "inet/common/Simsignals_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/physicallayer/base/packetlevel/FlatReceptionBase.h"
#include "inet/physicallayer/contract/packetlevel/SignalTag_m.h"

namespace inet {
namespace utils {
namespace filters {

Register_ResultFilter("dataAge", DataAgeFilter);

void DataAgeFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    if (auto packet = dynamic_cast<Packet *>(object)) {
        for (auto& region : packet->peekData()->getAllTags<CreationTimeTag>()) {
            WeightedHistogramRecorder::cWeight weight(region.getLength().get());
            fire(this, t, t - region.getTag()->getCreationTime(), &weight);
        }
    }
}

Register_ResultFilter("messageAge", MessageAgeFilter);

void MessageAgeFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    if (auto msg = dynamic_cast<cMessage *>(object))
        fire(this, t, t - msg->getCreationTime(), details);
}

Register_ResultFilter("messageTSAge", MessageTsAgeFilter);

void MessageTsAgeFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    if (auto msg = dynamic_cast<cMessage *>(object))
        fire(this, t, t - msg->getTimestamp(), details);
}

Register_ResultFilter("queueingTime", QueueingTimeFilter);

void QueueingTimeFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    if (auto msg = dynamic_cast<cMessage *>(object))
        fire(this, t, t - msg->getArrivalTime(), details);
}

Register_ResultFilter("receptionMinSignalPower", ReceptionMinSignalPowerFilter);

void ReceptionMinSignalPowerFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
#ifdef WITH_RADIO
    if (auto reception = dynamic_cast<inet::physicallayer::FlatReceptionBase *>(object)) {
        W minReceptionPower = reception->computeMinPower(reception->getStartTime(), reception->getEndTime());
        fire(this, t, minReceptionPower.get(), details);
    }
#endif  // WITH_RADIO
}

Register_ResultFilter("appPkSeqNo", ApplicationPacketSequenceNumberFilter);

void ApplicationPacketSequenceNumberFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    if (auto packet = dynamic_cast<Packet*>(object)) {
        if (auto applicationPacket = dynamicPtrCast<const ApplicationPacket>(packet->peekAtFront()))
            fire(this, t, (intval_t)applicationPacket->getSequenceNumber(), details);
    }
}


Register_ResultFilter("mobilityPos", MobilityPosFilter);

void MobilityPosFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    IMobility *module = dynamic_cast<IMobility *>(object);
    if (module) {
        Coord coord = module->getCurrentPosition();
        VoidPtrWrapper wrapper(&coord);
        fire(this, t, &wrapper, details);
    }
}

Register_ResultFilter("xCoord", XCoordFilter);

void XCoordFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    if (auto wrapper = dynamic_cast<VoidPtrWrapper *>(object))
        fire(this, t, ((Coord *)wrapper->getObject())->x, details);
}

Register_ResultFilter("yCoord", YCoordFilter);

void YCoordFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    if (auto wrapper = dynamic_cast<VoidPtrWrapper *>(object))
        fire(this, t, ((Coord *)wrapper->getObject())->y, details);
}

Register_ResultFilter("zCoord", ZCoordFilter);

void ZCoordFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    if (auto wrapper = dynamic_cast<VoidPtrWrapper *>(object))
        fire(this, t, ((Coord *)wrapper->getObject())->z, details);
}

Register_ResultFilter("sourceAddr", MessageSourceAddrFilter);

void MessageSourceAddrFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    if (auto *msg = dynamic_cast<Packet *>(object)) {
        L3AddressTagBase *addresses = msg->findTag<L3AddressReq>();
        if (!addresses)
            addresses = msg->findTag<L3AddressInd>();
        if (addresses != nullptr) {
            fire(this, t, addresses->getSrcAddress().str().c_str(), details);
        }
    }
}

Register_ResultFilter("throughput", ThroughputFilter);

void ThroughputFilter::emitThroughput(simtime_t endInterval, cObject *details)
{
    if (bytes == 0) {
        fire(this, endInterval, 0.0, details);
        lastSignal = endInterval;
    }
    else {
        double throughput = 8 * bytes / (endInterval - lastSignal).dbl();
        fire(this, endInterval, throughput, details);
        lastSignal = endInterval;
        bytes = 0;
        packets = 0;
    }
}

void ThroughputFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    if (auto packet = dynamic_cast<cPacket *>(object)) {
        const simtime_t now = simTime();
        packets++;
        if (packets >= packetLimit) {
            bytes += packet->getByteLength();
            emitThroughput(now, details);
        }
        else if (lastSignal + interval <= now) {
            emitThroughput(lastSignal + interval, details);
            if (emitIntermediateZeros) {
                while (lastSignal + interval <= now)
                    emitThroughput(lastSignal + interval, details);
            }
            else {
                if (lastSignal + interval <= now) { // no packets arrived for a long period
                    // zero should have been signaled at the beginning of this packet (approximation)
                    emitThroughput(now - interval, details);
                }
            }
            bytes += packet->getByteLength();
        }
        else
            bytes += packet->getByteLength();
    }
}

void ThroughputFilter::finish(cComponent *component, simsignal_t signalID)
{
    const simtime_t now = simTime();
    if (lastSignal < now) {
        cObject *details = nullptr;
        if (lastSignal + interval < now) {
            emitThroughput(lastSignal + interval, details);
            if (emitIntermediateZeros) {
                while (lastSignal + interval < now)
                    emitThroughput(lastSignal + interval, details);
            }
        }
        emitThroughput(now, details);
    }
}

Register_ResultFilter("liveThroughput", LiveThroughputFilter);

class TimerEvent : public cEvent
{
  protected:
     LiveThroughputFilter *target;
  public:
    TimerEvent(const char *name, LiveThroughputFilter *target) : cEvent(name), target(target) {}
    ~TimerEvent() {target->timerDeleted();}
    virtual cEvent *dup() const override {copyNotSupported(); return nullptr;}
    virtual cObject *getTargetObject() const override {return target;}
    virtual void execute() override {target->timerExpired();}
};

void LiveThroughputFilter::init(cComponent *component, cProperty *attrsProperty)
{
    cObjectResultFilter::init(component, attrsProperty);

    event = new TimerEvent("updateLiveThroughput", this);
    simtime_t now = simTime();
    event->setArrivalTime(now + interval);
    getSimulation()->getFES()->insert(event);
}

LiveThroughputFilter::~LiveThroughputFilter()
{
    if (event) {
        getSimulation()->getFES()->remove(event);
        delete event;
    }
}

void LiveThroughputFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    if (auto packet = dynamic_cast<cPacket *>(object))
        bytes += packet->getByteLength();
}

void LiveThroughputFilter::timerExpired()
{
    simtime_t now = simTime();
    double throughput = 8 * bytes / (now - lastSignal).dbl();
    fire(this, now, throughput, nullptr);
    lastSignal = now;
    bytes = 0;

    event->setArrivalTime(now + interval);
    getSimulation()->getFES()->insert(event);
}

void LiveThroughputFilter::timerDeleted()
{
    event = nullptr;
}

void LiveThroughputFilter::finish(cComponent *component, simsignal_t signalID)
{
    simtime_t now = simTime();
    if (lastSignal < now) {
        double throughput = 8 * bytes / (now - lastSignal).dbl();
        fire(this, now, throughput, nullptr);
    }
}


Register_ResultFilter("elapsedTime", ElapsedTimeFilter);

ElapsedTimeFilter::ElapsedTimeFilter()
{
    startTime = time(nullptr);
}

double ElapsedTimeFilter::getElapsedTime()
{
    time_t t = time(nullptr);
    return t - startTime;
}

void PacketDropReasonFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    if (check_and_cast<PacketDropDetails *>(details)->getReason() == reason)
        fire(this, t, object, details);
}

// TODO: replace these filters with a single filter that supports parameters when it becomes available in omnetpp
#define Register_PacketDropReason_ResultFilter(NAME, CLASS, REASON) class CLASS : public PacketDropReasonFilter { public: CLASS() { reason = REASON; } }; Register_ResultFilter(NAME, CLASS);
Register_PacketDropReason_ResultFilter("packetDropReasonIsUndefined", UndefinedPacketDropReasonFilter, -1);
Register_PacketDropReason_ResultFilter("packetDropReasonIsAddressResolutionFailed", AddressResolutionFailedPacketDropReasonFilter, ADDRESS_RESOLUTION_FAILED);
Register_PacketDropReason_ResultFilter("packetDropReasonIsForwardingDisabled", ForwardingDisabledPacketDropReasonFilter, FORWARDING_DISABLED);
Register_PacketDropReason_ResultFilter("packetDropReasonIsHopLimitReached", HopLimitReachedPacketDropReasonFilter, HOP_LIMIT_REACHED);
Register_PacketDropReason_ResultFilter("packetDropReasonIsIncorrectlyReceived", IncorrectlyReceivedPacketDropReasonFilter, INCORRECTLY_RECEIVED);
Register_PacketDropReason_ResultFilter("packetDropReasonIsInterfaceDown", InterfaceDownPacketDropReasonFilter, INTERFACE_DOWN);
Register_PacketDropReason_ResultFilter("packetDropReasonIsNoInterfaceFound", NoInterfaceFoundPacketDropReasonFilter, NO_INTERFACE_FOUND);
Register_PacketDropReason_ResultFilter("packetDropReasonIsNoRouteFound", NoRouteFoundPacketDropReasonFilter, NO_ROUTE_FOUND);
Register_PacketDropReason_ResultFilter("packetDropReasonIsNotAddressedToUs", NotAddressedToUsPacketDropReasonFilter, NOT_ADDRESSED_TO_US);
Register_PacketDropReason_ResultFilter("packetDropReasonIsQueueOverflow", QueueOverflowPacketDropReasonFilter, QUEUE_OVERFLOW);
Register_PacketDropReason_ResultFilter("packetDropReasonIsRetryLimitReached", RetryLimitReachedPacketDropReasonFilter, RETRY_LIMIT_REACHED);
Register_PacketDropReason_ResultFilter("packetDropReasonIsLifetimeExpired", LifetimeExpiredPacketDropReasonFilter, LIFETIME_EXPIRED);
Register_PacketDropReason_ResultFilter("packetDropReasonIsCongestion", CongestionPacketDropReasonFilter, CONGESTION);
Register_PacketDropReason_ResultFilter("packetDropReasonIsNoProtocolFound", NoProtocolFoundPacketDropReasonFilter, NO_PROTOCOL_FOUND);
Register_PacketDropReason_ResultFilter("packetDropReasonIsNoPortFound", NoPortFoundPacketDropReasonFilter, NO_PORT_FOUND);
Register_PacketDropReason_ResultFilter("packetDropReasonIsDuplicateDetected", DuplicateDetectedPacketDropReasonFilter, DUPLICATE_DETECTED);
Register_PacketDropReason_ResultFilter("packetDropReasonIsOther", OtherPacketDropReasonFilter, OTHER_PACKET_DROP);

Register_ResultFilter("minimumSnir", MinimumSnirFromSnirIndFilter);

void MinimumSnirFromSnirIndFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
#ifdef WITH_RADIO
    if (auto pk = dynamic_cast<Packet *>(object)) {
        auto tag = pk->findTag<SnirInd>();
        if (tag)
            fire(this, t, tag->getMinimumSnir(), details);
    }
#endif  // WITH_RADIO
}


Register_ResultFilter("packetErrorRate", PacketErrorRateFromErrorRateIndFilter);

void PacketErrorRateFromErrorRateIndFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
#ifdef WITH_RADIO
    if (auto pk = dynamic_cast<Packet *>(object)) {
        auto tag = pk->findTag<ErrorRateInd>();
        if (tag)
            fire(this, t, tag->getPacketErrorRate(), details);  //TODO isNaN?
    }
#endif  // WITH_RADIO
}


Register_ResultFilter("bitErrorRate", BitErrorRateFromErrorRateIndFilter);

void BitErrorRateFromErrorRateIndFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
#ifdef WITH_RADIO
    if (auto pk = dynamic_cast<Packet *>(object)) {
        auto tag = pk->findTag<ErrorRateInd>();
        if (tag)
            fire(this, t, tag->getBitErrorRate(), details);  //TODO isNaN?
    }
#endif  // WITH_RADIO
}


Register_ResultFilter("symbolErrorRate", SymbolErrorRateFromErrorRateIndFilter);

void SymbolErrorRateFromErrorRateIndFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
#ifdef WITH_RADIO
    if (auto pk = dynamic_cast<Packet *>(object)) {
        auto tag = pk->findTag<ErrorRateInd>();
        if (tag)
            fire(this, t, tag->getSymbolErrorRate(), details);  //TODO isNaN?
    }
#endif  // WITH_RADIO
}

Register_ResultFilter("localSignal", LocalSignalFilter);

void LocalSignalFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, bool b, cObject *details)
{
    fire(this, t, b, details);
}

void LocalSignalFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, intval_t l, cObject *details)
{
    fire(this, t, l, details);
}

void LocalSignalFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, uintval_t l, cObject *details)
{
    fire(this, t, l, details);
}

void LocalSignalFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, double d, cObject *details)
{
    fire(this, t, d, details);
}

void LocalSignalFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, const SimTime& v, cObject *details)
{
    fire(this, t, v, details);
}

void LocalSignalFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, const char *s, cObject *details)
{
    fire(this, t, s, details);
}

void LocalSignalFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    fire(this, t, object, details);
}

void LocalSignalFilter::receiveSignal(cComponent *source, simsignal_t signal, bool b, cObject *details)
{
    if (source == component)
        cResultListener::receiveSignal(source, signal, b, details);
}

void LocalSignalFilter::receiveSignal(cComponent *source, simsignal_t signal, intval_t l, cObject *details)
{
    if (source == component)
        cResultListener::receiveSignal(source, signal, l, details);
}

void LocalSignalFilter::receiveSignal(cComponent *source, simsignal_t signal, uintval_t l, cObject *details)
{
    if (source == component)
        cResultListener::receiveSignal(source, signal, l, details);
}

void LocalSignalFilter::receiveSignal(cComponent *source, simsignal_t signal, double d, cObject *details)
{
    if (source == component)
        cResultListener::receiveSignal(source, signal, d, details);
}

void LocalSignalFilter::receiveSignal(cComponent *source, simsignal_t signal, const SimTime& v, cObject *details)
{
    if (source == component)
        cResultListener::receiveSignal(source, signal, v, details);
}

void LocalSignalFilter::receiveSignal(cComponent *source, simsignal_t signal, const char *s, cObject *details)
{
    if (source == component)
        cResultListener::receiveSignal(source, signal, s, details);
}

void LocalSignalFilter::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    if (source == component)
        cResultListener::receiveSignal(source, signal, object, details);
}

} // namespace filters
} // namespace utils
} // namespace inet

