//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_PATH_H_
#define INET_TRANSPORTLAYER_QUIC_PATH_H_

#include "dplpmtud/Dplpmtud.h"
#include "PmtuValidator.h"
#include "Connection.h"
#include "Statistics.h"

namespace inet {
namespace quic {

class PmtuValidator;

class Path {
public:
    Path(Connection *connection, L3Address localAddr, uint16_t localPort, L3Address remoteAddr, uint16_t remotePort, bool useDplpmtud, Statistics *connStats);
    virtual ~Path();

    int getMaxQuicPacketSize();
    int getSafeQuicPacketSize();
    void setPmtuValidator(PmtuValidator *pmtuValidator);
    void onClose();

    L3Address getLocalAddr() {
        return localAddr;
    }
    L3Address getRemoteAddr() {
        return remoteAddr;
    }
    uint16_t getLocalPort() {
        return localPort;
    }
    uint16_t getRemotePort() {
        return remotePort;
    }
    Connection *getConnection() {
        return connection;
    }
    Dplpmtud *getDplpmtud() {
        return dplpmtud;
    }
    PmtuValidator *getPmtuValidator() {
        return pmtuValidator;
    }
    bool usesDplpmtud() {
        return useDplpmtud;
    }
    bool hasPmtuValidator() {
        return pmtuValidator != nullptr;
    }

    simtime_t rttVar;
    simtime_t smoothedRtt;

private:
    Connection *connection;
    L3Address localAddr;
    uint16_t localPort;
    L3Address remoteAddr;
    uint16_t remotePort;

    int maxQuicPacketSize;
    int safeQuicPacketSize;

    bool useDplpmtud;
    Dplpmtud *dplpmtud;
    PmtuValidator *pmtuValidator;

    Statistics *stats;

    int getLocalInterfaceMtu();
    int determineOverhead();
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_PATH_H_ */
