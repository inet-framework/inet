// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/linklayer/ieee80211/mesh/AirtimeLinkMetric.h"

#include <cmath>
#include <limits>
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"
#include "inet/linklayer/ieee80211/mesh/MeshDataAttempt.h"
#include "inet/linklayer/ieee80211/mesh/MeshPayloadTransmission.h"

namespace inet {
namespace ieee80211 {

Define_Module(AirtimeLinkMetric);

void AirtimeLinkMetric::initialize(int stage)
{
    OperationalBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        timing.reference(this,"hcfModule",true);
        mib.reference(this,"mibModule",true);
        WATCH_MAP(errorEstimates);
        WATCH_MAP(payloadEstimates);
    }
}

void AirtimeLinkMetric::handleMessageWhenUp(cMessage *) { throw cRuntimeError("Airtime metric uses typed direct contracts and attempt notifications"); }
void AirtimeLinkMetric::handleStartOperation(LifecycleOperation *)
{
    errorEstimates.clear();
    payloadEstimates.clear();
    check_and_cast<cModule *>(timing.get())->subscribe(Hcf::meshDataAttemptCompletedSignal,this);
    check_and_cast<cModule *>(timing.get())->subscribe(Hcf::meshPayloadTransmittedSignal,this);
}

void AirtimeLinkMetric::unsubscribeAttemptNotifications()
{
    if (timing) {
        auto module = check_and_cast<cModule *>(timing.get());
        if (module->isSubscribed(Hcf::meshDataAttemptCompletedSignal,this))
            module->unsubscribe(Hcf::meshDataAttemptCompletedSignal,this);
        if (module->isSubscribed(Hcf::meshPayloadTransmittedSignal,this))
            module->unsubscribe(Hcf::meshPayloadTransmittedSignal,this);
    }
}

AirtimeLinkMetric::~AirtimeLinkMetric() { unsubscribeAttemptNotifications(); }
void AirtimeLinkMetric::handleStopOperation(LifecycleOperation *) { unsubscribeAttemptNotifications(); errorEstimates.clear(); payloadEstimates.clear(); }
void AirtimeLinkMetric::handleCrashOperation(LifecycleOperation *) { unsubscribeAttemptNotifications(); errorEstimates.clear(); payloadEstimates.clear(); }

void AirtimeLinkMetric::receiveSignal(cComponent *, simsignal_t signal, cObject *, cObject *details)
{
    Enter_Method_Silent();
    if (operationalState != OPERATING)
        return;
    if (signal == Hcf::meshPayloadTransmittedSignal) {
        auto payload = check_and_cast<MeshPayloadTransmission *>(details);
        if (payload->msduCount < 1)
            throw cRuntimeError("Completed payload PPDU has no MSDUs");
        if (mib->isMeshPeerEstablished(payload->peer)) {
            auto updated = getPayloadEstimate(payload->peer) * 0.875 + payload->msduCount * 0.125;
            payloadEstimates[payload->peer] = updated;
        }
        return;
    }
    if (signal != Hcf::meshDataAttemptCompletedSignal)
        return;
    auto attempt = check_and_cast<MeshDataAttempt *>(details);
    if (mib->isMeshPeerEstablished(attempt->peer)) {
        // Local estimator policy: EWMA gain1/8, starting at zero. Direction is local->peer.
        auto& error = errorEstimates[attempt->peer];
        error = error * 0.875 + (attempt->acknowledged ? 0 : 0.125);
    }
}

double AirtimeLinkMetric::getErrorEstimate(MacAddress peer) const
{
    auto it = errorEstimates.find(peer);
    return it == errorEstimates.end() ? 0 : it->second;
}

double AirtimeLinkMetric::getPayloadEstimate(MacAddress peer) const
{
    auto it = payloadEstimates.find(peer);
    return it == payloadEstimates.end() ? 1 : it->second;
}

void AirtimeLinkMetric::resetAggregation(MacAddress peer)
{
    Enter_Method("resetAggregation");
    payloadEstimates.erase(peer);
}

void AirtimeLinkMetric::resetPeer(MacAddress peer)
{
    Enter_Method("resetPeer");
    errorEstimates.erase(peer);
    payloadEstimates.erase(peer);
}

uint32_t AirtimeLinkMetric::computeMetric(int metricId, simtime_t overhead, bps rate, double error, double n)
{
    double bitsPerSecond = rate.get();
    if ((metricId != 1 && metricId != 2) || overhead < SIMTIME_ZERO || !std::isfinite(bitsPerSecond) || bitsPerSecond <= 0
            || !std::isfinite(error) || error < 0 || error > 1 || !std::isfinite(n) || n < 1)
        throw cRuntimeError("Invalid airtime metric input");
    if (error == 1)
        return UINT32_MAX;
    // 14.10.2/Table14-4: n is fixed to1 for metric1; nominal Bt is8192bits.
    long double seconds = (static_cast<long double>(overhead.raw()) / SimTime::getScale()
        / (metricId == 1 ? 1 : n) + 8192.L / bitsPerSecond) / (1 - error);
    long double value = seconds * 100000000.L / (metricId == 1 ? 1024 : 1);
    // Preserve exact integer boundaries despite arithmetic roundoff; this is far
    // below a simulation tick expressed in either metric unit.
    auto integer = std::round(value);
    if (std::abs(value-integer) <= 8 * std::numeric_limits<long double>::epsilon() * std::max(1.L,std::abs(value)))
        value = integer;
    long double encoded = std::ceil(value);
    return encoded >= UINT32_MAX ? UINT32_MAX : std::max(uint32_t(1), uint32_t(encoded));
}

uint32_t AirtimeLinkMetric::getMetric(MacAddress peer)
{
    Enter_Method("getMetric");
    if (operationalState != OPERATING || !mib->isMeshPeerEstablished(peer))
        return UINT32_MAX;
    auto nominal = timing->getMeshLinkTiming(peer);
    return computeMetric(mib->getMeshConfiguration().pathSelectionMetricId,nominal.overhead,nominal.rate,getErrorEstimate(peer),getPayloadEstimate(peer));
}

bool AirtimeLinkMetric::supportsPeerRates(MacAddress peer, const Ieee80211RateSetState& rates)
{
    Enter_Method("supportsPeerRates");
    auto nominal = timing->getMeshLinkTiming(peer);
    // HT/VHT payload MCS support lives in the committed directional capability
    // record. The rates-only query still checks its legacy ACK dependency.
    bool high = nominal.dataMode->getHtMcsIndex() >= 0 || nominal.dataMode->getVhtMcsIndex() >= 0;
    return rates.supported.known && rates.operational.known
        && (high || (rates.supported.legacyRates.count(nominal.rate) && rates.operational.legacyRates.count(nominal.rate)))
        && rates.supported.legacyRates.count(nominal.ackMode->getDataMode()->getNetBitrate());
}

} // namespace ieee80211
} // namespace inet
