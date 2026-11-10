// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/linklayer/ieee80211/mesh/AirtimeLinkMetric.h"

#include <cmath>
#include <limits>
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"
#include "inet/linklayer/ieee80211/mesh/MeshDataAttempt.h"

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
    }
}

void AirtimeLinkMetric::handleMessageWhenUp(cMessage *) { throw cRuntimeError("Airtime metric uses typed direct contracts and attempt notifications"); }
void AirtimeLinkMetric::handleStartOperation(LifecycleOperation *)
{
    errorEstimates.clear();
    check_and_cast<cModule *>(timing.get())->subscribe(Hcf::meshDataAttemptCompletedSignal,this);
}

void AirtimeLinkMetric::unsubscribeAttemptNotifications()
{
    if (timing) {
        auto module = check_and_cast<cModule *>(timing.get());
        if (module->isSubscribed(Hcf::meshDataAttemptCompletedSignal,this))
            module->unsubscribe(Hcf::meshDataAttemptCompletedSignal,this);
    }
}

AirtimeLinkMetric::~AirtimeLinkMetric() { unsubscribeAttemptNotifications(); }
void AirtimeLinkMetric::handleStopOperation(LifecycleOperation *) { unsubscribeAttemptNotifications(); errorEstimates.clear(); }
void AirtimeLinkMetric::handleCrashOperation(LifecycleOperation *) { unsubscribeAttemptNotifications(); errorEstimates.clear(); }

void AirtimeLinkMetric::receiveSignal(cComponent *, simsignal_t signal, cObject *, cObject *details)
{
    Enter_Method_Silent();
    if (signal != Hcf::meshDataAttemptCompletedSignal || operationalState != OPERATING)
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

void AirtimeLinkMetric::resetPeer(MacAddress peer)
{
    Enter_Method("resetPeer");
    errorEstimates.erase(peer);
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
    return computeMetric(mib->getMeshConfiguration().pathSelectionMetricId,nominal.overhead,nominal.rate,getErrorEstimate(peer));
}

bool AirtimeLinkMetric::supportsPeerRates(MacAddress peer, const Ieee80211RateSetState& rates)
{
    Enter_Method("supportsPeerRates");
    auto nominal = timing->getMeshLinkTiming(peer);
    // The C5 fixed OFDM selector returns the same catalog tuple independently
    // of optional peer rates. Reject a candidate that cannot use it; never adapt.
    return rates.supported.known && rates.operational.known
        && rates.supported.legacyRates.count(nominal.rate) && rates.operational.legacyRates.count(nominal.rate)
        && rates.supported.legacyRates.count(nominal.ackMode->getDataMode()->getNetBitrate());
}

} // namespace ieee80211
} // namespace inet
