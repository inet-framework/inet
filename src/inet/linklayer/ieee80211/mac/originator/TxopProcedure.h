//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TXOPPROCEDURE_H
#define __INET_TXOPPROCEDURE_H

#include "inet/linklayer/ieee80211/mac/common/AccessCategory.h"
#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"

namespace inet {
namespace ieee80211 {

class INET_API TxopProcedure : public ModeSetListener
{
  public:
    static simsignal_t txopStartedSignal;
    static simsignal_t txopEndedSignal;

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
    ProtectionMechanism protectionMechanism = ProtectionMechanism::UNDEFINED_PROTECTION;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual s getTxopLimit(const physicallayer::IIeee80211Mode *mode, AccessCategory ac);
    virtual ProtectionMechanism selectProtectionMechanism(AccessCategory ac) const;

  public:
    virtual void startTxop(AccessCategory ac);
    virtual void endTxop();

    virtual simtime_t getStart() const;
    virtual simtime_t getLimit() const;
    virtual simtime_t getRemaining() const;
    virtual simtime_t getDuration() const;

    virtual bool isFinalFragment(const Ptr<const Ieee80211MacHeader>& header) const;
    virtual bool isTxopInitiator(const Ptr<const Ieee80211MacHeader>& header) const;
    virtual bool isTxopTerminator(const Ptr<const Ieee80211MacHeader>& header) const;

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

