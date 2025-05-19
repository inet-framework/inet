//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_PACKET_DPLPMTUDPROBEPACKET_H_
#define INET_TRANSPORTLAYER_QUIC_PACKET_DPLPMTUDPROBEPACKET_H_

#include "QuicPacket.h"
#include "../dplpmtud/Dplpmtud.h"

namespace inet {
namespace quic {

class Dplpmtud;

class DplpmtudProbePacket: public QuicPacket {
public:
    DplpmtudProbePacket(std::string name, Dplpmtud *dplpmtud);
    virtual ~DplpmtudProbePacket();

    virtual void onPacketLost() override;
    virtual void onPacketAcked() override;
    virtual bool isDplpmtudProbePacket() override;

private:
    Dplpmtud *dplpmtud;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_PACKET_DPLPMTUDPROBEPACKET_H_ */
