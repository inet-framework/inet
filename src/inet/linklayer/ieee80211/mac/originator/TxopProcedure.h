//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TXOPPROCEDURE_H
#define __INET_TXOPPROCEDURE_H

#include "inet/linklayer/ieee80211/mac/common/AccessCategory.h"
#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/common/TxopExchangePlan.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {
namespace ieee80211 {

class INET_API TxopProcedure : public ModeSetListener
{
  public:
    static simsignal_t txopStartedSignal;
    static simsignal_t txopEndedSignal;
    static simsignal_t txopExchangeAdmittedSignal;
    static simsignal_t txopExchangeRejectedSignal;

  public:
    // [...] transmitted under EDCA by a STA that initiates a TXOP, there are
    // two classes of duration settings: single protection and multiple protection.
    enum ProtectionMechanism {
        SINGLE_PROTECTION,
        MULTIPLE_PROTECTION,
        UNDEFINED_PROTECTION
    };

  protected:
    simtime_t start = -1;
    simtime_t limit = -1;
    simtime_t lastDuration = 0;
    bool acquired = false;
    AccessCategory accessCategory = AC_BE;
    int dataOrManagementTransmissions = 0;
    bool hasIdentity = false;
    TxopFrameIdentity identity;
    ProtectionMechanism protectionMechanism = ProtectionMechanism::UNDEFINED_PROTECTION;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual s getTxopLimit(physicallayer::Ieee80211ModeSet::PhyType phyType, AccessCategory ac);
    virtual ProtectionMechanism selectProtectionMechanism(AccessCategory ac) const;

  public:
    virtual void startTxop(AccessCategory ac);
    virtual void endTxop();

    virtual simtime_t getStart() const;
    virtual simtime_t getLimit() const;
    virtual simtime_t getRemaining() const;
    virtual simtime_t getDuration() const;

    virtual void transmissionStarted();
    virtual void recordTransmission(const TxopExchangePlan& plan, bool dataOrManagement);
    virtual TxopAdmissionDecision evaluate(const TxopExchangePlan& plan, simtime_t exchangeStart,
            simtime_t txnavEnd, bool continuation, const TxopExchangePlan *projectedCurrent = nullptr) const;
    virtual void commitAdmission(TxopExchangePlan& plan, const TxopAdmissionDecision& decision);
    bool isAcquired() const { return acquired; }
    bool hasStarted() const { return start >= SIMTIME_ZERO; }

    virtual ProtectionMechanism getProtectionMechanism() const { return protectionMechanism; }
};

class INET_API TxopDurationFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
    using cObjectResultFilter::receiveSignal;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif
