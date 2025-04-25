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
    Path(Connection *connection, L3Address localAddr, int localPort, L3Address remoteAddr, int remotePort, bool useDplpmtud, Statistics *connStats);
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
    int getLocalPort() {
        return localPort;
    }
    int getRemotePort() {
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
    int localPort;
    L3Address remoteAddr;
    int remotePort;

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
