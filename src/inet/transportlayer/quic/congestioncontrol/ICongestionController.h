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
