//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_APPLICATIONS_QUIC_CONGESTIONCONTROL_NEWRENOCONGESTIONCONTROLLER_H_
#define INET_APPLICATIONS_QUIC_CONGESTIONCONTROL_NEWRENOCONGESTIONCONTROLLER_H_

#include "ICongestionController.h"
#include "../Connection.h"

namespace inet {
namespace quic {

class NewRenoCongestionController: public ICongestionController {
public:
    NewRenoCongestionController();
    virtual ~NewRenoCongestionController();

    virtual void onPacketAcked(QuicPacket *ackedPacket) override;
    virtual void onPacketSent(QuicPacket *sentPacket) override;
    virtual void onPacketsLost(std::vector<QuicPacket *> *lostPackets, bool inPersistentCongestion) override;
    virtual uint32_t getRemainingCongestionWindow() override;
    virtual void setAppLimited(bool appLimited) override;
    virtual void readParameters(cModule *module) override;
    virtual void setStatistics(Statistics *stats) override;
    virtual void setPath(Path *path) override;

private:
    const double kLossReductionFactor = .5;
    const uint32_t kMaxDatagramSize = 1200;

    uint32_t bytesInFlight;
    simtime_t congestionRecoveryStartTime;
    uint32_t congestionWindow;
    uint32_t ssthresh;
    uint32_t partialBytesAcked;

    bool appLimited = true;
    bool allowOnePacket = false;

    Statistics *stats;
    simsignal_t cwndStat;
    simsignal_t bytesInFlightStat;
    simsignal_t partialBytesAckedStat;

    bool accurateIncreaseInNewRenoCongestionAvoidance;

    Path *path;

    uint32_t getInitialWindow();
    uint32_t getMinimumWindow();
    void congestionEvent(simtime_t sentTime);
    bool inCongestionRecovery(simtime_t sentTime);
    bool isAppLimited();
    void emitStatValue(simsignal_t signal, uint32_t value);
    uint32_t getMaxDatagramSize();
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_CONGESTIONCONTROL_NEWRENOCONGESTIONCONTROLLER_H_ */
