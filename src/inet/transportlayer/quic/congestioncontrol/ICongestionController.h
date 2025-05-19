//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_APPLICATIONS_QUIC_CONGESTIONCONTROL_ICONGESTIONCONTROLLER_H_
#define INET_APPLICATIONS_QUIC_CONGESTIONCONTROL_ICONGESTIONCONTROLLER_H_

#include "../packet/QuicPacket.h"
#include "../Statistics.h"
#include "../Path.h"

namespace inet {
namespace quic {

class Path;

class ICongestionController {
public:
    virtual ~ICongestionController() { };
    virtual void onPacketAcked(QuicPacket *ackedPacket) = 0;
    virtual void onPacketSent(QuicPacket *sentPacket) = 0;
    virtual void onPacketsLost(std::vector<QuicPacket *> *lostPackets, bool inPersistentCongestion) = 0;
    virtual uint32_t getRemainingCongestionWindow() = 0;
    virtual void setAppLimited(bool appLimited) = 0;
    virtual void readParameters(cModule *module) { };
    virtual void setStatistics(Statistics *stats) { };
    virtual void setPath(Path *path) { };
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_CONGESTIONCONTROL_ICONGESTIONCONTROLLER_H_ */
