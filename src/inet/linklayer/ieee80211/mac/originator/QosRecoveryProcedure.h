//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QOSRECOVERYPROCEDURE_H
#define __INET_QOSRECOVERYPROCEDURE_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/common/AccessCategory.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecoveryProcedure.h"

namespace inet {
namespace ieee80211 {

//
// References: 9.19.2.6 Retransmit procedures (IEEE 802.11-2012 STD)
// 802.11 Reference Design: Recovery Procedures and Retransmit Limits
// (https://warpproject.org/trac/wiki/802.11/MAC/Lower/Retransmissions)
/*
 * TODO stationRetryCounter
 */
class INET_API QosRecoveryProcedure : public cSimpleModule, public IRecoveryProcedure
{

  protected:
    ICwCalculator *cwCalculator = nullptr;

    // TODO why do we need Tid, is this class per AC or not? we should decide
    std::map<std::pair<Tid, SequenceControlField>, int> shortRetryCounter; // SRC
    std::map<std::pair<Tid, SequenceControlField>, int> longRetryCounter; // LRC

    // TODO these counters should be per AC, it's not done here but as separate recovery procedure modules
    int stationLongRetryCounter = 0; // QLRC
    int stationShortRetryCounter = 0; // QSRC

    int shortRetryLimit = -1;
    int longRetryLimit = -1;
    int rtsThreshold = -1;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    void incrementCounter(const Ptr<const Ieee80211DataHeader>& header, std::map<std::pair<Tid, SequenceControlField>, int>& retryCounter);
    void incrementStationSrc();
    void incrementStationLrc();
    void resetStationSrc() { stationShortRetryCounter = 0; }
    void resetStationLrc() { stationLongRetryCounter = 0; }
    void incrementContentionWindow();
    void resetContentionWindow();
    int doubleCw(int cw);
    int getRc(Packet *packet, const Ptr<const Ieee80211DataHeader>& header, std::map<std::pair<Tid, SequenceControlField>, int>& retryCounter);
    bool isMulticastFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header);

  public:
    virtual void multicastFrameTransmitted();

    virtual void ctsFrameReceived();
    virtual void ackFrameReceived(Packet *packet, const Ptr<const Ieee80211DataHeader>& ackedHeader);
    virtual void blockAckFrameReceived();

    virtual void rtsFrameTransmissionFailed(const Ptr<const Ieee80211DataHeader>& protectedHeader);
    virtual void dataFrameTransmissionFailed(Packet *packet, const Ptr<const Ieee80211DataHeader>& failedHeader);

    virtual bool isRetryLimitReached(Packet *packet, const Ptr<const Ieee80211DataHeader>& failedHeader);
    virtual int getRetryCount(Packet *packet, const Ptr<const Ieee80211DataHeader>& header);
    virtual bool isRtsFrameRetryLimitReached(Packet *packet, const Ptr<const Ieee80211DataHeader>& protectedHeader);

    virtual void retryLimitReached(Packet *packet, const Ptr<const Ieee80211DataHeader>& header);

    virtual int getLongRetryLimit() { return longRetryLimit; }
    virtual int getShortRetryLimit() { return shortRetryLimit; }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

