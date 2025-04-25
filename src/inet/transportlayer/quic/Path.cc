//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "Path.h"

namespace inet {
namespace quic {

Path::Path(Connection *connection, L3Address localAddr, int localPort, L3Address remoteAddr, int remotePort, bool useDplpmtud, Statistics *connStats) {
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
