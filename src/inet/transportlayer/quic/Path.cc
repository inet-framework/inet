//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "Path.h"

namespace inet {
namespace quic {

Path::Path(Connection *connection, L3Address localAddr, uint16_t localPort, L3Address remoteAddr, uint16_t remotePort, bool useDplpmtud, Statistics *connStats) {
    this->connection = connection;
    this->localAddr = localAddr;
    this->localPort = localPort;
    this->remoteAddr = remoteAddr;
    this->remotePort = remotePort;
    this->useDplpmtud = useDplpmtud;

    this->stats = new Statistics(connStats, "_pid=0");

    int maxPmtu = connection->getModule()->par("maxPmtu");
    int mtu = std::min(maxPmtu, getLocalInterfaceMtu());
    int overhead = determineOverhead();
    maxQuicPacketSize = mtu - overhead;
    if (maxQuicPacketSize < 1200) {
        throw cRuntimeError("maxQuicPacketSize is only %d bytes, It must be at least 1200 bytes.", maxQuicPacketSize);
    }
    safeQuicPacketSize = 1280 - overhead;

    this->dplpmtud = nullptr;
    this->pmtuValidator = nullptr;
    if (useDplpmtud) {
        this->dplpmtud = new Dplpmtud(this, mtu, overhead, stats);
    }
}

Path::~Path() {
    delete stats;
    if (dplpmtud != nullptr) {
        delete dplpmtud;
    }
    if (pmtuValidator != nullptr) {
        delete pmtuValidator;
    }
}

int Path::getLocalInterfaceMtu() {
    // determine MTU of outgoing interface
    IRoutingTable *rt = connection->getModule()->getRoutingTable();
    NetworkInterface *rtie = rt->getOutputInterfaceForDestination(remoteAddr);
    if (rtie == nullptr) {
        throw cRuntimeError("No interface for remote address %s found!", remoteAddr.str().c_str());
    }
    return rtie->getMtu();
}

int Path::determineOverhead() {
    int overhead = 0;

    if (remoteAddr.getType() == L3Address::IPv4) {
        overhead += 20; // IPv4 header (without options)
    } else if (remoteAddr.getType() == L3Address::IPv6) {
        overhead += 40; // IPv6 header (without extension headers)
    } else {
        throw cRuntimeError("Unknown L3Address type");
    }
    overhead += 8; // UDP header

    return overhead;
}

int Path::getMaxQuicPacketSize() {
    if (useDplpmtud) {
        if (this->pmtuValidator != nullptr) {
            if (this->getConnection()->getReliabilityManager()->reducePacketSize(dplpmtud->getMinPlpmtu(), dplpmtud->getPlpmtu())) {
                EV_DEBUG << "reduce packet size to trigger an ack even in case of a PMTU decrease" << endl;
                return dplpmtud->getMinPlpmtu();
            }
        }
        return dplpmtud->getPlpmtu();
    }
    return maxQuicPacketSize;
}

int Path::getSafeQuicPacketSize() {
    if (useDplpmtud) {
        return dplpmtud->getMinPlpmtu();
    }
    return safeQuicPacketSize;
}

void Path::setPmtuValidator(PmtuValidator *pmtuValidator) {
    if (this->pmtuValidator != nullptr) {
        delete this->pmtuValidator;
    }
    this->pmtuValidator = pmtuValidator;
}

void Path::onClose()
{
    if (dplpmtud != nullptr) {
        delete dplpmtud;
        dplpmtud = nullptr;
        useDplpmtud = false;
    }
    if (pmtuValidator != nullptr) {
        delete pmtuValidator;
        pmtuValidator = nullptr;
    }
}

} /* namespace quic */
} /* namespace inet */
