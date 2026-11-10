// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_AIRTIMELINKMETRIC_H
#define __INET_AIRTIMELINKMETRIC_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshLinkMetric.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshLinkTiming.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"

namespace inet {
namespace ieee80211 {

class INET_API AirtimeLinkMetric : public OperationalBase, public cListener, public IMeshLinkMetric
{
  protected:
    ModuleRefByPar<IMeshLinkTiming> timing;
    ModuleRefByPar<Ieee80211Mib> mib;
    std::map<MacAddress, double> errorEstimates;
    std::map<MacAddress, double> payloadEstimates;
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
    void unsubscribeAttemptNotifications();

  public:
    virtual ~AirtimeLinkMetric();
    virtual uint32_t getMetric(MacAddress peer) override;
    virtual void resetPeer(MacAddress peer) override;
    void resetAggregation(MacAddress peer) override;
    virtual bool supportsPeerRates(MacAddress peer, const Ieee80211RateSetState& rates) override;
    double getErrorEstimate(MacAddress peer) const;
    double getPayloadEstimate(MacAddress peer) const;
    // IEEE Std 802.11-2024, 14.10.2/Table14-4. Metric1 always uses n=1.
    static uint32_t computeMetric(int metricId, simtime_t overhead, units::values::bps rate, double error, double n = 1);
};

} // namespace ieee80211
} // namespace inet
#endif
