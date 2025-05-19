//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_APPLICATIONS_QUIC_CONGESTIONCONTROL_NOCONGESTIONCONTROLLER_H_
#define INET_APPLICATIONS_QUIC_CONGESTIONCONTROL_NOCONGESTIONCONTROLLER_H_

#include "ICongestionController.h"

namespace inet {
namespace quic {

class NoCongestionController: public ICongestionController {
public:
    NoCongestionController();
    virtual ~NoCongestionController();

    virtual void onPacketAcked(QuicPacket *ackedPacket) override;
    virtual void onPacketSent(QuicPacket *sentPacket) override;
    virtual void onPacketsLost(std::vector<QuicPacket *> *lostPackets, bool inPersistentCongestion) override;
    virtual uint32_t getRemainingCongestionWindow() override;
    virtual void setAppLimited(bool appLimited) override;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_CONGESTIONCONTROL_NOCONGESTIONCONTROLLER_H_ */
